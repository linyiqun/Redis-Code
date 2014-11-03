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

/* Exported API */
void bioInit(void); /* background I/O初始化操作 */
void bioCreateBackgroundJob(int type, void *arg1, void *arg2, void *arg3); /* 创建后台job,通过传入的3个参数初始化 */
unsigned long long bioPendingJobsOfType(int type); /* 返回type类型的job正在等待被执行的个数 */
void bioWaitPendingJobsLE(int type, unsigned long long num); /* 返回type类型的job正在等待被执行的个数 */
time_t bioOlderJobOfType(int type); 
void bioKillThreads(void); /* 杀死后台所有线程 */

/* Background job opcodes */
/* 定义了2种后台工作的类别 */
#define REDIS_BIO_CLOSE_FILE    0 /* Deferred close(2) syscall.文件的关闭 */
#define REDIS_BIO_AOF_FSYNC     1 /* Deferred AOF fsync.AOF文件的同步 */ 
/* BIO后台操作类型总数为2个 */
#define REDIS_BIO_NUM_OPS       2
