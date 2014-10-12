/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __SDS_H
#define __SDS_H

/* 最大分配内存1M */
#define SDS_MAX_PREALLOC (1024*1024)

#include <sys/types.h>
#include <stdarg.h>

/* 声明了sds的一种char类型 */
typedef char *sds;

/* 字符串结构体类型 */
struct sdshdr {
	//字符长度
    unsigned int len;
    //当前可用空间
    unsigned int free;
    //具体存放字符的buf
    char buf[];
};

/* 计算sds的长度，返回的size_t类型的数值 */
/* size_t,它是一个与机器相关的unsigned类型，其大小足以保证存储内存中对象的大小。 */
static inline size_t sdslen(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->len;
}

/* 根据sdshdr中的free标记获取可用空间 */
static inline size_t sdsavail(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->free;
}

sds sdsnewlen(const void *init, size_t initlen);   //根据给定长度，新生出一个sds
sds sdsnew(const char *init);    //根据给定的值，生出sds
sds sdsempty(void);    //清空sds操作
size_t sdslen(const sds s);   //获取sds的长度
sds sdsdup(const sds s);   //sds的复制方法
void sdsfree(sds s);   //sds的free释放方法
size_t sdsavail(const sds s);   //判断sds获取可用空间
sds sdsgrowzero(sds s, size_t len); // 扩展字符串到指定的长度 
sds sdscatlen(sds s, const void *t, size_t len);
sds sdscat(sds s, const char *t);    //sds连接上char字符
sds sdscatsds(sds s, const sds t);  //sds连接上sds
sds sdscpylen(sds s, const char *t, size_t len);  //字符串复制相关
sds sdscpy(sds s, const char *t); //字符串复制相关

sds sdscatvprintf(sds s, const char *fmt, va_list ap);   //字符串格式化输出，依赖已有的方法sprintf，效率不及下面自己写的
#ifdef __GNUC__
sds sdscatprintf(sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);
#endif

sds sdscatfmt(sds s, char const *fmt, ...);   //字符串格式化输出
sds sdstrim(sds s, const char *cset);       //字符串缩减
void sdsrange(sds s, int start, int end);   //字符串截取函数
void sdsupdatelen(sds s);   //更新字符串最新的长度
void sdsclear(sds s);   //字符串清空操作
int sdscmp(const sds s1, const sds s2);   //sds比较函数
sds *sdssplitlen(const char *s, int len, const char *sep, int seplen, int *count);  //字符串分割子字符串
void sdsfreesplitres(sds *tokens, int count);  //释放子字符串数组
void sdstolower(sds s);    //sds字符转小写表示
void sdstoupper(sds s);    //sds字符统一转大写
sds sdsfromlonglong(long long value);   //生出数组字符串
sds sdscatrepr(sds s, const char *p, size_t len);
sds *sdssplitargs(const char *line, int *argc);   //参数拆分
sds sdsmapchars(sds s, const char *from, const char *to, size_t setlen); //字符映射,"ho", "01", h映射为0， o映射为1
sds sdsjoin(char **argv, int argc, char *sep);   //以分隔符连接字符串子数组构成新的字符串

/* Low level functions exposed to the user API */
/* 开放给使用者的API */
sds sdsMakeRoomFor(sds s, size_t addlen);
void sdsIncrLen(sds s, int incr);
sds sdsRemoveFreeSpace(sds s);
size_t sdsAllocSize(sds s);

#endif
