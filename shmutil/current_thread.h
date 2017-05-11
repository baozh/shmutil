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
#include <time.h>

#include <stdio.h>

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

    inline int tid()
    {
      if (__builtin_expect(t_cachedTid == 0, 0))
      {
        cacheTid();
      }
      return t_cachedTid;
    }

    inline const char* tidString() // for logging
    {
      return t_tidString;
    }

    inline int tidStringLength() // for logging
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
}

#endif
