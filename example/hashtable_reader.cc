/*
 * hashtable_reader.cc
 *
 *      Created on: 2017.5.9
 *      Author: ZengHui Bao (bao_z_h@163.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "shm_mrsw_hashtable.h"

#define    SHM_KEY    67483

MrswHashtable gShmTable;

void printUsage()
{
    printf("reader usage;\n");
    printf("\t./reader get key (get the value of key)\n");
    printf("\t./reader ex key (get the existence of key)\n");
    printf("\t./reader pr (print all key/value in the hashtable)\n");
    printf("\t./reader st (get statistic info of hashtable)\n");
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printUsage();
    }

    ErrorCode ret;
    ret = gShmTable.init(SHM_KEY);
    if (ret != kOk)
    {
        printf("shmTable init failed! shmKey:%d, errCode:%d\n", SHM_KEY, ret);
        return 0;
    }

    if (strcmp(argv[1], "get") == 0)
    {
        if (argc < 3)
        {
            printf("Can't recognize input command!\n");
            printUsage();
            return 0;
        }

        std::string val;
        ret = gShmTable.getValue(argv[2], val);
        if (ret != kOk)
        {
            printf("shmTable getValue failed! shmKey:%d, key:%s, errCode:%d\n", SHM_KEY, argv[2], ret);
        }
        else
        {
            printf("getValue: key:%s, value:%s\n", argv[2], val.c_str());
        }
    }
    else if (strcmp(argv[1], "ex") == 0)
    {
        if (argc < 3)
        {
            printf("Can't recognize input command!\n");
            printUsage();
            return 0;
        }

        bool isExist = false;
        ret = gShmTable.isExist(argv[2], isExist);
        if (ret != kOk)
        {
            printf("shmTable isExist failed! shmKey:%d, key:%s, errCode:%d\n", SHM_KEY, argv[2], ret);
        }
        else
        {
            printf("isExist: key:%s, isExist:%d\n", argv[2], isExist);
        }
    }
    else if (strcmp(argv[1], "pr") == 0)
    {
        std::string key, val;
        int index;
        for (;;)
        {
            ret = gShmTable.getNext(key, val, index);
            if (ret != kOk && ret != kErrTraverseTableEnd)
            {
                printf("shmTable getNext failed! shmKey:%d, index:%d\n", SHM_KEY, index);
                break;
            }
            else if (ret != kErrTraverseTableEnd)
            {
                printf("getNext: key:%s, value:%s\n", key.c_str(), val.c_str());
            }
            else if (ret == kErrTraverseTableEnd)
            {
                break;
            }
        }
    }
    else if (strcmp(argv[1], "st") == 0)
    {
        int maxSlots, usedSlots, usedKeys;
        ret = gShmTable.getStats(maxSlots, usedSlots, usedKeys);
        if (ret != kOk)
        {
            printf("shmTable getStats failed! shmKey:%d\n", SHM_KEY);
        }
        else
        {
            printf("getStats: maxSlots:%d, usedSlots:%d, usedKeys:%d\n", maxSlots, usedSlots, usedKeys);
        }
    }
    else
    {
        printf("Can't recognize input command!\n");
        printUsage();
        return 0;
    }
}
