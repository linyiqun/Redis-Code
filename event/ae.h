/* A simple event-driven programming library. Originally I wrote this code
 * for the Jim's event-loop (Jim is a Tcl interpreter) but later translated
 * it in form of a library for easy reuse.
 *
 * ae是作者写的一个简单的事件驱动库，后面进行了转化，变得更为简单的复用
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __AE_H__
#define __AE_H__

#define AE_OK 0
#define AE_ERR -1

#define AE_NONE 0
#define AE_READABLE 1
#define AE_WRITABLE 2

#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_ALL_EVENTS (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_DONT_WAIT 4

#define AE_NOMORE -1

/* Macros */
#define AE_NOTUSED(V) ((void) V)

struct aeEventLoop;

/* Types and data structures */
/* 定义了一些方法 */
typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);
typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);
typedef void aeEventFinalizerProc(struct aeEventLoop *eventLoop, void *clientData);
typedef void aeBeforeSleepProc(struct aeEventLoop *eventLoop);

/* File event structure */
/* 文件事件结构体 */
typedef struct aeFileEvent {
	//只为读事件或者写事件中的1种
    int mask; /* one of AE_(READABLE|WRITABLE) */
    //读方法
    aeFileProc *rfileProc;
    //写方法
    aeFileProc *wfileProc;
    //客户端数据
    void *clientData;
} aeFileEvent;

/* Time event structure */
/* 时间事件结构体 */
typedef struct aeTimeEvent {
	//时间事件id
    long long id; /* time event identifier. */
    //时间秒数
    long when_sec; /* seconds */
    //时间毫秒
    long when_ms; /* milliseconds */
    //时间事件中的处理函数
    aeTimeProc *timeProc;
    //被删除的时候将会调用的方法
    aeEventFinalizerProc *finalizerProc;
    //客户端数据
    void *clientData;
    //时间结构体内的下一个结构体
    struct aeTimeEvent *next;
} aeTimeEvent;

/* A fired event */
/* fired结构体，用来表示将要被处理的文件事件 */
typedef struct aeFiredEvent {
	//文件描述符id
    int fd;
    int mask;
} aeFiredEvent;

/* State of an event based program */
typedef struct aeEventLoop {
	//目前创建的最高的文件描述符
    int maxfd;   /* highest file descriptor currently registered */
    int setsize; /* max number of file descriptors tracked */
    //下一个时间事件id
    long long timeEventNextId;
    time_t lastTime;     /* Used to detect system clock skew */
    //3种事件类型
    aeFileEvent *events; /* Registered events */
    aeFiredEvent *fired; /* Fired events */
    aeTimeEvent *timeEventHead;
    //事件停止标志符
    int stop;
    //这里存放的是event API的数据，包括epoll，select等事件
    void *apidata; /* This is used for polling API specific data */
    aeBeforeSleepProc *beforesleep;
} aeEventLoop;

/*
	 epoll是Linux(内核版本2.6及以上支持)下多路复用IO接口select/poll的增强版本，
	 它能显著提高程序在大量并发连接中只有少量活跃的情况下的系统CPU利用率，
	 因为它会复用文件描述符集合来传递结果而不用迫使开发者每次等待事件之前都必
	 须重新准备要被侦听的文件描述符集合，另一点原因就是获取事件的时候，它无须
	 遍历整个被侦听的描述符集，只要遍历那些被内核IO事件异步唤醒而加入Ready队
	 列的描述符集合就行了。

    Linux下select 模型和epoll模型区别：
    假设你在大学读书，住的宿舍楼有很多间房间，你的朋友要来找你。 select版宿
    管大妈就会带着你的朋友挨个房间去找，直到找到你为止。而 epoll版宿管大妈会
    先记下每位同学的房间号，你的朋友来时，只需告诉你的朋友你住在哪个房间即可
    ，不用亲自带着你的朋友满大楼找人。如果来了 10000个人，都要找自己住这栋楼
    的同学时， select版和epoll 版宿管大妈，谁的效率更高，不言自明。同理，在高
    并发服务器中，轮询 I/O是最耗时间的操作之一， select和epoll 的性能谁的性能
    更高，同样十分明了。*/
    /* select事件的模型 */
   /*（1）创建所关注的事件的描述符集合（fd_set)，对于一个描述符，
   		可以关注其上面的读(read)、写(write)、异常(exception)事件，
   		所以通常，要创建三个fd_set， 一个用来收集关注读事件的描述
   		符，一个用来收集关注写事件的描述符，另外一个用来收集关注
   		异常事件的描述符集合。
   	（2）轮询所有fd_set中的每一个fd ，检查是否有相应的事件发生，如果有，就进行处理。*/
   	
   	/* poll和上面的区别是可以复用文件描述符，上面对一个文件需要轮询3个文件描述符集合，而epoll只需要一个，效率更高 */
   	/* epoll是poll的升级版本，把描述符列表交给内核，一旦有事件发生，内核把发生事件的描述符列表通知给进程，这样就避
   		免了轮询整个描述符列表。效率极大提高 */	
   	/*kqueue 是 FreeBSD 上的一种的多路复用机制
   	   kqueue是针对传统的 select/poll 处理大量的文件描述符性能较低效而开发出来的。注册一批描述符到 kqueue 以后，当其中
   		的描述符状态发生变化时，kqueue 将一次性通知应用程序哪些描述符可读、可写或出错了。*/
	/* evport将某一个对象的特定 event 与 Event port 相关联 */
/* Prototypes */
aeEventLoop *aeCreateEventLoop(int setsize); /* 创建aeEventLoop，内部的fileEvent和Fired事件的个数为setSize个 */
void aeDeleteEventLoop(aeEventLoop *eventLoop); /* 删除EventLoop，释放相应的事件所占的空间 */
void aeStop(aeEventLoop *eventLoop); /* 设置eventLoop中的停止属性为1 */
int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask,
        aeFileProc *proc, void *clientData); /* 在eventLoop中创建文件事件 */
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask); /* 删除文件事件 */
int aeGetFileEvents(aeEventLoop *eventLoop, int fd); //根据文件描述符id，找出文件的属性，是读事件还是写事件
long long aeCreateTimeEvent(aeEventLoop *eventLoop, long long milliseconds,
        aeTimeProc *proc, void *clientData,
        aeEventFinalizerProc *finalizerProc); /* 在eventLoop中添加时间事件，创建的时间为当前时间加上自己传入的时间 */
int aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id); //根据时间id，删除时间事件，涉及链表的操作
int aeProcessEvents(aeEventLoop *eventLoop, int flags); /* 处理eventLoop中的所有类型事件 */
int aeWait(int fd, int mask, long long milliseconds); /* 让某事件等待 */
void aeMain(aeEventLoop *eventLoop); /* ae事件执行主程序 */
char *aeGetApiName(void);
void aeSetBeforeSleepProc(aeEventLoop *eventLoop, aeBeforeSleepProc *beforesleep); /* 每次eventLoop事件执行完后又重新开始执行时调用 */
int aeGetSetSize(aeEventLoop *eventLoop); /* 获取eventLoop的大小 */
int aeResizeSetSize(aeEventLoop *eventLoop, int setsize); /* EventLoop重新调整大小 */

#endif
