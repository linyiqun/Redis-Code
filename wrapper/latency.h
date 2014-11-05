/* latency.h -- latency monitor API header file
 * See latency.c for more information.
 *
 * ----------------------------------------------------------------------------
 *
 * Copyright (c) 2014, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __LATENCY_H
#define __LATENCY_H

#define LATENCY_TS_LEN 160 /* History length for every monitored event. */

/* Representation of a latency sample: the sampling time and the latency
 * observed in milliseconds. */
/* 延时样品例子 */
struct latencySample {
	//延时Sample创建的时间
    int32_t time; /* We don't use time_t to force 4 bytes usage everywhere. */
    //延时的具体时间， 单位为毫秒
    uint32_t latency; /* Latency in milliseconds. */
};

/* The latency time series for a given event. */
/* 针对某个事件采集的一系列延时sample */
struct latencyTimeSeries {
	//下一个延时Sample的下标
    int idx; /* Index of the next sample to store. */
    //最大的延时
    uint32_t max; /* Max latency observed for this event. */
    //最近的延时记录
    struct latencySample samples[LATENCY_TS_LEN]; /* Latest history. */
};

/* Latency statistics structure. */
/* 延时sample的数据统计结果结构体 */
struct latencyStats {
	//绝对最高的延时时间
    uint32_t all_time_high; /* Absolute max observed since latest reset. */
    //平均Sample延时时间
    uint32_t avg;           /* Average of current samples. */
    //Sample的最小延时时间
    uint32_t min;           /* Min of current samples. */
    //Sample的最大延时时间
    uint32_t max;           /* Max of current samples. */
    //平均相对误差，与平均延时相比
    uint32_t mad;           /* Mean absolute deviation. */
    //samples的总数
    uint32_t samples;       /* Number of non-zero samples. */
    //最早的延时记录点的创建时间
    time_t period;          /* Number of seconds since first event and now. */
};

void latencyMonitorInit(void); /* 延时监听初始化操作，创建Event字典对象 */
void latencyAddSample(char *event, mstime_t latency); /* 添加Sample到指定的Event对象的Sample列表中 */

/* Latency monitoring macros. */

/* Start monitoring an event. We just set the current time. */
/* 对某个事件设置监听，就是设置一下当前的时间 */
#define latencyStartMonitor(var) if (server.latency_monitor_threshold) { \
    var = mstime(); \
} else { \
    var = 0; \
}

/* End monitoring an event, compute the difference with the current time
 * to check the amount of time elapsed. */
/* 结束监听，算出过了多少时间 */
#define latencyEndMonitor(var) if (server.latency_monitor_threshold) { \
    var = mstime() - var; \
}

/* Add the sample only if the elapsed time is >= to the configured threshold. */
/* 如果延时时间超出server.latency_monitor_threshold，则将Sample加入延时列表中 */
#define latencyAddSampleIfNeeded(event,var) \
    if (server.latency_monitor_threshold && \
        (var) >= server.latency_monitor_threshold) \
          latencyAddSample((event),(var));

#endif /* __LATENCY_H */
