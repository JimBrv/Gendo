/*
 * Header : lib-list.h
 * Copyright : All right reserved by SecWin, 2010
 * List, Queue from linux kernel, use lib-queue.h for more list structure management.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */


#ifndef LIB_LIST_H
#define LIB_LIST_H

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */
struct lib_list {
	struct lib_list *next, *prev;
};

#define lib_list_node struct lib_list

#define LIB_LIST_INIT(name) { &(name), &(name) }
#define INIT_LIB_LIST(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

#define LIB_LIST_HEAD(name) struct lib_list name = LIB_LIST_INIT(name)

#define LIB_LIST_HEAD_STATIC(name) static struct lib_list name = LIB_LIST_INIT(name)

#define lib_list_entry(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - ((int) &((type *)0)->member) );})

#define lib_list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

#define lib_list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); \
        	pos = pos->prev)

#define lib_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

#define lib_list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; pos != (head); \
        	pos = n, n = pos->prev)

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void
__lib_list_add(struct lib_list *new, struct lib_list *prev, struct lib_list *next)
{
	new->next = next;
	new->prev = prev;
	prev->next = new;
    next->prev = new;
}

/*
 * lib_list_add - add a new entry
 * new: new entry to be added
 * head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void
lib_list_add(struct lib_list *new, struct lib_list *head)
{
	__lib_list_add(new, head, head->next);
}

/*
 * lib_list_add_tail - add a new entry
 * new: new entry to be added
 * head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void
lib_list_add_tail(struct lib_list *new, struct lib_list *head)
{
	__lib_list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void
__lib_list_del(struct lib_list *prev, struct lib_list *next)
{
	if (!prev || !next) return;  // avoid an initialized list node which hasn't add to any list  
	//if (!next->prev || !prev->next) return; 
	next->prev = prev;
	prev->next = next;
}

/*
 * lib_list_del - deletes entry from list.
 * entry: the element to delete from the list.
 * Note: lib_list_empty on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void
lib_list_del(struct lib_list *entry)
{
	__lib_list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

/*
 * lib_list_del_init - deletes entry from list and reinitialize it.
 * entry: the element to delete from the list.
 */
static inline void
lib_list_del_init(struct lib_list *entry)
{
	__lib_list_del(entry->prev, entry->next);
	INIT_LIB_LIST(entry);
}

static inline void
lib_list_free(struct lib_list *head, void (*free) (struct lib_list *entry))
{
	struct lib_list *node1, *node2;

	lib_list_for_each_safe(node1, node2, head) {
		lib_list_del_init(node1);
		if (node1 == NULL)
			continue;
		else
			free(node1);
	}
}

/*
 * lib_list_move - delete from one list and add as another's head
 * list: the entry to move
 * head: the head that will precede our entry
 */
static inline void
lib_list_move(struct lib_list *list, struct lib_list *head)
{
	__lib_list_del(list->prev, list->next);
	lib_list_add(list, head);
}

/*
 * lib_list_move_tail - delete from one list and add as another's tail
 * list: the entry to move
 * head: the head that will follow our entry
 */
static inline void
lib_list_move_tail(struct lib_list *list, struct lib_list *head)
{
	__lib_list_del(list->prev, list->next);
	lib_list_add_tail(list, head);
}

/*
 * lib_list_empty - tests whether a list is empty
 * head: the list to test.
 */
static inline int
lib_list_empty(const struct lib_list *head)
{
	return head->next == head;
}

static inline void
__lib_list_splice(struct lib_list *list, struct lib_list *head)
{
	struct lib_list *first = list->next;
	struct lib_list *last = list->prev;
	struct lib_list *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

/*
 * lib_list_splice - join two lists
 * list: the new list to add.
 * head: the place to add it in the first list.
 */
static inline void
lib_list_splice(struct lib_list *list, struct lib_list *head)
{
	if (!lib_list_empty(list))
		__lib_list_splice(list, head);
}

/*
 * lib_list_splice_init - join two lists and reinitialise the emptied list.
 * list: the new list to add.
 * head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void
lib_list_splice_init(struct lib_list *list, struct lib_list *head)
{
	if (!lib_list_empty(list)) {
		__lib_list_splice(list, head);
		INIT_LIB_LIST(list);
	}
}

static inline int
lib_list_get_count(struct lib_list *head)
{
	struct lib_list *pos, *posTmp;
	int ret = 0;

	lib_list_for_each_safe(pos, posTmp, head) {
		ret++;
	}
	return ret;
}

#endif	/* !LIB_LIST_H */

