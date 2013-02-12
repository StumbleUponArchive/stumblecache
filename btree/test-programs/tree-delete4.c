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

void setup(btree_tree *tmp)
{
	/* Testing with full root node */
	insert_item(tmp, 'F');
	insert_item(tmp, 'Q');
	insert_item(tmp, 'Z');
	insert_item(tmp, 'E');
	insert_item(tmp, 'A');
	insert_item(tmp, 'B');
	insert_item(tmp, 'P');
	insert_item(tmp, 'G');
	insert_item(tmp, 'R');
	insert_item(tmp, 'Y');
	insert_item(tmp, 'H');
	insert_item(tmp, 'C');
	insert_item(tmp, 'D');
	insert_item(tmp, 'S');
	btree_dump(tmp);

	btree_delete(tmp, 'H');
	btree_dump(tmp);

	btree_delete(tmp, 'G');
	btree_dump(tmp);

	btree_delete(tmp, 'P');
	btree_dump(tmp);

	btree_delete(tmp, 'F');
	btree_dump(tmp);

	btree_delete(tmp, 'D');
	btree_dump(tmp);

	btree_delete(tmp, 'Q');
	btree_dump(tmp);
}

int main(void)
{
	btree_tree *tmp;
	int error = 0;

	tmp = btree_create("test.mmap", 3, 400, 1024, &error);
	if (!tmp) {
		printf("Couldn't create tree from disk image error %d.\n", error);
		exit(1);
	}

	setup(tmp);

	/* Should cause collapse */
	btree_delete(tmp, 'E');
	btree_dump(tmp);
	/* Another bit less */
	btree_delete(tmp, 'B');
	btree_delete(tmp, 'C');
	btree_dump(tmp);
	/* Should cause collapse to root */
	btree_delete(tmp, 'Z');
	btree_dump(tmp);
	/* Clean up */
	btree_delete(tmp, 'S');
	btree_delete(tmp, 'A');
	btree_delete(tmp, 'R');
	btree_delete(tmp, 'Y');
	btree_dump(tmp);


	setup(tmp);
	btree_dump(tmp);
	/* Should cause collapse */
	btree_delete(tmp, 'Z');
	btree_dump(tmp);
	/* Add a few more to create a split */
	insert_item(tmp, 'T');
	insert_item(tmp, 'Z');
	btree_dump(tmp);
	/* Should cause collapse */
	btree_delete(tmp, 'A');
	btree_dump(tmp);
	/* Delete root node */
	btree_delete(tmp, 'S');
	btree_dump(tmp);
	/* Delete root node (3x) */
	btree_delete(tmp, 'R');
	btree_delete(tmp, 'E');
	btree_delete(tmp, 'T');
	btree_dump(tmp);
	/* Clean up */
	btree_delete(tmp, 'B');
	btree_delete(tmp, 'Z');
	btree_delete(tmp, 'Y');
	btree_delete(tmp, 'C');
	btree_dump(tmp);

	setup(tmp);
	btree_dump(tmp);
	/* Should cause collapse */
	btree_delete(tmp, 'B');
	btree_dump(tmp);

	btree_close(tmp);

	return 0;
}
