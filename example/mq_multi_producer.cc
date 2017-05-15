/*
 * mq_multi_producer.cc
 *
 *      Created on: 2017.5.9
 *      Author: ZengHui Bao (bao_z_h@163.com)
 */

#include <stdio.h>
#include <string>
#include "shm_msg_queue.h"

#define    SHM_KEY    67486
#define    SHM_SIZE   1024*1024

ShmMQLockProducer gProducer;

int main(int argc, char* argv[])
{
    ErrorCode ret;
    ret = gProducer.init(SHM_KEY, SHM_SIZE, "./");
    if (ret != kOk)
    {
        printf("shmMQ init failed! shmKey:%d, shmSize:%d, errCode:%d\n", SHM_KEY, SHM_SIZE, ret);
        return 0;
    }

    ret = gProducer.clearMQ();
    if (ret != kOk)
    {
        printf("shmMQ clearMQ failed! shmKey:%d, errCode:%d\n", SHM_KEY, ret);
        return 0;
    }

    std::string val;
    for (int index = 0; index < 10000; index++)
    {
        val = "";
        int randomTimes = rand() % 10;
        if (randomTimes == 0)
        {
            randomTimes = 5;
        }

        for (int i = 0; i < randomTimes; i++)
        {
            val += "baozh";
        }

        ret = gProducer.produce(val.c_str(), val.length());
        if (ret != kOk)
        {
            printf("shmMQ produce failed! shmKey:%d, valLen:%lu, errCode:%d\n", SHM_KEY, val.length(), ret);
            return 0;
        }
        else
        {
            printf("shmMQ produce: data:%s, dataLen:%lu\n", val.c_str(), val.length());
        }
        sleep(10);
    }
}
