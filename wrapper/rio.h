/*
 * Copyright (c) 2009-2012, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
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
 */


#ifndef __REDIS_RIO_H
#define __REDIS_RIO_H

#include <stdio.h>
#include <stdint.h>
#include "sds.h"

struct _rio {
    /* Backend functions.
     * Since this functions do not tolerate short writes or reads the return
     * value is simplified to: zero on error, non zero on complete success. */
    /* 数据流的读方法 */
    size_t (*read)(struct _rio *, void *buf, size_t len);
    /* 数据流的写方法 */
    size_t (*write)(struct _rio *, const void *buf, size_t len);
    /* 获取当前的读写偏移量 */
    off_t (*tell)(struct _rio *);
    /* The update_cksum method if not NULL is used to compute the checksum of
     * all the data that was read or written so far. The method should be
     * designed so that can be called with the current checksum, and the buf
     * and len fields pointing to the new block of data to add to the checksum
     * computation. */
    /* 当读入新的数据块的时候，会更新当前的校验和 */
    void (*update_cksum)(struct _rio *, const void *buf, size_t len);

    /* The current checksum */
    /* 当前的校验和 */
    uint64_t cksum;

    /* number of bytes read or written */
    /* 当前读取的或写入的字节大小 */
    size_t processed_bytes;

    /* maximum single read or write chunk size */
    /* 最大的单次读写的大小 */
    size_t max_processing_chunk;

    /* Backend-specific vars. */
    /* rio中I/O变量 */
    union {
    	//buffer结构体
        struct {
        	//buffer具体内容
            sds ptr;
            //偏移量
            off_t pos;
        } buffer;
        //文件结构体
        struct {
            FILE *fp;
            off_t buffered; /* Bytes written since last fsync. */
            //同步的最小大小
            off_t autosync; /* fsync after 'autosync' bytes written. */
        } file;
    } io;
};

typedef struct _rio rio;

/* The following functions are our interface with the stream. They'll call the
 * actual implementation of read / write / tell, and will update the checksum
 * if needed. */
/* rio的写方法 */
static inline size_t rioWrite(rio *r, const void *buf, size_t len) {
    while (len) {
    	//判断当前操作字节长度是否超过最大长度
        size_t bytes_to_write = (r->max_processing_chunk && r->max_processing_chunk < len) ? r->max_processing_chunk : len;
        //写入新的数据时，更新校验和
        if (r->update_cksum) r->update_cksum(r,buf,bytes_to_write);
        //执行写方法
        if (r->write(r,buf,bytes_to_write) == 0)
            return 0;
        buf = (char*)buf + bytes_to_write;
        len -= bytes_to_write;
        //操作字节数增加
        r->processed_bytes += bytes_to_write;
    }
    return 1;
}

/* rio的读方法 */
static inline size_t rioRead(rio *r, void *buf, size_t len) {
    while (len) {
    	//判断当前操作字节长度是否超过最大长度
        size_t bytes_to_read = (r->max_processing_chunk && r->max_processing_chunk < len) ? r->max_processing_chunk : len;
        //读数据方法
        if (r->read(r,buf,bytes_to_read) == 0)
            return 0;
        //读数据时，更新校验和
        if (r->update_cksum) r->update_cksum(r,buf,bytes_to_read);
        buf = (char*)buf + bytes_to_read;
        len -= bytes_to_read;
        r->processed_bytes += bytes_to_read;
    }
    return 1;
}

/* 获取当前偏移量 */
static inline off_t rioTell(rio *r) {
    return r->tell(r);
}

void rioInitWithFile(rio *r, FILE *fp); /* 初始化rio中的file变量 */
void rioInitWithBuffer(rio *r, sds s); /* 初始化rio中的buffer变量 */

/* rio写入不同类型数据方法，最终调用的是riowrite方法 */
size_t rioWriteBulkCount(rio *r, char prefix, int count);
size_t rioWriteBulkString(rio *r, const char *buf, size_t len);
size_t rioWriteBulkLongLong(rio *r, long long l);
size_t rioWriteBulkDouble(rio *r, double d);

void rioGenericUpdateChecksum(rio *r, const void *buf, size_t len); /* 计算校验和用的是循环冗余校验算法 */
void rioSetAutoSync(rio *r, off_t bytes); /* 设置多少大小值时进行，自动同步 */

#endif
