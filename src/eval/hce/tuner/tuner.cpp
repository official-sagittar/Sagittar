#include "tuner.h"

#include "core/position.h"
#include "core/types.h"
#include "eval/hce/eval.h"
#include "eval/hce/tuner/base.h"

namespace sagittar::eval::hce::tuner {

    using ThreadPool = dp::thread_pool<>;

    constexpr size_t k_NB_COLOR     = 2;
    constexpr size_t k_NB_PIECETYPE = 6;
    constexpr size_t k_NB_SQUARE    = 64;

    constexpr size_t k_N_PARAMS = k_NB_PIECETYPE                  // Piece Scores
                                + (k_NB_PIECETYPE * k_NB_SQUARE)  // PSQT
                                + (9 + 14 + 15 + 28)              // Mobility
                                + 1;                              // Bishop Pair
    struct EvalTrace {
        I32 piece_counts[k_NB_PIECETYPE][k_NB_COLOR]            = {};
        I32 psq_counts[k_NB_PIECETYPE][k_NB_SQUARE][k_NB_COLOR] = {};
        I32 mobility[4][28][k_NB_COLOR]                         = {};
        I32 bishop_pair[k_NB_COLOR]                             = {};
    };

    static ParameterVector initParameters() {
        ParameterVector params{};

        initParamArray(params, k_PIECE_SCORES, k_NB_PIECETYPE);
        initParamArray2d(params, k_PSQT_SCORES, k_NB_PIECETYPE, k_NB_SQUARE);
        initParamArray(params, k_MOBILITY_BONUS[PieceType::KNIGHT - 1], 9);
        initParamArray(params, k_MOBILITY_BONUS[PieceType::BISHOP - 1], 14);
        initParamArray(params, k_MOBILITY_BONUS[PieceType::ROOK - 1], 15);
        initParamArray(params, k_MOBILITY_BONUS[PieceType::QUEEN - 1], 28);
        initParamSingle(params, k_BISHOP_PAIR);

        if (params.size() != k_N_PARAMS)
        {
            throw std::runtime_error("ParameterVector size mismatch");
        }

        return params;
    }

    static EvalTrace createEvalTrace(const Position& pos) {
        EvalTrace trace{};

        for (int sq = Square::A1; sq <= Square::H8; sq++)
        {
            const Piece p = pos.pieceOn(static_cast<Square>(sq));

            if (p == Piece::NO_PIECE)
            {
                continue;
            }

            const PieceType pt = pieceTypeOf(p);
            const Color     c  = pieceColorOf(p);

            trace.piece_counts[pt][c]++;

            if (c == Color::WHITE)
            {
                trace.psq_counts[pt][k_SQUARES_MIRRORED[sq]][c]++;

                if ((pt != PieceType::PAWN) && (pt != PieceType::KING))
                {
                    const int mobility_count =
                      mobility<Color::WHITE>(pos, pt, static_cast<Square>(sq));
                    trace.mobility[pt - 1][mobility_count][c]++;
                }
            }
            else
            {
                trace.psq_counts[pt][sq][c]++;

                if ((pt != PieceType::PAWN) && (pt != PieceType::KING))
                {
                    const int mobility_count =
                      mobility<Color::BLACK>(pos, pt, static_cast<Square>(sq));
                    trace.mobility[pt - 1][mobility_count][c]++;
                }
            }
        }

        // Bishop Pair
        if (pos.pieces(Color::WHITE, PieceType::BISHOP).hasMultiple())
        {
            trace.bishop_pair[Color::WHITE]++;
        }
        if (pos.pieces(Color::BLACK, PieceType::BISHOP).hasMultiple())
        {
            trace.bishop_pair[Color::BLACK]++;
        }

        return trace;
    }

    static void initEntryCoeffs(Entry& e, const EvalTrace& trace) {
        size_t index = 0;
        initCoeffArray(e, trace.piece_counts, k_NB_PIECETYPE, index);
        initCoeffArray2d(e, trace.psq_counts, k_NB_PIECETYPE, k_NB_SQUARE, index);
        initCoeffArray(e, trace.mobility[PieceType::KNIGHT - 1], 9, index);
        initCoeffArray(e, trace.mobility[PieceType::BISHOP - 1], 14, index);
        initCoeffArray(e, trace.mobility[PieceType::ROOK - 1], 15, index);
        initCoeffArray(e, trace.mobility[PieceType::QUEEN - 1], 28, index);
        initCoeffSingle(e, trace.bishop_pair, index++);
    }

