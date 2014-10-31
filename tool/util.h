/*
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

#ifndef __REDIS_UTIL_H
#define __REDIS_UTIL_H

#include "sds.h"

int stringmatchlen(const char *p, int plen, const char *s, int slen, int nocase); /*支持glob-style的通配符格式,如*表示任意一个或多个字符,?表示任意字符,[abc]表示方括号中任意一个字母。*/
int stringmatch(const char *p, const char *s, int nocase); /*支持glob-style的通配符格式,长度的计算直接放在方法内部了，直接传入模式和原字符串*/
long long memtoll(const char *p, int *err); /* 内存大小转化为单位为字节大小的数值表示 */
int ll2string(char *s, size_t len, long long value); /* long long类型转化为string类型 */
int string2ll(const char *s, size_t slen, long long *value); /* String类型转换为long long类型 */
int string2l(const char *s, size_t slen, long *value); /* String类型转换为long类型，核心调用的方法还是string2ll()方法 */
int d2string(char *buf, size_t len, double value); /* double类型转化为String类型 */
sds getAbsolutePath(char *filename); /* 获取输入文件名的绝对路径 */
int pathIsBaseName(char *path); /* 判断一个路径是否就是纯粹的文件名，不是相对路径或是绝对路径 */

#endif
