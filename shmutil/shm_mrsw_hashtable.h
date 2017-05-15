/*
 * shm_mrsw_hashtable.h
 *
 *      Descript: mutli-reader-single-writer hashtable implementation in shared memory.
 *      Created on: 2017.5.9
 *      Author: ZengHui Bao (bao_z_h@163.com)
 */

#ifndef SHMUTIL_SHM_MRSW_HASHTABLE_H
#define SHMUTIL_SHM_MRSW_HASHTABLE_H

#include <sys/shm.h>
#include <string>
#include "error_code.h"
#include "qlibc.h"
#include "simple_lru.h"
#include "mutex.h"

class MrswHashtable
{
public:
    MrswHashtable();
    ~MrswHashtable();
    ErrorCode init(key_t shmkey, int maxSlotNum = 800000, bool isLruEliminate = true);
    ErrorCode getValue(const std::string &key, std::string &val);
    ErrorCode setValue(const std::string &key, const std::string &val);
    ErrorCode isExist(const std::string &key, bool &exist);
    ErrorCode deleteKey(const std::string &key);
    ErrorCode getNext(std::string &key, std::string &val, int &index);
    ErrorCode getStats(int &maxSlots, int &usedSlots, int &usedKeys);
    ErrorCode clearTable();

private:
    MrswHashtable(const MrswHashtable&);              //disable copy
    MrswHashtable& operator=(const MrswHashtable&);   //disable copy
    ErrorCode getShm(key_t shmkey, bool isLruEliminate);
    ErrorCode initLruMem(qhasharr_t* tbl);
    ErrorCode getNextKVPair(qhasharr_t *tbl, std::string &tblkey, std::string &tblval, int &idx);
    ErrorCode verifyShmValue(std::string &tblval);
    std::string encodeValueStr(const std::string &val);
    ErrorCode __setValue(qhasharr_t *tbl, const std::string &key, const std::string &val);
    bool isInit();

private:
    qhasharr_t *table_;
    SimpleLru lru_;
    bool isLruEliminate_;
    MutexLock mutex_;
    bool isInit_;
};

#endif
