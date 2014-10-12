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

/* 标记列表头节点和尾结点的标识 */
#define ZIPLIST_HEAD 0
#define ZIPLIST_TAIL 1

unsigned char *ziplistNew(void);    //创建新列表
unsigned char *ziplistPush(unsigned char *zl, unsigned char *s, unsigned int slen, int where);  //像列表中推入数据
unsigned char *ziplistIndex(unsigned char *zl, int index);   //索引定位到列表的某个位置
unsigned char *ziplistNext(unsigned char *zl, unsigned char *p);   //获取当前列表位置的下一个值
unsigned char *ziplistPrev(unsigned char *zl, unsigned char *p);   //获取当期列表位置的前一个值
unsigned int ziplistGet(unsigned char *p, unsigned char **sval, unsigned int *slen, long long *lval);   //获取列表的信息
unsigned char *ziplistInsert(unsigned char *zl, unsigned char *p, unsigned char *s, unsigned int slen); //向列表中插入数据
unsigned char *ziplistDelete(unsigned char *zl, unsigned char **p); //列表中删除某个结点
unsigned char *ziplistDeleteRange(unsigned char *zl, unsigned int index, unsigned int num);   //从index索引对应的结点开始算起，删除num个结点
unsigned int ziplistCompare(unsigned char *p, unsigned char *s, unsigned int slen);   //列表间的比较方法
unsigned char *ziplistFind(unsigned char *p, unsigned char *vstr, unsigned int vlen, unsigned int skip); //在列表中寻找某个结点
unsigned int ziplistLen(unsigned char *zl);   //返回列表的长度
size_t ziplistBlobLen(unsigned char *zl);   //返回列表的二进制长度，返回的是字节数
