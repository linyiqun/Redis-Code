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

#define SLOWLOG_ENTRY_MAX_ARGC 32
#define SLOWLOG_ENTRY_MAX_STRING 128

/* This structure defines an entry inside the slow log list */
/* 慢日志结构体，将会插入到slowLogList,慢日志列表中 */
typedef struct slowlogEntry {
    robj **argv;
    int argc;
    //自身的id标识
    long long id;       /* Unique entry identifier. */
    //query操作所消耗的时间，单位为nanoseconds
    long long duration; /* Time spent by the query, in nanoseconds. */
    //查询发发生的时间
    time_t time;        /* Unix time at which the query was executed. */
} slowlogEntry;

/* Exported API */
void slowlogInit(void); /* slowlog初始化操作 */
void slowlogPushEntryIfNeeded(robj **argv, int argc, long long duration); /* slowLogEntry压入列表操作 */

/* Exported commands */
/* 开放给系统使用的命令 */
void slowlogCommand(redisClient *c);
