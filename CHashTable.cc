#include "CHashTable.h"

#define TABLE_SIZE (8*1024*1024)

/***************************************************************
 * constructor
 ***************************************************************/
CHashTable::CHashTable()
    : m_table()
{
    m_table.reserve(TABLE_SIZE);
}


/***************************************************************
 * insert
 ***************************************************************/
void CHashTable::insert(const CHashEntry& hashEntry)
{
    uint32_t ix = hashEntry.m_hashValue & (TABLE_SIZE - 1);
    m_table[ix] = hashEntry; // Overwrite any existing value
} // end of insert


/***************************************************************
 * find
 ***************************************************************/
bool CHashTable::find(uint64_t hashValue, CHashEntry& hashEntry) const
{
    uint32_t ix = hashValue & (TABLE_SIZE - 1);
    if (m_table[ix].m_hashValue == hashValue)
    {
        hashEntry = m_table[ix];
        return true;
    }
    return false;
} // end of find

