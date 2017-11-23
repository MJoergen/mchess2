#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "CBoard.h"
#include "CPerft.h"
#include "CPerftSuite.h"
#include "CSearchSuite.h"
#include "ai.h"
#include "trace.h"

#ifdef ENABLE_TRACE
std::ostream *gpTrace = 0;
#endif

/***************************************************************
 * main
 *
 * This is where it all starts!
 ***************************************************************/
int main(int argc, char **argv)
{
    bool uciMode = false;
    srand(time(0)); // Seed the random number generator

    CBoard board;
    AI ai(board);

    int c;

    while ((c = getopt(argc, argv, "t:f:s:p:h")) != -1)
    {
        switch (c)
        {
#ifdef ENABLE_TRACE
            case 't' : gpTrace = new CTrace(optarg); break;
#else
            case 't' : std::cout << "Trace not supported" << std::endl; return 1;
#endif
            case 's' : {
                           CSearchSuite searchSuite(optarg);
                           if (!searchSuite.DoTest())
                           {
                               std::cout << searchSuite << std::endl;
                               exit(0);
                           }
                       }
                       break;

            case 'p' : {
                           CPerftSuite perftSuite(optarg);
                           if (!perftSuite.DoTest())
                           {
                               std::cout << perftSuite << std::endl;
                               exit(0);
                           }
                       }
                       break;

            case 'f' : {
                           std::ifstream fenFile;
                           fenFile.open(optarg);
                           if (!fenFile.is_open())
                           {
                               std::cout << "Could not open file: " << optarg << std::endl;
                           }
                           std::string fen;
                           getline(fenFile, fen);
                           if (board.read_from_fen(fen.c_str()))
                           {
                               std::cout << "Error reading from FEN" << std::endl;
                           }
                       }
                       break;

            case 'h' :
            default : {
                          std::cout << "Options:" << std::endl;
                          std::cout << "-t <file> : Trace search to file" << std::endl;
                          std::cout << "-s <file> : Run search on test suite" << std::endl;
                          std::cout << "-p <file> : Run performance test on test suite" << std::endl;
                          std::cout << "-f <file> : Read initial position from FEN file" << std::endl;
                          std::cout << "-h        : Show this message" << std::endl;
                          exit(1);
                      }
        }
    }

    TRACE("Starting trace" << std::endl);

    while (true) // Repeat forever
    {
        if (!uciMode)
        {
            std::cout << board;
            std::cout << "Input command : ";
        }
        std::string str;
        getline(std::cin, str);
        TRACE(str << std::endl);
        if (!uciMode)
        {
            std::cout << std::endl;
        }
        if (std::cin.eof() || str == "quit")
        {
            exit(1);
        }
        if (str == "uci")
        {
#define DEF_STR(x) #x
#define DEF_XSTR(x) DEF_STR(x)
            std::cout << "id name " << DEF_XSTR(NAME) << std::endl;
            std::cout << "id author MJ" << std::endl;
            std::cout << "uciok" << std::endl;
            uciMode = true;
        }
        if (str == "isready")
        {
            std::cout << "readyok" << std::endl;
        }
        if (str == "ucinewgame")
        {
            board.newGame();
        }
        if (str.compare(0, 9, "position ") == 0)
        {
            if (str.compare(9, 9, "startpos ") == 0)
            {
                board.newGame();
            } else if (str.compare(9, 4, "fen ") == 0)
            {
                if (board.read_from_fen(str.substr(13).c_str()))
                {
                    std::cout << "Error reading from FEN" << std::endl;
                }
            }

            size_t pos = str.find("moves ");
            if (pos != std::string::npos)
            {
                pos += 6;
                CMove move;
                const char *p = str.c_str()+pos;
                std::cout << p << std::endl;
                while ((p = move.FromString(p)) != NULL)
                {
                    if (!board.IsMoveValid(move))
                    {
                        std::cout << "Invalid move " << move << std::endl;
                        std::cout << board << std::endl;
                        std::cout << p << std::endl;
                        break;
                    }
                    board.make_move(move);
                }
            }
        }

        if (str.compare(0, 6, "perft ") == 0)
        {
            CPerft perft(board);

            unsigned int max_depth = atoi(str.c_str()+6);
            if (max_depth < 1)
                max_depth = 1;

            for (unsigned int depth = 1; depth <= max_depth; depth++)
            {
                perft.clear();
                perft.search(depth);
                std::cout << "depth=" << depth << " " << perft << std::endl;
            }
        }

        if (str.compare(0, 5, "move ") == 0)
        {
            CMove move;

            if (move.FromString(str.c_str()+5) == NULL)
            {
                std::cout << "Try again. Use long notation, e.g. e2e4" << std::endl;
                continue; // Go back to beginning
            }

            if (board.IsMoveValid(move))
            {
                board.make_move(move);
                bool check = board.isOtherKingInCheck();
                board.undo_move(move);
                if (check)
                {
                    std::cout << "You are in CHECK. Play another move." << std::endl;
                    continue;
                }

                std::cout << "You move : " << move << std::endl;
                board.make_move(move);
            }
        } // end of "move "

        if (str.compare(0, 2, "go") == 0)
        {
            const char *p = str.c_str() + 2;
            while (*p == ' ')
                ++p;

            int wtime_ms = 300000;
            int btime_ms = 300000;
            int movesToGo = 40;

            while (*p)
            {
                if (strncmp(p, "wtime", 5) == 0)
                {
                    p += 5;
                    while (*p == ' ')
                        ++p;
                    wtime_ms = strtol(p, (char **)&p, 10);
                }
                else if (strncmp(p, "btime", 5) == 0)
                {
                    p += 5;
                    while (*p == ' ')
                        ++p;
                    btime_ms = strtol(p, (char **)&p, 10);
                }
                else if (strncmp(p, "movestogo", 9) == 0)
                {
                    p += 9;
                    while (*p == ' ')
                        ++p;
                    movesToGo = strtol(p, (char **)&p, 10);
                }
                else /* Skip rest of line */
                    break;

                while (*p == ' ')
                    ++p;
            } // end of while

            CMove best_move = ai.find_best_move(wtime_ms, btime_ms, movesToGo);

            if (!best_move.Valid())
            {
                // Oops. No legal move was found
                bool check = board.isOtherKingInCheck();
                if (check)
                {
                    std::cout << "I am checkmated. YOU WON!" << std::endl;
                }
                else
                {
                    std::cout << "I have no legal moves. It is a STALE MATE!" << std::endl;
                }
                break;
            }

            std::cout << "bestmove " << best_move << std::endl;

            board.make_move(best_move);
        } // end of "go"

        if (str == "show")
        {
            CMoveList moves;
            board.find_legal_moves(moves);
            for (unsigned int i=0; i<moves.size(); ++i)
            {
                const CMove & move = moves[i];
                board.make_move(move);
                if (!board.isOtherKingInCheck())
                {
                    std::cout << move << " ";
                }
                board.undo_move(move);
            }
            std::cout << std::endl;
        }
    } // end of while (true)

    return 0;
} // end of int main()

