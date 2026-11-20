#include "move.h"

namespace sagittar {

    static const char* g_promotion_piece_str = "xxxxxxxxnbrqnbrq";

    void Move::toString(std::ostringstream& ss) const {
        const auto from_sq = from();
        const auto to_sq   = to();
        const auto f       = flag();

        ss << (char) k_FILE_STR[sq2file(from_sq)];
        ss << (int) (sq2rank(from_sq) + 1);
        ss << (char) k_FILE_STR[sq2file(to_sq)];
        ss << (int) (sq2rank(to_sq) + 1);

        if (isPromotion())
        {
            ss << (char) g_promotion_piece_str[f];
        }
    }

    void Move::display() const {
        std::ostringstream ss;
        toString(ss);
        std::cout << ss.str() << std::flush;
    }

}
