/*
 * Header : lib-hash.h
 * Copyright : All right reserved by SecWin, 2010
 * hash wrappers
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
 
#ifndef LIB_HASH_H
#define LIB_HASH_H

#include "lib-list.h"
#include "lib-thread.h"

typedef struct _hash_node_s {
	struct lib_list node;
}HASH_NODE_T, *PHASH_NODE_T;

typedef struct _hash_table_s {
	HASH_NODE_T *head;
	int          size;
	int        (*create_key) (HASH_NODE_T *node, void *p);
	void        *p;
   	THD_LOCK_T  lock;
}HASH_TABLE_T, *PHASH_TABLE_T;

HASH_TABLE_T *hash_create(int size, int (*create_key) (HASH_NODE_T *, void *), void *p);
void hash_free(HASH_TABLE_T *table);
void hash_free_all(HASH_TABLE_T *table, void(*free)(HASH_NODE_T *node));
int  hash_init(HASH_NODE_T *node);
int  hash_add(HASH_NODE_T *node, HASH_TABLE_T *table);
int  hash_del(HASH_NODE_T *node, HASH_TABLE_T *table);
int  hash_num(HASH_TABLE_T *table);
HASH_NODE_T  *hash_walk(HASH_TABLE_T * table, int key, int (*func) (HASH_NODE_T *, void *), void *p);
int  hash_empty(HASH_TABLE_T *table);
void hash_walk_all(HASH_TABLE_T *table, void(*pFun)(HASH_NODE_T *node));

#define hash_entry(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - ((int) &((type *)0)->member) );})

#endif
