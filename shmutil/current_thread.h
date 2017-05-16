// port from muduo/base
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_CURRENTTHREAD_H
#define MUDUO_BASE_CURRENTTHREAD_H

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

namespace CurrentThread
{
    extern __thread int t_cachedTid;
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;

    inline pid_t gettid()
    {
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }

    inline void cacheTid()
    {
        if (t_cachedTid == 0)
        {
            t_cachedTid = gettid();
            t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
        }
    }

    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }

    const char* tidString(); // for logging
    int tidStringLength(); // for logging

    inline bool isMainThread()
    {
        return tid() == ::getpid();
    }
};

#endif
