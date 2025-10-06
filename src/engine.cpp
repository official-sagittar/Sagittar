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

    void Engine::perft(const int depth) {
        core::TranspositionTable<core::TTClient::PERFT, uint64_t, uint32_t> tt(tt_size_mb);
        using clock = std::chrono::high_resolution_clock;
        auto start  = clock::now();
        auto nodes  = core::divide(pos, depth, tt);
        auto end    = clock::now();
        auto elapsed_ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Elapsed = " << elapsed_ms
                  << " ms\nNPS = " << (double) (nodes / (elapsed_ms / 1000.0f)) << std::endl;
    }

    void Engine::display_position() const { pos.display(); }

}
