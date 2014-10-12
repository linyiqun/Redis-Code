/* adlist.c - A generic doubly linked list implementation
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


#include <stdlib.h>
#include "adlist.h"
#include "zmalloc.h"

/* Create a new list. The created list can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
 /* 创建结点列表 */
list *listCreate(void)
{
    struct list *list;

	//申请空间，如果失败了就直接返回NULL
    if ((list = zmalloc(sizeof(*list))) == NULL)
        return NULL;
    //初始化操作，头尾结点，，3个公共的函数指针全部赋值为NULL
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;
    return list;
}

/* Free the whole list.
 *
 * This function can't fail. */
/* 释放整个列表 */
void listRelease(list *list)
{
    unsigned long len;
    listNode *current, *next;
	
	//找到当前结点，也就是头结点
    current = list->head;
    len = list->len;
    while(len--) {
    	//while循环依次释放结点
        next = current->next;
        //如果列表有free释放方法定义，每个结点都必须调用自己内部的value方法
        if (list->free) list->free(current->value);
        //采用redis新定义的在zfree方式释放结点，与zmalloc对应，不是free！！ 
        zfree(current);
        current = next;
    }
    //最后再次释放list同样是zfree
    zfree(list);
}

/* Add a new node to the list, to head, contaning the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
 /* 列表添加头结点 */
list *listAddNodeHead(list *list, void *value)
{
    listNode *node;
	//定义新的listNode，并赋值函数指针
    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
    	//当此时没有任何结点时，头尾结点是同一个结点，前后指针为NULL
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
    	//设置此结点next与前头结点的位置关系
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    
    //结点计数递增并返回
    list->len++;
    return list;
}

/* Add a new node to the list, to tail, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
 /* 列表添加尾结点，操作大体上与增加头结点一样，不加以描述了 */
list *listAddNodeTail(list *list, void *value)
{
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    return list;
}

/* 在old_node结点的前面或后面插入新结点 */
list *listInsertNode(list *list, listNode *old_node, void *value, int after) {
    listNode *node;
	//新申请结点，并赋值好函数指针
    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    
    if (after) {
    	//如果是在目标结点的后面插入的情况，将新结点的next指针指向老结点的next
        node->prev = old_node;
        node->next = old_node->next;
        if (list->tail == old_node) {
        	//如果老结点已经是最后一个结点了，则新的结点直接成为尾部结点
            list->tail = node;
        }
    } else {
    	//如果是在目标结点的前面插入的情况，将新结点的preview指针指向老结点的preview
        node->next = old_node;
        node->prev = old_node->prev;
        if (list->head == old_node) {
        	//如果老结点已经是头结点了，则新的结点直接成为头部结点
            list->head = node;
        }
    }
    //检查Node的前后结点还有没有未连接的操作
    if (node->prev != NULL) {
        node->prev->next = node;
    }
    if (node->next != NULL) {
        node->next->prev = node;
    }
    list->len++;
    return list;
}

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
/* 列表删除某结点 */
void listDelNode(list *list, listNode *node)
{
    if (node->prev)
    	//如果结点prev结点存在，prev的结点的下一及诶单指向Node的next结点
        node->prev->next = node->next;
    else
    	//如果不存在说明是被删除的是头结点，则重新赋值Node的next为新头结点
        list->head = node->next;
     //后半操作类似
    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;
    //同样要调用list的free函数
    if (list->free) list->free(node->value);
    zfree(node);
    list->len--;
}

/* Returns a list iterator 'iter'. After the initialization every
 * call to listNext() will return the next element of the list.
 *
 * This function can't fail. */
/* 获取列表呢迭代器 */
listIter *listGetIterator(list *list, int direction)
{
    listIter *iter;
	//申请空间，失败了就直接返回NULL
    if ((iter = zmalloc(sizeof(*iter))) == NULL) return NULL;
    if (direction == AL_START_HEAD)
    	//如果方向定义的是从头开始，则迭代器的next指针指向列表头结点
        iter->next = list->head;
    else
    	//如果方向定义的是从尾开始，则迭代器的next指针指向列表尾结点
        iter->next = list->tail;
     //赋值好迭代器方向并返回
    iter->direction = direction;
    return iter;
}

