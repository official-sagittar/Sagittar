#include "board.h"

namespace sagittar {

    namespace core {

        static constexpr bool aligned(const Square x, const Square y) {
            const int rx = SQ_TO_RANK(x), ry = SQ_TO_RANK(y);
            const int fx = SQ_TO_FILE(x), fy = SQ_TO_FILE(y);
            return (rx == ry) || (fx == fy) || (rx - fx == ry - fy) || (rx + fx == ry + fy);
        }

        static constexpr BitBoard compute_ray(const Square x, const Square y) {
            if (x == y)
            {
                return 0ULL;
            }

            const Rank rx = SQ_TO_RANK(x);
            const Rank ry = SQ_TO_RANK(y);
            const File fx = SQ_TO_FILE(x);
            const File fy = SQ_TO_FILE(y);

            if (!aligned(x, y))
            {
                return 0ULL;
            }

            const int dr = (ry > rx) - (ry < rx);
            const int df = (fy > fx) - (fy < fx);

            BitBoard ray = 0ULL;

            int r = rx, f = fx;

            ray |= BB(RF_TO_SQ(r, f));

            while (r != ry || f != fy)
            {
                r += dr;
                f += df;
                ray |= BB(RF_TO_SQ(r, f));
            }

            return ray;
        }

        static constexpr BitBoard compute_between(const Square x, const Square y) {
            BitBoard ray = compute_ray(x, y);
            if (!ray)
            {
                return 0ULL;
            }
            // remove both endpoints
            return ray & ~(BB(x) | BB(y));
        }

        constexpr BitBoard compute_line(const Square x, const Square y) {
            if (x == y)
            {
                return 0ULL;
            }

            const Rank rx = SQ_TO_RANK(x);
            const Rank ry = SQ_TO_RANK(y);
            const File fx = SQ_TO_FILE(x);
            const File fy = SQ_TO_FILE(y);

            if (!aligned(x, y))
            {
                return 0ULL;
            }

            const int dr = (ry > rx) - (ry < rx);
            const int df = (fy > fx) - (fy < fx);

            BitBoard bb = 0ULL;

            for (int r = rx + dr, f = fx + df; r >= 0 && r < 8 && f >= 0 && f < 8; r += dr, f += df)
            {
                bb |= BB(RF_TO_SQ(r, f));
            }

            for (int r = rx - dr, f = fx - df; r >= 0 && r < 8 && f >= 0 && f < 8; r -= dr, f -= df)
            {
                bb |= BB(RF_TO_SQ(r, f));
            }

            bb |= BB(x);

            return bb;
        }

        static constexpr auto BETWEEN_BB = []() {
            std::array<std::array<BitBoard, 64>, 64> lines{};
            for (int x = 0; x < 64; ++x)
            {
                for (int y = 0; y < 64; ++y)
                {
                    lines[x][y] = compute_between(static_cast<Square>(x), static_cast<Square>(y));
                }
            }
            return lines;
        }();

        static constexpr auto RAY_BB = []() {
            std::array<std::array<BitBoard, 64>, 64> rays{};
            for (int x = A1; x <= H8; ++x)
            {
                for (int y = A1; y <= H8; ++y)
                {
                    rays[x][y] = compute_ray(static_cast<Square>(x), static_cast<Square>(y));
                }
            }
            return rays;
        }();

        static constexpr auto LINE_BB = []() {
            std::array<std::array<BitBoard, 64>, 64> inf{};
            for (int x = A1; x <= H8; ++x)
            {
                for (int y = A1; y <= H8; ++y)
                {
                    inf[x][y] = compute_line(static_cast<Square>(x), static_cast<Square>(y));
                }
            }
            return inf;
        }();

        inline BitBoard between(const Square x, const Square y) { return BETWEEN_BB[x][y]; }

        inline BitBoard ray(const Square x, const Square y) { return RAY_BB[x][y]; }

        inline BitBoard line(const Square x, const Square y) { return LINE_BB[x][y]; }

    }

}
