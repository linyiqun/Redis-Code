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

#include "redis.h"
#include <math.h>

/* 下面是方法的归类 */
void hashTypeTryConversion(robj *o, robj **argv, int start, int end) /* 当hashType为ziplist时，判断对象长度是否超出了服务端可接受的ziplist最大长度，超过则转成哈希字典类型*/
void hashTypeTryObjectEncoding(robj *subject, robj **o1, robj **o2) /* 当robj用的是字典的编码方式的时候，则经过编码转换 */
int hashTypeGetFromZiplist(robj *o, robj *field,unsigned char **vstr,unsigned int *vlen,long long *vll) /* 获取ziplist压缩列表中的某个索引位置上的值 */
int hashTypeGetFromHashTable(robj *o, robj *field, robj **value) /* 获取哈希字典中的某个值 */
robj *hashTypeGetObject(robj *o, robj *field) /* 获取某个key对应的对象类型 */
int hashTypeExists(robj *o, robj *field)   /* hastType类型判断某个键是否存在 */
int hashTypeSet(robj *o, robj *field, robj *value) /* hashType设置操作，分2种情况，ziplist,和字典hashtable */
int hashTypeDelete(robj *o, robj *field)  /* hashType删除操作，分为ziplist的删除操作，和hashtable的删除操作 */
unsigned long hashTypeLength(robj *o)   /* hashType求长度操作 */
hashTypeIterator *hashTypeInitIterator(robj *subject)  /* 获取hashType迭代器 */
void hashTypeReleaseIterator(hashTypeIterator *hi) /* 释放hashType迭代器 */
int hashTypeNext(hashTypeIterator *hi) /* 通过hashType迭代器获取下一个元素 */
void hashTypeCurrentFromZiplist(hashTypeIterator *hi, int what,unsigned char **vstr,unsigned int *vlen,long long *vll) /* 根据当前迭代器的位置，获取当前ziplist的所在位置的key位置，或value该位置上的值 */
void hashTypeCurrentFromHashTable(hashTypeIterator *hi, int what, robj **dst) /* 根据当前迭代器的位置，获取当前dict的所在位置的key位置，或value该位置上的值 */
robj *hashTypeCurrentObject(hashTypeIterator *hi, int what) /* 根据当前迭代器的位置，获取当前key对象 */
robj *hashTypeLookupWriteOrCreate(redisClient *c, robj *key) /* 根据c客户端对象，找到key是否存在，创建或实现添加操作  */
void hashTypeConvertZiplist(robj *o, int enc) /* 从ziplist压缩表到hashtable的转换 */
void hashTypeConvert(robj *o, int enc) /* 对象转换操作，例如从ziplist到dict的转换 */

/* 哈希命令类型 */
void hsetCommand(redisClient *c)  /* 客户端设置指令 */
void hsetnxCommand(redisClient *c) /* 客户端设置下一个位置指令 */
void hmsetCommand(redisClient *c) /* 客户单设置命令，如果没有key,还有后续操作 */
void hincrbyCommand(redisClient *c) /* 客户端添加value值操作 */
void hincrbyfloatCommand(redisClient *c) /* 客户端添加float类型value值操作 */
static void addHashFieldToReply(redisClient *c, robj *o, robj *field) /*  */
void hgetCommand(redisClient *c) /* 客户端获取操作，如果没找到，直接不做任何操作 */
void hmgetCommand(redisClient *c) /* 客户端获取key操作，如果为空，会返回一些了NULL值 */
void hdelCommand(redisClient *c) /* 客户端删除操作 */
void hlenCommand(redisClient *c) /* 客户端求长度命令 */
static void addHashIteratorCursorToReply(redisClient *c, hashTypeIterator *hi, int what) /* 客户端添加hashType迭代器操作 */
void genericHgetallCommand(redisClient *c, int flags) /* 客户端获取操作原始方法，可以添加flag参数 */
void hkeysCommand(redisClient *c) /* 客户端获取key值命令 */
void hvalsCommand(redisClient *c) /* 客户端获取val值命令 */
void hgetallCommand(redisClient *c) /* 客户端获取key;value 2个值都获取 */
void hexistsCommand(redisClient *c) /* 客户端判断记录是否存在操作 */
void hscanCommand(redisClient *c) /* 客户端扫描操作 */

