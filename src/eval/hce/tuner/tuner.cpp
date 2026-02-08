#include "tuner.h"

#include "core/position.h"
#include "eval/hce/tuner/base.h"

namespace sagittar::eval::hce::tuner {

    constexpr size_t NB_COLOR     = 2;
    constexpr size_t NB_PIECETYPE = 7;
    constexpr size_t NB_SQUARE    = 64;

    constexpr size_t N_PARAMS = NB_PIECETYPE                 // Piece Scores
                              + (NB_PIECETYPE * NB_SQUARE);  // PSQT

    struct EvalTrace {
        i32 piece_counts[NB_PIECETYPE][NB_COLOR]          = {};
        i32 psq_counts[NB_PIECETYPE][NB_SQUARE][NB_COLOR] = {};
    };

    static ParameterVector init_parameters() {
        ParameterVector params{};

        init_param_array(params, PIECE_SCORES, NB_PIECETYPE);
        init_param_array_2d(params, PSQT_SCORES, NB_PIECETYPE, NB_SQUARE);

        if (params.size() != N_PARAMS)
        {
            throw std::runtime_error("ParameterVector size mismatch");
        }

        return params;
    }

    static EvalTrace create_eval_trace(const Position& pos) {
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
                trace.psq_counts[pt][SQUARES_MIRRORED[sq]][c]++;
            }
            else
            {
                trace.psq_counts[pt][sq][c]++;
            }
        }

        return trace;
    }

    static void init_entry_coeffs(Entry& e, const EvalTrace& trace) {
        init_coeff_array(e, trace.piece_counts, NB_PIECETYPE);
        init_coeff_array_2d(e, trace.psq_counts, NB_PIECETYPE, NB_SQUARE);
    }

    static double extract_wdl(std::string_view fen) {
        if (fen.find("[1.0]") != (size_t) std::string::npos)
        {
            return 1.0;
        }
        else if (fen.find("[0.0]") != (size_t) std::string::npos)
        {
            return 0.0;
        }

        return 0.5;
    }

    static Entry create_entry(const std::string& fen) {
        Position pos{};
        pos.setFen(fen);

        const EvalTrace trace = create_eval_trace(pos);

        Entry e{};

        init_entry_coeffs(e, trace);
        if (e.coefficients.size() != N_PARAMS)
        {
            throw std::runtime_error("Coefficients size mismatch");
        }

        e.stm   = pos.stm();
        e.phase = pos_phase(pos);  // 0 => MG; 256 => EG
        e.wdl   = extract_wdl(fen);

        return e;
    }

    static double linear_eval(const Entry& entry, const ParameterVector& params) {
        // -INFINITY    => BLACK WINNING
        // 0            => DRAW
        // +INFINITY    => WHITE WINNING

        double eval_mg = 0;
        double eval_eg = 0;

        for (size_t i = 0; i < N_PARAMS; i++)
        {
            const i32             coeff = entry.coefficients[i];
            const ParameterTuple& param = params[i];

            eval_mg += coeff * param[MG];
            eval_eg += coeff * param[EG];
        }

        double eval = static_cast<double>(scale_eval(eval_mg, eval_eg, entry.phase));

        const double tempo_bonus = static_cast<double>(
          scale_eval(mg_score(TEMPO_BONUS), eg_score(TEMPO_BONUS), entry.phase));
        eval += tempo_bonus;

        return eval;
    }

    static double sigmoid(const double K, const double E) {
        // E = -INFINITY    -> 0    => BLACK Winning
        // E = 0            -> 0.5  => Draw
        // E = +INFINITY    -> 1    => WHITE Winning

        return 1.0 / (1.0 + exp(-K * E / 400.0));
    }

    static double
    mse(const std::vector<Entry>& entries, const ParameterVector& params, const double K) {
        double total_error = 0.0;

        for (const Entry& entry : entries)
        {
            const double eval        = linear_eval(entry, params);
            const double sig         = sigmoid(K, eval);
            const double diff        = entry.wdl - sig;
            const double entry_error = diff * diff;
            total_error += entry_error;
        }

        const double avg_error = total_error / static_cast<double>(entries.size());

        return avg_error;
    }

    static void update_gradient_single(ParameterVector&       gradient,
                                       const Entry&           entry,
                                       const ParameterVector& params,
                                       const double           K) {
        // Forward pass: eval -> sigmoid -> residual w.r.t target WDL
        const double eval = linear_eval(entry, params);
        const double sig  = sigmoid(K, eval);

        // Backprop
        const double res = (entry.wdl - sig) * sig * (1 - sig);

        // Phase blend: split residual between middle-game and end-game
        const double w_eg = entry.phase / 256.0;
        const double w_mg = 1.0 - w_eg;

        const double mg_base = res * w_mg;
        const double eg_base = res * w_eg;

        for (size_t i = 0; i < N_PARAMS; i++)
        {
            // Accumulate gradient per parameter for both phases
            gradient[i][MG] += mg_base * entry.coefficients[i];
            gradient[i][EG] += eg_base * entry.coefficients[i];
        }
    }

    static void compute_gradient(ParameterVector&          gradient,
                                 const std::vector<Entry>& entries,
                                 const ParameterVector&    params,
                                 const double              K) {
        for (const Entry& e : entries)
        {
            update_gradient_single(gradient, e, params, K);
        }
    }

    static double compute_optimal_K(const std::vector<Entry>& entries,
                                    const ParameterVector&    params,
                                    const double              K_init = 2.0) {
        constexpr double rate           = 10;
        constexpr double delta          = 1e-5;
        constexpr double deviation_goal = 1e-6;

        double K         = K_init;
        double deviation = 1;

        while (std::fabs(deviation) > deviation_goal)
        {
            const double err_up   = mse(entries, params, K + delta);
            const double err_down = mse(entries, params, K - delta);
            deviation             = (err_up - err_down) / (2.0 * delta);
            std::cout << "Current K: " << K << ", Error Up: " << err_up
                      << ", Error Down: " << err_down << ", Deviation: " << deviation << std::endl;
            K -= deviation * rate;
        }

        return K;
    }

    static void run(ParameterVector&          params,
                    const std::vector<Entry>& entries,
                    const double              K,
                    const size_t              epoch                       = 5000,
                    const double              beta1                       = 0.9,
                    const double              beta2                       = 0.999,
                    const double              learning_rate_init          = 0.1,
                    const size_t              learning_rate_drop_interval = 500,
                    const double              learning_rate_drop_ratio    = 0.5) {

        if (entries.empty())
            return;

        ParameterVector momentum(N_PARAMS);
        ParameterVector velocity(N_PARAMS);

        double learning_rate = learning_rate_init;

        for (size_t i = 1; i <= epoch; i++)
        {
            ParameterVector gradient(N_PARAMS);
            compute_gradient(gradient, entries, params, K);

            for (size_t param = 0; param < N_PARAMS; param++)
            {
                // Scale raw gradients by K factor and average over batch
                const double mg_grad = (-K / 400.0) * gradient[param][MG] / entries.size();
                const double eg_grad = (-K / 400.0) * gradient[param][EG] / entries.size();

                // Update 1st moment (momentum) estimates
                momentum[param][MG] = beta1 * momentum[param][MG] + (1.0 - beta1) * mg_grad;
                momentum[param][EG] = beta1 * momentum[param][EG] + (1.0 - beta1) * eg_grad;

                // Update 2nd moment (variance) estimates
                velocity[param][MG] =
                  beta2 * velocity[param][MG] + (1.0 - beta2) * std::pow(mg_grad, 2);
                velocity[param][EG] =
                  beta2 * velocity[param][EG] + (1.0 - beta2) * std::pow(eg_grad, 2);

                // Apply Adam step scaled by learning rate
                params[param][MG] -=
                  learning_rate * momentum[param][MG] / (1e-8 + std::sqrt(velocity[param][MG]));
                params[param][EG] -=
                  learning_rate * momentum[param][EG] / (1e-8 + std::sqrt(velocity[param][EG]));
            }

            if (i % 100 == 0)
            {
                const double error = mse(entries, params, K);
                std::cout << "Error = " << error << std::endl;
            }

            if (i % learning_rate_drop_interval == 0)
            {
                learning_rate *= learning_rate_drop_ratio;
            }
        }
    }

    static void read_epd(std::vector<Entry>& entries, const std::filesystem::path& epd_path) {
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

            entries.emplace_back(create_entry(line));
        }
    }

    void tune(const std::filesystem::path& epd_path, const TunerSettings& settings) {
        std::cout << "Reading .epd file: " << epd_path << std::endl;
        std::vector<Entry> entries{};
        read_epd(entries, epd_path);
        std::cout << "Found " << (size_t) entries.size() << " Entries" << std::endl;

        std::cout << "Reading initial params" << std::endl;
        ParameterVector params = init_parameters();

        std::cout << "Initial Parameters:\n" << std::endl;
        print_param_array(params, 0, NB_PIECETYPE);
        print_psqt(params, NB_PIECETYPE);
        std::cout << "No. of Parameters: " << (size_t) N_PARAMS << std::endl;

        double K = 0.0;
        if (settings.compute_k)
        {
            std::cout << "Computing Optimal K" << std::endl;
            K = compute_optimal_K(entries, params, settings.K);
            std::cout << "Optimal K = " << (double) K << std::endl;
        }
        else
        {
            K = settings.K;
            std::cout << "Using K = " << (double) K << std::endl;
        }

        std::cout << "Beginning to tune" << std::endl;
        run(params, entries, K, settings.epochs, settings.beta1, settings.beta2,
            settings.learning_rate_init, settings.learning_rate_drop_interval,
            settings.learning_rate_drop_ratio);

        std::cout << "Tuned Parameters:" << std::endl;
        print_param_array(params, 0, NB_PIECETYPE);
        print_psqt(params, NB_PIECETYPE);

        std::cout << "Tuning complete" << std::endl;
    }

}
