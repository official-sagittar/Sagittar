#include "move.h"

namespace sagittar {

    namespace move {

        static const char* PROMOTION_PIECE_STR = "xxxxxxxxnbrqnbrq";

        Move::Move() :
            from(Square::NO_SQ),
            to(Square::NO_SQ),
            flag(MOVE_QUIET),
            score(0) {}

        Move::Move(const Move& other) :
            from(other.from),
            to(other.to),
            flag(other.flag),
            score(other.score) {}

        Move::Move(const Square from, const Square to, const MoveFlag flag) :
            from(from),
            to(to),
            flag(flag),
            score(0) {}

        void Move::setScore(const u16 s) { score = s; }

        Square Move::getFrom() const { return from; }

        Square Move::getTo() const { return to; }

        MoveFlag Move::getFlag() const { return flag; }

        u16 Move::getScore() const { return score; }

        u32 Move::id() const { return (score << 16) | (flag << 12) | (to << 6) | from; }

        void Move::toString(std::ostringstream& ss) const {
            ss << (char) FILE_STR[sq2file(from)];
            ss << (int) (sq2rank(from) + 1);
            ss << (char) FILE_STR[sq2file(to)];
            ss << (int) (sq2rank(to) + 1);

            if (isPromotion(flag))
            {
                ss << (char) PROMOTION_PIECE_STR[flag];
            }
        }

        void Move::display() const {
            std::ostringstream ss;
            toString(ss);
            std::cout << ss.str() << std::flush;
        }

        Move& Move::operator=(Move const& rhs) {
            from  = rhs.from;
            to    = rhs.to;
            flag  = rhs.flag;
            score = rhs.score;
            return *this;
        }

        bool Move::operator==(Move const& rhs) { return id() == rhs.id(); }
    }
}
