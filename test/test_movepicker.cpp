#include "containers.h"
#include "doctest/doctest.h"
#include "fen.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "pch.h"
#include "position.h"
#include "search.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("Movepicker") {

    TEST_CASE("movepicker::next::all") {
        search::SearcherData data;

        core::Position pos;
        fen::parseFEN(&pos, "4k3/8/8/1r1q1n1p/2B1P1P1/2N5/5q2/1R1RK3 w - - 0 1");

        int i                    = 0;
        int capture_move_done_at = -1;

        const move::Move pvmove(Square::E1, Square::F2, move::MoveFlag::MOVE_CAPTURE);

        std::array<move::ExtMove, MOVES_MAX>          buffer{};
        search::MovePicker<movegen::MovegenType::ALL> move_picker(buffer.data(), pos, pvmove, data,
                                                                  0);

        while (move_picker.hasNext())
        {
            const move::Move move = move_picker.next();

            if (i == 0)
            {
                REQUIRE(move == pvmove);
                // Immediatly moves to next phase
                REQUIRE(move_picker.phase() == search::MovePickerPhase::CAPTURES);
            }
            else
            {
                if (move.isCapture())
                {
                    REQUIRE(move_picker.phase() == search::MovePickerPhase::CAPTURES);
                    REQUIRE(move != pvmove);
                }
                else
                {
                    // Check that the phase should never be KILLERS
                    // Since no Killer quite moves have been provided
                    REQUIRE(move_picker.phase() != search::MovePickerPhase::KILLERS);
                    REQUIRE(move_picker.phase() == search::MovePickerPhase::QUIETS);

                    // Check if Quite moves come after Captures
                    if (capture_move_done_at == -1)
                    {
                        capture_move_done_at = i;
                    }
                    else
                    {
                        REQUIRE(i > capture_move_done_at);
                    }
                }
            }

            ++i;
        }

        // Check if all moves are processed
        REQUIRE(i == move_picker.size());
    }

    TEST_CASE("movepicker::next::all with Killers") {
        search::SearcherData data;

        core::Position pos;
        fen::parseFEN(&pos, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

        int i                    = 0;
        int capture_move_done_at = -1;
        int killers              = 0;
        int killer_move_done_at  = -1;

        const move::Move pvmove(Square::D5, Square::E6, move::MoveFlag::MOVE_CAPTURE);
        data.killer_moves[0][0] = move::Move(Square::F3, Square::D3, move::MoveFlag::MOVE_QUIET);
        data.killer_moves[1][0] = move::Move(Square::D2, Square::E3, move::MoveFlag::MOVE_QUIET);

        std::array<move::ExtMove, MOVES_MAX>          buffer{};
        search::MovePicker<movegen::MovegenType::ALL> move_picker(buffer.data(), pos, pvmove, data,
                                                                  0);

        while (move_picker.hasNext())
        {
            const move::Move move = move_picker.next();

            if (i == 0)
            {
                REQUIRE(move == pvmove);
                // Immediatly moves to next phase
                REQUIRE(move_picker.phase() == search::MovePickerPhase::CAPTURES);
            }
            else
            {
                if (move.isCapture())
                {
                    REQUIRE(move_picker.phase() == search::MovePickerPhase::CAPTURES);
                    REQUIRE(move != pvmove);
                }
                else
                {
                    // Check if Quite moves come after Captures
                    if (capture_move_done_at == -1)
                    {
                        capture_move_done_at = i;
                    }
                    else
                    {
                        REQUIRE(i > capture_move_done_at);
                    }

                    if (killers < 2)
                    {
                        REQUIRE(move_picker.phase() == search::MovePickerPhase::KILLERS);
                        killers++;
                    }
                    else
                    {
                        REQUIRE(move_picker.phase() == search::MovePickerPhase::QUIETS);
                        if (killer_move_done_at == -1)
                        {
                            killer_move_done_at = i;
                        }
                        else
                        {
                            REQUIRE(i > killer_move_done_at);
                        }
                    }
                }
            }

            ++i;
        }

        // Check if all moves are processed
        REQUIRE(i == move_picker.size());
    }

    TEST_CASE("movepicker::next::captures") {
        search::SearcherData data;

        core::Position pos;
        fen::parseFEN(&pos, "4k3/8/8/1r1q1n1p/2B1P1P1/2N5/5q2/1R1RK3 w - - 0 1");

        int i = 0;

        const move::Move pvmove(Square::E1, Square::F2, move::MoveFlag::MOVE_CAPTURE);

        std::array<move::ExtMove, MOVES_MAX>               buffer{};
        search::MovePicker<movegen::MovegenType::CAPTURES> move_picker(buffer.data(), pos, pvmove,
                                                                       data, 0);
        while (move_picker.hasNext())
        {
            const move::Move move = move_picker.next();

            if (i == 0)
            {
                REQUIRE(move == pvmove);
                // Immediatly moves to next phase
                REQUIRE(move_picker.phase() == search::MovePickerPhase::CAPTURES);
            }
            else
            {
                REQUIRE(move.isCapture());
                REQUIRE(move_picker.phase() == search::MovePickerPhase::CAPTURES);
                REQUIRE(move != pvmove);
            }

            ++i;
        }

        // Check if all moves are processed
        REQUIRE(i == move_picker.size());
    }
}
