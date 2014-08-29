#include <sstream>
#include "CMoveList.h"

std::ostream& operator <<(std::ostream &os, const CMoveList &rhs)
{
    return os << rhs.ToShortString();
} // end of std::ostream& operator <<(std::ostream &os, const CMoveList &rhs)

std::string CMoveList::ToShortString() const
{
    std::stringstream ss;
    for (std::vector<CMove>::const_iterator it = m_moveList.begin(); it != m_moveList.end(); ++it)
    {
        ss << (*it).ToShortString() << " ";
    }

    return ss.str();
}

std::string CMoveList::ToLongString() const
{
    std::stringstream ss;
    for (std::vector<CMove>::const_iterator it = m_moveList.begin(); it != m_moveList.end(); ++it)
    {
        ss << (*it).ToLongString() << " ";
    }

    return ss.str();

}

