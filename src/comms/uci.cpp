#include "uci.h"
#include "core/move.h"
#include "search/types.h"

namespace sagittar {

    namespace comms {

        namespace uci {

            enum class TaskType {
                PERFT,
                SEARCH
            };

            struct TaskData {
                TaskType task;
                union {
                    int                perft_depth;
                    search::SearchInfo search_info;
                };
            };

            UCIHandler::UCIHandler(Engine& engine) :
                engine(engine) {}

            void UCIHandler::handle_uci() {
                std::ostringstream ss;
                ss << "id name Sagittar v0.1.0\n";
                ss << "id author the Sagittar developers (see AUTHORS file)\n";
                ss << "option name Hash type spin default 16 min 1 max 512\n";
                ss << "option name Threads type spin default 1 min 1 max 1\n";
                ss << "uciok";
                std::cout << ss.str() << std::endl;
            }

            void UCIHandler::handle_isready() { std::cout << "readyok" << std::endl; }

            void UCIHandler::handle_ucinewgame() { engine.reset(); }

            void UCIHandler::handle_setoption(std::string& input) {
                std::istringstream ss(input);
                std::string        cmd, name, id, valuename, value;

                ss >> cmd >> name >> id >> valuename >> value;

                if (id == "Hash")
                {
                    const size_t ttsize = static_cast<std::size_t>(std::stoi(value));
                    if (ttsize >= 1 && ttsize <= 512)
                    {
                        engine.set_tt_size_mb(ttsize);
                    }
                }
            }

            void UCIHandler::handle_position(std::string& input) {
                std::istringstream ss(input);
                std::string        segment;
                const size_t       moves_pos      = input.find("moves");
                bool               is_valid_input = true;

                ss >> segment;  // Discard command

                ss >> segment;
                if (segment == "startpos")
                {
                    is_valid_input =
                      engine.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
                }
                else if (segment == "fen")
                {
                    const size_t len =
                      (moves_pos == std::string::npos) ? input.length() : moves_pos;
                    segment = input.substr(13, len - 13);

                    is_valid_input = engine.set_fen(segment);
                    if (!is_valid_input) [[unlikely]]
                    {
                        std::cerr << "Invalid FEN!" << std::endl;
                    }
                }

                if (is_valid_input && (moves_pos != std::string::npos))
                {
                    segment = input.substr(moves_pos + 6);
                    std::stringstream movesss(segment);
                    std::string       move;
                    while (movesss >> move)
                    {
                        const bool is_valid = engine.do_move(move);
                        if (!is_valid) [[unlikely]]
                        {
                            std::cerr << "Invalid Move!" << std::endl;
                            break;
                        }
                    }
                }
            }

            std::future<void> UCIHandler::handle_go(std::string& input) {
                std::istringstream ss(input);
                std::string        token;

                TaskData task_data;
                task_data.task        = TaskType::SEARCH;
                task_data.search_info = {};

                ss >> token;

                while (ss >> token)
                {
                    if (token == "perft")
                    {
                        int depth;
                        ss >> depth;

                        task_data.task        = TaskType::PERFT;
                        task_data.perft_depth = depth;

                        break;
                    }
                    else if (token == "infinite")
                    {
                        task_data.search_info.infinite = true;
                    }
                    else if (token == "wtime")
                    {
                        int value;
                        ss >> value;
                        task_data.search_info.wtime = value;
                    }
                    else if (token == "btime")
                    {
                        int value;
                        ss >> value;
                        task_data.search_info.btime = value;
                    }
                    else if (token == "winc")
                    {
                        int value;
                        ss >> value;
                        task_data.search_info.winc = value;
                    }
                    else if (token == "binc")
                    {
                        int value;
                        ss >> value;
                        task_data.search_info.binc = value;
                    }
                    else if (token == "movestogo")
                    {
                        int value;
                        ss >> value;
                        task_data.search_info.movestogo = value;
                    }
                    else if (token == "movetime")
                    {
                        int value;
                        ss >> value;
                        task_data.search_info.movetime = value;
                    }
                    else if (token == "depth")
                    {
                        int value;
                        ss >> value;
                        task_data.search_info.depth = value;
                    }
                }

                std::future<void> f;

                switch (task_data.task)
                {
                    case TaskType::PERFT :
                        f = std::async(std::launch::async, [this, task_data] {
                            (void) engine.perft(task_data.perft_depth);
                        });
                        break;

                    case TaskType::SEARCH : {
                        auto progress_handler = [](const search::SearchResult& result) {
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
                            ss << " depth " << (int) result.depth;
                            ss << " nodes " << (int) result.nodes;
                            ss << " time " << (int) result.time;
                            ss << " nps " << (int) (result.nodes * 1000 / (result.time + 1));
                            std::cout << ss.str() << std::endl;
                        };

                        auto complete_hander = [](const search::SearchResult& result) {
                            std::cout << "bestmove " << core::move_tostring(result.bestmove)
                                      << std::endl;
                        };

                        f = std::async(std::launch::async,
                                       [this, task_data, progress_handler, complete_hander] {
                                           (void) engine.search(task_data.search_info,
                                                                progress_handler, complete_hander);
                                       });

                        break;
                    }

                    default :
                        break;
                }

                return f;
            }

            void UCIHandler::handle_display() { engine.display_position(); }

            void UCIHandler::start() {
                std::string       input;
                std::future<void> uci_go_future;

                while (std::getline(std::cin, input))
                {
                    if (input == "uci")
                    {
                        handle_uci();
                    }
                    else if (input == "isready")
                    {
                        handle_isready();
                    }
                    else if (input == "ucinewgame")
                    {
                        handle_ucinewgame();
                    }
                    else if (input.rfind("setoption", 0) == 0)
                    {
                        handle_setoption(input);
                    }
                    else if (input.rfind("position", 0) == 0)
                    {
                        handle_position(input);
                        engine.reset_for_search();
                    }
                    else if (input.rfind("go", 0) == 0)
                    {
                        uci_go_future = handle_go(input);
                    }
                    else if (input == "d")
                    {
                        handle_display();
                    }
                    else if (input == "stop")
                    {
                        engine.stop_search();
                        if (uci_go_future.valid())
                        {
                            uci_go_future.wait();
                        }
                    }
                    else if (input == "quit")
                    {
                        engine.stop_search();
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

}
