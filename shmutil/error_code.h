/*
 * error_code.h
 *
 *      Created on: 2017.5.9
 *      Author: ZengHui Bao (bao_z_h@163.com)
 */

#ifndef SHMUTIL_ERROR_CODE_H
#define SHMUTIL_ERROR_CODE_H

enum ErrorCode {
    kOk = 0,
    kErrShmGet = 1,
    kErrShmAt = 2,
    kErrShmInit = 3,
    kErrInvalidParams = 4,
    kErrTraverseTableEnd = 5,
    kErrNotFound = 6,
    kErrDataMess = 7,
    kErrLruInit = 8,
    kErrSetSameValue = 9,
    kErrNotEnoughSpace = 10,
    kErrStoreKVFailed = 11,
    kErrShmNotInit = 12,

    kErrOther = 10000,
};

#endif //SHMUTIL_ERROR_CODE_H
