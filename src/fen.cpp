#include "fen.h"
#include "movegen.h"
#include "types.h"
#include "utils.h"

namespace sagittar {

    namespace fen {

        void parseFEN(board::Board* board, std::string fen, const bool full) {
            std::string        segment;
            std::istringstream ss(fen);

            // Reset board
            board->reset();

            // Parse Piece placement data
            u8 rank = Rank::RANK_8;
            u8 file = File::FILE_A;
            ss >> segment;
            for (const char& ch : segment)
            {
                if (ch == '/')
                {
                    rank--;
                    file = File::FILE_A;
                }
                else if (isdigit(ch))
                {
                    const u8 empty_squares = ch - '0';
                    if (empty_squares < 1 || empty_squares > 8) [[unlikely]]
                    {
                        throw std::invalid_argument("Invalid FEN!");
                    }
                    file += empty_squares;
                }
                else if (isalpha(ch))
                {
                    std::size_t piece_id = PIECES_STR.find(ch);
                    if (piece_id == std::string::npos) [[unlikely]]
                    {
                        throw std::invalid_argument("Invalid FEN!");
                    }
                    board->setPiece(static_cast<Piece>(piece_id), rf2sq(rank, file++));
                }
            }

            // Parse Active color
            ss >> segment;
            if (segment == "w")
            {
                board->setActiveColor(Color::WHITE);
            }
            else if (segment == "b")
            {
                board->setActiveColor(Color::BLACK);
            }
            else [[unlikely]]
            {
                throw std::invalid_argument("Invalid FEN!");
            }

            // Parse Castling availability
            ss >> segment;
            if (segment == "-")
            {
                board->addCastelingRights(board::CastleFlag::NOCA);
            }
            else
            {
                if (segment.find('K') != std::string::npos)
                {
                    board->addCastelingRights(board::CastleFlag::WKCA);
                }
                if (segment.find('Q') != std::string::npos)
                {
                    board->addCastelingRights(board::CastleFlag::WQCA);
                }
                if (segment.find('k') != std::string::npos)
                {
                    board->addCastelingRights(board::CastleFlag::BKCA);
                }
                if (segment.find('q') != std::string::npos)
                {
                    board->addCastelingRights(board::CastleFlag::BQCA);
                }
            }

            // Parse En passant target square
            ss >> segment;
            if (segment == "-")
            {
                board->setEnpassantTarget(Square::NO_SQ);
            }
            else
            {
                const Rank rank = static_cast<Rank>((segment[1] - '0') - 1);
                if (board->getActiveColor() == Color::WHITE)
                {
                    if (rank != Rank::RANK_6) [[unlikely]]
                    {
                        throw std::invalid_argument("Invalid FEN!");
                    }
                }
                else
                {
                    if (rank != Rank::RANK_3) [[unlikely]]
                    {
                        throw std::invalid_argument("Invalid FEN!");
                    }
                }
                const File   file         = static_cast<File>(segment[0] - 'a');
                const Square ep_target_sq = rf2sq(rank, file);
                board->setEnpassantTarget(ep_target_sq);
            }

            // Parse Halfmove clock
            ss >> segment;
            if (!full || segment.empty() || segment == "-")
            {
                board->setHalfmoveClock(0);
            }
            else
            {
                board->setHalfmoveClock(std::stoi(segment));
            }

            // Parse Fullmove number
            ss >> segment;
            if (!full || segment.empty() || segment == "-")
            {
                board->setFullmoveNumber(1);
            }
            else
            {
                board->setFullmoveNumber(std::stoi(segment));
            }

            // Set checkers
            const Piece     king = pieceCreate(PieceType::KING, board->getActiveColor());
            board::BitBoard bb   = board->getBitboard(king);
            const Square    sq   = static_cast<Square>(utils::bitScanForward(&bb));
            const Color     them = colorFlip(board->getActiveColor());
            board->setCheckers(movegen::getSquareAttackers(*board, sq, them));

            // Reset Hash
            board->resetHash();
        }

        std::string toFEN(const board::Board& board) {
            std::ostringstream oss;

            // Piece placement data
            for (i8 rank = Rank::RANK_8; rank >= Rank::RANK_1; rank--)
            {
                u8 empty = 0;
                for (u8 file = File::FILE_A; file <= File::FILE_H; file++)
                {
                    const Square sq    = rf2sq(rank, file);
                    const Piece  piece = board.getPiece(sq);
                    if (piece != Piece::NO_PIECE)
                    {
                        if (empty > 0)
                        {
                            oss << (int) empty;
                            empty = 0;
                        }
                        oss << PIECES_STR[piece];
                    }
                    else
                    {
                        empty++;
                    }
                }
                if (empty > 0)
                {
                    oss << (int) empty;
                }
                if (rank != Rank::RANK_1)
                {
                    oss << "/";
                }
            }
            oss << " ";

            // Active color
            if (board.getActiveColor() == Color::WHITE)
            {
                oss << "w";
            }
            else
            {
                oss << "b";
            }
            oss << " ";

            // Castling availability
            if (board.getCastelingRights() == board::CastleFlag::NOCA)
            {
                oss << "-";
            }
            else
            {
                if (board.getCastelingRights() & board::CastleFlag::WKCA)
                {
                    oss << "K";
                }
                if (board.getCastelingRights() & board::CastleFlag::WQCA)
                {
                    oss << "Q";
                }
                if (board.getCastelingRights() & board::CastleFlag::BKCA)
                {
                    oss << "k";
                }
                if (board.getCastelingRights() & board::CastleFlag::BQCA)
                {
                    oss << "q";
                }
            }
            oss << " ";

            // En passant target
            if (board.getEnpassantTarget() == Square::NO_SQ)
            {
                oss << "-";
            }
            else
            {
                const Square ep_target = board.getEnpassantTarget();
                const File   file      = sq2file(ep_target);
                const Rank   rank      = sq2rank(ep_target);
                oss << FILE_STR[file] << (int) rank + 1;
            }
            oss << " ";

            // Halfmove clock
            oss << (int) board.getHalfmoveClock() << " ";

            // Fullmove number
            oss << (int) board.getFullmoveNumber() << " ";

            return oss.str();
        }

    }

}
