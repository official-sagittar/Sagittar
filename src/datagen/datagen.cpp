#include "datagen.h"
#include "move.h"
#include "search.h"

namespace sagittar {

    namespace datagen {

        DataGenerator::DataGenerator(Engine& engine) :
            engine(engine),
            N(0),
            S(0ULL),
            book("None"),
            generated(0) {}

        void DataGenerator::selfplay() {
            search::SearchInfo info;
            info.depth   = 5;
            info.timeset = false;
            search::SearchResult searchResult;
            do
            {
                std::cout << "info string genfens " << engine.getPositionAsFEN() << std::endl;
                searchResult = engine.search(info);
                if (searchResult.bestmove == move::Move())
                {
                    // Game has terminated - checkmate or stalemate
                    break;
                }
                const board::DoMoveResult doMoveResult = engine.doMove(searchResult.bestmove);
                if (doMoveResult != board::DoMoveResult::LEGAL) [[unlikely]]
                {
                    break;
                }
                engine.resetForSearch();
            } while (++generated < N);
        }

        void DataGenerator::start() {
            if (book != "None")
            {
                std::ifstream file(book);
                if (!file)
                {
                    throw std::runtime_error("Cannot open file: " + book);
                }
                const u32 linecount = std::count(std::istreambuf_iterator<char>(file),
                                                 std::istreambuf_iterator<char>(), '\n');
                std::cout << "info string found " << (int) linecount << " book lines" << std::endl;
                std::mt19937_64 rng(S);
                while (generated < N)
                {
                    const u32 random_line = rng() % linecount;
                    file.clear();
                    file.seekg(0, std::ios::beg);
                    std::string line;
                    for (u32 i = 0; i < random_line; ++i)
                    {
                        if (!std::getline(file, line))
                        {
                            throw std::out_of_range("Line number exceeds file length.");
                        }
                    }
                    engine.reset();
                    engine.setPositionFromFEN(line);
                    selfplay();
                }
            }
            else
            {
                while (generated < N)
                {
                    engine.reset();
                    engine.setStartpos();
                    selfplay();
                }
            }
        }

        void DataGenerator::genfens(std::string& input) {
            std::istringstream iss(input);
            std::string        token;

            iss >> token;
            if (token == "genfens")
            {
                iss >> token;
                N = std::stoi(token);
            }
            else
            {
                // Error
            }

            iss >> token;
            if (token == "seed")
            {
                iss >> token;
                S = std::stoull(token);
            }
            else
            {
                // Error
            }

            iss >> token;
            if (token == "book")
            {
                iss >> book;
            }
            else
            {
                // Error
            }

            start();
        }

    }
}
