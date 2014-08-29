#ifndef _TRACE_H_
#define _TRACE_H_

#include <stdio.h>
#include <streambuf>
#include <ostream>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

/***************************************************************
 * This file defines the following three macros:
 * #define TRACE(x) : Appends the argument to the trace file.
 * #define TRACE_FUNCTION(x) : Automatically traces entrance to and exit from
 *                             the calling function.
 * #define DEBUG(x) : Execute the argument, if enabled.
 ***************************************************************/


/***************************************************************
 * CAppendFile
 *
 * Helper class. It essentially encapsulates a file descriptor.
 ***************************************************************/
class CAppendFile
{
    public:
        CAppendFile(const char *fileName)
            : fd()
        {
            fd = open(fileName, O_CREAT | O_WRONLY | O_APPEND, 0644);
        }
        ~CAppendFile()
        {
            if (fd != -1)
                close(fd);
        }
        int Write(char *buffer, int num)
        {
            return write(fd, buffer, num);
        }
    private:
        int fd;
};


/***************************************************************
 * CTraceBuf
 *
 * Helper class. It essentially encapsulates a stream buffer
 ***************************************************************/
class CTraceBuf : public std::streambuf
{
    private:
        CTraceBuf(const CTraceBuf&);
        const CTraceBuf& operator =(const CTraceBuf&);
    protected:
        static const int bufferSize = 200;
        char buffer[bufferSize];

    public:
        CTraceBuf(const char *fileName) : fileName(fileName)
        {
            setp(buffer, buffer + (bufferSize-1));
        }

        virtual ~CTraceBuf()
        {
            sync();
        }

    protected:
        int flushBuffer()
        {
            int num = pptr() - pbase();
            {
                CAppendFile fd(fileName);

                if (fd.Write(buffer, num) != num)
                {
                    return EOF;
                }
            }

            pbump (-num);
            return num;
        }

        virtual int overflow(int c)
        {
            if (c != EOF)
            {
                *pptr() = c;
                pbump(1);
            }

            if (flushBuffer() == EOF)
            {
                return EOF;
            }

            return c;
        }

        virtual int sync()
        {
            if (flushBuffer() == EOF)
            {
                return -1;
            }
            return 0;
        }
    private:
        const char *fileName;
}; /* end of class CTraceBuf */


/***************************************************************
 * CTrace
 *
 * Helper class. It essentially encapsulates an output stream.
 ***************************************************************/
class CTrace : public std::ostream
{
    protected:
        CTraceBuf buf;
    public:
        CTrace(const char *fileName) :
            std::ostream(&buf) ,
            buf(fileName)
    {
    }
}; /* end of class CTrace */

#ifdef ENABLE_TRACE

extern std::ostream *gpTrace;


/***************************************************************
 * CTraceFunction
 *
 * Helper class. Automatically traces entrance to and exit from the calling
 * function.
 ***************************************************************/
class CTraceFunction
{
    private:
        CTraceFunction(const CTraceFunction&);
        const CTraceFunction& operator =(const CTraceFunction&);
    public:
        CTraceFunction(const char *functionName) :
            functionName(functionName)
        {
            if (gpTrace) {*gpTrace << "+" << functionName << std::endl;}
        }

        ~CTraceFunction()
        {
            if (gpTrace) {*gpTrace << "-" << functionName << std::endl;}
        }

    private:
        const char *functionName;
}; /* end of class CTraceFunction */

// These are the three macros supported
#define TRACE(x) if (gpTrace) {*gpTrace << x;}
#define TRACE_FUNCTION(x) CTraceFunction temp(x);
#define DEBUG(x) x

#else // ENABLE_TRACE

#define TRACE(x)
#define TRACE_FUNCTION(x)
#define DEBUG(x)

#endif // ENABLE_TRACE

#endif // _TRACE_H_

