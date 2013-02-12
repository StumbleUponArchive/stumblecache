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
   | Authors: Derick Rethans <derick@derickrethans.nl>                    |
   +----------------------------------------------------------------------+
 */

#include "btree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t insert_item(btree_tree *tmp, uint64_t index) {
	uint32_t data_idx;
	size_t *size;
	time_t *time;
	void *data;

	btree_insert(tmp, index);
	btree_get_data(tmp, index, &data_idx, &data, &size, &time);
	printf("%u\n", data_idx);
	btree_data_unlock(tmp, data_idx);
	return data_idx;
}

int main(void)
{
	btree_tree *tmp;
	char data[1024];
	int error = 0;

	memset(data, 4, 1023);
	memset(data+1023, 0, 1);

	tmp = btree_create("test.mmap", 3, 400, 1024, &error);
	if (!tmp) {
		printf("Couldn't create tree from disk image error %d.\n", error);
		exit(1);
	}

	/* Testing with full root node */
	insert_item(tmp, 'F');
	insert_item(tmp, 'Q');
	insert_item(tmp, 'Z');
	insert_item(tmp, 'E');
	insert_item(tmp, 'A');
	insert_item(tmp, 'B');
	insert_item(tmp, 'P');
	btree_dump(tmp);
	/* Delete internal node (the root node in this case) */
	btree_delete(tmp, 'F');
	btree_dump(tmp);

	/* Delete internal node (the root node in this case, but now the previous child has less than N keys) */
	btree_delete(tmp, 'E');
	btree_dump(tmp);

	/* Delete internal node (the root node in this case, but now the previous child and next child have less than N keys) */
	btree_delete(tmp, 'P'); /* Left: ABQZ */
	btree_dump(tmp);

	/* Delete node so that the root node becomes one sized */
	btree_delete(tmp, 'A');
	btree_delete(tmp, 'B');
	btree_delete(tmp, 'Z');
	btree_dump(tmp);

	/* Delete last node */
	btree_delete(tmp, 'Q');
	btree_dump(tmp);

	/* Add one more just to see if that works */
	insert_item(tmp, 'W');
	btree_dump(tmp);

	btree_close(tmp);

	return 0;
}