    static double extractWdl(std::string_view fen) {
        if (fen.find("[1.0]") != (size_t) std::string::npos)
        {
            return 1.0;
        }
        else if (fen.find("[0.0]") != (size_t) std::string::npos)
        {
            return 0.0;
        }
        else if (fen.find("[0.5]") != (size_t) std::string::npos)
        {
            return 0.5;
        }

        throw std::runtime_error("Could not find WDL marker");
    }

    static Entry createEntry(const std::string& fen) {
        Position pos{};
        pos.setFen(fen);

        const EvalTrace trace = createEvalTrace(pos);

        Entry e{};
        e.coefficients.reserve(32);

        initEntryCoeffs(e, trace);

        e.stm = pos.stm();

        e.phase        = posPhase(pos);  // 0 => MG; 256 => EG
        e.pfactors[MG] = 1.0 - (e.phase / 256.0);
        e.pfactors[EG] = e.phase / 256.0;

        e.wdl   = extractWdl(fen);
        e.seval = evaluate(pos);

        return e;
    }

    static double linearEval(const Entry& entry, const ParameterVector& params) {
        // -INFINITY    => BLACK WINNING
        // 0            => DRAW
        // +INFINITY    => WHITE WINNING

        double eval_mg = 0;
        double eval_eg = 0;

        for (const Coefficient& c : entry.coefficients)
        {
            const ParameterTuple& param = params[c.index];

            eval_mg += c.value * param[MG];
            eval_eg += c.value * param[EG];
        }

        const double eval = eval_mg * entry.pfactors[MG] + eval_eg * entry.pfactors[EG];

        const double tempo_bonus =
          mgScore(k_TEMPO_BONUS) * entry.pfactors[MG] + egScore(k_TEMPO_BONUS) * entry.pfactors[EG];

        return eval + (entry.stm == Color::WHITE ? tempo_bonus : -tempo_bonus);
    }

    static bool checkEntriesEval(const std::vector<Entry>& entries, const ParameterVector& params) {
        for (const Entry& entry : entries)
        {
            const Score seval_w_pov = (entry.stm == Color::WHITE) ? entry.seval : -entry.seval;
            const Score coeff_eval  = static_cast<Score>(std::llround(linearEval(entry, params)));
            if (std::abs(seval_w_pov - coeff_eval) > 2)
            {
                std::cerr << "Eval mismatch: Eval WHITE PoV =" << seval_w_pov
                          << " Linear Eval =" << coeff_eval << std::endl;
                return false;
            }
        }

        return true;
    }

    static double sigmoid(const double K, const double E) {
        // E = -INFINITY    -> 0    => BLACK Winning
        // E = 0            -> 0.5  => Draw
        // E = +INFINITY    -> 1    => WHITE Winning

        return 1.0 / (1.0 + exp(-K * E / 400.0));
    }

    static double mse(ThreadPool&               pool,
                      const size_t              nthreads,
                      const std::vector<Entry>& entries,
                      const ParameterVector&    params,
                      const double              K) {
        if (entries.empty())
        {
            return 0.0;
        }

        std::vector<std::future<double>> futs;
        futs.reserve(nthreads);

        const size_t n = entries.size();

        for (size_t tid = 0; tid < nthreads; ++tid)
        {
            const size_t start = (tid * n) / nthreads;
            const size_t end   = ((tid + 1) * n) / nthreads;

            futs.push_back(pool.enqueue([start, end, &entries, &params, K]() -> double {
                double local = 0.0;
                for (size_t i = start; i < end; ++i)
                {
                    const auto&  entry = entries[i];
                    const double eval  = linearEval(entry, params);
                    const double sig   = sigmoid(K, eval);
                    const double diff  = entry.wdl - sig;
                    local += diff * diff;
                }
                return local;
            }));
        }

        double total_error = 0.0;
        for (auto& f : futs)
        {
            total_error += f.get();
        }

        return total_error / static_cast<double>(n);
    }

