/*
 * shm_msg_queue.cc
 *
 *      Descript: msg queue implementation in shared memory.
 *      Created on: 2017.5.9
 *      Author: ZengHui Bao (bao_z_h@163.com)
 */

#include <sys/shm.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>
#include "shm_msg_queue.h"

ShmMQ::ShmMQ()
{
    shmKey_ = 0;
    shmSize_ = 0;
    shmId_ = 0;
    shmMem_ = NULL;
    pstat_ = NULL;
    head_ = NULL;
    tail_ = NULL;
    block_ = NULL;
    blockSize_ = 0;
    isInit_ = false;
}

ShmMQ::~ShmMQ()
{
    fini();
}

ErrorCode ShmMQ::getmemory(int shmKey, int shmSize, bool& isCreate)
{
    isCreate = false;
    shmSize += sizeof(MQStatInfo);
    shmId_ = shmget(shmKey, shmSize, IPC_CREAT | IPC_EXCL | 0666);
    if (-1 == shmId_)
    {
        if (EEXIST == errno)
        {
            shmId_ = shmget(shmKey, 0, 0644);
            if (-1 == shmId_) return kErrShmGet;

            shmMem_ = shmat(shmId_, NULL, 0);
            if (likely(shmMem_ != (void*)-1))
            {
                pstat_ = (MQStatInfo *)shmMem_;
                shmMem_ = (void*)((unsigned long)shmMem_ + sizeof(MQStatInfo));
                return kOk;
            }
            else
            {
                fprintf( stderr, "[getmemory] shmat failed! shmKey:%x,shmSize:%d, errno:%d\n", shmKey_, shmSize_, errno);
                return kErrShmAt;
            }
        }

        fprintf(stderr, "[getmemory] Failed to create share memory, key:%d, errno:%d\n", shmKey, errno);
        return kErrShmGet;
    }

    isCreate = true;
    shmMem_ = shmat(shmId_, NULL, 0);
    if (likely(shmMem_ != (void*)-1))
    {
        pstat_ = (MQStatInfo *) shmMem_;
        shmMem_ = (void *) ((unsigned long) shmMem_ + sizeof(MQStatInfo));
        return kOk;
    }
    else
    {
        fprintf( stderr, "[getmemory] shmat failed! shmKey:%x,shmSize:%d, errno:%d\n", shmKey_, shmSize_, errno);
        return kErrShmAt;
    }
}

void ShmMQ::fini()
{
    if (pstat_)    // pstat_就是地址开始
    {
        shmdt((const void*)pstat_);
        pstat_ = NULL;
    }
    isInit_ = false;
}

bool ShmMQ::isInit()
{
    return isInit_;
}

ErrorCode ShmMQ::init(int shmKey, int shmSize)
{
    fini();

    shmKey_ = shmKey;
    shmSize_ = shmSize;

    ErrorCode ret;
    bool isCreate = false;
    if ((ret = getmemory(shmKey_, shmSize_, isCreate)) == kOk && isCreate)
    {
        memset(shmMem_, 0x0, sizeof(uint32_t) * 2);
    }
    else if (ret != kOk)
    {
        fprintf(stderr, "[init] get shm memory failed! ret:%d\n", ret);
        return ret;
    }

    //the head of data section
    head_ = (uint32_t*)shmMem_;
    //the tail of data section
    tail_ = head_ + 1;
    // pid field
    pid_ = (pid_t *)(tail_ + 1);
    //data section base address
    block_ = (char*)(pid_ + 1);
    //data section length
    blockSize_ = shmSize_ - sizeof(uint32_t) * 2 - sizeof(pid_t);

    *pid_ = 0;
    isInit_ = true;
    return kOk;
}

ErrorCode ShmMQ::clearMQ()
{
    if (unlikely(!isInit())) return kErrShmNotInit;

    //clear head and tail of shm queue
    *pid_ = getpid();     //标识此进程正在clear mq
    *head_ = 0;
    *tail_ = 0;
    pstat_->msgCount_.getAndSet(0);
    *pid_ = 0;            //clear完毕，重置为0.

    return kOk;
}

