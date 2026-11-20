#pragma once

#include "commons/pch.h"
#include "core/types.h"
#include "eval/hce/defs.h"
#include <cstddef>

namespace sagittar::eval::hce::tuner {

    using Parameter       = double;
    using ParameterTuple  = std::array<Parameter, 2>;
    using ParameterVector = std::vector<ParameterTuple>;

    struct Coefficient {
        I32 index;
        I32 value;
    };

    struct Entry {
        std::vector<Coefficient> coefficients{};
        Color                    stm{Color::WHITE};
        I32                      phase{0};
        std::array<double, 2>    pfactors{};
        double                   wdl{0};
        Score                    seval{0};
    };

    void initParamSingle(ParameterVector& params, const I32 p) {
        params.push_back({static_cast<double>(mgScore(p)), static_cast<double>(egScore(p))});
    }

    template<typename T>
    void initParamArray(ParameterVector& params, const T& parr, const size_t n) {
        for (size_t i = 0; i < n; i++)
        {
            initParamSingle(params, parr[i]);
        }
    }

    template<typename T>
    void initParamArray2d(ParameterVector& params, const T& parr, const size_t m, const size_t n) {
        for (size_t i = 0; i < m; i++)
        {
            initParamArray(params, parr[i], n);
        }
    }

    template<typename T>
    void initCoeffSingle(Entry& entry, const T& trace, const size_t index) {
        const I32 coeff = trace[Color::WHITE] - trace[Color::BLACK];
        if (coeff != 0)
        {
            entry.coefficients.push_back({static_cast<I32>(index), coeff});
        }
    }

    template<typename T>
    void initCoeffArray(Entry& entry, const T& trace, const size_t n, size_t& index) {
        for (size_t i = 0; i < n; i++)
        {
            initCoeffSingle(entry, trace[i], index++);
        }
    }

    template<typename T>
    void
    initCoeffArray2d(Entry& entry, const T& trace, const size_t m, const size_t n, size_t& index) {
        for (size_t i = 0; i < m; i++)
        {
            initCoeffArray(entry, trace[i], n, index);
        }
    }

    constexpr I32 roundValue(const Parameter value) { return static_cast<I32>(std::round(value)); }

    void printParamSingle(const ParameterTuple& p, std::string end = "\n") {
        std::cout << "s(" << roundValue(p[MG]) << ", " << roundValue(p[EG]) << ")," << end;
    }

    void printParamArray(ParameterVector& params, const size_t start, const size_t end) {
        for (size_t i = start; i < end; i++)
        {
            printParamSingle(params[i]);
        }
    }

    size_t printPsqt(ParameterVector& params, const size_t start) {
        size_t index = start;

        std::string pt_names[] = {"PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING"};

        std::cout << "constexpr std::array<PSQT, 6> k_PSQT_SCORES = []() {\n";
        std::cout << "std::array<PSQT, 6> table{};" << std::endl;

        for (int pt = PieceType::PAWN; pt <= PieceType::KING; pt++)
        {
            std::cout << "table[PieceType::" << pt_names[pt] << "] = {\n";
            for (int sq = Square::A1; sq <= Square::H8; sq++)
            {
                printParamSingle(params[index++], "\t");
                if (sq % 8 == 7)
                {
                    std::cout << std::endl;
                }
            }
            std::cout << "};" << std::endl;
        }

        std::cout << "return table;\n";
        std::cout << "}();" << std::endl;

        return index;
    }

    size_t printMobility(ParameterVector& params, const size_t start) {
        size_t index = start;

        std::cout << "constexpr std::array<std::array<I32, 28>, 4> k_MOBILITY_BONUS = []() {\n";
        std::cout << "std::array<std::array<I32, 28>, 4> table{};" << std::endl;

        int n_terms = 9;
        std::cout << "table[0] = {" << std::endl;
        printParamArray(params, index, index + n_terms);
        std::cout << "};" << std::endl;
        index += n_terms;

        n_terms = 14;
        std::cout << "table[1] = {" << std::endl;
        printParamArray(params, index, index + n_terms);
        std::cout << "};" << std::endl;
        index += n_terms;

        n_terms = 15;
        std::cout << "table[2] = {" << std::endl;
        printParamArray(params, index, index + n_terms);
        std::cout << "};" << std::endl;
        index += n_terms;

        n_terms = 28;
        std::cout << "table[3] = {" << std::endl;
        printParamArray(params, index, index + n_terms);
        std::cout << "};" << std::endl;
        index += n_terms;

        std::cout << "return table;\n";
        std::cout << "}();" << std::endl;

        return index;
    }

}