/*-----------------------------------------------------------------------------
 * Hash type API
 *----------------------------------------------------------------------------*/
 *
/* Check the length of a number of objects to see if we need to convert a
 * ziplist to a real hash. Note that we only check string encoded objects
 * as their string length can be queried in constant time. */
/* 当hashType为ziplist时，判断对象长度是否超出了服务端可接受的ziplist最大长度，超过则转成哈希字典类型*/
void hashTypeTryConversion(robj *o, robj **argv, int start, int end) {
    int i;

    if (o->encoding != REDIS_ENCODING_ZIPLIST) return;

    for (i = start; i <= end; i++) {
        if (argv[i]->encoding == REDIS_ENCODING_RAW &&
            sdslen(argv[i]->ptr) > server.hash_max_ziplist_value)
        {
        	//判断对象长度是否超出了服务端可接受的ziplist最大长度，超过则转成哈希字典类型
            hashTypeConvert(o, REDIS_ENCODING_HT);
            break;
        }
    }
}

/* Encode given objects in-place when the hash uses a dict. */
/* 当robj用的是字典的编码方式的时候，则经过编码转换 */
void hashTypeTryObjectEncoding(robj *subject, robj **o1, robj **o2) {
    if (subject->encoding == REDIS_ENCODING_HT) {
        if (o1) *o1 = tryObjectEncoding(*o1);
        if (o2) *o2 = tryObjectEncoding(*o2);
    }
}

/* Get the value from a ziplist encoded hash, identified by field.
 * Returns -1 when the field cannot be found. */
/* 获取ziplist压缩列表中的某个索引位置上的值 */
int hashTypeGetFromZiplist(robj *o, robj *field,
                           unsigned char **vstr,
                           unsigned int *vlen,
                           long long *vll)
{
    unsigned char *zl, *fptr = NULL, *vptr = NULL;
    int ret;

    redisAssert(o->encoding == REDIS_ENCODING_ZIPLIST);

    field = getDecodedObject(field);

    zl = o->ptr;
    fptr = ziplistIndex(zl, ZIPLIST_HEAD);
    if (fptr != NULL) {
    	//找到索引位置
        fptr = ziplistFind(fptr, field->ptr, sdslen(field->ptr), 1);
        if (fptr != NULL) {
            /* Grab pointer to the value (fptr points to the field) */
            //指向其中的value值的位置
            vptr = ziplistNext(zl, fptr);
            redisAssert(vptr != NULL);
        }
    }
    
    //field用完之后减少一个引用计数 
    decrRefCount(field);

    if (vptr != NULL) {
    	//获取该值
        ret = ziplistGet(vptr, vstr, vlen, vll);
        redisAssert(ret);
        return 0;
    }

    return -1;
}

/* Get the value from a hash table encoded hash, identified by field.
 * Returns -1 when the field cannot be found. */
/* 获取哈希字典中的某个值 */
int hashTypeGetFromHashTable(robj *o, robj *field, robj **value) {
    dictEntry *de;

    redisAssert(o->encoding == REDIS_ENCODING_HT);
    
    //通过robj->ptr里面存的dict总类或ziplist类开始寻找
    de = dictFind(o->ptr, field);
    if (de == NULL) return -1;
    //获取其中的value值
    *value = dictGetVal(de);
    return 0;
}

/* Higher level function of hashTypeGet*() that always returns a Redis
 * object (either new or with refcount incremented), so that the caller
 * can retain a reference or call decrRefCount after the usage.
 *
 * The lower level function can prevent copy on write so it is
 * the preferred way of doing read operations. */