bool ShmMQ::check(uint32_t head, uint32_t tail)
{
    if (likely( head <= blockSize_ && tail <= blockSize_ ) )
    {
        // head & tail 合法
        return true;
    }

    bool isNeedClear = false;
    if ( *pid_ == 0 )
    {
        // need clear
        isNeedClear = true;
    }
    else
    {
        if ( !checkProcExist(*pid_) )
        {
            // process of do clearing not exist, need clear
            isNeedClear = true;
        }

        // other process is doing clearing
    }

    if (isNeedClear)
    {
        clearMQ();
    }
    return false;
}

ErrorCode ShmMQ::enqueue(const void* data, uint32_t dataLen)
{
    if (unlikely(!isInit())) return kErrShmNotInit;

    uint32_t head = *head_;
    uint32_t tail = *tail_;

    // check if head & tail is valid
    if (unlikely( !check(head, tail) ))
    {
        return kErrDataMess;
    }

    uint32_t freeLen = head > tail ? head - tail : head + blockSize_ - tail;
    uint32_t tailLen = blockSize_ - tail;

    char sHead[MSG_HEAD_SIZE + MSG_MAGIC_NUM_LEN] = {0};
    char *psHead = sHead;
    uint32_t totalLen = dataLen + MSG_HEAD_SIZE + 2 * MSG_MAGIC_NUM_LEN;

    //as to 4 possible queue freeLen,enqueue data in 4 ways
    //  first, if no enough space?
    if (unlikely(freeLen <= totalLen))
    {
        fprintf(stderr, "[enqueue] shmMQ is full, freeLen:%d, dataLen:%d, so enqueue failed!\n", freeLen, totalLen);
        return kErrNotEnoughSpace;
    }

    *(uint32_t *)psHead = MSG_MAGIC_NUM;    //set magic number
    memcpy(psHead + MSG_MAGIC_NUM_LEN, &totalLen, sizeof(uint32_t));

    //  second, if tail space > 4+4+len+4
    //  copy 8 byte, copy data
    if (tailLen >= totalLen)
    {
        //append head  //数据块 左右两边都要加上幻数
        memcpy(block_ + tail, psHead, MSG_MAGIC_NUM_LEN + MSG_HEAD_SIZE);
        //append data
        memcpy(block_ + tail + MSG_MAGIC_NUM_LEN + MSG_HEAD_SIZE , data, dataLen);
        //append MSG_MAGIC_NUM
        *(uint32_t*)(block_ + tail + MSG_MAGIC_NUM_LEN + MSG_HEAD_SIZE + dataLen) = MSG_MAGIC_NUM;
        //recalculate tail_ position
        *tail_ += (dataLen + MSG_HEAD_SIZE + MSG_MAGIC_NUM_LEN * 2);
    }
        //  third, if tail space > 4+4 && < 4+4+len+4
    else if (tailLen >= MSG_HEAD_SIZE + MSG_MAGIC_NUM_LEN && tailLen < MSG_HEAD_SIZE + 2 * MSG_MAGIC_NUM_LEN + dataLen)
    {
        //append head, 4+4 byte
        memcpy(block_ + tail, psHead, MSG_MAGIC_NUM_LEN + MSG_HEAD_SIZE);

        //separate data into two parts
        size_t firstLen = tailLen - MSG_MAGIC_NUM_LEN - MSG_HEAD_SIZE;
        size_t secondLen = dataLen + MSG_MAGIC_NUM_LEN - firstLen;

        if (secondLen >= MSG_MAGIC_NUM_LEN)
        {
            //append first part of data, tail-4-8
            memcpy(block_ + tail + MSG_MAGIC_NUM_LEN + MSG_HEAD_SIZE, data, firstLen);
            //append left, second part of data
            memcpy(block_, ((char*)data) + firstLen, secondLen - MSG_MAGIC_NUM_LEN);
            *(uint32_t *)(block_ + secondLen - MSG_MAGIC_NUM_LEN) = MSG_MAGIC_NUM;
        }
        else
        {
            // copy user data
            memcpy(block_ + tail + MSG_MAGIC_NUM_LEN + MSG_HEAD_SIZE, data, firstLen - MSG_MAGIC_NUM_LEN + secondLen);

            //append "SPPX" to the data
            if (secondLen == 3)
            {
                *(char*)(block_ + blockSize_ - 1) = 'S';
                *(char*)(block_) = 'P';
                *(char*)(block_ + 1) = 'P';
                *(char*)(block_ + 2) = 'X';
            }
            else if (secondLen == 2)
            {
                *(char*)(block_ + blockSize_ - 2) = 'S';
                *(char*)(block_ + blockSize_ - 1) = 'P';
                *(char*)(block_) = 'P';
                *(char*)(block_ + 1) = 'X';
            }
            else if (secondLen == 1)
            {
                *(char*)(block_ + blockSize_ - 3) = 'S';
                *(char*)(block_ + blockSize_ - 2) = 'P';
                *(char*)(block_ + blockSize_ - 1) = 'P';
                *(char*)(block_) = 'X';
            }
        }

        //recalculate tail_ position
        int offset = (int) totalLen - (int)blockSize_;
        int newTail = (int)tail + offset;
        *tail_ = (uint32_t)newTail;
    }
        //  fourth, if tail space < 4+8
    else
    {
        //append first part of head, copy tail byte
        memcpy(block_ + tail, psHead, tailLen);

        //append second part of head, copy 8-tail byte
        uint32_t secondLen = MSG_MAGIC_NUM_LEN + MSG_HEAD_SIZE - tailLen;
        memcpy(block_, psHead + tailLen, secondLen);

        //append data
        memcpy(block_ + secondLen, data, dataLen);

        //append MSG_MAGIC_NUM
        *(uint32_t*)(block_ + secondLen + dataLen) = MSG_MAGIC_NUM;

        //recalculate tail_ position
        *tail_ = secondLen + dataLen + MSG_MAGIC_NUM_LEN;
    }

    // 数据包计数
    pstat_->msgCount_.increment();
    return kOk;
}

