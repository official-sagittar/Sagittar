#include "uci.h"
#include "board.h"
#include "utils.h"

namespace sagittar {

    namespace uci {

        UCIHandler::UCIHandler(Engine& engine) :
            engine(engine) {}

        void UCIHandler::handleUCI() {
            std::ostringstream ss;
            ss << "id name " << engine.getName() << "\n";
            ss << "id author the pixie developers (see AUTHORS file)\n";
            ss << "option name Hash type spin default 1 min 1 max 1\n";
            ss << "option name Threads type spin default 1 min 1 max 1\n";
            ss << "uciok";
            std::cout << ss.str() << std::endl;
        }

        void UCIHandler::handleIsReady() { std::cout << "readyok" << std::endl; }

        void UCIHandler::handleUCINewgame() { engine.reset(); }

        void UCIHandler::handleDisplay() { engine.displayBoard(); }

        void UCIHandler::handlePosition(std::string& input) {
            std::istringstream ss(input);
            std::string        segment;
            const std::size_t  moves_pos      = input.find("moves");
            bool               is_valid_input = true;

            // Discard command
            ss >> segment;

            ss >> segment;
            if (segment == "startpos")
            {
                engine.setStartpos();
            }
            else if (segment == "fen")
            {
                const std::size_t len =
                  (moves_pos == std::string::npos) ? input.length() : moves_pos;
                segment = input.substr(13, len - 13);
                // clang-format off
                try
                {
                    engine.setPositionFromFEN(segment);
                }
                catch (const std::invalid_argument& e)
                {
                    is_valid_input = false;
                    std::cerr << e.what() << std::endl;
                }
                // clang-format on
            }

            if (is_valid_input && (moves_pos != std::string::npos))
            {
                segment = input.substr(moves_pos + 6);
                std::stringstream movesss(segment);
                std::string       move;
                while (movesss >> move)
                {
                    if (engine.doMove(move) != board::DoMoveResult::LEGAL) [[unlikely]]
                    {
                        std::cerr << "Invalid Move!" << std::endl;
                    }
                }
            }
        }

        std::future<void> UCIHandler::handleGo(std::string& input) {
            std::istringstream ss(input);
            std::string        token;

            int movestogo = 30, movetime = -1, time = -1, inc = 0, depth = -1;

            ss >> token;

            while (ss >> token)
            {
                if (token == "infinite")
                {
                    // Do nothing
                }
                else if (token == "wtime")
                {
                    int value;
                    ss >> value;
                    time = value;
                }
                else if (token == "btime")
                {
                    int value;
                    ss >> value;
                    time = value;
                }
                else if (token == "winc")
                {
                    int value;
                    ss >> value;
                    inc = value;
                }
                else if (token == "binc")
                {
                    int value;
                    ss >> value;
                    inc = value;
                }
                else if (token == "movestogo")
                {
                    int value;
                    ss >> value;
                    movestogo = value;
                }
                else if (token == "movetime")
                {
                    int value;
                    ss >> value;
                    movetime = value;
                }
                else if (token == "depth")
                {
                    int value;
                    ss >> value;
                    depth = value;
                }
            }

            if (movetime != -1)
            {
                time      = movetime;
                movestogo = 1;
            }

            if (depth == -1)
            {
                depth = search::MAX_DEPTH;
            }

            search::SearchInfo info;
            info.depth     = depth;
            info.timeset   = false;
            info.starttime = utils::currtimeInMilliseconds();
            info.stoptime  = 0ULL;

            if (time > 0)
            {
                info.timeset = true;
                time /= movestogo;
                time -= 50;
                info.stoptime = info.starttime + time + inc;
            }

            auto searchProgressReportHandler = [](const search::SearchResult& result) {
                std::ostringstream ss;
                ss << "info score ";
                if (result.is_mate)
                {
                    ss << "mate " << (int) result.mate_in;
                }
                else
                {
                    ss << "cp " << (int) result.score;
                }
                ss << " depth " << (unsigned int) result.depth;
                ss << " nodes " << (unsigned long long) result.nodes;
                ss << " time " << (unsigned long long) result.time;
                ss << " nps " << (unsigned long long) (result.nodes * 1000 / (result.time + 1));
                ss << " pv ";
                for (const auto& m : result.pv)
                {
                    m.toString(ss);
                    ss << " ";
                }
                std::cout << ss.str() << std::endl;
            };

            auto searchCompleteReportHander = [](const search::SearchResult& result) {
                std::ostringstream ss;
                result.bestmove.toString(ss);
                std::cout << "bestmove " << ss.str() << std::endl;
            };

            std::future<void> f = std::async(std::launch::async, [this, info,
                                                                  searchProgressReportHandler,
                                                                  searchCompleteReportHander] {
                (void) engine.search(info, searchProgressReportHandler, searchCompleteReportHander);
            });

            return f;
        }

        void UCIHandler::start() {
            std::string       input;
            std::future<void> uci_go_future;

            while (std::getline(std::cin, input))
            {
                if (input == "uci")
                {
                    handleUCI();
                }
                else if (input == "isready")
                {
                    handleIsReady();
                }
                else if (input == "ucinewgame")
                {
                    handleUCINewgame();
                }
                else if (input.rfind("setoption", 0) == 0)
                {
                    // TODO
                }
                else if (input.rfind("position", 0) == 0)
                {
                    handlePosition(input);
                    engine.resetForSearch();
                }
                else if (input.rfind("go", 0) == 0)
                {
                    uci_go_future = handleGo(input);
                }
                else if (input == "d")
                {
                    handleDisplay();
                }
                else if (input == "stop")
                {
                    engine.stopSearch();
                    if (uci_go_future.valid())
                    {
                        uci_go_future.wait();
                    }
                }
                else if (input == "quit")
                {
                    engine.stopSearch();
                    if (uci_go_future.valid())
                    {
                        uci_go_future.wait();
                    }
                    break;
                }
            }
        }

    }

}