/* 获取某个key的对象 */
robj *hashTypeGetObject(robj *o, robj *field) {
    robj *value = NULL;

    if (o->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *vstr = NULL;
        unsigned int vlen = UINT_MAX;
        long long vll = LLONG_MAX;

        if (hashTypeGetFromZiplist(o, field, &vstr, &vlen, &vll) == 0) {
        	//在ziplist中获取值
            if (vstr) {
                value = createStringObject((char*)vstr, vlen);
            } else {
                value = createStringObjectFromLongLong(vll);
            }
        }

    } else if (o->encoding == REDIS_ENCODING_HT) {
        robj *aux;

        if (hashTypeGetFromHashTable(o, field, &aux) == 0) {
        	//对象被引用了，计数递增
            incrRefCount(aux);
            value = aux;
        }
    } else {
        redisPanic("Unknown hash encoding");
    }
    return value;
}

/* Test if the specified field exists in the given hash. Returns 1 if the field
 * exists, and 0 when it doesn't. */
int hashTypeExists(robj *o, robj *field) {
    if (o->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *vstr = NULL;
        unsigned int vlen = UINT_MAX;
        long long vll = LLONG_MAX;

        if (hashTypeGetFromZiplist(o, field, &vstr, &vlen, &vll) == 0) return 1;
    } else if (o->encoding == REDIS_ENCODING_HT) {
        robj *aux;

        if (hashTypeGetFromHashTable(o, field, &aux) == 0) return 1;
    } else {
        redisPanic("Unknown hash encoding");
    }
    return 0;
}

/* Add an element, discard the old if the key already exists.
 * Return 0 on insert and 1 on update.
 * This function will take care of incrementing the reference count of the
 * retained fields and value objects. */
/* hashType设置操作，分2种情况，ziplist,和字典hashtable */
int hashTypeSet(robj *o, robj *field, robj *value) {
    int update = 0;

    if (o->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *zl, *fptr, *vptr;
        
        //首先对field和value进行解码
        field = getDecodedObject(field);
        value = getDecodedObject(value);

        zl = o->ptr;
        fptr = ziplistIndex(zl, ZIPLIST_HEAD);
        if (fptr != NULL) {
            fptr = ziplistFind(fptr, field->ptr, sdslen(field->ptr), 1);
            if (fptr != NULL) {
                /* Grab pointer to the value (fptr points to the field) */
                vptr = ziplistNext(zl, fptr);
                redisAssert(vptr != NULL);
                update = 1;

                //设置的操作，其实先删除，再插入语一个新值
                /* Delete value */
                zl = ziplistDelete(zl, &vptr);

                /* Insert new value */
                zl = ziplistInsert(zl, vptr, value->ptr, sdslen(value->ptr));
            }
        }

        if (!update) {
            /* Push new field/value pair onto the tail of the ziplist */
            zl = ziplistPush(zl, field->ptr, sdslen(field->ptr), ZIPLIST_TAIL);
            zl = ziplistPush(zl, value->ptr, sdslen(value->ptr), ZIPLIST_TAIL);
        }
        o->ptr = zl;
        //用完之后，引用计数递减
        decrRefCount(field);
        decrRefCount(value);

        /* Check if the ziplist needs to be converted to a hash table */
        if (hashTypeLength(o) > server.hash_max_ziplist_entries)
            hashTypeConvert(o, REDIS_ENCODING_HT);
    } else if (o->encoding == REDIS_ENCODING_HT) {
    	//如果是字典，直接替换
        if (dictReplace(o->ptr, field, value)) { /* Insert */
            incrRefCount(field);
        } else { /* Update */
            update = 1;
        }
        //用完之后，引用计数递减
        incrRefCount(value);
    } else {
        redisPanic("Unknown hash encoding");
    }
    return update;
}

/* Delete an element from a hash.
 * Return 1 on deleted and 0 on not found. */
