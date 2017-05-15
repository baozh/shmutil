/*
 * hashtable_writer.cc
 *
 *      Created on: 2017.5.9
 *      Author: ZengHui Bao (bao_z_h@163.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include "shm_mrsw_hashtable.h"

#define    SHM_KEY    67483

MrswHashtable gShmTable;

int main(int argc, char* argv[])
{
    ErrorCode ret;
    ret = gShmTable.init(SHM_KEY);
    if (ret != kOk)
    {
        printf("[main] shmTable init failed! shmKey:%d, errCode:%d\n", SHM_KEY, ret);
        return 0;
    }

    ret = gShmTable.clearTable();
    if (ret != kOk)
    {
        printf("[main] shmTable clearTable failed! shmKey:%d, errCode:%d\n", SHM_KEY, ret);
        return 0;
    }

    int lastSet = 0;
    char keyStr[128] = {0};
    char valStr[128] = {0};
    for (int index = 0; index < 10000; index++)
    {
        if (rand()%2 == 0)
        {
            snprintf(keyStr, 128, "key%d", index);
            snprintf(valStr, 128, "value%d", index);
            ret = gShmTable.setValue(keyStr, valStr);
            if (ret != kOk)
            {
                printf("[main] shmTable setValue failed! shmKey:%d, key:%s, value:%s, errCode:%d\n",
                       SHM_KEY, keyStr, valStr, ret);
                return 0;
            }
            lastSet = index;
        }
        else
        {
            snprintf(keyStr, 128, "key%d", lastSet);
            ret = gShmTable.deleteKey(keyStr);
            if (ret != kOk)
            {
                printf("[main] shmTable deleteKey failed! shmKey:%d, key:%s, errCode:%d\n",
                       SHM_KEY, keyStr, ret);
                return 0;
            }
        }
        sleep(10);
    }
    return 0;
}
