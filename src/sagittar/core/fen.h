#pragma once

#include "commons/pch.h"
#include "sagittar/core/board.h"

namespace sagittar {

    namespace core {

        namespace fen {

            void parseFEN(board::Board*, std::string, const bool full = true);

            std::string toFEN(const board::Board&);

        }  // namespace fen

    }  // namespace core

}  // namespace sagittar