int hashTypeDelete(robj *o, robj *field) {
    int deleted = 0;

    if (o->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *zl, *fptr;

        field = getDecodedObject(field);

        zl = o->ptr;
        fptr = ziplistIndex(zl, ZIPLIST_HEAD);
        if (fptr != NULL) {
            fptr = ziplistFind(fptr, field->ptr, sdslen(field->ptr), 1);
            if (fptr != NULL) {
                zl = ziplistDelete(zl,&fptr);
                zl = ziplistDelete(zl,&fptr);
                o->ptr = zl;
                deleted = 1;
            }
        }

        decrRefCount(field);

    } else if (o->encoding == REDIS_ENCODING_HT) {
        if (dictDelete((dict*)o->ptr, field) == REDIS_OK) {
            deleted = 1;

            /* Always check if the dictionary needs a resize after a delete. */
            if (htNeedsResize(o->ptr)) dictResize(o->ptr);
        }

    } else {
        redisPanic("Unknown hash encoding");
    }

    return deleted;
}

/* Return the number of elements in a hash. */
unsigned long hashTypeLength(robj *o) {
    unsigned long length = ULONG_MAX;

    if (o->encoding == REDIS_ENCODING_ZIPLIST) {
        length = ziplistLen(o->ptr) / 2;
    } else if (o->encoding == REDIS_ENCODING_HT) {
        length = dictSize((dict*)o->ptr);
    } else {
        redisPanic("Unknown hash encoding");
    }

    return length;
}

/* 获取hashType迭代器 */
hashTypeIterator *hashTypeInitIterator(robj *subject) {
    hashTypeIterator *hi = zmalloc(sizeof(hashTypeIterator));
    hi->subject = subject;
    hi->encoding = subject->encoding;

    if (hi->encoding == REDIS_ENCODING_ZIPLIST) {
        hi->fptr = NULL;
        hi->vptr = NULL;
    } else if (hi->encoding == REDIS_ENCODING_HT) {
    	//实际主要调用的方法是获取字典迭代器
        hi->di = dictGetIterator(subject->ptr);
    } else {
        redisPanic("Unknown hash encoding");
    }

    return hi;
}

void hashTypeReleaseIterator(hashTypeIterator *hi) {
    if (hi->encoding == REDIS_ENCODING_HT) {
        dictReleaseIterator(hi->di);
    }

    zfree(hi);
}

/* Move to the next entry in the hash. Return REDIS_OK when the next entry
 * could be found and REDIS_ERR when the iterator reaches the end. */
int hashTypeNext(hashTypeIterator *hi) {
    if (hi->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *zl;
        unsigned char *fptr, *vptr;

        zl = hi->subject->ptr;
        fptr = hi->fptr;
        vptr = hi->vptr;

        if (fptr == NULL) {
            /* Initialize cursor */
            redisAssert(vptr == NULL);
            fptr = ziplistIndex(zl, 0);
        } else {
            /* Advance cursor */
            redisAssert(vptr != NULL);
            fptr = ziplistNext(zl, vptr);
        }
        if (fptr == NULL) return REDIS_ERR;

        /* Grab pointer to the value (fptr points to the field) */
        vptr = ziplistNext(zl, fptr);
        redisAssert(vptr != NULL);

        /* fptr, vptr now point to the first or next pair */
        hi->fptr = fptr;
        hi->vptr = vptr;
    } else if (hi->encoding == REDIS_ENCODING_HT) {
        if ((hi->de = dictNext(hi->di)) == NULL) return REDIS_ERR;
    } else {
        redisPanic("Unknown hash encoding");
    }
    return REDIS_OK;
}

/* Get the field or value at iterator cursor, for an iterator on a hash value
 * encoded as a ziplist. Prototype is similar to `hashTypeGetFromZiplist`. */
/* 根据当前迭代器的位置，获取当前ziplist的所在位置的key位置，或value该位置上的值 */
void hashTypeCurrentFromZiplist(hashTypeIterator *hi, int what,
                                unsigned char **vstr,
                                unsigned int *vlen,
                                long long *vll)
{
    int ret;

    redisAssert(hi->encoding == REDIS_ENCODING_ZIPLIST);

    if (what & REDIS_HASH_KEY) {
    	//ziplist获取当前key的位置，通过的是hi->fptr指针,key在ziplist其实是index位置的意思，在dict就是我们说的key
        ret = ziplistGet(hi->fptr, vstr, vlen, vll);
        redisAssert(ret);
    } else {
    	//ziplist获取当前value的值，通过的是hi->vptr指针
        ret = ziplistGet(hi->vptr, vstr, vlen, vll);
        redisAssert(ret);
    }
}

