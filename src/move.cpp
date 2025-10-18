#include "move.h"

namespace sagittar {

    namespace move {

        static const char* PROMOTION_PIECE_STR = "xxxxxxxxnbrqnbrq";

        Move::Move() :
            m_data(0) {}

        Move::Move(const Move& other) :
            m_data(other.m_data) {}

        Move::Move(const Square from, const Square to, const MoveFlag flag) {
            m_data = (flag << 12) | (to << 6) | from;
        }

        Move::Move(const u16 data) :
            m_data(data) {}

        Move& Move::operator=(const Move& rhs) {
            m_data = rhs.m_data;
            return *this;
        }

        bool Move::operator==(const Move& rhs) const { return id() == rhs.id(); }

        bool Move::operator!=(const Move& rhs) const { return id() != rhs.id(); };

        Move Move::fromId(const u16 id) { return Move(id); }

        Square Move::from() const { return static_cast<Square>(m_data & 0x3F); }

        Square Move::to() const { return static_cast<Square>((m_data >> 6) & 0x3F); }

        MoveFlag Move::flag() const { return static_cast<MoveFlag>((m_data >> 12) & 0xF); }

        u16 Move::id() const { return m_data; }

        bool Move::isCapture() const { return (flag() & 0x4); }

        bool Move::isPromotion() const { return (flag() & 0x8); }

        void Move::toString(std::ostringstream& ss) const {
            const auto from_sq = from();
            const auto to_sq   = to();
            const auto f       = flag();

            ss << (char) FILE_STR[sq2file(from_sq)];
            ss << (int) (sq2rank(from_sq) + 1);
            ss << (char) FILE_STR[sq2file(to_sq)];
            ss << (int) (sq2rank(to_sq) + 1);

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
    }
}