//这里要求 buf_size >= 读出的消息的data_len + 4, 最后4位是存的幻数
//返回的data_len 返回真正的 消息数据的长度.
ErrorCode ShmMQ::dequeue(void* buf, uint32_t bufSize, uint32_t&dataLen)
{
    if (unlikely(!isInit())) return kErrShmNotInit;

    uint32_t head = *head_;
    uint32_t tail = *tail_;
    // check if head & tail is valid
    if ( unlikely( !check(head, tail) ) )
    {
        dataLen = 0;
        return kErrDataMess;
    }

    if (head == tail)
    {
        dataLen = 0;
        return kErrShmMQEmpty;
    }

    pstat_->msgCount_.decrement();   // 尽快减，以通知到竞争者

    uint32_t usedLen = tail > head ? tail - head : tail + blockSize_ - head;
    char sHead[MSG_MAGIC_NUM_LEN + MSG_HEAD_SIZE];
    char *psHead = sHead;

    // get head
    // if head + 4 > block_size
    if (head + MSG_MAGIC_NUM_LEN + MSG_HEAD_SIZE > blockSize_)
    {
        uint32_t firstSize = blockSize_ - head;
        uint32_t secondSize = MSG_MAGIC_NUM_LEN + MSG_HEAD_SIZE - firstSize;
        memcpy(psHead, block_ + head, firstSize);
        memcpy(psHead + firstSize, block_, secondSize);
        head = secondSize;
    }
    else
    {
        memcpy(psHead, block_ + head, MSG_MAGIC_NUM_LEN + MSG_HEAD_SIZE);
        head += (MSG_HEAD_SIZE + MSG_MAGIC_NUM_LEN);
    }

    //get meta data
    uint32_t totalLen = *(uint32_t*)(psHead + MSG_MAGIC_NUM_LEN);

    //check safe area  检测幻数，如果失败，则清空数据
    if (*(uint32_t*)(psHead) != MSG_MAGIC_NUM)
    {
        fprintf(stderr, "[dequeue] check magic num failed! realNum:%x, magicNum:%x\n", *(uint32_t*)(psHead),
                MSG_MAGIC_NUM);
        if (check(*head_, *tail_) )
        {
            // head&tail是合法的，也需要清空共享内存; 非法时，在do_check内部已经执行清空动作
            clearMQ();
        }
        dataLen = 0;
        return kErrDataMess;
    }

    /*解决共享内存乱问题
     * 当内存乱掉（totalLen <= used_le)的时候，let head:=tail, 即共享内存清空*/
    //assert(totalLen <= usedLen);
    if (unlikely(totalLen > usedLen))  //当total_len > used_len时，清空内存中的数据
    {
        fprintf(stderr, "[dequeue] check shm validation failed! shmKey:%d, shmSize:%d, shmId:%d, totalLen:%d, usedLen_:%d\n",
                shmKey_, shmSize_, shmId_, totalLen, usedLen);

        if (check(*head_, *tail_) )
        {
            // 清空共享内存
            clearMQ();
        }
        dataLen = 0;
        return kErrDataMess;
    }

    dataLen = totalLen - MSG_HEAD_SIZE - MSG_MAGIC_NUM_LEN;
    if (unlikely(dataLen > bufSize))   //传入的缓冲区大小 小于 消息大小
    {
        return kErrInputBufNotEnoughSpace;
    }

    if (head + dataLen > blockSize_)
    {
        uint32_t firstSize = blockSize_ - head;
        uint32_t secondSize = dataLen - firstSize;
        memcpy(buf, block_ + head , firstSize);
        memcpy(((char*)buf) + firstSize, block_ , secondSize);

        *head_ = secondSize;
    }
    else
    {
        memcpy(buf, block_ + head , dataLen);
        *head_ = head + dataLen;
    }

    dataLen -= MSG_MAGIC_NUM_LEN;
    if (*(uint32_t*)(((char*)buf) + dataLen) != MSG_MAGIC_NUM)
    {
        fprintf(stderr, "[dequeue] check magic num failed! realNum:%x, magicNum:%x\n", *(uint32_t*)(psHead),
                MSG_MAGIC_NUM);
        if (check(*head_, *tail_) )
        {
            // 清空共享内存
            clearMQ();
        }
        dataLen = 0;
        return kErrDataMess;
    }
    pstat_->processCount_.increment();
    return kOk;
}

