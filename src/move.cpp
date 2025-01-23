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

        Move Move::fromId(const u16 id) {
            const Square   from = static_cast<Square>(id & 0x3F);
            const Square   to   = static_cast<Square>((id >> 6) & 0x3F);
            const MoveFlag flag = static_cast<MoveFlag>((id >> 12) & 0xF);
            return Move(from, to, flag);
        }

        void Move::setScore(const u32 s) { score = s; }

        Square Move::getFrom() const { return from; }

        Square Move::getTo() const { return to; }

        MoveFlag Move::getFlag() const { return flag; }

        u32 Move::getScore() const { return score; }

        u16 Move::id() const { return (flag << 12) | (to << 6) | from; }

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

        Move& Move::operator=(const Move& rhs) {
            from  = rhs.from;
            to    = rhs.to;
            flag  = rhs.flag;
            score = rhs.score;
            return *this;
        }

        bool Move::operator==(const Move& rhs) const { return id() == rhs.id(); }

        bool Move::operator!=(const Move& rhs) const { return id() != rhs.id(); };
    }
}