/* Get the field or value at iterator cursor, for an iterator on a hash value
 * encoded as a ziplist. Prototype is similar to `hashTypeGetFromHashTable`. */
void hashTypeCurrentFromHashTable(hashTypeIterator *hi, int what, robj **dst) {
    redisAssert(hi->encoding == REDIS_ENCODING_HT);

    if (what & REDIS_HASH_KEY) {
        *dst = dictGetKey(hi->de);
    } else {
        *dst = dictGetVal(hi->de);
    }
}

/* A non copy-on-write friendly but higher level version of hashTypeCurrent*()
 * that returns an object with incremented refcount (or a new object). It is up
 * to the caller to decrRefCount() the object if no reference is retained. */
robj *hashTypeCurrentObject(hashTypeIterator *hi, int what) {
    robj *dst;

    if (hi->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *vstr = NULL;
        unsigned int vlen = UINT_MAX;
        long long vll = LLONG_MAX;

        hashTypeCurrentFromZiplist(hi, what, &vstr, &vlen, &vll);
        if (vstr) {
            dst = createStringObject((char*)vstr, vlen);
        } else {
            dst = createStringObjectFromLongLong(vll);
        }

    } else if (hi->encoding == REDIS_ENCODING_HT) {
        hashTypeCurrentFromHashTable(hi, what, &dst);
        incrRefCount(dst);

    } else {
        redisPanic("Unknown hash encoding");
    }

    return dst;
}

robj *hashTypeLookupWriteOrCreate(redisClient *c, robj *key) {
    robj *o = lookupKeyWrite(c->db,key);
    if (o == NULL) {
        o = createHashObject();
        dbAdd(c->db,key,o);
    } else {
        if (o->type != REDIS_HASH) {
            addReply(c,shared.wrongtypeerr);
            return NULL;
        }
    }
    return o;
}

/* 从ziplist压缩表到hashtable的转换 */
void hashTypeConvertZiplist(robj *o, int enc) {
    redisAssert(o->encoding == REDIS_ENCODING_ZIPLIST);

    if (enc == REDIS_ENCODING_ZIPLIST) {
    	//如果转换方法是ziplist则什么都不做，因为原本传来的就是ziplist
        /* Nothing to do... */

    } else if (enc == REDIS_ENCODING_HT) {
    	//如果目标是转hashtable，则进行转换
        hashTypeIterator *hi;   //迭代器
        dict *dict;
        int ret;
        
        //根据robj变量获取迭代器
        hi = hashTypeInitIterator(o);
        //创建空哈希字典总类
        dict = dictCreate(&hashDictType, NULL);

		//根据迭代器遍历数据
        while (hashTypeNext(hi) != REDIS_ERR) {
            robj *field, *value;
            
            //获取key，value值
            field = hashTypeCurrentObject(hi, REDIS_HASH_KEY);
            //获取的值域都要进行编码
            field = tryObjectEncoding(field);
            value = hashTypeCurrentObject(hi, REDIS_HASH_VALUE);
            value = tryObjectEncoding(value);
            
            //添加键值
            ret = dictAdd(dict, field, value);
            if (ret != DICT_OK) {
            	//ret判断是否添加成功
                redisLogHexDump(REDIS_WARNING,"ziplist with dup elements dump",
                    o->ptr,ziplistBlobLen(o->ptr));
                redisAssert(ret == DICT_OK);
            }
        }
        
        //释放变量
        hashTypeReleaseIterator(hi);
        zfree(o->ptr);

	    //改变编码方式为哈希表
        o->encoding = REDIS_ENCODING_HT;
        o->ptr = dict;

    } else {
        redisPanic("Unknown hash encoding");
    }
}

