#ifndef _CTIME_H_
#define _CTIME_H_

#include <time.h>

class CTime
{
    public:
        friend class CTimeDiff;

        CTime() : m_time(clock())
        {
        }

        CTime& operator += (int timeMs)
        {
            m_time += timeMs * (CLOCKS_PER_SEC / 1000);
            return *this;
        }

        bool operator < (const CTime& rhs)
        {
            return m_time < rhs.m_time;
        }


    private:
        clock_t m_time;
}; // end of class CTime

class CTimeDiff
{
    public:
        CTimeDiff(const CTime& start) :
            m_time(clock() - start.m_time)
            {}

        unsigned int millisecs() const {return m_time / (CLOCKS_PER_SEC / 1000);}

    private:
        clock_t m_time;
}; // end of class CTimeDiff

#endif // _CTIME_H_


