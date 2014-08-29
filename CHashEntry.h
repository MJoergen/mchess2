#ifndef _CHASHENTRY_H_
#define _CHASHENTRY_H_

#include "CMove.h"
#include "CBoard.h"

// For performance reasons, this bit structure must fit into 1 byte.
typedef struct
{
    uint8_t nodeType : 2;
    uint8_t level    : 6;
} t_nodeTypeAndLevel;

typedef enum
{
    nodeExact = 0,
    nodeUpper,
    nodeLower
} e_nodeType;


// For performance reasons, this structure must fit into 16 bytes.
class CHashEntry
{
    public:
        friend class CHashTable;
        friend class AI;
        friend std::ostream& operator <<(std::ostream &os, const CHashEntry &rhs);

        CHashEntry() : m_hashValue(), m_bestMove(), m_nodeTypeAndLevel(), m_searchValue()
            {}
        void set(const CBoard& board);
        void update(const CBoard& board, const CMove& move);
        std::string ToString() const;

        operator uint64_t() const {return m_hashValue; }

    private:
        uint64_t           m_hashValue;        // 8 bytes
        CMove              m_bestMove;         // 5 bytes
        t_nodeTypeAndLevel m_nodeTypeAndLevel; // 1 byte
        int16_t            m_searchValue;      // 2 bytes

}; // end of CHashEntry

#endif // _CHASHENTRY_H_

