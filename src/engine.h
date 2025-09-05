#pragma once

#include "core/move.h"
#include "core/position.h"
#include "pch.h"

namespace sagittar {

    class Engine {
       public:
        Engine();
        void reset();
        void set_tt_size_mb(const size_t tt_size_mb);
        bool set_fen(std::string fen);
        bool do_move(const core::Move move);
        bool do_move(const std::string& move);
        void perft(const int depth);
        void display_position() const;
        ~Engine() = default;

       private:
        core::Position pos;
        size_t         tt_size_mb;
    };

}
