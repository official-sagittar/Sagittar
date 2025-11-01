#include "uci.h"
#include "commons/utils.h"
#include "core/position.h"
#include "search/params.h"
#include "search/types.h"

namespace sagittar::comms::uci {

    UCIHandler::UCIHandler(Engine& engine) :
        engine(engine) {}

    void UCIHandler::handleUCI() {
        std::ostringstream ss;
        ss << "id name " << engine.getName() << "\n";
        ss << "id author the Sagittar developers (see AUTHORS file)\n";
        ss << "option name Hash type spin default 16 min 1 max 512\n";
        ss << "option name Threads type spin default 1 min 1 max 1\n";
#ifdef EXTERNAL_TUNE
        for (auto& param : params::params())
        {
            ss << "option name " << param.name;
            ss << " type spin";
            ss << " default " << (int) param.default_value;
            ss << " min " << (int) param.min;
            ss << " max " << (int) param.max;
            ss << "\n";
        }
#endif
        ss << "uciok";
        std::cout << ss.str() << std::endl;
    }

    void UCIHandler::handleIsReady() { std::cout << "readyok" << std::endl; }

    void UCIHandler::handleUCINewgame() { engine.reset(); }

    void UCIHandler::handleSetOption(std::string& input) {
        std::istringstream ss(input);
        std::string        cmd, name, id, valuename, value;

        ss >> cmd >> name >> id >> valuename >> value;

        if (id == "Hash")
        {
            const std::size_t ttsize = static_cast<std::size_t>(std::stoi(value));
            if (ttsize >= 1 && ttsize <= 512)
            {
                engine.setTranspositionTableSize(ttsize);
            }
        }
#ifdef EXTERNAL_TUNE
        else
        {
            if (!params::set(id, std::stoi(value)))
            {
                std::cerr << "Invalid option!" << std::endl;
            }
        }
#endif
    }

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
            engine.setPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }
        else if (segment == "fen")
        {
            const std::size_t len = (moves_pos == std::string::npos) ? input.length() : moves_pos;
            segment               = input.substr(13, len - 13);
            // clang-format off
                try
                {
                    engine.setPosition(segment);
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
                if (engine.doMove(move) != DoMoveResult::LEGAL) [[unlikely]]
                {
                    std::cerr << "Invalid Move!" << std::endl;
                }
            }
        }
    }

    std::future<void> UCIHandler::handleGo(std::string& input) {
        std::istringstream ss(input);
        std::string        token;

        search::SearchInfo info;

        ss >> token;

        while (ss >> token)
        {
            if (token == "infinite")
            {
                info.infinite = true;
            }
            else if (token == "wtime")
            {
                int value;
                ss >> value;
                info.wtime = value;
            }
            else if (token == "btime")
            {
                int value;
                ss >> value;
                info.btime = value;
            }
            else if (token == "winc")
            {
                int value;
                ss >> value;
                info.winc = value;
            }
            else if (token == "binc")
            {
                int value;
                ss >> value;
                info.binc = value;
            }
            else if (token == "movestogo")
            {
                int value;
                ss >> value;
                info.movestogo = value;
            }
            else if (token == "movetime")
            {
                int value;
                ss >> value;
                info.movetime = value;
            }
            else if (token == "depth")
            {
                int value;
                ss >> value;
                info.depth = value;
            }
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
            ss << " nodes " << (size_t) result.nodes;
            ss << " time " << (unsigned long long) result.time;
            ss << " hashfull " << (unsigned int) result.hashfull;
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

        std::future<void> f =
          std::async(std::launch::async, [this, info, searchProgressReportHandler,
                                          searchCompleteReportHander] {
              (void) engine.search(info, searchProgressReportHandler, searchCompleteReportHander);
          });

        return f;
    }

    void UCIHandler::handleDisplay() { engine.display(); }

#ifdef EXTERNAL_TUNE
    void UCIHandler::handleDisplayParams() {
        std::ostringstream ss;
        for (auto& param : params::params())
        {
            ss << param.name;
            ss << ", int";
            ss << ", " << (int) param.value;
            ss << ", " << (int) param.min;
            ss << ", " << (int) param.max;
            ss << ", " << (int) param.step;
            ss << ", 0.002\n";
        }
        std::cout << ss.str() << std::endl;
    }
#endif

    void UCIHandler::start() {
        std::string       input;
        std::future<void> uci_go_future;

#ifdef EXTERNAL_TUNE
        handleDisplayParams();
#endif

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
                handleSetOption(input);
            }
            else if (input.rfind("position", 0) == 0)
            {
                engine.resetForSearch();
                handlePosition(input);
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