void hashTypeConvert(robj *o, int enc) {
    if (o->encoding == REDIS_ENCODING_ZIPLIST) {
        hashTypeConvertZiplist(o, enc);
    } else if (o->encoding == REDIS_ENCODING_HT) {
        redisPanic("Not implemented");
    } else {
        redisPanic("Unknown hash encoding");
    }
}

/*-----------------------------------------------------------------------------
 * Hash type commands
 *----------------------------------------------------------------------------*/

/* hashType处理客户端的命令请求 */
void hsetCommand(redisClient *c) {
    int update;
    robj *o;

    if ((o = hashTypeLookupWriteOrCreate(c,c->argv[1])) == NULL) return;
    hashTypeTryConversion(o,c->argv,2,3);
    hashTypeTryObjectEncoding(o,&c->argv[2], &c->argv[3]);
    //命令的操作都是通过，客户端中的对象，和存在于里面的命令参数组成
    update = hashTypeSet(o,c->argv[2],c->argv[3]);
    //操作完添加回复
    addReply(c, update ? shared.czero : shared.cone);
    //发送通知表示命令执行完毕，预测者会触发窗口上的显示
    signalModifiedKey(c->db,c->argv[1]);
    notifyKeyspaceEvent(REDIS_NOTIFY_HASH,"hset",c->argv[1],c->db->id);
    
    //客户端命令执行成功，因为客户单此时的数据时最新的，服务端的脏数据就自然多了一个，
    server.dirty++;
}

void hsetnxCommand(redisClient *c) {
    robj *o;
    if ((o = hashTypeLookupWriteOrCreate(c,c->argv[1])) == NULL) return;
    hashTypeTryConversion(o,c->argv,2,3);

    if (hashTypeExists(o, c->argv[2])) {
        addReply(c, shared.czero);
    } else {
        hashTypeTryObjectEncoding(o,&c->argv[2], &c->argv[3]);
        hashTypeSet(o,c->argv[2],c->argv[3]);
        addReply(c, shared.cone);
        signalModifiedKey(c->db,c->argv[1]);
        notifyKeyspaceEvent(REDIS_NOTIFY_HASH,"hset",c->argv[1],c->db->id);
        server.dirty++;
    }
}

void hmsetCommand(redisClient *c) {
    int i;
    robj *o;

    if ((c->argc % 2) == 1) {
        addReplyError(c,"wrong number of arguments for HMSET");
        return;
    }

    if ((o = hashTypeLookupWriteOrCreate(c,c->argv[1])) == NULL) return;
    hashTypeTryConversion(o,c->argv,2,c->argc-1);
    for (i = 2; i < c->argc; i += 2) {
        hashTypeTryObjectEncoding(o,&c->argv[i], &c->argv[i+1]);
        hashTypeSet(o,c->argv[i],c->argv[i+1]);
    }
    addReply(c, shared.ok);
    signalModifiedKey(c->db,c->argv[1]);
    notifyKeyspaceEvent(REDIS_NOTIFY_HASH,"hset",c->argv[1],c->db->id);
    server.dirty++;
}

void hincrbyCommand(redisClient *c) {
    long long value, incr, oldvalue;
    robj *o, *current, *new;

    if (getLongLongFromObjectOrReply(c,c->argv[3],&incr,NULL) != REDIS_OK) return;
    if ((o = hashTypeLookupWriteOrCreate(c,c->argv[1])) == NULL) return;
    if ((current = hashTypeGetObject(o,c->argv[2])) != NULL) {
        if (getLongLongFromObjectOrReply(c,current,&value,
            "hash value is not an integer") != REDIS_OK) {
            decrRefCount(current);
            return;
        }
        decrRefCount(current);
    } else {
        value = 0;
    }

    oldvalue = value;
    if ((incr < 0 && oldvalue < 0 && incr < (LLONG_MIN-oldvalue)) ||
        (incr > 0 && oldvalue > 0 && incr > (LLONG_MAX-oldvalue))) {
        addReplyError(c,"increment or decrement would overflow");
        return;
    }
    value += incr;
    new = createStringObjectFromLongLong(value);
    hashTypeTryObjectEncoding(o,&c->argv[2],NULL);
    hashTypeSet(o,c->argv[2],new);
    decrRefCount(new);
    addReplyLongLong(c,value);
    signalModifiedKey(c->db,c->argv[1]);
    notifyKeyspaceEvent(REDIS_NOTIFY_HASH,"hincrby",c->argv[1],c->db->id);
    server.dirty++;
}

