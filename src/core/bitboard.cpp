#include "bitboard.h"

#include "core/types.h"

#include <array>

namespace sagittar {

    static constexpr BitBoard compute_ray(const Square x, const Square y) {
        if (x == y)
        {
            return 0ULL;
        }

        const Rank rx = x.rank();
        const Rank ry = y.rank();
        const File fx = x.file();
        const File fy = y.file();

        if (!isAligned(x, y))
        {
            return 0ULL;
        }

        const int dr = (ry > rx) - (ry < rx);
        const int df = (fy > fx) - (fy < fx);

        BitBoard ray{};

        int r = rx, f = fx;

        ray |= BB(Square::create(r, f));

        while (r != ry || f != fy)
        {
            r += dr;
            f += df;
            ray |= BB(Square::create(r, f));
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

        const Rank rx = x.rank();
        const Rank ry = y.rank();
        const File fx = x.file();
        const File fy = y.file();

        if (!isAligned(x, y))
        {
            return 0ULL;
        }

        const int dr = (ry > rx) - (ry < rx);
        const int df = (fy > fx) - (fy < fx);

        BitBoard bb{};

        for (int r = rx + dr, f = fx + df;
             r >= Rank::RANK_1 && r <= Rank::RANK_8 && f >= File::FILE_A && f <= File::FILE_H;
             r += dr, f += df)
        {
            bb |= BB(Square::create(r, f));
        }

        for (int r = rx - dr, f = fx - df;
             r >= Rank::RANK_1 && r <= Rank::RANK_8 && f >= File::FILE_A && f <= File::FILE_H;
             r -= dr, f -= df)
        {
            bb |= BB(Square::create(r, f));
        }

        bb |= BB(x);

        return bb;
    }

    static constexpr auto RAY_BB = []() {
        std::array<std::array<BitBoard, 64>, 64> rays{};

        for (const Square x : Square::all())
        {
            for (const Square y : Square::all())
            {
                rays[x.index()][y.index()] = compute_ray(x, y);
            }
        }

        return rays;
    }();

    static constexpr auto LINE_BB = []() {
        std::array<std::array<BitBoard, 64>, 64> lines{};

        for (const Square x : Square::all())
        {
            for (const Square y : Square::all())
            {
                lines[x.index()][y.index()] = compute_line(x, y);
            }
        }

        return lines;
    }();

    static constexpr auto BETWEEN_BB = []() {
        std::array<std::array<BitBoard, 64>, 64> bet{};

        for (const Square x : Square::all())
        {
            for (const Square y : Square::all())
            {
                bet[x.index()][y.index()] = compute_between(x, y);
            }
        }

        return bet;
    }();

    BitBoard ray(const Square x, const Square y) { return RAY_BB[x.index()][y.index()]; }

    BitBoard line(const Square x, const Square y) { return LINE_BB[x.index()][y.index()]; }

    BitBoard between(const Square x, const Square y) { return BETWEEN_BB[x.index()][y.index()]; }

}  // namespace sagittar
