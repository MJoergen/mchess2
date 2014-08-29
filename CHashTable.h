#ifndef _CHASHTABLE_H_
#define _CHASHTABLE_H_

#include <vector>

#include "CBoard.h"
#include "CHashEntry.h"

/***************************************************************
 * declaration of CHashTable
 *
 * This is a wrapper for the std::vector class.
 * It contains an array of hash values.
 ***************************************************************/
class CHashTable
{
    public:
        CHashTable();
        void insert(const CHashEntry& hashEntry);
        bool find(uint64_t hashValue, CHashEntry& hashEntry) const;
        
    private:
        std::vector<CHashEntry> m_table;
}; // end of CHashTable

#endif // _CHASHTABLE_H_

