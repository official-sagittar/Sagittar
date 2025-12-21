#include "tuner.h"

#include <filesystem>
#include <fstream>

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

        // To prevent overflows
        // NOTE: Depending on the value of K, it will clamp E
        // K = 1; |E| ~ 6000cp
        // K = 2; |E| ~ 3000cp
        // K = 3; |E| ~ 2000cp
        // K = 4; |E| ~ 1500cp
        // And so on...
        static constexpr double MAX_EXP = 15.0;

        const double x = std::clamp(-K * E / 400.0, -MAX_EXP, MAX_EXP);
        return 1.0 / (1.0 + std::exp(x));
    }

    static double
    mse(const std::vector<Entry>& entries, const ParameterVector& params, const double K) {
        double total_error = 0;

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

        if (std::abs(res) < 1e-12)
        {
            return;
        }

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

    static void normalize_psqt_params(ParameterVector& params) {
        for (size_t pt = 0; pt < NB_PIECETYPE; ++pt)
        {
            double mg_sum = 0.0;
            double eg_sum = 0.0;

            const size_t base = NB_PIECETYPE + pt * NB_SQUARE;

            const size_t start_sq = (pt == PieceType::PAWN) ? Square::A2 : Square::A1;
            const size_t end_sq   = (pt == PieceType::PAWN) ? Square::H7 : Square::H8;
            const size_t n_sq     = (pt == PieceType::PAWN) ? (NB_SQUARE - 16) : NB_SQUARE;

            // 1) Compute mean
            for (size_t sq = start_sq; sq <= end_sq; ++sq)
            {
                mg_sum += params[base + sq][MG];
                eg_sum += params[base + sq][EG];
            }

            const double mg_mean = mg_sum / n_sq;
            const double eg_mean = eg_sum / n_sq;

            // 2) Subtract mean
            for (size_t sq = start_sq; sq <= end_sq; ++sq)
            {
                params[base + sq][MG] -= mg_mean;
                params[base + sq][EG] -= eg_mean;
            }
        }
    }

    static double compute_optimal_K(const std::vector<Entry>& entries,
                                    const ParameterVector&    params,
                                    const double              K_init = 2.0) {
        constexpr double delta     = 1e-3;
        constexpr double lr        = 0.1;
        constexpr double tol       = 1e-6;
        constexpr size_t max_iters = 1000;

        double K = K_init;

        for (size_t it = 0; it < max_iters; ++it)
        {
            const double err_up   = mse(entries, params, K + delta);
            const double err_down = mse(entries, params, K - delta);
            const double grad     = (err_up - err_down) / (2.0 * delta);
            K -= lr * grad;
            K = std::clamp(K, 0.1, 10.0);
            if (std::abs(grad) < tol)
                break;
        }

        return K;
    }

    static void run(ParameterVector&          params,
                    const std::vector<Entry>& entries,
                    const double              K,
                    const size_t              epoch               = 5000,
                    const double              beta1               = 0.9,
                    const double              beta2               = 0.999,
                    const double              learning_rate_init  = 0.1,
                    const double              learning_rate_decay = 0.001) {

        if (entries.empty())
            return;

        static constexpr double ADAM_EPS = 1e-8;

        ParameterVector momentum(N_PARAMS);
        ParameterVector velocity(N_PARAMS);

        double beta1_pow = 1.0;
        double beta2_pow = 1.0;

        const double invN       = 1.0 / entries.size();
        const double k_over_400 = K / 400.0;

        for (size_t i = 1; i <= epoch; i++)
        {
            // Exponential learning rate decay
            const double learning_rate =
              learning_rate_init * std::exp(-learning_rate_decay * (i - 1));

            // Track beta powers for bias correction of moving averages
            beta1_pow *= beta1;
            beta2_pow *= beta2;

            ParameterVector gradient(N_PARAMS);
            compute_gradient(gradient, entries, params, K);

            for (size_t param = 0; param < N_PARAMS; param++)
            {
                const double decay = (param < NB_PIECETYPE) ? 1e-3 : 1e-4;

                // Scale raw gradients by K factor and average over batch
                const double mg_grad = -k_over_400 * gradient[param][MG] * invN;
                const double eg_grad = -k_over_400 * gradient[param][EG] * invN;

                // Update 1st moment (momentum) estimates
                momentum[param][MG] = beta1 * momentum[param][MG] + (1.0 - beta1) * mg_grad;
                momentum[param][EG] = beta1 * momentum[param][EG] + (1.0 - beta1) * eg_grad;

                // Update 2nd moment (variance) estimates
                velocity[param][MG] =
                  beta2 * velocity[param][MG] + (1.0 - beta2) * (mg_grad * mg_grad);
                velocity[param][EG] =
                  beta2 * velocity[param][EG] + (1.0 - beta2) * (eg_grad * eg_grad);

                // Bias correction (1st moment): undo bias from zero-initialized momentum
                const double mg_m_hat = momentum[param][MG] / (1.0 - beta1_pow);
                const double eg_m_hat = momentum[param][EG] / (1.0 - beta1_pow);

                // Bias correction (2nd moment): keep variance estimate unbiased early on
                const double mg_v_hat = velocity[param][MG] / (1.0 - beta2_pow);
                const double eg_v_hat = velocity[param][EG] / (1.0 - beta2_pow);

                // AdamW-style decoupled weight decay keeps L2 penalty separate from adaptive step
                const double mg_update =
                  mg_m_hat / (std::sqrt(mg_v_hat) + ADAM_EPS) + decay * params[param][MG];
                const double eg_update =
                  eg_m_hat / (std::sqrt(eg_v_hat) + ADAM_EPS) + decay * params[param][EG];

                // Apply AdamW step scaled by learning rate
                params[param][MG] -= learning_rate * mg_update;
                params[param][EG] -= learning_rate * eg_update;
            }

            if (i % 5 == 0)
            {
                normalize_psqt_params(params);
            }

            if (i % 100 == 0)
            {
                const double error = mse(entries, params, K);
                std::cout << "Error = " << error << std::endl;
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

    void tune() {
        std::string data_path = "data.epd";

        ParameterVector params = init_parameters();
        normalize_psqt_params(params);

        print_param_array(params, 0, NB_PIECETYPE);
        print_psqt(params, NB_PIECETYPE);

        std::vector<Entry> entries{};
        read_epd(entries, data_path);

        double K = compute_optimal_K(entries, params);
        std::cout << "K = " << (double) K << std::endl;

        std::array<size_t, 3> epochs = {1000, 3000, 5000};

        for (const auto epoch : epochs)
        {
            run(params, entries, K, epoch);
            K = compute_optimal_K(entries, params);
            std::cout << "K = " << (double) K << std::endl;
        }

        print_param_array(params, 0, NB_PIECETYPE);
        print_psqt(params, NB_PIECETYPE);
    }

}
