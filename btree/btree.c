/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 StumbleUpon Inc.                             |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans    <derick@derickrethans.nl>                 |
   |          Elizabeth M Smith <auroraeosrose@php.net>                   |
   +----------------------------------------------------------------------+
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "btree.h"
#include "set.h"

/* printf statement */
#ifdef HAVE_STUMBLECACHE
#	ifdef HAVE_CONFIG_H
#		include "config.h"
#	endif
#	include "php.h"
#	define BTREE_PRINT(...) { php_printf(__VA_ARGS__); }
#else
#	define BTREE_PRINT(...) { printf(__VA_ARGS__); }
#endif

#define NODE_COUNT(nr_of_items, order)  (((2 * nr_of_items) / order) + 1)
#define BTREE_FREELIST_SIZE(nr_of_items) ((unsigned int) (ceil(nr_of_items / 4096.0) * 4096))
#define BTREE_DATA_EXTRA (1 + sizeof(size_t) + sizeof(time_t))

/* Forward declarations */
static btree_node* btree_find_branch(btree_tree *t, btree_node *node, uint64_t key, uint32_t *i);
static void *btree_get_data_location(btree_tree *t, uint32_t idx);
static int btree_search_internal(btree_tree *t, btree_node *node, uint64_t key, uint32_t *idx);

/* ----------------------------------------------------------------
	Lock APIS
------------------------------------------------------------------*/

#ifndef LOCK_DEBUG
#	define LOCK_DEBUG 0
#endif

/**
 * btree_admin_lock
 * @access private
 * @param btree to lock
 * @return int 0 on success or errno on failure
 *
 * Uses fcntl write lock on portion of mmaped file that is the header
 * location
 */
static int btree_admin_lock(btree_tree *t)
{
	struct flock fls;

	fls.l_type   = F_WRLCK;
	fls.l_whence = SEEK_SET;
	fls.l_start  = 0;
	fls.l_len    = t->data - t->mmap;

#if LOCK_DEBUG
	printf("LOCKW   %4x - %4x\n", (unsigned int) fls.l_start, (unsigned int) fls.l_len);
#endif

	if (fcntl(t->fd, F_SETLKW, &fls) == -1) {
		return errno;
	}
	return 0;
}

/**
 * btree_admin_lockr
 * @access private
 * @param btree to lock
 * @return int 0 on success or errno on failure
 *
 * Uses fcntl read lock on portion of mmaped file that is the header
 * location
 */
static int btree_admin_lockr(btree_tree *t)
{
	struct flock fls;

	fls.l_type   = F_RDLCK;
	fls.l_whence = SEEK_SET;
	fls.l_start  = 0;
	fls.l_len    = t->data - t->mmap;

#if LOCK_DEBUG
	printf("LOCKW   %4x - %4x\n", (unsigned int) fls.l_start, (unsigned int) fls.l_len);
#endif

	if (fcntl(t->fd, F_SETLKW, &fls) == -1) {
		return errno;
	}
	return 0;
}

/**
 * btree_admin_unlock
 * @access private
 * @param btree to unlock
 * @return int 0 on success or errno on failure
 *
 * releases all locks on portion of mmaped file that is the header
 * location
 */
static int btree_admin_unlock(btree_tree *t)
{
	struct flock fls;

	fls.l_type   = F_UNLCK;
	fls.l_whence = SEEK_SET;
	fls.l_start  = 0;
	fls.l_len    = t->data - t->mmap;

#if LOCK_DEBUG
	printf("UNLOCK  %4x - %4x", (unsigned int) fls.l_start, (unsigned int) fls.l_len);
#endif

	if (fcntl(t->fd, F_SETLKW, &fls) == -1) {
#if LOCK_DEBUG
		printf(" X: %d\n", errno);
#endif
		return errno;
	}
#if LOCK_DEBUG
	printf(" V\n");
#endif
	return 0;
}

/**
 * btree_data_lock_helper
 * @access private
 * @param btree to deal with
 * @param data index location
 * @param type of lock to perform
 * @return int 0 on success or errno on failure
 *
 * helper for locking and unlocking data locations
 */
inline static int btree_data_lock_helper(btree_tree *t, uint32_t idx, short type)
{
	struct flock fls;

	fls.l_type   = type;
	fls.l_whence = SEEK_SET;
	fls.l_start  = btree_get_data_location(t, idx) - t->mmap;
	fls.l_len    = t->header->item_size + BTREE_DATA_EXTRA;

#if LOCK_DEBUG
	printf("%7s %4x - %4x", type == F_RDLCK ? "DLOCKR" : (type == F_WRLCK ? "DLOCKW" : "UNLOCKD" ), (unsigned int) fls.l_start, (unsigned int) fls.l_len);
#endif

	if (fcntl(t->fd, F_SETLKW, &fls) == -1) {
#if LOCK_DEBUG
		printf(" X: %d\n", errno);
#endif
		return errno;
	}
#if LOCK_DEBUG
	printf(" V\n");
#endif
	return 0;
}

/**
 * btree_data_lockr
 * @access private
 * @param btree to deal with
 * @param data index location
 * @return int 0 on success or errno on failure
 *
 * lock data for reading
 */
