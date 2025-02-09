#include "datagen.h"
#include "engine.h"

namespace sagittar {

    namespace datagen {

        void genfens(std::string& input) {
            std::istringstream iss(input);
            std::string        token;

            u32         N    = 0;
            u64         S    = 0ULL;
            std::string book = "None";

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

            if (book != "None")
            {
                std::ifstream file(book);
                if (!file)
                {
                    throw std::runtime_error("Cannot open file: " + book);
                }
                const u32       linecount = std::count(std::istreambuf_iterator<char>(file),
                                                       std::istreambuf_iterator<char>(), '\n');
                std::mt19937_64 rng(S);
                std::uniform_int_distribution<u32> dist(0, linecount);
                const u32                          random_line = dist(rng);
#ifdef DEBUG
                assert(random_line >= 0 && random_line <= linecount);
#endif
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
                std::cout << N << "\t" << line << std::endl;
            }
        }

    }

}