ShmMQProducer::ShmMQProducer()
{
    mq_ = NULL;
}

ShmMQProducer::~ShmMQProducer()
{
    fini();
}

ErrorCode ShmMQProducer::init(int shmKey, int shmSize)
{
    fini();

    mq_ = new ShmMQ;
    return mq_->init(shmKey, shmSize);
}

void ShmMQProducer::fini()
{
    if (mq_ != NULL)
    {
        delete mq_;
        mq_ = NULL;
    }
}

ErrorCode ShmMQProducer::clearMQ()
{
    if (unlikely(!mq_)) return kErrShmNotInit;
    return mq_->clearMQ();
}

ErrorCode ShmMQProducer::produce(const void* data, uint32_t dataLen)
{
    if (unlikely(!mq_)) return kErrShmNotInit;
    return mq_->enqueue(data, dataLen);
}

ShmMQLockProducer::ShmMQLockProducer()
{
    mfd_ = 0;
}
ShmMQLockProducer::~ShmMQLockProducer()
{
    fini();
}

ErrorCode ShmMQLockProducer::init(int shmKey, int shmSize, std::string lockFilePath)
{
    fini();
    ErrorCode ret = ShmMQProducer::init(shmKey, shmSize);
    if ( ret != kOk )
    {
        return ret;
    }

    char mfile[128] = {0};
    snprintf(mfile, sizeof(mfile) - 1, "%s/.mq_producer_%x.lock", lockFilePath.c_str(), shmKey);
    mfd_ = open(mfile, O_RDWR | O_CREAT , 0666);
    if ( mfd_ < 0 )
    {
        return kErrOpenLockFileFailed;
    }
    return kOk;
}

