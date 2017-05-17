# shmutil
some utilities of shared memory.

### Feature

`shmutil` implements a mutli-reader-single-writer hashtable and ring queue in shared memory.

Features of hashtable:

- Support mutli-reader-single-writer operating hashtable concurrently, not support multi-writers operating hashtable at same time.

    If you want to handle problem of multi processes writing the hashtable, please use cross-process lock(such as file-lock) utility to synchronize multiple processes.
     
- Note: the key size can not be longer than 96 bytes.

    If you wish to support large key longer than 96 bytes, please change `_Q_HASHARR_KEYSIZE` value in `qlibc.h`.
    
- Support LRU elimination while shm space is full.

Features of ring queue:

- Support single-reader-single-writer lock-free operating queue concurrently, and also provide locked version to handle multiple reader or multiple writer operating queue at same time.
 
- Its a FIFO queue. If shm space is full while pushing, `shmutil` will return `kErrNotEnoughSpace` error.

### Usage

Please refer to example directory for how to use it.


### Thanks

Some ideas is inspired from other open-source projects, thanks for [MSEC][1], [QCONF][2], [libshmcache][3] projects.

[1]: https://github.com/Tencent/MSEC

[2]: https://github.com/qihoo360/qconf

[3]: https://github.com/happyfish100/libshmcache
