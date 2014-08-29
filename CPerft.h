#ifndef _CPERFT_H_
#define _CPERFT_H_

#include "CHashTable.h"
#include "CBoard.h"

class CPerft
{
    public:

        friend std::ostream& operator <<(std::ostream &os, const CPerft &rhs);

        CPerft(const CBoard &board) : m_board(board), m_nodes(), m_leafnodes(),
            m_moveList(), m_hashEntry() {clear();}

        void clear();
        void search(int level);
        unsigned long getLeafNodes() const {return m_leafnodes; }

        std::string ToString() const;

    private:
        CBoard        m_board;
        unsigned long m_nodes;
        unsigned long m_leafnodes;
        CMoveList     m_moveList;
        CHashEntry    m_hashEntry;

}; // end of class CPerft

#endif // _CPERFT_H_

