/*
 * shm_msg_queue.h
 *
 *      Descript: msg queue implementation in shared memory.
 *      Created on: 2017.5.9
 *      Author: ZengHui Bao (bao_z_h@163.com)
 */

#ifndef SHMUTIL_SHM_MSG_QUEUE_H
#define SHMUTIL_SHM_MSG_QUEUE_H

#include <unistd.h>
#include <string>
#include "atomic.h"
#include "common.h"
#include "error_code.h"

#define MSG_HEAD_SIZE      4
#define MSG_MAGIC_NUM_LEN  4
#define MSG_MAGIC_NUM  0x58505053    //magic num

//共享内存管道统计
struct MQStat
{
    uint32_t usedLen_;      //共享内存管道已用长度
    uint32_t freeLen_;      //共享内存管道空闲长度
    uint32_t totalLen_;     //共享内存管道总长度
    uint32_t shmKey_;       //共享内存key
    uint32_t shmId_;        //共享内存id
    uint32_t shmSize_;      //共享内存大小
};

struct MQStatInfo {
    AtomicInt32 msgCount_;     // msg count in the queue
    AtomicInt32 processCount_; // processed count last interval
};

//共享内存管道
class ShmMQ
{
public:
    ShmMQ();
    ~ShmMQ();
    ErrorCode init(int shmKey, int shmSize);
    ErrorCode clearMQ();
    ErrorCode enqueue(const void* data, uint32_t dataLen);
    ErrorCode dequeue(void* buf, uint32_t bufSize, uint32_t& dataLen);

    inline ErrorCode getstat(MQStat& mqStat) {
        if (unlikely(!isInit())) return kErrShmNotInit;
        uint32_t head = *head_;
        uint32_t tail = *tail_;
        mqStat.usedLen_ = (tail >= head) ? tail - head : tail + blockSize_ - head;
        mqStat.freeLen_ = head > tail ? head - tail : head + blockSize_ - tail;
        mqStat.totalLen_ = blockSize_;
        mqStat.shmKey_ = shmKey_;
        mqStat.shmId_ = shmId_;
        mqStat.shmSize_ = shmSize_;
        return kOk;
    }

    inline ErrorCode __attribute__((always_inline)) isEmpty(bool& isEmpty) {
        if (unlikely(!isInit())) return kErrShmNotInit;
        isEmpty = pstat_->msgCount_.get() == 0;
        return kOk;
    }

    inline ErrorCode __attribute__((always_inline)) getMsgCount(int& msgCount) {
        if (unlikely(!isInit())) return kErrShmNotInit;
        msgCount = pstat_->msgCount_.get();
        return kOk;
    }

private:
    ShmMQ(const ShmMQ&);              //disable copy
    ShmMQ& operator=(const ShmMQ&);   //disable copy
    bool check(uint32_t head, uint32_t tail); // check if head & tail is valid
    ErrorCode getmemory(int shmKey, int shmSize, bool& isCreate);
    void fini();
    bool isInit();

private:
    int shmKey_;
    int shmSize_;
    int shmId_;
    void* shmMem_;
    MQStatInfo * pstat_;
    volatile uint32_t* head_;   //read_pos, just index, not memory address
    volatile uint32_t* tail_;   //write_pos, just index, not memory address
    volatile pid_t* pid_;       //pid of process doing clear operation
    char* block_;               //shm 起始位置
    uint32_t blockSize_;        //整个shm可写的大小（除了前面的Q_STATINFO, head ,tail, pid）
    bool isInit_;
};

//生产者（不带锁）
class ShmMQProducer
{
public:
    ShmMQProducer();
    virtual ~ShmMQProducer();
    virtual ErrorCode init(int shmKey, int shmSize);
    virtual ErrorCode clearMQ();

    virtual ErrorCode produce(const void* data, uint32_t dataLen);
    virtual ErrorCode getstat(MQStat &mqStat) {
        if (unlikely(!mq_)) return kErrShmNotInit;
        return mq_->getstat(mqStat);
    }

protected:
    virtual void fini();
    ShmMQ* mq_;
};

//生产者（带锁）
class ShmMQLockProducer : public ShmMQProducer
{
public:
    ShmMQLockProducer();
    ~ShmMQLockProducer();
    ErrorCode init(int shmKey, int shmSize, std::string lockFilePath);
    ErrorCode clearMQ();
    ErrorCode produce(const void* data, uint32_t dataLen);

protected:
    void fini();
    int mfd_;

};

//消费者（不带锁）
class ShmMQComsumer
{
public:
    ShmMQComsumer();
    virtual ~ShmMQComsumer();
    virtual ErrorCode init(int shmKey, int shmSize);
    virtual ErrorCode clear();
    virtual ErrorCode comsume(void* buf, uint32_t bufSize, uint32_t& dataLen);
    virtual ErrorCode getstat(MQStat &mqStat)
    {
        if (unlikely(!mq_)) return kErrShmNotInit;
        return mq_->getstat(mqStat);
    }

protected:
    void fini();
    ShmMQ* mq_;
};

//消费者（带锁）
class ShmMQLockComsumer : public ShmMQComsumer
{
public:
    ShmMQLockComsumer();
    ~ShmMQLockComsumer();
    ErrorCode init(int shmKey, int shmSize, std::string lockFilePath);
    ErrorCode comsume(void* buf, uint32_t bufSize, uint32_t& dataLen);
    ErrorCode clearMQ();

protected:
    int mfd_;
    void fini();
};

#endif
