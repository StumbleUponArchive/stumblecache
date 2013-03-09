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

#define BTREE_HEADER_SIZE 4096
#define BTREE_MAX_ORDER       102
#define BTREE_T(t)            (t->header->order)
#define BTREE_T2(t)           (2 * t->header->order)

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#include "set.h"

typedef struct {
	uint64_t key;
	uint32_t idx;       /* index into data portion */
	uint32_t expire_ts; /* if 0, then not in use */
} btree_key; /* 16 bytes */

typedef struct {
	char          marker[4];
	uint32_t      idx;        /* the node's index into the data store */
	uint16_t      nr_of_keys; /* number of keys in use */
	char          leaf;       /* whether it's a leaf node or not */
	char          dummy[1];
	btree_key     keys[BTREE_MAX_ORDER * 2 - 1];
	uint32_t      branch[BTREE_MAX_ORDER * 2];
} btree_node; /* 4 + 4 + 2 + 1 + 1 + 203*16 + 204*4 = 4076, which fits nicely in a page */

typedef struct {
	uint32_t version;
	uint32_t order;
	uint32_t max_items;
	uint32_t item_count;
	size_t   item_size;
	uint32_t node_count;
	uint32_t next_node_idx;
	uint32_t root_node_idx;
} btree_header;

struct _btree_tree {
	btree_header *header;
	btree_node   *root;
	int           fd;
	void         *mmap;
	dr_set        freelist;
	void         *nodes;
	void         *data;
	const char   *path;
	uint64_t      file_size;
};

typedef struct _btree_tree btree_tree;

#if defined(__GNUC__) && __GNUC__ >= 4
#	define BTREE_API __attribute__ ((visibility("default")))
#else
#	define BTREE_API
#endif

BTREE_API btree_tree *btree_open(const char *path, int *error);
BTREE_API btree_tree *btree_create(const char *path, uint32_t order, uint32_t nr_of_items, size_t data_size, int *error);
BTREE_API int btree_close(btree_tree *t);
BTREE_API int btree_empty(btree_tree *t);

BTREE_API int btree_get_data(btree_tree *t, uint64_t key, uint32_t *idx, void **data, size_t **data_size, time_t **ts);
BTREE_API int btree_set_data(btree_tree *t, uint64_t key, void *data, size_t data_size, time_t ts);
BTREE_API int btree_get_data_ptr(btree_tree *t, uint64_t key, uint32_t *idx, void **data, size_t **data_size, time_t **ts);
BTREE_API int btree_data_unlock(btree_tree *t, uint32_t idx);
BTREE_API int btree_inc_data(btree_tree *t, uint64_t key);

BTREE_API int btree_search(btree_tree *t, btree_node *node, uint64_t key);
BTREE_API int btree_insert(btree_tree *t, uint64_t key);
BTREE_API int btree_delete(btree_tree *t, uint64_t key);

BTREE_API int btree_dump(btree_tree *t);
BTREE_API int btree_dump_test(btree_tree *t);
BTREE_API int btree_dump_dot(btree_tree *t);
