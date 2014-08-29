#ifndef _CPERFTSUITE_H_
#define _CPERFTSUITE_H_

#include <ostream>
#include <fstream>
#include "CBoard.h"

class CPerftSuite
{
    public:
        friend std::ostream& operator <<(std::ostream &os, const CPerftSuite &rhs);

        CPerftSuite(const char *fileName) : m_passCount(0), m_failCount(0),
            m_testSuiteFile(), m_board()  {open(fileName);}
        ~CPerftSuite() {close();}

        bool DoTest();

        std::string ToString() const;

    private:
        int           m_passCount;
        int           m_failCount;
        std::ifstream m_testSuiteFile;
        CBoard        m_board;

        void  open(const char *fileName);
        void  close();

}; // end of class CPerftSuite

#endif // _CPERFTSUITE_H_