static int btree_data_lockr(btree_tree *t, uint32_t idx)
{
	return btree_data_lock_helper(t, idx, F_RDLCK);
}

/**
 * btree_data_lockw
 * @access private
 * @param btree to deal with
 * @param data index location
 * @return int 0 on success or errno on failure
 *
 * lock data for writing
 */
static int btree_data_lockw(btree_tree *t, uint32_t idx)
{
	return btree_data_lock_helper(t, idx, F_WRLCK);
}

/**
 * btree_data_unlock
 * @access private
 * @param btree to deal with
 * @param data index location
 * @return int 0 on success or errno on failure
 *
 * unlock data index location
 */
BTREE_API int btree_data_unlock(btree_tree *t, uint32_t idx)
{
	return btree_data_lock_helper(t, idx, F_UNLCK);
}

/* ----------------------------------------------------------------
	Allocations and Internal APIS
------------------------------------------------------------------*/

/**
 * btree_allocate
 * @access private
 * @param path to the file to use, must be absolute
 * @param order maximum number of pointers that can be stored in a node with a limit of BTREE_MAX_ORDER
 * @param maximum total number of items allowed
 * @param size of the data in each node
 * @return int 0 on success or errno on failure
 *
 * Uses posix_fallocate to ensure that any required storage for regular file data is allocated on
 * the file system storage media
 */
static int btree_allocate(const char *path, uint32_t order, uint32_t nr_of_items, size_t data_size)
{
	int fd, err;
	uint64_t bytes;
	char buffer[4096];
	uint32_t node_count = 0;

	/**
	 * Header:   4096
	 * Freelist: ceil((nr_of_items + 7) / 8)
	 * Nodes:    4096 * (nr_of_items / order)
	 * Data:     nr_of_items * (length-marker + ts + data_size + '\0' delimiter)
	 */
	node_count = NODE_COUNT(nr_of_items, order);
	bytes = BTREE_HEADER_SIZE +
		BTREE_FREELIST_SIZE(nr_of_items) +
		(node_count * 4096) +
		(nr_of_items * (BTREE_DATA_EXTRA + data_size));

	fd = open(path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		return errno;
	}

	memset(buffer, 0, 4096);
	/**
	 * This will keep attempting to do the posix_fallocate
	 * As long as signals are being handled and interrupting it
         * this is safe to do without a lock because the call itself takes
         * care of race conditions
	 */
	do {
		err = posix_fallocate(fd, 0, bytes);
	} while(err == EINTR);

	if (err) {
		close(fd);
		unlink(path);
		return err;
	}
	close(fd);
	return 0;
}

/**
 * btree_allocate_node
 * @access private
 * @param btree struct pointer, caller owns memory
 * @return new btree node
 *
 * sets up a new btree node
 */
static btree_node *btree_allocate_node(btree_tree *t)
{
	btree_node *tmp_node;

	tmp_node = t->nodes + (t->header->next_node_idx * 4096);

	tmp_node->marker[0] = 'N';

	tmp_node->idx = t->header->next_node_idx;
	t->header->next_node_idx++;

	return tmp_node;
}

/**
 * btree_get_node
 * @access private
 * @param btree struct pointer
 * @param index of node to return
 * @return btree node requested at index
 *
 * Returns the btree node requested at the given index
 */
inline static btree_node *btree_get_node(btree_tree *t, uint32_t idx)
{
	return (btree_node*) (t->nodes + (idx * 4096));
}

/**
 * btree_get_data_location
 * @access private
 * @param btree struct pointer
 * @param index of node to return
 * @return pointer to a data memory location
 *
 * Returns a pointer to the spot in memory where the data is
 */
inline static void *btree_get_data_location(btree_tree *t, uint32_t idx)
{
	return t->data + (idx * (t->header->item_size + BTREE_DATA_EXTRA));
}

/* ----------------------------------------------------------------
	Basic Lifecycle APIs
------------------------------------------------------------------*/

/**
 * btree_open_file
 * @access private
 * @param btree struct pointer, caller owns memory
 * @param path to the file to use, must be absolute
 * @return int 0 on success or errno on failure
 *
 * Attempts to stat and open a file, and then mmaps it for use
 * used by btree_open and btree_create
 */
