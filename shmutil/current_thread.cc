//
// Created by bzh on 17-5-15.
//

#include "current_thread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;

    const char* tidString() // for logging
    {
        return t_tidString;
    }

    int tidStringLength() // for logging
    {
        return t_tidStringLength;
    }
};
