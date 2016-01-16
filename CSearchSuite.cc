#include <sstream>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "CSearchSuite.h"

/***************************************************************
 * open
 ***************************************************************/
void CSearchSuite::open(const char *fileName)
{
    m_testSuiteFile.open(fileName);

    std::string logFileName(fileName);
    logFileName += ".log";
    m_testSuiteLogFile.open(logFileName.c_str(), std::ofstream::trunc);
} // end of open
   

/***************************************************************
 * close
 ***************************************************************/
void CSearchSuite::close()
{
    m_testSuiteFile.close();
    m_testSuiteLogFile.close();
} // end of close


/***************************************************************
 * operator <<
 ***************************************************************/
std::ostream& operator <<(std::ostream &os, const CSearchSuite &rhs)
{
    return os << rhs.ToString();
}


/***************************************************************
 * ToString
 ***************************************************************/
std::string CSearchSuite::ToString() const
{
    std::stringstream ss;
    ss << m_passCount << " test cases passed!" << std::endl;
    ss << m_failCount << " test cases failed!" << std::endl;
    return ss.str();
} // end of ToString


/***************************************************************
 * DoTest
 * Returns true on any errors.
 ***************************************************************/
bool CSearchSuite::DoTest()
{
    if (!m_testSuiteFile.is_open())
    {
        std::cerr << "Could not open file. " << std::endl;
        return true;
    }

    while (true)
    {
        std::string line;
        getline(m_testSuiteFile, line);
        if (m_testSuiteFile.eof())
        {
            return false;
        }

        if (line[0] == '#')
            continue; // Skip comments

        const char *p;
        if (m_board.read_from_fen(line.c_str(), &p))
        {
            std::cerr << "Error reading from FEN" << std::endl;
            return false;
        }

        // Read command
        while (*p == ' ')
            ++p;

        std::string id;
        CMoveList expBestMoves;
        CMoveList avoidMoves;

        std::cout << m_board;

        // Parse input line
        while (*p)
        {
            if (p[0] == 'i' and p[1] == 'd') {
                p += 2;
                while (*p == ' ')
                    ++p;
                if (*p == '"')
                {
                    ++p;
                    const char *p2 = p;
                    while (*p2 != '"')
                        p2++;
                    id = std::string(p, p2);
                    p=p2+1;
                }
            }
            else if (p[0] == 'b' and p[1] == 'm') {
                p += 2;
                while (*p == ' ')
                {
                    ++p;
                    CMove move = m_board.readMove(p, &p);
                    if (!move.Valid())
                    {
                        std::cout << "Invalid move" << std::endl;
                        exit(-1);
                    }
                    expBestMoves.push_back(move);
                }

            }
            else if (p[0] == 'a' and p[1] == 'm') {
                p += 2;
                while (*p == ' ')
                {
                    ++p;
                    CMove move = m_board.readMove(p, &p);
                    if (!move.Valid())
                    {
                        std::cout << "Invalid move" << std::endl;
                        exit(-1);
                    }
                    avoidMoves.push_back(move);
                }
            }
            else if (p[0] == 'c') {
                p++;
                while (*p != ';')
                {
                    ++p;
                }
            }

            if (*p == ';')
            {
                ++p;
                while (*p == ' ')
                    ++p;
            }
        } // while (*p)

        CMove best_move = m_ai.find_best_move(60*1000, 60*1000, 1); // Search for one minute

        m_testSuiteLogFile << id << " : " << best_move << " : "; 

        if (avoidMoves.is_in(best_move))
        {
            m_testSuiteLogFile << "Wrong! " << "Should be avoided" << std::endl;
            m_failCount++;
        }
        else if (expBestMoves.is_in(best_move))
        {
            m_testSuiteLogFile << "Correct!" << std::endl;
            m_passCount++;
        }
        else
        {
            m_testSuiteLogFile << "Wrong! Expected " << expBestMoves << std::endl;
            m_failCount++;
        }

    } // end of while

} // end of DoTest 