    static void updateGradientSingle(ParameterVector&       gradient,
                                     const Entry&           entry,
                                     const ParameterVector& params,
                                     const double           K) {
        // Forward pass: eval -> sigmoid -> residual w.r.t target WDL
        const double eval = linearEval(entry, params);
        const double sig  = sigmoid(K, eval);

        // Backprop
        const double res = (entry.wdl - sig) * sig * (1 - sig);

        const double mg_base = res * entry.pfactors[MG];
        const double eg_base = res * entry.pfactors[EG];

        for (const Coefficient& c : entry.coefficients)
        {
            // Accumulate gradient per parameter for both phases
            gradient[c.index][MG] += mg_base * c.value;
            gradient[c.index][EG] += eg_base * c.value;
        }
    }

    static void computeGradient(ThreadPool&               pool,
                                const size_t              nthreads,
                                ParameterVector&          gradient,
                                const std::vector<Entry>& entries,
                                const ParameterVector&    params,
                                const double              K) {
        if (entries.empty())
        {
            return;
        }

        std::vector<ParameterVector> thread_gradients(nthreads, ParameterVector(k_N_PARAMS));

        const size_t n = entries.size();

        for (size_t tid = 0; tid < nthreads; ++tid)
        {
            const size_t start = (tid * n) / nthreads;
            const size_t end   = ((tid + 1) * n) / nthreads;

            pool.enqueue_detach([tid, start, end, &thread_gradients, &entries, &params, K]() {
                auto& g = thread_gradients[tid];
                for (size_t i = start; i < end; ++i)
                {
                    updateGradientSingle(g, entries[i], params, K);
                }
            });
        }

        pool.wait_for_tasks();

        // Reduce thread gradients into output gradient
        for (size_t tid = 0; tid < nthreads; ++tid)
        {
            const auto& tg = thread_gradients[tid];
            for (size_t p = 0; p < k_N_PARAMS; ++p)
            {
                gradient[p][MG] += tg[p][MG];
                gradient[p][EG] += tg[p][EG];
            }
        }
    }

    static double computeOptimalK(ThreadPool&               pool,
                                  const size_t              nthreads,
                                  const std::vector<Entry>& entries,
                                  const ParameterVector&    params,
                                  const double              K_init) {
        constexpr double rate           = 10;
        constexpr double delta          = 1e-5;
        constexpr double deviation_goal = 1e-6;

        double k         = K_init;
        double deviation = 1;

        while (std::fabs(deviation) > deviation_goal)
        {
            const double err_up   = mse(pool, nthreads, entries, params, k + delta);
            const double err_down = mse(pool, nthreads, entries, params, k - delta);
            deviation             = (err_up - err_down) / (2.0 * delta);
            std::cout << "Current K: " << k << ", Error Up: " << err_up
                      << ", Error Down: " << err_down << ", Deviation: " << deviation << std::endl;
            k -= deviation * rate;
        }

        return k;
    }

    static void run(ThreadPool&               pool,
                    const size_t              nthreads,
                    ParameterVector&          params,
                    const std::vector<Entry>& entries,
                    const double              K,
                    const TunerSettings&      settings) {

        if (entries.empty())
            return;

        ParameterVector momentum(k_N_PARAMS);
        ParameterVector velocity(k_N_PARAMS);

        double learning_rate = settings.learning_rate_init;

        for (size_t i = 1; i <= settings.epochs; i++)
        {
            ParameterVector gradient(k_N_PARAMS);
            computeGradient(pool, nthreads, gradient, entries, params, K);

            // Adam bias-correction factors (depend only on epoch index i)
            const double bias_correction1 = 1.0 - std::pow(settings.beta1, static_cast<double>(i));
            const double bias_correction2 = 1.0 - std::pow(settings.beta2, static_cast<double>(i));

            for (size_t param = 0; param < k_N_PARAMS; param++)
            {
                // Scale raw gradients by K factor and average over batch
                const double mg_grad = (-K / 200.0) * gradient[param][MG] / entries.size();
                const double eg_grad = (-K / 200.0) * gradient[param][EG] / entries.size();

                // Update 1st moment (momentum) estimates
                momentum[param][MG] =
                  settings.beta1 * momentum[param][MG] + (1.0 - settings.beta1) * mg_grad;
                momentum[param][EG] =
                  settings.beta1 * momentum[param][EG] + (1.0 - settings.beta1) * eg_grad;

                // Update 2nd moment (variance) estimates
                velocity[param][MG] = settings.beta2 * velocity[param][MG]
                                    + (1.0 - settings.beta2) * std::pow(mg_grad, 2);
                velocity[param][EG] = settings.beta2 * velocity[param][EG]
                                    + (1.0 - settings.beta2) * std::pow(eg_grad, 2);

                // Bias-corrected 1st and 2nd moment estimates
                const double m_mg = momentum[param][MG] / bias_correction1;
                const double m_eg = momentum[param][EG] / bias_correction1;
                const double v_mg = velocity[param][MG] / bias_correction2;
                const double v_eg = velocity[param][EG] / bias_correction2;

                // Update params
                params[param][MG] -= learning_rate * m_mg / (1e-8 + std::sqrt(v_mg));
                params[param][EG] -= learning_rate * m_eg / (1e-8 + std::sqrt(v_eg));
            }

            if (i % 100 == 0)
            {
                const double error = mse(pool, nthreads, entries, params, K);
                std::cout << "Epoch = " << (size_t) i << "\tError = " << error
                          << "\tLearning Rate = " << (double) learning_rate << std::endl;
            }

            if (i % settings.learning_rate_drop_interval == 0)
            {
                learning_rate *= settings.learning_rate_drop_ratio;
            }
        }
    }