void ShmMQLockProducer::fini()
{
    ShmMQProducer::fini();
    if ( mfd_ > 0 )
    {
        close(mfd_);
    }
}

ErrorCode ShmMQLockProducer::clearMQ()
{
    int ret = flock(mfd_, LOCK_EX);
    if (unlikely(ret != 0))
    {
        fprintf(stderr, "[clearMQ] producer lock file failed! fd:%d\n", mfd_);
        return kErrLockFileFailed;
    }

    ErrorCode r = ShmMQProducer::clearMQ();
    flock(mfd_, LOCK_UN);
    return r;
}

ErrorCode ShmMQLockProducer::produce(const void* data, uint32_t dataLen)
{
    int ret = flock(mfd_, LOCK_EX);
    if ( likely( ret == 0 ) )
    {
        ErrorCode r = ShmMQProducer::produce(data, dataLen);
        flock(mfd_, LOCK_UN);
        return r;
    }
    return kErrLockFileFailed;
}

ShmMQComsumer::ShmMQComsumer()
{
    mq_ = NULL;
}

ShmMQComsumer::~ShmMQComsumer()
{
    fini();
}

ErrorCode ShmMQComsumer::init(int shmKey, int shmSize)
{
    fini();
    mq_ = new ShmMQ;
    return mq_->init(shmKey, shmSize);
}

void ShmMQComsumer::fini()
{
    if (mq_)
    {
        delete mq_;
        mq_ = NULL;
    }
}

ErrorCode ShmMQComsumer::clear()
{
    if (unlikely(!mq_)) return kErrShmNotInit;
    return mq_->clearMQ();
}

ErrorCode ShmMQComsumer::comsume(void* buf, uint32_t bufSize, uint32_t&dataLen)
{
    if (unlikely(!mq_)) return kErrShmNotInit;
    bool isEmpty = false;
    if (mq_->isEmpty(isEmpty) == kOk && isEmpty)
    {
        return kErrShmMQEmpty;
    }
    return mq_->dequeue(buf, bufSize, dataLen);
}

ShmMQLockComsumer::ShmMQLockComsumer()
{
    mfd_ = 0;
}

ShmMQLockComsumer::~ShmMQLockComsumer()
{
    fini();
}

ErrorCode ShmMQLockComsumer::init(int shmKey, int shmSize, std::string lockFilePath)
{
    fini();

    ErrorCode ret = ShmMQComsumer::init(shmKey, shmSize);
    if ( ret != kOk )
    {
        return ret;
    }

    char mfile[128] = {0};
    snprintf(mfile, sizeof(mfile) - 1, "%s/.mq_comsumer_%x.lock", lockFilePath.c_str(), shmKey);
    mfd_ = open(mfile, O_RDWR | O_CREAT , 0666);
    if ( mfd_ < 0 )
    {
        return kErrOpenLockFileFailed;
    }
    return kOk;
}

void ShmMQLockComsumer::fini()
{
    ShmMQComsumer::fini();
    if ( mfd_ > 0 )
    {
        close(mfd_);
    }
}

ErrorCode ShmMQLockComsumer::clearMQ()
{
    int ret = flock(mfd_, LOCK_EX);
    if (unlikely(ret != 0))
    {
        fprintf(stderr, "[clearMQ] consumer lock file failed! fd:%d\n", mfd_);
        return kErrLockFileFailed;
    }

    ErrorCode r = ShmMQComsumer::clear();
    flock(mfd_, LOCK_UN);
    return r;
}

inline ErrorCode ShmMQLockComsumer::comsume(void* buf, uint32_t bufSize, uint32_t&dataLen)
{
    int ret = flock(mfd_, LOCK_EX);
    if ( likely( ret == 0 ) )
    {
        ErrorCode r = ShmMQComsumer::comsume(buf, bufSize, dataLen);
        flock(mfd_, LOCK_UN);
        return r;
    }
    return kErrLockFileFailed;
}