static int btree_open_file(btree_tree *t, const char *path)
{
	int fd, success = 0;
	struct stat fileinfo;

	success = stat(path, &fileinfo);
	if (success == -1) {
		return errno;
	}
	fd = open(path, O_RDWR);
	if (fd == -1) {
		return errno;
	}

	t->fd = fd;
	t->mmap = mmap(NULL, fileinfo.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (t->mmap == MAP_FAILED) {
		int temp_error = errno;
		close(fd);
		t->fd = 0;
		t->mmap = NULL;
		return temp_error;
	}

	t->file_size = fileinfo.st_size;
	t->header = (btree_header*) t->mmap;

	return 0;
}

/**
 * btree_init
 * @access private
 * @param btree struct, caller allocates and owns
 * @return void
 *
 * Empties out the current contents of a btree and sets up a new root node
 */
static void btree_init(btree_tree *t)
{
	btree_node *tmp_node;

	t->header->version = 1;
	t->header->next_node_idx = 0;
	t->header->node_count = NODE_COUNT(t->header->max_items, t->header->order);

	t->freelist.size = t->header->max_items;
	t->freelist.setinfo = t->mmap + BTREE_HEADER_SIZE;
	dr_set_init(&(t->freelist));
	
	t->nodes = t->mmap +
		BTREE_HEADER_SIZE +
		BTREE_FREELIST_SIZE(t->header->max_items);

	t->data = t->mmap +
		BTREE_HEADER_SIZE +
		BTREE_FREELIST_SIZE(t->header->max_items) +
		(t->header->node_count * 4096);

	tmp_node = btree_allocate_node(t);
	tmp_node->leaf = 1;
	tmp_node->nr_of_keys = 0;

	t->root = tmp_node;
}

/**
 * btree_open
 * @access public
 * @param path to the file to use for mmap, must be absolute
 * @param error code for error handling on other side
 * @return struct with open btree
 *
 * This will attempt to open an existing btree
 * Must be unmapped and freed using btree_close after use
 */
BTREE_API btree_tree *btree_open(const char *path, int *error)
{
	btree_tree *t;
	*error = 0;

	t = malloc(sizeof(btree_tree));
	if (t == NULL) {
		*error = errno;
		return NULL;
	}

	*error = btree_open_file(t, path);
	if (*error != 0) {
		free(t);
		return NULL;
	}

	t->freelist.size = t->header->max_items;
	t->freelist.setinfo = t->mmap + BTREE_HEADER_SIZE;

	t->nodes = t->mmap +
		BTREE_HEADER_SIZE +
		BTREE_FREELIST_SIZE(t->header->max_items);

	t->data = t->mmap +
		BTREE_HEADER_SIZE +
		BTREE_FREELIST_SIZE(t->header->max_items) +
		(t->header->node_count * 4096);

	t->root = btree_get_node(t, t->header->root_node_idx);

	return t;
}

/**
 * btree_create
 * @access public
 * @param path to the file to use for mmap, must be absolute
 * @param order maximum number of pointers that can be stored in a node with a limit of BTREE_MAX_ORDER
 * @param maximum total number of items allowed
 * @param size of the data in each node
 * @param error code for error handling on other side
 * @return btree struct
 *
 * This will create a new btree
 * Must be unmapped and closed using btree_close after use
 */
BTREE_API btree_tree *btree_create(const char *path, uint32_t order, uint32_t nr_of_items, size_t data_size, int *error)
{
	btree_tree *tmp_tree;
	*error = 0;

	if (order > BTREE_MAX_ORDER) {
		order = BTREE_MAX_ORDER;
	}

	*error = btree_allocate(path, order, nr_of_items, data_size);
	if (*error != 0) {
		return NULL;
	}

	tmp_tree = malloc(sizeof(btree_tree));
	if (tmp_tree == NULL) {
		*error = errno;
		return NULL;
	}

	*error = btree_open_file(tmp_tree, path);
	if (*error != 0) {
		free(tmp_tree);
		return NULL;
	}

	tmp_tree->path = path;
	tmp_tree->header->order = order;
	tmp_tree->header->max_items = nr_of_items;
	tmp_tree->header->item_size = data_size;

	btree_init(tmp_tree);

	return tmp_tree;
}

/**
 * btree_empty
 * @access public
 * @param btree struct
 * @return int 0 on success or error value
 *
 * This will attempt to lock the current btree file and
 * completely empty all data by re-initing it as if it were new
 */
BTREE_API int btree_empty(btree_tree *t)
{
	int error;

	error = btree_admin_lock(t);
	if (error != 0) {
		return error;
	}

	btree_init(t);

	error = btree_admin_unlock(t);
	if (error != 0) {
		return error;
	}
	return 0;
}

/**
 * btree_close
 * @access public
 * @param btree struct
 * @return int 0 on success or error value
 *
 * Closes an open file descriptor, munmaps any data and frees the struct
 * This is cleanup for btree_open and/or btree_create
 */
BTREE_API int btree_close(btree_tree *t)
{
	if (close(t->fd) == -1) {
		return errno;
	}
	if (munmap(t->mmap, t->file_size) == -1) {
		return errno;
	}
	free(t);
	return 0;
}

/* ----------------------------------------------------------------
	Data APIs
------------------------------------------------------------------*/

/**
 * btree_get_data
 * @access public
 * @param btree struct
 * @param key for data
 * @param index of data - out
 * @param pointer to data - out
 * @param data size - out
 * @param time - out
 * @return int 0 on success or error value
 *
 * Gets data from location in btree - does a READ lock on the data
 * User needs to call btree_unlock_data when finished
 */
BTREE_API int btree_get_data(btree_tree *t, uint64_t key, uint32_t *idx, void **data, size_t **data_size, time_t **ts)
{
	void *location;
	int error;

	if (1 == btree_search_internal(t, t->root, key, idx)) {
		error = btree_data_lockr(t, *idx);
		if (error != 0) {
			return error;
		}
	} else {
		return 404; /* Data not found */
	}

	location = btree_get_data_location(t, *idx);
	*data_size = ((size_t*)location);
	*ts = (time_t*) (location + sizeof(size_t));
	*data = location + sizeof(size_t) + sizeof(time_t);

	return 0;
}

/**
 * btree_get_data_ptr
 * @access public
 * @param btree struct
 * @param key for data
 * @param index of data - out
 * @param pointer to data - out
 * @param data size - out
 * @param time - out
 * @return int 0 on success or error value
 *
 * Gets data from location in btree - does a WRITE lock on the data
 * User needs to call btree_unlock_data when finished
 */
BTREE_API int btree_get_data_ptr(btree_tree *t, uint64_t key, uint32_t *idx, void **data, size_t **data_size, time_t **ts)
{
	void *location;
	int error;

	if (1 == btree_search_internal(t, t->root, key, idx)) {
		error = btree_data_lockw(t, *idx);
		if (error != 0) {
			return error;
		}
	} else {
		return 404; /* Data not found */
	}

	location = btree_get_data_location(t, *idx);
	*data_size = ((size_t*)location);
	*ts = (time_t*) (location + sizeof(size_t));
	*data = location + sizeof(size_t) + sizeof(time_t);

	return 0;
}

/**
 * btree_set_data
 * @access public
 * @param btree struct
 * @param key
 * @param pointer to data
 * @param data size
 * @param time
 * @return int 0 on success or error value
 *
 * Writes data to a location in a btree, handles all it's own locking
 */
BTREE_API int btree_set_data(btree_tree *t, uint64_t key, void *data, size_t data_size, time_t ts)
{
	void *location;
	int error;
	uint32_t idx;

	if (data_size > t->header->item_size) {
		return 414; /* Request URI too long - data > item_size */
	}

	if (1 == btree_search_internal(t, t->root, key, &idx)) {
		error = btree_data_lockw(t, idx);
		if (error != 0) {
			return error;
		}
	} else {
		return 404; /* Data not found */
	}

	location = btree_get_data_location(t, idx);
	*((size_t*)location) = data_size;
	*(time_t*) (location + sizeof(size_t)) = ts;
	memcpy(location + sizeof(size_t) + sizeof(time_t), data, data_size);

	error = btree_data_unlock(t, idx);
	if (error != 0) {
		return error;
	}
	return 0;
}

/* ----------------------------------------------------------------
	Search APIs
------------------------------------------------------------------*/

/**
 * btree_search_internal
 * @access private
 * @param btree struct
 * @param btree node
 * @param key
 * @param index to fill
 * @return 1 if found, 0 if not
 *
 * does the actual search of the tree to find an index location
 * can be called recursively as the nodes are walked
 */
static int btree_search_internal(btree_tree *t, btree_node *node, uint64_t key, uint32_t *idx)
{
	int i = 0;
	while (i < node->nr_of_keys && key > node->keys[i].key) {
		i++;
	}

	if (i < node->nr_of_keys && key == node->keys[i].key) {
		if (idx) {
			*idx = node->keys[i].idx;
		}
		return 1;
	}

	if (node->leaf) {
		return 0;
	} else {
		btree_node *tmp_node = btree_get_node(t, node->branch[i]);
		return btree_search_internal(t, tmp_node, key, idx);
	}
}

/**
 * btree_search
 * @access public
 * @param btree struct
 * @param btree node
 * @param key
 * @param index
 * @return 0 on success, error code on failure
 *
 * tries to find a key in the node in the tree - and fills the index if successful
 * locks for reading, does NOT lock location
 */
BTREE_API int btree_search(btree_tree *t, btree_node *node, uint64_t key)
{
	int found, error = 0;
	uint32_t idx;

	error = btree_admin_lockr(t);
	if (error != 0) {
		return error;
	}

	found = btree_search_internal(t, node, key, &idx);

	error = btree_admin_unlock(t);
	if (error != 0) {
		return error;
	}
	if (1 == found) {
		return 0;
	} else {
		return 404; /* key not found */
	}
	return error;
}

/* ----------------------------------------------------------------
	Node APIs
------------------------------------------------------------------*/

/**
 * btree_split_child
 * @access private
 * @param btree struct
 * @param btree node
 * @param key
 * @param btree child node
 * @return void
 *
 * splits a node into two
 */
static void btree_split_child(btree_tree *t, btree_node *parent, uint32_t key_nr, btree_node *child)
{
	uint32_t j;

	btree_node *tmp_node = btree_allocate_node(t);
	tmp_node->leaf = child->leaf;
	tmp_node->nr_of_keys = BTREE_T(t) - 1;

	for (j = 0; j < BTREE_T(t) - 1; j++) {
		tmp_node->keys[j] = child->keys[j + BTREE_T(t)];
	}
	if (!child->leaf) {
		for (j = 0; j < BTREE_T(t); j++) {
			tmp_node->branch[j] = child->branch[j + BTREE_T(t)];
		}
	}
	child->nr_of_keys = BTREE_T(t) - 1;

	for (j = parent->nr_of_keys + 1; j > key_nr; j--) {
		parent->branch[j] = parent->branch[j - 1];
	}
	parent->branch[key_nr + 1] = tmp_node->idx;

	for (j = parent->nr_of_keys; j > key_nr; j--) {
		parent->keys[j] = parent->keys[j - 1];
	}
	parent->keys[key_nr] = child->keys[BTREE_T(t) - 1];
	parent->nr_of_keys++;
}

/**
 * btree_find_branch
 * @access private
 * @param btree struct
 * @param btree node
 * @param key
 * @param i
 * @return void
 *
 * finds the branch for the node and calls btree_get_node 
 */
static btree_node* btree_find_branch(btree_tree *t, btree_node *node, uint64_t key, uint32_t *i)
{
	*i = node->nr_of_keys;
	while (*i > 0 && key < node->keys[(*i) - 1].key) {
		(*i)--;
	}
	return btree_get_node(t, node->branch[*i]);
}

/**
 * btree_insert_non_full
 * @access private
 * @param btree struct
 * @param btree node
 * @param key
 * @param data index
 * @return void
 *
 * inserts the node into the btree - either immediately if it's a leaf or find the branch, split
 * the child and call this recursively until a leaf is found
 */
static void btree_insert_non_full(btree_tree *t, btree_node *node, uint64_t key, uint32_t data_idx)
{
	uint32_t i;
	btree_node *tmp_node;

	i = node->nr_of_keys;
	if (node->leaf) {
		while (i > 0 && key < node->keys[i - 1].key) {
			node->keys[i] = node->keys[i - 1];
			i--;
		}
		node->keys[i].key = key;
		node->nr_of_keys++;

		/* Fetch data index, and set it to the idx element here too */
		node->keys[i].idx = data_idx;

		/* Do administrative jobs */
		dr_set_add(&(t->freelist), data_idx);
		t->header->item_count++;
	} else {
		tmp_node = btree_find_branch(t, node, key, &i);
		if (tmp_node->nr_of_keys == BTREE_T2(t) - 1) {
			btree_split_child(t, node, i, tmp_node);
			if (key > node->keys[i].key) {
				i++;
			}
		}
		btree_insert_non_full(t, btree_get_node(t, node->branch[i]), key, data_idx);
	}
}

/**
 * btree_insert_internal
 * @access private
 * @param btree struct
 * @param key
 * @param data index
 * @return void
 *
 * does the actual insert of the new node - depending on which side of the tree
 * it needs to go on this can include a split, or just a call to insert_non_full
 */
static void btree_insert_internal(btree_tree *t, uint64_t key, uint32_t data_idx)
{
	btree_node *r = t->root;

	if (r->nr_of_keys == BTREE_T2(t) - 1) {
		btree_node *tmp_node;

		tmp_node = btree_allocate_node(t);
		t->root = tmp_node;
		t->header->root_node_idx = tmp_node->idx;
		tmp_node->leaf = 0;
		tmp_node->nr_of_keys = 0;
		tmp_node->branch[0] = r->idx;
		btree_split_child(t, tmp_node, 0, r);
		btree_insert_non_full(t, tmp_node, key, data_idx);
	} else {
		btree_insert_non_full(t, r, key, data_idx);
	}
}

/**
 * btree_node_insert_key
 * @access private
 * @param btree struct
 * @param btree node
 * @param pos
 * @param key
 * @return void
 *
 * puts a new key into a node and moves other keys if necessary
 */
static void btree_node_insert_key(btree_tree *t, btree_node *node, uint32_t pos, btree_key key)
{
	uint32_t i = node->nr_of_keys;

	while (i > pos) {
		node->keys[i] = node->keys[i-1];
		i--;
	}
	node->keys[pos] = key;
	node->nr_of_keys++;
}

/**
 * btree_insert
 * @access public
 * @param btree struct
 * @param insertion key
 * @param pointer to data index
 * @return int 0 on success or error value
 *
 * Inserts data as key in tree IF it doesn't already exist
 */
BTREE_API int btree_insert(btree_tree *t, uint64_t key)
{
	btree_node *r = t->root;
	unsigned int tmp_data_idx;
	int error = 0;

	error = btree_admin_lock(t);
	if (error != 0) {
		return error;
	}

	if (t->header->item_count >= t->header->max_items) {
		error = btree_admin_unlock(t);
		if (error != 0) {
			return error;
		}
		return 413; /* Request Entity Too Large - item count > max items allowed */
	}
	if (btree_search_internal(t, r, key, NULL)) {
		error = btree_admin_unlock(t);
		if (error != 0) {
			return error;
		}
		return 409; /* Conflict - item already exists */
	}
	if (!dr_set_find_first(&(t->freelist), &tmp_data_idx)) {
		error = btree_admin_unlock(t);
		if (error != 0) {
			return error;
		}
		return 500; /* Woah, btree error, couldn't retrieve that node! */
	}
	btree_insert_internal(t, key, tmp_data_idx);

	error = btree_admin_unlock(t);
	if (error != 0) {
		return error;
	}
	return 0;
}

/**
 * btree_delete_key_idx_from_node
 * @access private
 * @param btree node
 * @param data index
 * @return void
 *
 * Removes the key with index "idx" and shifts all other keys
 */
static void btree_delete_key_idx_from_node(btree_node *node, uint64_t idx)
{
	int i;

	for (i = idx; i < node->nr_of_keys - 1; i++) {
		node->keys[i] = node->keys[i + 1];
	}
	node->nr_of_keys--;
}

/**
 * btree_delete_branch_idx_from_node
 * @access private
 * @param btree node
 * @param data index
 * @return void
 *
 * Removes the branch with index "idx" and shifts all other branches
 */
static void btree_delete_branch_idx_from_node(btree_node *node, uint64_t idx)
{
	int i;

	if (!node->leaf) {
		for (i = idx; i <= node->nr_of_keys; i++) {
			node->branch[i] = node->branch[i + 1];
		}
	}
}

/**
 * btree_check_key_in_node
 * @access private
 * @param btree node
 * @param key to check
 * @param branch index to fill
 * @param data index to fill
 * @return 1 on success, 0 on failure
 *
 * checks to see if a key is inside a note and fills branch/data indexes if found
 */
static unsigned int btree_check_key_in_node(btree_node *node, uint64_t key, uint32_t *idx, uint32_t *data_idx)
{
	int i = 0;
	while (i < node->nr_of_keys && key > node->keys[i].key) {
		i++;
	}
	if (i < node->nr_of_keys && key == node->keys[i].key) {
		*idx = i;
		*data_idx = node->keys[i].idx;
		return 1;
	}
	return 0;
}

/**
 * btree_find_smallest
 * @access private
 * @param btree struct
 * @param btree node
 * @return btree node
 *
 * find smallest node in a branch
 */
static btree_node *btree_find_smallest(btree_tree *t, btree_node *node)
{
	btree_node *ptr = node;
	while (!ptr->leaf) {
		ptr = btree_get_node(t, ptr->branch[0]);
	}
	return ptr;
}

/**
 * btree_find_greatest
 * @access private
 * @param btree struct
 * @param btree node
 * @return btree node
 *
 * find greatest node in a branch
 */
static btree_node *btree_find_greatest(btree_tree *t, btree_node *node)
{
	btree_node *ptr = node;
	while (!ptr->leaf) {
		ptr = btree_get_node(t, ptr->branch[ptr->nr_of_keys]);
	}
	return ptr;
}

/**
 * btree_merge
 * @access private
 * @param btree struct
 * @param btree node a
 * @param btree node x
 * @param branch index
 * @param btree node b
 * @param data index
 * @return void
 *
 * merge two branches
 */
static void btree_merge(btree_tree *t, btree_node *a, btree_node *x, uint32_t idx, btree_node *b)
{
	int i = a->nr_of_keys, j, k = a->nr_of_keys;

	a->keys[i] = x->keys[idx];
	a->nr_of_keys++;
	for (j = 0; j < b->nr_of_keys; j++) {
		i++;
		a->keys[i] = b->keys[j];
	}
	a->nr_of_keys += b->nr_of_keys;

	if (!x->leaf) {
		for (j = 0; j <= b->nr_of_keys; j++) {
			k++;
			a->branch[k] = b->branch[j];
		}
	}
}

/**
 * btree_delete_internal
 * @access private
 * @param btree struct
 * @param btree node
 * @param key that was requested for deletion
 * @param current key
 * @param data index
 * @return int 1 on success, 0 on failure
 *
 * Removes a node at key from the btree
 * Locks the entire tree until complete
 */
static int btree_delete_internal(btree_tree *t, btree_node *node, uint64_t key_to_delete, uint64_t key, uint32_t *data_idx_to_free)
{
	uint32_t idx;
	uint32_t data_idx;

	/* if x is a leaf then
	 *   if k is in x then
	 *     delete k from x and return true
	 *   else return false //k is not in subtree
	 */
	if (node->leaf) {
		if (btree_check_key_in_node(node, key, &idx, &data_idx)) {
			btree_delete_key_idx_from_node(node, idx);
			btree_delete_branch_idx_from_node(node, idx);
			if (key == key_to_delete) {
				*data_idx_to_free = data_idx;
			}
			return 1;
		} else {
			return 0;
		}
	} else {
		/* if k is in x then */
		if (btree_check_key_in_node(node, key, &idx, &data_idx)) {
			btree_node *y, *node_with_prev_key;
			/* Record the data_idx for this key */
			if (key == key_to_delete) {
				*data_idx_to_free = data_idx;
			}
			/*   y = the child of x that precedes k
			 *   if y has at least t keys then
			 *     k' = the predecessor of k (use B-Tree-FindLargest)
			 *     Copy k' over k //i.e., replace k with k'
			 *     B-Tree-Delete(y, k') //Note: recursive call
			 */
			y = btree_get_node(t, node->branch[idx]);
			if (y->nr_of_keys >= BTREE_T(t)) {
				node_with_prev_key = btree_find_greatest(t, y);
				node->keys[idx] = node_with_prev_key->keys[y->nr_of_keys-1];
				return btree_delete_internal(t, y, key_to_delete, node_with_prev_key->keys[y->nr_of_keys-1].key, data_idx_to_free);
			} else {
				btree_node *z, *node_with_next_key;
			/*   else //y has t-1 keys
			 *     z = the child of x that follows k
			 *     if z has at least t keys then
			 *       k' = the successor of k
			 *       Copy k' over k //i.e., replace k with k'
			 *       B-Tree-Delete(z, k') //Note: recursive call
			 */
				z = btree_get_node(t, node->branch[idx + 1]);
				if (z->nr_of_keys >= BTREE_T(t)) {
					node_with_next_key = btree_find_smallest(t, z);
					node->keys[idx] = node_with_next_key->keys[0];
					btree_delete_internal(t, z, key_to_delete, node_with_next_key->keys[0].key, data_idx_to_free);
				} else {
			/*     else //both y and z have t-1 keys
			 *       merge k and all of z into y //y now contains 2t-1 keys.
			 *       //k and the pointer to z will be deleted from x.
			 *       B-Tree-Delete(y, k) //Note: recursive call
			 */
					btree_merge(t, y, node, idx, z);
					btree_delete_key_idx_from_node(node, idx);
					btree_delete_branch_idx_from_node(node, idx + 1);
					btree_delete_internal(t, y, key_to_delete, key, data_idx_to_free);
					if (t->root->nr_of_keys == 0 && !t->root->leaf) {
						t->root = btree_get_node(t, t->root->branch[0]);
					}
				}
			}
		} else {
			btree_node *c;
			uint32_t i;

			c = btree_find_branch(t, node, key, &i);
			if (c->nr_of_keys <= BTREE_T(t) - 1) {
				btree_node *z, *node_with_prev_key, *node_with_next_key;
				btree_key tmp_key;

				/* Is there a left sibling with T or more keys? */
				if (i > 0) { /* otherwise there is no left sibling */
					z = btree_get_node(t, node->branch[i - 1]);
					if (z->nr_of_keys > BTREE_T(t) - 1) {
						node_with_prev_key = btree_find_greatest(t, z);
						btree_node_insert_key(t, c, 0, node_with_prev_key->keys[z->nr_of_keys-1]);
						btree_delete_internal(t, z, key_to_delete, node_with_prev_key->keys[z->nr_of_keys-1].key, data_idx_to_free);

						/* Swap parent and first key in C */
						tmp_key = node->keys[i-1];
						node->keys[i-1] = c->keys[0];
						c->keys[0] = tmp_key;
						goto proceed;
					}
				}

				/* Is there a left sibling with T or more keys? */
				if (i < node->nr_of_keys) { /* otherwise there is no right sibling */
					z = btree_get_node(t, node->branch[i + 1]);
					if (z->nr_of_keys > BTREE_T(t) - 1) {
						node_with_next_key = btree_find_smallest(t, z);
						btree_node_insert_key(t, c, c->nr_of_keys, node_with_next_key->keys[0]);
						btree_delete_internal(t, z, key_to_delete, node_with_next_key->keys[0].key, data_idx_to_free);

						/* Swap parent and last key in C */
						tmp_key = node->keys[i];
						node->keys[i] = c->keys[c->nr_of_keys - 1];
						c->keys[c->nr_of_keys - 1] = tmp_key;
						goto proceed;
					}
				}

				/* No siblings, so we need to merge. */
				/* Is there a left sibling? */
				if (i > 0) { /* otherwise there is no left sibling */
					z = btree_get_node(t, node->branch[i - 1]);
					btree_merge(t, z, node, i - 1, c);
					btree_delete_branch_idx_from_node(node, i);
					btree_delete_key_idx_from_node(node, i - 1);
					if (t->root->nr_of_keys == 0 && !t->root->leaf) {
						t->root = btree_get_node(t, t->root->branch[0]);
					}
					return btree_delete_internal(t, z, key_to_delete, key, data_idx_to_free);
				}
				/* Is there a right sibling? */
				if (i < node->nr_of_keys) { /* otherwise there is no right sibling */
					z = btree_get_node(t, node->branch[i + 1]);
					btree_merge(t, c, node, i, z);
					btree_delete_branch_idx_from_node(node, i + 1);
					btree_delete_key_idx_from_node(node, i);
					if (t->root->nr_of_keys == 0 && !t->root->leaf) {
						t->root = btree_get_node(t, t->root->branch[0]);
					}
					return btree_delete_internal(t, c, key_to_delete, key, data_idx_to_free);
				}
			}
proceed:
			return btree_delete_internal(t, c, key_to_delete, key, data_idx_to_free);
		}
	}
	return 0;
}

/**
 * btree_delete
 * @access public
 * @param btree struct
 * @return int 0 on success or error value
 *
 * Removes a node at key from the btree
 * Locks the entire tree until complete
 */
BTREE_API int btree_delete(btree_tree *t, uint64_t key)
{
	int error = 0;
	btree_node *n = t->root;
	uint32_t data_idx = 99999999; /* filled with the index to free */

	error = btree_admin_lock(t);
	if (error != 0) {
		return error;
	}

	if (btree_delete_internal(t, n, key, key, &data_idx)) {
		/* Do administrative jobs */
		t->header->item_count--;
		dr_set_remove(&(t->freelist), data_idx);
		error = btree_admin_unlock(t);
		if (error != 0) {
			return error;
		}
		return 0;
	}
	error = btree_admin_unlock(t);
	if (error != 0) {
		return error;
	}
	return 404; /* humorous attempt at node not found */
}

/* ----------------------------------------------------------------
	Debugging Helpers
------------------------------------------------------------------*/

/**
 * btree_dump_node_dot
 * @access private
 * @param btree struct
 * @param btree node
 * @return void
 *
 * Outputs (using printf or php_printf) a physical representation
 * of the tree and it's data in a table format
 */
static void btree_dump_node_dot(btree_tree *t, btree_node *node)
{
	int i;

	BTREE_PRINT("\n\"IDX%d\" [\nlabel=\"{{", node->idx);
	for (i = 0; i < node->nr_of_keys; i++) {
		BTREE_PRINT("%s%lu", i ? " | " : "", node->keys[i].key);
	}
	BTREE_PRINT("} ");
	if (!node->leaf) {
		BTREE_PRINT("| {");
		for (i = 0; i < node->nr_of_keys + 1; i++) {
			BTREE_PRINT("%s<n%d>%d", i ? " | " : "", i, node->branch[i]);
		}
		BTREE_PRINT("}}\"\n];\n");
		for (i = 0; i < node->nr_of_keys + 1; i++) {
			BTREE_PRINT("\"IDX%d\":n%d->\"IDX%d\";\n", node->idx, i, node->branch[i]);
		}
		for (i = 0; i < node->nr_of_keys + 1; i++) {
			btree_dump_node_dot(t, btree_get_node(t, node->branch[i]));
		}
	} else {
		BTREE_PRINT("}\"\n];\n");
	}
}

/**
 * btree_dump_dot
 * @access public
 * @param btree struct
 * @return 0 on success, errno on failure
 *
 * Outputs (using printf or php_printf) a physical representation
 * of the tree and it's data in a table format
 */
BTREE_API int btree_dump_dot(btree_tree *t)
{
	int error;

	error = btree_admin_lock(t);
	if (error != 0) {
		return error;
	}
	BTREE_PRINT("digraph g {\ngraph [ rankdir = \"TB\" ];\nnode [ fontsize = \"16\" shape = \"record\" ];\n");
	btree_dump_node_dot(t, t->root);
	BTREE_PRINT("}\n");
	error = btree_admin_unlock(t);
	if (error != 0) {
		return error;
	}
	return 0;
}

/**
 * btree_dump_node_test
 * @access private
 * @param btree struct
 * @param btree node
 * @param level to output
 * @return void
 *
 * Recursive helper for printing out the contents of a node,
 * used by btree_dump_test - provides more information than the normal dump
 */
static void btree_dump_node_test(btree_tree *t, btree_node *node, int level)
{
	int i;

	BTREE_PRINT("\n%*sIDX: %d: ", level * 2, "", node->idx);
	if (!node->leaf) {
		BTREE_PRINT("(%d) ", node->branch[0]);
	}
	for (i = 0; i < node->nr_of_keys; i++) {
		BTREE_PRINT("%lu ", node->keys[i].key);
		if (!node->leaf) {
			BTREE_PRINT("(%d) ", node->branch[i+1]);
		}
	}
	if (!node->leaf) {
		for (i = 0; i < node->nr_of_keys + 1; i++) {
			btree_dump_node_test(t, btree_get_node(t, node->branch[i]), level + 1);
		}
	}
}

/**
 * btree_dump_test
 * @access public
 * @param btree struct
 * @return void
 *
 * Recursive helper for printing out the contents of a node,
 * used by btree_dump_test - provides more information than the normal dump
 * only available with debug on
 */
BTREE_API int btree_dump_test(btree_tree *t)
{
	int error;

	error = btree_admin_lock(t);
	if (error != 0) {
		return error;
	}
	btree_dump_node_test(t, t->root, 0);
	BTREE_PRINT("\n");
	error = btree_admin_unlock(t);
	if (error != 0) {
		return error;
	}
	return 0;
}

/**
 * btree_dump_node
 * @access private
 * @param btree struct
 * @return void
 *
 * Recursive helper for printing out the contents of a node,
 * used by btree_dump
 */
static void btree_dump_node(btree_tree *t, btree_node *node)
{
	int i;

	BTREE_PRINT("\nIDX: %d\n", node->idx);
	for (i = 0; i < node->nr_of_keys; i++) {
		BTREE_PRINT("%9lu ", node->keys[i].key);
	}
	if (!node->leaf) {
		BTREE_PRINT("\n");
		for (i = 0; i < node->nr_of_keys + 1; i++) {
			BTREE_PRINT("%9d ", node->branch[i]);
		}
		for (i = 0; i < node->nr_of_keys + 1; i++) {
			btree_dump_node(t, btree_get_node(t, node->branch[i]));
		}
	}
}

/**
 * btree_dump
 * @access public
 * @param btree struct
 * @return 0 on success, errno on failure
 *
 * Outputs (using printf or php_printf) a physical representation
 * of the tree and it's data, only really good for debugging
 */
BTREE_API int btree_dump(btree_tree *t)
{
	int error;

	error = btree_admin_lock(t);
	if (error != 0) {
		return error;
	}
	BTREE_PRINT("-------\n");
	btree_dump_node(t, t->root);
	BTREE_PRINT("\n-------\n");
	error = btree_admin_unlock(t);
	if (error != 0) {
		return error;
	}
	return 0;
}
