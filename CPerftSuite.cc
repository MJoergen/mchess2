#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "CPerftSuite.h"
#include "CPerft.h"

/***************************************************************
 * open
 ***************************************************************/
void CPerftSuite::open(const char *fileName)
{
    m_testSuiteFile.open(fileName);
} // end of open
   

/***************************************************************
 * close
 ***************************************************************/
void CPerftSuite::close()
{
    m_testSuiteFile.close();
} // end of close


/***************************************************************
 * operator <<
 ***************************************************************/
std::ostream& operator <<(std::ostream &os, const CPerftSuite &rhs)
{
    return os << rhs.ToString();
}


/***************************************************************
 * ToString
 ***************************************************************/
std::string CPerftSuite::ToString() const
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
bool CPerftSuite::DoTest()
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

        const char *p;
        if (m_board.read_from_fen(line.c_str(), &p))
        {
            std::cerr << "Error reading from FEN" << std::endl;
            return true;
        }

        while (true)
        {
            p = strchr(p, ';');
            if (!p)
            {
                m_passCount++;
                break;
            }
            if (*(++p) == 'D')
            {
                CPerft perft(m_board);

                char *np;
                unsigned int depth = strtol(++p, &np, 10);
                p=np;
                unsigned long expLeafNodes = strtol(p, &np, 10);
                std::cout << "depth " << depth << " leafnodes " << expLeafNodes << std::endl;
                if (depth >= 1 && expLeafNodes < 10000000)
                {
                    perft.clear();
                    perft.search(depth);
                    if (perft.getLeafNodes() != expLeafNodes)
                    {
                        std::cout << "FAILED!" << std::endl;
                        std::cout << m_board << std::endl;
                        std::cout << perft << std::endl;
                        std::cout << std::endl;
                        m_failCount++;
                        break;
                    }
                }

                p=np;
            }
        } // end of while true (reading a line)

    } // end of while true (reading from file)

} // end of DoTest 