/* Release the iterator memory */
/* 释放迭代器内存 */
void listReleaseIterator(listIter *iter) {
    zfree(iter);
}

/* Create an iterator in the list private iterator structure */
/* 相当于重置迭代器为方向从头开始 */
void listRewind(list *list, listIter *li) {
    li->next = list->head;
    li->direction = AL_START_HEAD;
}

/* 重置迭代器为方向从尾部开始 */
void listRewindTail(list *list, listIter *li) {
    li->next = list->tail;
    li->direction = AL_START_TAIL;
}

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage patter
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * */
 /* 根据迭代器获取下一结点 */
listNode *listNext(listIter *iter)
{
	//获取当前迭代器的当前结点
    listNode *current = iter->next;

    if (current != NULL) {
        if (iter->direction == AL_START_HEAD)
        	//如果方向为从头部开始，则当前结点等于当前的结点的下一结点
            iter->next = current->next;
        else
        	//如果方向为从尾部开始，则当前结点等于当前的结点的上一结点
            iter->next = current->prev;
    }
    return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
/* 列表赋值方法，传入的参数为原始列表 */
list *listDup(list *orig)
{
    list *copy;
    listIter *iter;
    listNode *node;

	//如果创建列表失败则直接返回
    if ((copy = listCreate()) == NULL)
        return NULL;
    //为新列表赋值好3个函数指针
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;
    //获得从头方向开始的迭代器
    iter = listGetIterator(orig, AL_START_HEAD);
    while((node = listNext(iter)) != NULL) {
    	//从前往后遍历结点
        void *value;

        if (copy->dup) {
        	//如果定义了列表复制方法，则调用dup方法
            value = copy->dup(node->value);
            if (value == NULL) {
            	//如果发生OOM内存溢出问题，直接释放所有空间
                listRelease(copy);
                listReleaseIterator(iter);
                return NULL;
            }
        } else
        	//没定义直接复制函数指针
            value = node->value;
        if (listAddNodeTail(copy, value) == NULL) {
        	//后面的结点都是从尾部逐一添加结点，如果内存溢出，同上操作
            listRelease(copy);
            listReleaseIterator(iter);
            return NULL;
        }
    }
    //最后释放迭代器
    listReleaseIterator(iter);
    return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
/* 关键字搜索Node结点此时用到了list的match方法了 */
listNode *listSearchKey(list *list, void *key)
{
    listIter *iter;
    listNode *node;
	
	//获取迭代器
    iter = listGetIterator(list, AL_START_HEAD);
    while((node = listNext(iter)) != NULL) {
    	//遍历循环
        if (list->match) {
        	//如果定义了list的match方法，则调用match方法
            if (list->match(node->value, key)) {
            	//如果方法返回true，则代表找到结点，释放迭代器
                listReleaseIterator(iter);
                return node;
            }
        } else {
        	//如果没有定义list 的match方法，则直接比较函数指针
            if (key == node->value) {
            	//如果相等，则代表找到结点，释放迭代器
                listReleaseIterator(iter);
                return node;
            }
        }
    }
    listReleaseIterator(iter);
    return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
/* 根据下标值返回相应的结点*/
/*下标有2种表示形式，从头往后一次0， 1， 2，...从后往前是 ...-3， -2， -1.-1为最后一个结点*/
listNode *listIndex(list *list, long index) {
    listNode *n;

    if (index < 0) {
    	//如果index为负数，则从后往前数
        index = (-index)-1;
        n = list->tail;
        while(index-- && n) n = n->prev;
    } else {
    	//如果index为正数，则从前往后数
        n = list->head;
        while(index-- && n) n = n->next;
    }
    return n;
}

/* Rotate the list removing the tail node and inserting it to the head. */
/* rotate操作其实就是把尾部结点挪到头部，原本倒数第二个结点变为尾部结点 */
void listRotate(list *list) {
    listNode *tail = list->tail;

	//如果长度为不足，直接返回，之前宏定义的方法
    if (listLength(list) <= 1) return;

    /* Detach current tail */
    //替换新的尾部结点，原结点后挪一个位置
    list->tail = tail->prev;
    list->tail->next = NULL;
    
    /* Move it as head */
   	//设置新结点
    list->head->prev = tail;
    tail->prev = NULL;
    tail->next = list->head;
    list->head = tail;
}
