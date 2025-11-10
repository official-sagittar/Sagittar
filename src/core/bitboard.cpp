#include "bitboard.h"

namespace sagittar {

    static constexpr BitBoard compute_ray(const Square x, const Square y) {
        if (x == y)
        {
            return 0ULL;
        }

        const Rank rx = sq2rank(x);
        const Rank ry = sq2rank(y);
        const File fx = sq2file(x);
        const File fy = sq2file(y);

        if (!isAligned(x, y))
        {
            return 0ULL;
        }

        const int dr = (ry > rx) - (ry < rx);
        const int df = (fy > fx) - (fy < fx);

        BitBoard ray = 0ULL;

        int r = rx, f = fx;

        ray |= BB(rf2sq(r, f));

        while (r != ry || f != fy)
        {
            r += dr;
            f += df;
            ray |= BB(rf2sq(r, f));
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

        const Rank rx = sq2rank(x);
        const Rank ry = sq2rank(y);
        const File fx = sq2file(x);
        const File fy = sq2file(y);

        if (!isAligned(x, y))
        {
            return 0ULL;
        }

        const int dr = (ry > rx) - (ry < rx);
        const int df = (fy > fx) - (fy < fx);

        BitBoard bb = 0ULL;

        for (int r = rx + dr, f = fx + df;
             r >= Rank::RANK_1 && r <= Rank::RANK_8 && f >= File::FILE_A && f <= File::FILE_H;
             r += dr, f += df)
        {
            bb |= BB(rf2sq(r, f));
        }

        for (int r = rx - dr, f = fx - df;
             r >= Rank::RANK_1 && r <= Rank::RANK_8 && f >= File::FILE_A && f <= File::FILE_H;
             r -= dr, f -= df)
        {
            bb |= BB(rf2sq(r, f));
        }

        bb |= BB(x);

        return bb;
    }

    static constexpr auto RAY_BB = []() {
        std::array<std::array<BitBoard, 64>, 64> rays{};

        for (int x = Square::A1; x <= Square::H8; ++x)
        {
            for (int y = Square::A1; y <= Square::H8; ++y)
            {
                rays[x][y] = compute_ray(static_cast<Square>(x), static_cast<Square>(y));
            }
        }

        return rays;
    }();

    static constexpr auto LINE_BB = []() {
        std::array<std::array<BitBoard, 64>, 64> lines{};

        for (int x = Square::A1; x <= Square::H8; ++x)
        {
            for (int y = Square::A1; y <= Square::H8; ++y)
            {
                lines[x][y] = compute_line(static_cast<Square>(x), static_cast<Square>(y));
            }
        }

        return lines;
    }();

    static constexpr auto BETWEEN_BB = []() {
        std::array<std::array<BitBoard, 64>, 64> bet{};

        for (int x = Square::A1; x <= Square::H8; ++x)
        {
            for (int y = Square::A1; y <= Square::H8; ++y)
            {
                bet[x][y] = compute_between(static_cast<Square>(x), static_cast<Square>(y));
            }
        }

        return bet;
    }();

    BitBoard ray(const Square x, const Square y) { return RAY_BB[x][y]; }

    BitBoard line(const Square x, const Square y) { return LINE_BB[x][y]; }

    BitBoard between(const Square x, const Square y) { return BETWEEN_BB[x][y]; }

}
