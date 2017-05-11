/******************************************************************************
 * qLibc - http://www.qdecoder.org
 *
 * Copyright (c) 2010-2012 Seungyoung Kim.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************
 * $Id: qlibc.h 126 2012-06-11 22:20:01Z seungyoung.kim $
 ******************************************************************************/

/**
 * qlibc header file
 *
 * @file qlibc.h
 */

#ifndef _QLIBC_H
#define _QLIBC_H

#define _Q_PRGNAME "qlibc"  /*!< qlibc human readable name */
#define _Q_VERSION "2.1.0"  /*!< qlibc version number string */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <pthread.h>

/******************************************************************************
 * COMMON DATA STRUCTURES
 ******************************************************************************/

typedef struct qnobj_s qnobj_t;      /*!< named-object type*/

/**
 * named-object data structure.
 */
struct qnobj_s
{
    char *name;         /*!< object name */     //key
    void *data;         /*!< data */            //value
    size_t name_size;   /*!< object name size */
    size_t data_size;   /*!< data size */
};

/******************************************************************************
 * Static Hash Table Container - works in fixed size memory
 * qhasharr.c
 ******************************************************************************/

/* tunable knobs */
#define _Q_HASHARR_KEYSIZE (96)    /*!< knob for maximum key size. */
#define _Q_HASHARR_VALUESIZE (128)  /*!< knob for maximum data size in a slot. */

/* types */
typedef struct qhasharr_s qhasharr_t;
typedef struct qhasharr_slot_s qhasharr_slot_t;

/* public functions */
extern qhasharr_t *qhasharr(void *memory, size_t memsize);
extern size_t qhasharr_calculate_memsize(int max);
extern int qhasharr_init(qhasharr_t *tbl, qhasharr_slot_t **_tbl_slots);

/* capsulated member functions */
extern bool qhasharr_put(qhasharr_t *tbl, const char *key, size_t key_size, const void *value, size_t val_size);
extern bool qhasharr_putstr(qhasharr_t *tbl, const char *key, const char *str);
extern bool qhasharr_putint(qhasharr_t *tbl, const char *key, int64_t num);
extern bool qhasharr_exist(qhasharr_t *tbl, const char *key, size_t key_size);
extern void *qhasharr_get(qhasharr_t *tbl, const char *key, size_t key_size, size_t *val_size);
extern char *qhasharr_getstr(qhasharr_t *tbl, const char *key);
extern int64_t qhasharr_getint(qhasharr_t *tbl, const char *key);
extern bool qhasharr_getnext(qhasharr_t *tbl, qnobj_t *obj, int *idx);
extern bool qhasharr_remove(qhasharr_t *tbl, const char *key, size_t key_size);
extern int  qhasharr_size(qhasharr_t *tbl, int *maxslots, int *usedslots);
extern void qhasharr_clear(qhasharr_t *tbl);

union _slot_data
{
    /*!< key/value data */
    struct _Q_HASHARR_SLOT_KEYVAL
    {
        unsigned char value[_Q_HASHARR_VALUESIZE];  /*!< value */

        char key[_Q_HASHARR_KEYSIZE];  /*!< key string, can be cut */
        uint16_t  keylen;              /*!< original key length */
        unsigned char keymd5[16];      /*!< md5 hash of the key */
    } pair;

    /*!< extended data block, used only when the count value is -2 */
    struct _Q_HASHARR_SLOT_EXT
    {
        unsigned char value[sizeof(struct _Q_HASHARR_SLOT_KEYVAL)];
    } ext;
} ;

/**
 * qhasharr internal data slot structure
 */
struct qhasharr_slot_s
{
    short  count;   /*!< hash collision counter. 0 indicates empty slot,
                     -1 is used for collision resolution, -2 is used for
                     indicating linked block */
                //当hash冲突时，桶链表的第一个结点的slot.count:记录此hash值对应的 key/value entry的个数
                //             桶链表的第一个结点之后的结点： 每个结点的首个slot的count置为-1
                //当count为-2时，表示 value_len超过96，需要多个slot来存储。在link来指示下一个slot的信息.
                          //此时，第一个slot的count置为>0，之后的slot的count置为-2, value存储到data->ext中.
                //当count为0时，表示这个是空slot.
    uint32_t  hash; /*!< key hash. we use FNV32 */
                     //当key/value entry占用多个slot时，第二个以后的slot->hash都保存 上一个slot的slot index
                     //注：无论冲突了多少、次序是什么，slots[entry->hash] 绐终保存key/value对应的桶链表的第一个结点的首个slot
                     //所以在插入时，如果slot[hash]的位置被别人(即不是此hash值的entry为了冲突处理占了这个位置)占了，需要将这个位置的数据 移到别的空slot，然后将key/value数据保存到这个slot
    uint8_t size;   /*!< value size in this slot*/   //这个slot下的保存 value的字节数
    int link;       /*!< next link */                //若一个value占用比较大，需要多个slot保存。这个link指示下一个slot.
                                                     //最后一个slot的link置为-1.
    union _slot_data data;
};

/**
 * qhasharr container
 */
struct qhasharr_s
{
    /* private variables - do not access directly */
    int maxslots;       /*!< number of maximum slots */
    int usedslots;      /*!< number of used slots */
    int num;            /*!< number of stored keys */
    char slots[];       /*!< data area pointer */
};

/******************************************************************************
 * UTILITIES SECTION
 ******************************************************************************/
extern bool qhashmd5(const void *data, size_t nbytes, void *retbuf);
extern uint32_t qhashmurmur3_32(const void *data, size_t nbytes);

#ifdef __cplusplus
}
#endif

#endif /*_QLIBC_H */