void hincrbyfloatCommand(redisClient *c) {
    double long value, incr;
    robj *o, *current, *new, *aux;

    if (getLongDoubleFromObjectOrReply(c,c->argv[3],&incr,NULL) != REDIS_OK) return;
    if ((o = hashTypeLookupWriteOrCreate(c,c->argv[1])) == NULL) return;
    if ((current = hashTypeGetObject(o,c->argv[2])) != NULL) {
        if (getLongDoubleFromObjectOrReply(c,current,&value,
            "hash value is not a valid float") != REDIS_OK) {
            decrRefCount(current);
            return;
        }
        decrRefCount(current);
    } else {
        value = 0;
    }

    value += incr;
    new = createStringObjectFromLongDouble(value);
    hashTypeTryObjectEncoding(o,&c->argv[2],NULL);
    hashTypeSet(o,c->argv[2],new);
    addReplyBulk(c,new);
    signalModifiedKey(c->db,c->argv[1]);
    notifyKeyspaceEvent(REDIS_NOTIFY_HASH,"hincrbyfloat",c->argv[1],c->db->id);
    server.dirty++;

    /* Always replicate HINCRBYFLOAT as an HSET command with the final value
     * in order to make sure that differences in float pricision or formatting
     * will not create differences in replicas or after an AOF restart. */
    aux = createStringObject("HSET",4);
    rewriteClientCommandArgument(c,0,aux);
    decrRefCount(aux);
    rewriteClientCommandArgument(c,3,new);
    decrRefCount(new);
}

static void addHashFieldToReply(redisClient *c, robj *o, robj *field) {
    int ret;

    if (o == NULL) {
        addReply(c, shared.nullbulk);
        return;
    }

    if (o->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *vstr = NULL;
        unsigned int vlen = UINT_MAX;
        long long vll = LLONG_MAX;

        ret = hashTypeGetFromZiplist(o, field, &vstr, &vlen, &vll);
        if (ret < 0) {
            addReply(c, shared.nullbulk);
        } else {
            if (vstr) {
                addReplyBulkCBuffer(c, vstr, vlen);
            } else {
                addReplyBulkLongLong(c, vll);
            }
        }

    } else if (o->encoding == REDIS_ENCODING_HT) {
        robj *value;

        ret = hashTypeGetFromHashTable(o, field, &value);
        if (ret < 0) {
            addReply(c, shared.nullbulk);
        } else {
            addReplyBulk(c, value);
        }

    } else {
        redisPanic("Unknown hash encoding");
    }
}

void hgetCommand(redisClient *c) {
    robj *o;

    if ((o = lookupKeyReadOrReply(c,c->argv[1],shared.nullbulk)) == NULL ||
        checkType(c,o,REDIS_HASH)) return;

    addHashFieldToReply(c, o, c->argv[2]);
}

void hmgetCommand(redisClient *c) {
    robj *o;
    int i;

    /* Don't abort when the key cannot be found. Non-existing keys are empty
     * hashes, where HMGET should respond with a series of null bulks. */
    o = lookupKeyRead(c->db, c->argv[1]);
    if (o != NULL && o->type != REDIS_HASH) {
        addReply(c, shared.wrongtypeerr);
        return;
    }

    addReplyMultiBulkLen(c, c->argc-2);
    for (i = 2; i < c->argc; i++) {
        addHashFieldToReply(c, o, c->argv[i]);
    }
}

