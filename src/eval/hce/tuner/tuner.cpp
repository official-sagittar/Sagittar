#include "tuner.h"

#include "core/position.h"
#include "eval/hce/tuner/base.h"

namespace sagittar::eval::hce::tuner {

    namespace {

        constexpr size_t NB_COLOR     = 2;
        constexpr size_t NB_PIECETYPE = 7;
        constexpr size_t NB_SQUARE    = 64;

        constexpr size_t N_PARAMS = NB_PIECETYPE                 // Piece Scores
                                  + (NB_PIECETYPE * NB_SQUARE);  // PSQT

        struct EvalTrace {
            i32 piece_counts[NB_PIECETYPE][NB_COLOR]          = {};
            i32 psq_counts[NB_PIECETYPE][NB_SQUARE][NB_COLOR] = {};
        };

        ParameterVector init_parameters() {
            ParameterVector params{};

            init_param_array(params, PIECE_SCORES, NB_PIECETYPE);
            init_param_array_2d(params, PSQT_SCORES, NB_PIECETYPE, NB_SQUARE);

            return params;
        }

        EvalTrace create_eval_trace(const Position& pos) {
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

        void init_entry_coeffs(Entry& e, const EvalTrace& trace) {
            init_coeff_array(e, trace.piece_counts, NB_PIECETYPE);
            init_coeff_array_2d(e, trace.psq_counts, NB_PIECETYPE, NB_SQUARE);
        }

        double extract_wdl(std::string_view fen) {
            if (fen.find("[1-0]") != (size_t) std::string::npos)
            {
                return 1.0;
            }
            else if (fen.find("[0-1]") != (size_t) std::string::npos)
            {
                return 0.0;
            }

            return 0.5;
        }

        Entry create_entry(const std::string& fen) {
            Position pos{};
            pos.setFen(fen);

            const EvalTrace trace = create_eval_trace(pos);

            Entry e{};
            init_entry_coeffs(e, trace);
            e.stm   = pos.stm();
            e.phase = pos_phase(pos);  // 0 => MG; 256 => EG
            e.wdl   = extract_wdl(fen);

            return e;
        }

        double linear_eval(const Entry& entry, const ParameterVector& params) {
            // -INFINITY    => BLACK WINNING
            // 0            => DRAW
            // +INFINITY    => WHITE WINNING

            double eval_mg = 0;
            double eval_eg = 0;

            for (size_t i = 0; i < N_PARAMS; i++)
            {
                const i32            coeff = entry.coefficients[i];
                const ParameterTuple param = params[i];

                eval_mg += coeff * param[MG];
                eval_eg += coeff * param[EG];
            }

            double eval = static_cast<double>(scale_eval(eval_mg, eval_eg, entry.phase));

            return eval;
        }

        double sigmoid(const double K, const double E) {
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

        double mse(const std::span<Entry> entries, const ParameterVector& params, const double K) {
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

        void update_gradient_single(ParameterVector&       gradient,
                                    const Entry&           entry,
                                    const ParameterVector& params,
                                    const double           K) {
            const double eval = linear_eval(entry, params);
            const double sig  = sigmoid(K, eval);
            const double res  = (entry.wdl - sig) * sig * (1 - sig);

            if (std::abs(res) < 1e-12)
            {
                return;
            }

            const double w_eg = entry.phase / 256.0;
            const double w_mg = 1.0 - w_eg;

            const double mg_base = res * w_mg;
            const double eg_base = res * w_eg;

            for (size_t i = 0; i < N_PARAMS; i++)
            {
                gradient[i][MG] += mg_base * entry.coefficients[i];
                gradient[i][EG] += eg_base * entry.coefficients[i];
            }
        }

        void compute_gradient(ParameterVector&       gradient,
                              const std::span<Entry> entries,
                              const ParameterVector& params,
                              const double           K) {
            for (const Entry& e : entries)
            {
                update_gradient_single(gradient, e, params, K);
            }
        }

        void normalize_psqt_params(ParameterVector& params) {
            for (size_t pt = 0; pt < NB_PIECETYPE; ++pt)
            {
                double mg_sum = 0.0;
                double eg_sum = 0.0;

                const int base = NB_PIECETYPE + pt * NB_SQUARE;

                // 1) Compute mean
                for (size_t sq = 0; sq < NB_SQUARE; ++sq)
                {
                    mg_sum += params[base + sq][MG];
                    eg_sum += params[base + sq][EG];
                }

                const double mg_mean = mg_sum / NB_SQUARE;
                const double eg_mean = eg_sum / NB_SQUARE;

                // 2) Subtract mean
                for (size_t sq = 0; sq < NB_SQUARE; ++sq)
                {
                    params[base + sq][MG] -= mg_mean;
                    params[base + sq][EG] -= eg_mean;
                }
            }
        }

        void run(ParameterVector&       params,
                 const std::span<Entry> entries,
                 const double           K,
                 const size_t           epoch                       = 5000,
                 const double           beta1                       = 0.9,
                 const double           beta2                       = 0.999,
                 double                 learning_rate               = 0.1,
                 const size_t           learning_rate_drop_interval = 500,
                 const double           learning_rate_drop_ratio    = 0.5) {

            if (entries.empty())
                return;

            static constexpr double ADAM_EPS = 1e-8;

            ParameterVector momentum{};
            ParameterVector velocity{};

            double beta1_pow = 1.0;
            double beta2_pow = 1.0;

            const double invN       = 1.0 / entries.size();
            const double k_over_400 = K / 400.0;

            for (size_t i = 1; i <= epoch; i++)
            {
                beta1_pow *= beta1;
                beta2_pow *= beta2;

                ParameterVector gradient{};
                compute_gradient(gradient, entries, params, K);

                for (size_t param = 0; param < N_PARAMS; param++)
                {
                    const double decay = (param < NB_PIECETYPE) ? 1e-3 : 1e-4;

                    const double mg_grad = -k_over_400 * gradient[param][MG] * invN;
                    const double eg_grad = -k_over_400 * gradient[param][EG] * invN;

                    momentum[param][MG] = beta1 * momentum[param][MG] + (1.0 - beta1) * mg_grad;
                    momentum[param][EG] = beta1 * momentum[param][EG] + (1.0 - beta1) * eg_grad;

                    velocity[param][MG] =
                      beta2 * velocity[param][MG] + (1.0 - beta2) * (mg_grad * mg_grad);
                    velocity[param][EG] =
                      beta2 * velocity[param][EG] + (1.0 - beta2) * (eg_grad * eg_grad);

                    const double mg_m_hat = momentum[param][MG] / (1.0 - beta1_pow);
                    const double eg_m_hat = momentum[param][EG] / (1.0 - beta1_pow);

                    const double mg_v_hat = velocity[param][MG] / (1.0 - beta2_pow);
                    const double eg_v_hat = velocity[param][EG] / (1.0 - beta2_pow);

                    const double mg_update =
                      mg_m_hat / (std::sqrt(mg_v_hat) + ADAM_EPS) + decay * params[param][MG];
                    const double eg_update =
                      eg_m_hat / (std::sqrt(eg_v_hat) + ADAM_EPS) + decay * params[param][EG];

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

                if (i % learning_rate_drop_interval == 0)
                {
                    learning_rate *= learning_rate_drop_ratio;
                }
            }
        }

    }

    void tune() {
        ParameterVector params = init_parameters();
        normalize_psqt_params(params);

        print_param_array(params, 0, NB_PIECETYPE);
        auto index = print_psqt(params, NB_PIECETYPE);

        const Entry e = create_entry("4k3/8/8/8/8/8/8/3QK3 b - - 0 1 - [1-0]");

        std::cout << (double) e.wdl << std::endl;

        const double eval = linear_eval(e, params);
        std::cout << (double) eval << std::endl;

        const auto sig = sigmoid(2.5, eval);
        std::cout << (double) sig << std::endl;
    }

}
