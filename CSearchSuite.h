#ifndef _CSEARCHSUITE_H_
#define _CSEARCHSUITE_H_

#include <ostream>
#include <fstream>
#include "CBoard.h"
#include "ai.h"

class CSearchSuite
{
    public:
        friend std::ostream& operator <<(std::ostream &os, const CSearchSuite &rhs);

        CSearchSuite(const char *fileName) : m_passCount(0), m_failCount(0),
            m_testSuiteFile(), m_testSuiteLogFile(), m_board(), m_ai(m_board)
            {open(fileName);}
        ~CSearchSuite() {close();}

        bool DoTest();

        std::string ToString() const;

    private:
        int           m_passCount;
        int           m_failCount;
        std::ifstream m_testSuiteFile;
        std::ofstream m_testSuiteLogFile;
        CBoard        m_board;
        AI            m_ai;

        void  open(const char *fileName);
        void  close();

}; // end of class CSearchSuite

#endif // _CSEARCHSUITE_H_