void hdelCommand(redisClient *c) {
    robj *o;
    int j, deleted = 0, keyremoved = 0;

    if ((o = lookupKeyWriteOrReply(c,c->argv[1],shared.czero)) == NULL ||
        checkType(c,o,REDIS_HASH)) return;

    for (j = 2; j < c->argc; j++) {
        if (hashTypeDelete(o,c->argv[j])) {
            deleted++;
            if (hashTypeLength(o) == 0) {
                dbDelete(c->db,c->argv[1]);
                keyremoved = 1;
                break;
            }
        }
    }
    if (deleted) {
        signalModifiedKey(c->db,c->argv[1]);
        notifyKeyspaceEvent(REDIS_NOTIFY_HASH,"hdel",c->argv[1],c->db->id);
        if (keyremoved)
            notifyKeyspaceEvent(REDIS_NOTIFY_GENERIC,"del",c->argv[1],
                                c->db->id);
        server.dirty += deleted;
    }
    addReplyLongLong(c,deleted);
}

void hlenCommand(redisClient *c) {
    robj *o;
    if ((o = lookupKeyReadOrReply(c,c->argv[1],shared.czero)) == NULL ||
        checkType(c,o,REDIS_HASH)) return;

    addReplyLongLong(c,hashTypeLength(o));
}

static void addHashIteratorCursorToReply(redisClient *c, hashTypeIterator *hi, int what) {
    if (hi->encoding == REDIS_ENCODING_ZIPLIST) {
        unsigned char *vstr = NULL;
        unsigned int vlen = UINT_MAX;
        long long vll = LLONG_MAX;

        hashTypeCurrentFromZiplist(hi, what, &vstr, &vlen, &vll);
        if (vstr) {
            addReplyBulkCBuffer(c, vstr, vlen);
        } else {
            addReplyBulkLongLong(c, vll);
        }

    } else if (hi->encoding == REDIS_ENCODING_HT) {
        robj *value;

        hashTypeCurrentFromHashTable(hi, what, &value);
        addReplyBulk(c, value);

    } else {
        redisPanic("Unknown hash encoding");
    }
}

void genericHgetallCommand(redisClient *c, int flags) {
    robj *o;
    hashTypeIterator *hi;
    int multiplier = 0;
    int length, count = 0;

    if ((o = lookupKeyReadOrReply(c,c->argv[1],shared.emptymultibulk)) == NULL
        || checkType(c,o,REDIS_HASH)) return;

    if (flags & REDIS_HASH_KEY) multiplier++;
    if (flags & REDIS_HASH_VALUE) multiplier++;

    length = hashTypeLength(o) * multiplier;
    addReplyMultiBulkLen(c, length);

    hi = hashTypeInitIterator(o);
    while (hashTypeNext(hi) != REDIS_ERR) {
        if (flags & REDIS_HASH_KEY) {
            addHashIteratorCursorToReply(c, hi, REDIS_HASH_KEY);
            count++;
        }
        if (flags & REDIS_HASH_VALUE) {
            addHashIteratorCursorToReply(c, hi, REDIS_HASH_VALUE);
            count++;
        }
    }

    hashTypeReleaseIterator(hi);
    redisAssert(count == length);
}

void hkeysCommand(redisClient *c) {
    genericHgetallCommand(c,REDIS_HASH_KEY);
}

void hvalsCommand(redisClient *c) {
    genericHgetallCommand(c,REDIS_HASH_VALUE);
}

void hgetallCommand(redisClient *c) {
    genericHgetallCommand(c,REDIS_HASH_KEY|REDIS_HASH_VALUE);
}

void hexistsCommand(redisClient *c) {
    robj *o;
    if ((o = lookupKeyReadOrReply(c,c->argv[1],shared.czero)) == NULL ||
        checkType(c,o,REDIS_HASH)) return;

    addReply(c, hashTypeExists(o,c->argv[2]) ? shared.cone : shared.czero);
}

void hscanCommand(redisClient *c) {
    robj *o;
    unsigned long cursor;

    if (parseScanCursorOrReply(c,c->argv[2],&cursor) == REDIS_ERR) return;
    if ((o = lookupKeyReadOrReply(c,c->argv[1],shared.emptyscan)) == NULL ||
        checkType(c,o,REDIS_HASH)) return;
    scanGenericCommand(c,o,cursor);
}
