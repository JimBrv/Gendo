#include "lib-base.h"
#include "lib-io.h"
#include "lib-debug.h"
#include "lib-mem.h"

#define IO_SET_HASH_SIZE 200

static inline int io_hash_fd(int fd)
{
    return fd % IO_SET_HASH_SIZE;
}

static int io_hash_creat_key(HASH_NODE_T *pNode, void *p)
{
    lib_io_t *pio = NULL;

    pio = hash_entry(pNode, lib_io_t, node);
    return io_hash_fd(pio->fd);
}

static int io_compare_fd(HASH_NODE_T *pNode, void *p)
{
    int fd = *((int *) p);
    lib_io_t *pio = hash_entry(pNode, lib_io_t, node);

    if (fd == pio->fd) {
        return ERROR;
    }
    return OK;
}

lib_io_t *io_get_by_fd(lib_io_set_t *pset, const int fd)
{
    HASH_NODE_T *pnode = NULL;
    int key = io_hash_fd(fd);
    lib_io_t *pio = NULL;

    pnode = hash_walk(pset->ht, key, io_compare_fd, (void *)&fd);
    if (pnode == NULL) {
        return NULL;
    }
    pio = hash_entry(pnode, lib_io_t, node);
    return pio;
}


int io_add(lib_io_set_t *pset, lib_io_t *pio)
{
    int ret = hash_add(&(pio->node), pset->ht); 
    if (ret) {
        lib_error(MODULE_UTL,"io add to hash_table failed\n");
        return ret;
    }
    pset->cnt++;
    return ret;
}

int io_del(lib_io_set_t *pset, lib_io_t *pio)
{
    int ret = hash_del(&(pio->node), pset->ht);   
    if (ret) {
        lib_error(MODULE_UTL,"io add to hash_table failed\n");
        return ret;
    }
    pset->cnt--;
    return ret;
}

int io_del_by_fd(lib_io_set_t *pset, const int fd)
{
    lib_io_t *pio = io_get_by_fd(pset, fd);
    if (!pio) {
        lib_error(MODULE_UTL,"io_del_by_fd can't find fd=%d IO in hash_table!!!\n", fd);
        return ERROR;     
    }
    return io_del(pset, pio);
}


lib_io_set_t *io_set_new()
{
    lib_io_set_t *pset = NULL;
    pset = mem_malloc(sizeof(lib_io_set_t));
    assert(pset != NULL);
    pset->ht = hash_create(IO_SET_HASH_SIZE, io_hash_creat_key, NULL);
    if (pset->ht == NULL) {
        lib_error(MODULE_UTL,"Io_set hash_create failed\n");
        mem_free((void*)pset);
        return NULL;
    }
    return pset;
}

lib_io_t *io_new()
{
    lib_io_t *pio = (lib_io_t *)mem_malloc(sizeof(lib_io_t));
    assert(pio != NULL);
    io_init(pio);
    return pio;
}

