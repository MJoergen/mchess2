#include <iomanip>
#include <sstream>
#include <assert.h>

#include "CHashTable.h"
#include "CPerft.h"
#include "trace.h"

/***************************************************************
 * clear
 ***************************************************************/
void CPerft::clear()
{
    m_nodes = 0;
    m_leafnodes = 0;
    m_hashEntry.set(m_board);
} // end of clear


/***************************************************************
 * search
 ***************************************************************/
void CPerft::search(int level)
{
    TRACE(m_moveList << std::endl);
    m_nodes++;

    if (level == 0)
    {
        m_leafnodes++;
        return;
    }

    CMoveList moves;
    m_board.find_legal_moves(moves);

    for (unsigned int i=0; i<moves.size(); ++i)
    {
        CMove move = moves[i];
#ifdef DEBUG_HASH
        uint32_t oldHash = m_board.calcHash();
        CHashEntry hashCopy(m_hashEntry);
#endif
        m_hashEntry.update(m_board, move);
        m_board.make_move(move);
        m_moveList.push_back(move);
#ifdef DEBUG_HASH
        {
            CHashEntry m_newHash;
            m_newHash.set(m_board);
            if (m_newHash != m_hashEntry)
            {
                TRACE(m_moveList.ToLongString() << std::endl);
                TRACE("New hash failure(2):" << std::hex << (m_newHash ^ m_hashEntry) << std::endl);
                TRACE(m_board);
                assert(false);
            }
        }
#endif
        if (!m_board.isOtherKingInCheck())
        {
            search(level-1);
        }
        m_board.undo_move(move);
        m_hashEntry.update(m_board, move);
        m_moveList.pop_back();
#ifdef DEBUG_HASH
        uint32_t newHash = m_board.calcHash();
        if (oldHash != newHash)
        {
            TRACE("Hash failure" << std::endl);
            TRACE(m_board);
            assert(false);
        }
        if (hashCopy != m_hashEntry)
        {
            TRACE("New hash failure" << std::endl);
            TRACE(m_board);
            assert(false);
        }
#endif
    }
} // end of search


/***************************************************************
 * ToString
 ***************************************************************/
std::string CPerft::ToString() const
{
    std::stringstream ss;
    ss << "nodes = "      << std::setw(8) << m_nodes;
    ss << " leafnodes = " << std::setw(8) << m_leafnodes;
    return ss.str();
} // end of ToString


/***************************************************************
 * operator << 
 ***************************************************************/
std::ostream& operator <<(std::ostream &os, const CPerft &rhs)
{
    return os << rhs.ToString();
} // end of std::ostream& operator <<(std::ostream &os, const CPerft &rhs)

