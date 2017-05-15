//
// Created by bzh on 17-5-15.
//

#include "current_thread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;

    pid_t gettid()
    {
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }

    void cacheTid()
    {
        if (t_cachedTid == 0)
        {
            t_cachedTid = gettid();
            t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
        }
    }

    int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }

    const char* tidString() // for logging
    {
        return t_tidString;
    }

    int tidStringLength() // for logging
    {
        return t_tidStringLength;
    }

    bool isMainThread()
    {
        return tid() == ::getpid();
    }

    void sleepUsec(int64_t usec)
    {
        struct timespec ts = {0, 0};
        ts.tv_sec = static_cast<time_t>(usec / 1000000);
        ts.tv_nsec = static_cast<long>(usec % 1000000 * 1000);
        ::nanosleep(&ts, NULL);
    }
};