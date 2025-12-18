#include "tuner.h"

#include "core/position.h"
#include "eval/hce/tuner/base.h"

namespace sagittar::eval::hce::tuner {

    constexpr size_t NB_COLOR     = 2;
    constexpr size_t NB_PIECETYPE = 7;
    constexpr size_t NB_SQUARE    = 64;

    constexpr i32 N_PARAMS = NB_PIECETYPE                 // Piece Scores
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
        Position pos;
        pos.setFen(fen);

        const EvalTrace trace = create_eval_trace(pos);

        Entry e{};
        init_entry_coeffs(e, trace);
        e.stm   = pos.stm();
        e.phase = pos_phase(pos);
        e.wdl   = extract_wdl(fen);

        return e;
    }

    double linear_eval(const Entry& entry, const ParameterVector& params) {
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

    constexpr double sigmoid(const double K, const double E) {
        return 1.0 / (1.0 + std::exp(-K * E / 400.0));
    }

    double mse(const std::span<Entry> entries, const ParameterVector& params, const double K) {
        double total_error = 0;

        for (const Entry& entry : entries)
        {
            const double eval        = linear_eval(entry, params);
            const double sig         = sigmoid(K, eval);
            const double diff        = entry.wdl - sig;
            const double entry_error = std::pow(diff, 2);
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
            return;

        const double w_eg = entry.phase / 256.0;
        const double w_mg = 1.0 - w_eg;

        const double mg_base = res * w_mg;
        const double eg_base = res * w_eg;

        for (int i = 0; i < N_PARAMS; i++)
        {
            gradient[i][MG] += mg_base * entry.coefficients[i];
            gradient[i][EG] += eg_base * entry.coefficients[i];
        }
    }

    void tune() {
        ParameterVector params = init_parameters();

        print_param_array(params, 0, NB_PIECETYPE);
        auto index = print_psqt(params, NB_PIECETYPE);

        const Entry e =
          create_entry("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 - [1-0]");

        const double eval = linear_eval(e, params);
        std::cout << (double) eval << std::endl;
    }

}
