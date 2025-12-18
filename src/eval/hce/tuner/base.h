#pragma once

#include "commons/pch.h"
#include "core/types.h"
#include "eval/hce/defs.h"

namespace sagittar::eval::hce::tuner {

    using Parameter       = double;
    using ParameterTuple  = std::array<Parameter, 2>;
    using ParameterVector = std::vector<ParameterTuple>;

    struct Entry {
        std::vector<i32> coefficients{};
        Color            stm{Color::WHITE};
        i32              phase{0};
        double           wdl{0};
    };

    void init_param_single(ParameterVector& params, const i32 p) {
        params.push_back({static_cast<double>(mg_score(p)), static_cast<double>(eg_score(p))});
    }

    template<typename T>
    void init_param_array(ParameterVector& params, const T& parr, const size_t n) {
        for (size_t i = 0; i < n; i++)
        {
            init_param_single(params, parr[i]);
        }
    }

    template<typename T>
    void
    init_param_array_2d(ParameterVector& params, const T& parr, const size_t m, const size_t n) {
        for (size_t i = 0; i < m; i++)
        {
            init_param_array(params, parr[i], n);
        }
    }

    template<typename T>
    void init_coeff_single(Entry& entry, const T& trace) {
        const auto coeff = trace[Color::WHITE] - trace[Color::BLACK];
        entry.coefficients.push_back(coeff);
    }

    template<typename T>
    void init_coeff_array(Entry& entry, const T& trace, const size_t n) {
        for (size_t i = 0; i < n; i++)
        {
            init_coeff_single(entry, trace[i]);
        }
    }

    template<typename T>
    void init_coeff_array_2d(Entry& entry, const T& trace, const size_t m, const size_t n) {
        for (size_t i = 0; i < m; i++)
        {
            init_coeff_array(entry, trace[i], n);
        }
    }

    void print_param_single(const ParameterTuple& p, std::string end = "\n") {
        std::cout << "S(" << p[MG] << ", " << p[EG] << ")," << end;
    }

    void print_param_array(ParameterVector& params, const size_t start, const size_t end) {
        for (size_t i = start; i < end; i++)
        {
            print_param_single(params[i]);
        }
    }

    size_t print_psqt(ParameterVector& params, const size_t start) {
        size_t index = start;

        for (int pt = PieceType::PIECE_TYPE_INVALID; pt <= PieceType::KING; pt++)
        {
            for (int sq = Square::A1; sq <= Square::H8; sq++)
            {
                print_param_single(params[index++], "\t");
                if (sq % 8 == 7)
                {
                    std::cout << std::endl;
                }
            }
            std::cout << std::endl;
        }

        return index;
    }

}
