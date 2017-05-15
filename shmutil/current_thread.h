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
    pid_t gettid();
    void cacheTid();
    int tid();
    const char* tidString(); // for logging
    int tidStringLength(); // for logging
    bool isMainThread();
    void sleepUsec(int64_t usec);
};

#endif
