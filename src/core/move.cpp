#include "move.h"

#include "core/types.h"

#include <iostream>
#include <sstream>

namespace sagittar {

    static const char* PROMOTION_PIECE_STR = "xxxxxxxxnbrqnbrq";

    void Move::toString(std::ostringstream& ss) const {
        const auto from_sq = from();
        const auto to_sq   = to();
        const auto f       = flag();

        ss << (char) FILE_STR[from_sq.file()];
        ss << (int) (from_sq.rank() + 1);
        ss << (char) FILE_STR[to_sq.file()];
        ss << (int) (to_sq.rank() + 1);

        if (isPromotion())
        {
            ss << (char) PROMOTION_PIECE_STR[f];
        }
    }

    void Move::display() const {
        std::ostringstream ss;
        toString(ss);
        std::cout << ss.str() << std::flush;
    }

}  // namespace sagittar
