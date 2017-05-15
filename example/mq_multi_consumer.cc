/*
 * mq_multi_consumer.cc
 *
 *      Created on: 2017.5.9
 *      Author: ZengHui Bao (bao_z_h@163.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "shm_msg_queue.h"

#define    SHM_KEY    67486
#define    SHM_SIZE   1024*1024

ShmMQLockComsumer gConsumer;

int main(int argc, char* argv[])
{
    ErrorCode ret;
    ret = gConsumer.init(SHM_KEY, SHM_SIZE, "./");
    if (ret != kOk)
    {
        printf("shmMQ init failed! shmKey:%d, shmSize:%d, errCode:%d\n", SHM_KEY, SHM_SIZE, ret);
        return 0;
    }

    char buf[1024*1024] = {0};
    uint32_t bufSize = 1024*1024;
    uint32_t dataLen = 0;
    std::string val;
    while (true)
    {
        while (ret == kOk)
        {
            ret = gConsumer.comsume(buf, bufSize, dataLen);
            if (ret != kOk)
            {
                if (ret == kErrShmMQEmpty)
                {
                    printf("current mq is empty, please retry later.\n");
                    break;
                }
                else
                {
                    printf("consume failed!\n");
                    return 0;
                }
            }
            else
            {
                val.assign(buf, dataLen);
                printf("consume: val:%s, valSize:%d\n", val.c_str(), dataLen);
            }
        }
        sleep(1);
        ret = kOk;
    }
}
