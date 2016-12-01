/*
 * Filename : lib_hash.c
 * Copyright : All right reserved by SecWin, 2010
 * Base socket wrapper
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */

#include "lib-base.h"
#include "lib-socket.h"
#include "lib-debug.h"
#include "lib-list.h"
#include "lib-mem.h"
#include "lib-hash.h"


/*
 * Create A Hash Table
 * Parameters in : size - hash table size
 * 		   create_key - count a key for the user node
 * 		   cmd_key - compare two key
 * Returns : hash table or NULL
 */
HASH_TABLE_T *hash_create(int size, int (*create_key) (HASH_NODE_T *, void *), void *p)
{
    int i;
    HASH_TABLE_T *table;

    table = (HASH_TABLE_T *) mem_malloc(sizeof (HASH_TABLE_T));
    if (!table) {
	lib_error(MODULE_UTL, "malloc\n");
	return NULL;
    }

    /* zero for table in mem_malloc */
    table->head = mem_malloc(sizeof(HASH_NODE_T) * size);
    if (!(table->head)) {
	lib_error(MODULE_UTL, "malloc\n");
	mem_free(table);
	return NULL;
    }

    for (i = 0; i < size; i++) {
		INIT_LIB_LIST(&(((table->head)[i]).node));
    }

    table->size = size;
    table->create_key = create_key;
    table->p = p;
	thd_lock_init(&table->lock, NULL);

   
    return table;
}

/* Free a hash table, Note: only free hash table head, invote it only when table is empty
 * Parameters in : table - hash table head
 * Returns : none
 */
void hash_free(HASH_TABLE_T * table)
{
    mem_free(table->head);
    mem_free(table);
}

/* Init hash node, MUST invote it when creating a user node.
 * Parameters in : node - hash node
 * Returns : success
 */
int hash_init(HASH_NODE_T * node)
{
    INIT_LIB_LIST(&(node->node));
    return 0;
}

/* Add a hash node to hash table 
 * Parameters in : table - hash table head
 * 		   node - hash table node
 * Returns : success or failed
 */
int hash_add(HASH_NODE_T * node, HASH_TABLE_T * table)
{
    int key  = 0;
    int ret  = 0;
    
    thd_lock(&table->lock);
    key = (table->create_key) (node, table->p);
    if ((key < 0) || (key >= table->size)) {
	    lib_error(MODULE_UTL, "key\n");
        ret = -1;
	    goto out;
    }
    lib_list_add_tail(&(node->node), &((table->head[key]).node));
out:
    thd_unlock(&table->lock);
    return ret;
}

/* Delete a hash node from hash table
 * Parameters in : node - current deleted node.
 * Returns : success
 */
int hash_del(HASH_NODE_T * node, HASH_TABLE_T * table)
{
    thd_lock(&table->lock);
    lib_list_del(&(node->node));
    thd_unlock(&table->lock);
    return 0;
}

int hash_num(HASH_TABLE_T * table)
{
    int i, num = 0;
    struct lib_list *entry;
    HASH_NODE_T *head;

    thd_lock(&table->lock);
    for (i = 0; i < table->size; i++) {
	    head = &(table->head[i]);
	    lib_list_for_each(entry, &(head->node)) {
	        num++;
	    }
    }
    thd_unlock(&table->lock);

    return num;
}

/* Walk whole link list of the same key, return current node when func returns -1
 * Parameters in : head - link list of the same key
 * 		   func - user function for stop walk
 * 		   p - parameter of user function 'func'
 * returns : current node when user function 'func' return -1
 */
static HASH_NODE_T *_hash_walk(HASH_NODE_T * head, int (*func) (HASH_NODE_T *, void *), void *p)
{
    int ret;
    struct lib_list *entry;
    HASH_NODE_T *node;

    lib_list_for_each(entry, &(head->node)) {
	node = lib_list_entry(entry, HASH_NODE_T, node);
	ret = func(node, p);
	if (ret < 0)
	    return node;
    }

    return NULL;
}

/* Walk hash table, return current node when func returns -1, walk whole hash table 
 * if parameter 'key' equal to hash table size, otherwise walk link list of parameter 'key' 
 * Parameters in : table - hash table head
 * 		   key - hash key
 * 		   func - user function
 * 		   p - user parameter of user function 'func'
 * Returns : current node when user function 'func' return -1
 */
HASH_NODE_T *hash_walk(HASH_TABLE_T * table, int key, 
		       int (*func)(HASH_NODE_T *, void *),
		       void *p)
{
    int i;
    HASH_NODE_T *node = NULL;

    thd_lock(&table->lock);

    if ((key >= 0) && (key < table->size)) {
	    node = _hash_walk(&(table->head[key]), func, p);
    }

    if (key == table->size) {
	    for (i = 0; i < table->size; i++) {
	        node = _hash_walk(&(table->head[i]), func, p);
	        if (node)
		        break;
	    }
    }
    
    thd_unlock(&table->lock);
    return node;
}

void hash_free_all(HASH_TABLE_T * table, void (*free) (HASH_NODE_T * node))
{
    int i;
    HASH_NODE_T *node;
    struct lib_list *node1, *node2;

    if(table == NULL) {
	    lib_error(MODULE_UTL, "hash_free_all entry NULL\n");
	    return;
    }

    thd_lock(&table->lock);    
    for (i = 0; i < table->size; i++) {
	    lib_list_for_each_safe(node1, node2, &((&(table->head[i]))->node)) {
	        lib_list_del_init(node1);
	        if (node1 == NULL) {
		        continue;
	        } else {
		        node = lib_list_entry(node1, HASH_NODE_T, node);
		        free(node);
	        }
	    }
    }
    thd_unlock(&table->lock);
    
    mem_free(table->head);
    mem_free(table);
}

int hash_empty(HASH_TABLE_T * table)
{
    int i;

    if (table == NULL) {
	    return 1;
    }
    for (i = 0; i < table->size; i++) {
	    if (!lib_list_empty(&((&(table->head[i]))->node))) {
	        return 0;
	    }
    }
    return 1;
}

void hash_walk_all(HASH_TABLE_T * table, void (*pFun) (HASH_NODE_T * node))
{
    int i;
    HASH_NODE_T *node;
    struct lib_list *node1, *node2;

    if (table == NULL) {
	    lib_error(MODULE_UTL, "hash_walk_all entry NULL\n");
	    return;
    }
    
    thd_lock(&table->lock);
    for (i = 0; i < table->size; i++) {
	    lib_list_for_each_safe(node1, node2, &((&(table->head[i]))->node)) {
	        if (node1 == NULL) {
		        continue;
	        } else {
		        node = lib_list_entry(node1, HASH_NODE_T, node);
		        pFun(node);
	        }
	    }
    }
    thd_unlock(&table->lock);
    return;
}
