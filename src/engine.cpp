#include "engine.h"
#include "core/movegen.h"
#include "core/perft.h"
#include "core/tt.h"
#include "core/utils.h"

namespace sagittar {

    Engine::Engine() :
        tt_size_mb(core::TT_SIZE_DEFAULT) {
        core::position_init();
        core::movegen_init();
        pos.reset();
    }

    void Engine::reset() { pos.reset(); }

    void Engine::set_tt_size_mb(const size_t tt_size_mb) { this->tt_size_mb = tt_size_mb; }

    bool Engine::set_fen(std::string fen) { return pos.set_fen(fen); }

    bool Engine::do_move(const std::string& move_str) {
        const auto [is_legal_move, new_pos] = pos.do_move(move_str);
        if (is_legal_move)
        {
            pos = new_pos;
        }
        return is_legal_move;
    }

    void Engine::perft(const int depth) {}

    void Engine::display_position() const { pos.display(); }

}
