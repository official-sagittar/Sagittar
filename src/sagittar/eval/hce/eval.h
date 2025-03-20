#pragma once

#include "commons/pch.h"
#include "sagittar/core/board.h"
#include "sagittar/core/types.h"

namespace sagittar {

    namespace eval {

        namespace hce {

            using namespace core::types;

            void initialize();

            Score evaluateBoard(const core::board::Board&);

            bool isEndGame(const core::board::Board&);

        }

    }

}
