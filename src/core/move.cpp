#include "move.h"

namespace sagittar {

    namespace core {

        std::string move_tostring(const Move m) {
            const Square   from = MOVE_FROM(m);
            const Square   to   = MOVE_TO(m);
            const MoveFlag flag = MOVE_FLAG(m);

            std::ostringstream oss;

            oss << static_cast<char>('a' + SQ_TO_FILE(from));
            oss << static_cast<char>('1' + SQ_TO_RANK(from));
            oss << static_cast<char>('a' + SQ_TO_FILE(to));
            oss << static_cast<char>('1' + SQ_TO_RANK(to));
            if (MOVE_IS_PROMOTION(m))
            {
                static const char promo_map[4] = {'n', 'b', 'r', 'q'};
                oss << promo_map[flag & 0x3];
            }

            return oss.str();
        }

        MoveList::MoveList() :
            size(0) {
            moves  = {};
            scores = {};
        }

        void MoveList::add_move(const Move move) {
            if (size < MOVE_LIST_SIZE_MAX)
                moves.at(size++) = move;
        }

    }

}
