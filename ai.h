#ifndef _AI_H_
#define _AI_H_

#include "CBoard.h"
#include "CMoveList.h"
#include "CHashTable.h"
#include "CTime.h"

class AI
{
public:
    AI(CBoard& board) : m_board(board), m_nodes(), m_hashTable(), m_hashEntry(),
        m_moveList(), m_timeEnd(), m_pvSearch(), m_killerMove()
        {m_moveList.clear();}

    CMove find_best_move(int wTime = 0, int bTime = 0, int movesToGo = 0);

private:
    int search(int alpha, int beta, int level, CMoveList& pv);
    int quiescence(int alpha, int beta, CMoveList& pv);

    CBoard&         m_board;
    unsigned long   m_nodes;
    CHashTable      m_hashTable;
    CHashEntry      m_hashEntry;
    CMoveList       m_moveList;
    CTime           m_timeEnd;
    bool            m_pvSearch;
    CMove           m_killerMove;
}; // end of class AI

#endif // _AI_H_

