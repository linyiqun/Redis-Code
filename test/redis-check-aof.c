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

#include "fmacros.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "config.h"

/* 宏定义错误的方法 */
#define ERROR(...) { \
    char __buf[1024]; \
    sprintf(__buf, __VA_ARGS__); \
    //输出错误的位置，和错误的内容
    sprintf(error, "0x%16llx: %s", (long long)epos, __buf); \
}

static char error[1024];
static off_t epos;

/* 方法API */
int consumeNewline(char *buf) /* 消除buf前面的换行符，即比较buf字符串中的前2个字符 */
int readLong(FILE *fp, char prefix, long *target) /* 从文件中读取long类型值 */
int readBytes(FILE *fp, char *target, long length) /* 从文件中读取字节 */
int readString(FILE *fp, char** target) /* 文件中读取字符串 */
int readArgc(FILE *fp, long *target) /* 文件中读取参数,首字符以“*”开头 */
off_t process(FILE *fp) /* 返回fp文件的偏移量 */
	
/* 消除buf前面的换行符，即比较buf字符串中的前2个字符 */
int consumeNewline(char *buf) {
    if (strncmp(buf,"\r\n",2) != 0) {
    	//如果不是等于"\r\n"，则提示出错
        ERROR("Expected \\r\\n, got: %02x%02x",buf[0],buf[1]);
        return 0;
    }
    return 1;
}

/* 从文件中读取long类型值 */
int readLong(FILE *fp, char prefix, long *target) {
    char buf[128], *eptr;
    //定位到文件的读取位置
    epos = ftello(fp);
    //将文件中的内容读取到buf中
    if (fgets(buf,sizeof(buf),fp) == NULL) {
    	//如果为空直接返回
        return 0;
    }
    
    //如果读取到的首字符不等于预期值，则提示报错
    if (buf[0] != prefix) {
        ERROR("Expected prefix '%c', got: '%c'",buf[0],prefix);
        return 0;
    }
    
    //将字符串值转成long类型值
    *target = strtol(buf+1,&eptr,10);
    
    return consumeNewline(eptr);
}

/* 从文件中读取字节 */
int readBytes(FILE *fp, char *target, long length) {
    long real;
    //定位到文件的读取位置
    epos = ftello(fp);
    //将字节读取到Target字符串中,返回字符串的长度
    real = fread(target,1,length,fp);
    if (real != length) {
    	//如果长度与给定的预期长度不等，则提示出错
        ERROR("Expected to read %ld bytes, got %ld bytes",length,real);
        return 0;
    }
    return 1;
}

/* 文件中读取字符串 */
int readString(FILE *fp, char** target) {
    long len;
    *target = NULL;
    if (!readLong(fp,'$',&len)) {
        return 0;
    }

    /* Increase length to also consume \r\n */
    //增加长度，消除"\r\n"换行符
    len += 2;
    *target = (char*)malloc(len);
    if (!readBytes(fp,*target,len)) {
        return 0;
    }
    if (!consumeNewline(*target+len-2)) {
        return 0;
    }
    (*target)[len-2] = '\0';
    return 1;
}

/* 文件中读取参数,首字符以“*”开头 */
int readArgc(FILE *fp, long *target) {
    return readLong(fp,'*',target);
}

/* 先介绍off_t的概念，off_t的官方解释 */
/* This is a data type defined in the sys/types.h header file (of fundamental type unsigned long) and is used to measure the file offset in bytes from the beginning of the file. It is defined as a signed, 32-bit integer, */
/* but if the programming environment enables large files off_t is defined to be a signed, 64-bit integer */
/* 意思：off_t是被作为描述文件中的字节偏移量的，默认类型有符号32位，但是这个类型会因为系统环境改变而改变，有时会变为有符号类型64号 */
/* 返回fp文件的偏移量 */
off_t process(FILE *fp) {
    long argc;
    off_t pos = 0;
    int i, multi = 0;
    char *str;

    while(1) {
    	//如果multi为初始值，进行文件定位
        if (!multi) pos = ftello(fp);
        //从文件中读取参数数值，参数值为字符串长度
        if (!readArgc(fp, &argc)) break;

		//遍历文件中的逐个字符串
        for (i = 0; i < argc; i++) {
            if (!readString(fp,&str)) break;
            if (i == 0) {
                if (strcasecmp(str, "multi") == 0) {
                    if (multi++) {
                        ERROR("Unexpected MULTI");
                        break;
                    }
                } else if (strcasecmp(str, "exec") == 0) {
                    if (--multi) {
                        ERROR("Unexpected EXEC");
                        break;
                    }
                }
            }
            free(str);
        }

        /* Stop if the loop did not finish */
        /* 直接退出如果循环没有结束 */
        if (i < argc) {
            if (str) free(str);
            break;
        }
    }

    if (feof(fp) && multi && strlen(error) == 0) {
        ERROR("Reached EOF before reading EXEC for MULTI");
    }
    if (strlen(error) > 0) {
        printf("%s\n", error);
    }
    return pos;
}

/* 测试程序 */
int main(int argc, char **argv) {
    char *filename;
    int fix = 0;

    if (argc < 2) {
        printf("Usage: %s [--fix] <file.aof>\n", argv[0]);
        exit(1);
    } else if (argc == 2) {
        filename = argv[1];
    } else if (argc == 3) {
        if (strcmp(argv[1],"--fix") != 0) {
            printf("Invalid argument: %s\n", argv[1]);
            exit(1);
        }
        filename = argv[2];
        fix = 1;
    } else {
        printf("Invalid arguments\n");
        exit(1);
    }

    FILE *fp = fopen(filename,"r+");
    if (fp == NULL) {
        printf("Cannot open file: %s\n", filename);
        exit(1);
    }

    struct redis_stat sb;
    if (redis_fstat(fileno(fp),&sb) == -1) {
        printf("Cannot stat file: %s\n", filename);
        exit(1);
    }

    off_t size = sb.st_size;
    if (size == 0) {
        printf("Empty file: %s\n", filename);
        exit(1);
    }

    off_t pos = process(fp);
    //diff为剩余的空间
    off_t diff = size-pos;
    printf("AOF analyzed: size=%lld, ok_up_to=%lld, diff=%lld\n",
        (long long) size, (long long) pos, (long long) diff);
    if (diff > 0) {
        if (fix) {
            char buf[2];
            printf("This will shrink the AOF from %lld bytes, with %lld bytes, to %lld bytes\n",(long long)size,(long long)diff,(long long)pos);
            printf("Continue? [y/N]: ");
            if (fgets(buf,sizeof(buf),stdin) == NULL ||
                strncasecmp(buf,"y",1) != 0) {
                    printf("Aborting...\n");
                    exit(1);
            }
            
            //截断文件的操作，从问价头部到后面的偏移量，没有用的空间截去
            if (ftruncate(fileno(fp), pos) == -1) {
                printf("Failed to truncate AOF\n");
                exit(1);
            } else {
                printf("Successfully truncated AOF\n");
            }
        } else {
            printf("AOF is not valid\n");
            exit(1);
        }
    } else {
        printf("AOF is valid\n");
    }

    fclose(fp);
    return 0;
}