    static void readEpd(std::vector<Entry>& entries, const std::filesystem::path& epd_path) {
        std::ifstream file(epd_path);

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open EPD file");
        }

        std::string line;
        while (std::getline(file, line))
        {
            // Skip blank/whitespace-only lines
            if (line.find_first_not_of(" \t\r\n") == std::string::npos)
            {
                continue;
            }

            entries.emplace_back(createEntry(line));
        }
    }

    void tune(const std::filesystem::path& epd_path, const TunerSettings& settings) {
        std::cout << "Reading .epd file: " << epd_path << std::endl;
        std::vector<Entry> entries{};
        readEpd(entries, epd_path);
        std::cout << "Found " << (size_t) entries.size() << " Entries" << std::endl;

        std::cout << "Reading initial params" << std::endl;
        ParameterVector params = initParameters();

        std::cout << "Initial Parameters:\n" << std::endl;
        printParamArray(params, 0, k_NB_PIECETYPE);
        auto next = printPsqt(params, k_NB_PIECETYPE);
        next      = printMobility(params, next);
        printParamSingle(params[next++]);
        std::cout << "No. of Parameters: " << (size_t) k_N_PARAMS << std::endl;

        if (!checkEntriesEval(entries, params))
        {
            std::cerr << "Eval check failed!" << std::endl;
            return;
        }
        std::cout << "Eval check succeeded" << std::endl;

        if (settings.retune_from_zero)
        {
            std::cout << "Resetting Parameters to zero" << std::endl;
            for (size_t i = 0; i < k_N_PARAMS; i++)
            {
                params[i][MG] = params[i][EG] = 0;
            }
            std::cout << "Zero-ed Parameters:\n" << std::endl;
            printParamArray(params, 0, k_NB_PIECETYPE);
            printPsqt(params, k_NB_PIECETYPE);
        }

        std::cout << "Using dp::thread-pool version " << THREADPOOL_VERSION << '\n';

        const size_t nthreads =
          std::max<size_t>(1, std::min(settings.thread_count, entries.size()));
        std::cout << "Using " << (size_t) nthreads << " threads" << std::endl;

        ThreadPool pool(nthreads);

        double k = 0.0;
        if (settings.compute_k)
        {
            std::cout << "Computing Optimal K" << std::endl;
            k = computeOptimalK(pool, nthreads, entries, params, settings.K);
            std::cout << "Optimal K = " << (double) k << std::endl;
        }
        else
        {
            k = settings.K;
            std::cout << "Using K = " << (double) k << std::endl;
        }

        std::cout << "Beginning to tune" << std::endl;
        run(pool, nthreads, params, entries, k, settings);

        std::cout << "Tuned Parameters:" << std::endl;
        printParamArray(params, 0, k_NB_PIECETYPE);
        next = printPsqt(params, k_NB_PIECETYPE);
        next = printMobility(params, next);
        printParamSingle(params[next++]);

        std::cout << "Tuning complete" << std::endl;
    }

}
