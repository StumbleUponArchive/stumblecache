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

int main(void)
{
	btree_tree *tmp;
	uint32_t data_idx;
	size_t *size;
	time_t *ts;
	char *data;
	int error = 0;

	/* testing too large data */
	tmp = btree_create("test.mmap", 3, 6, 10, &error);
	if (!tmp) {
		printf("Couldn't create tree from disk image, errno %d.\n", error);
		exit(1);
	}

	if (btree_insert(tmp, 'X')) {
		btree_set_data(tmp, 'X', "HelloWorl1", 10, time(NULL));
		btree_get_data(tmp, 'X', &data_idx, (void **)&data, &size, &ts);
		printf("%s %zd\n", data, *size);
		btree_data_unlock(tmp, data_idx);
	}

	if (btree_insert(tmp, 'Q')) {
		btree_set_data(tmp, 'Q', "HelloWorld2", 10, time(NULL));
		btree_get_data(tmp, 'Q', &data_idx, (void **)&data, &size, &ts);		
		printf("%s %zd\n", data, *size);
		btree_data_unlock(tmp, data_idx);
	}

	if (btree_insert(tmp, 'D')) {
		btree_set_data(tmp, 'D', "HelloWorld3", 11, time(NULL));
		btree_get_data(tmp, 'D', &data_idx, (void **)&data, &size, &ts);
		printf("%s %zd\n", data, *size);
		btree_data_unlock(tmp, data_idx);
	}

	if (btree_insert(tmp, 'Z')) {
		btree_set_data(tmp, 'Z', "HelloWorl4", 11, time(NULL));
		btree_get_data(tmp, 'Z', &data_idx, (void **)&data, &size, &ts);
		printf("%s %zd\n", data, *size);
		btree_data_unlock(tmp, data_idx);
	}

	if (btree_insert(tmp, 'A')) {
		btree_set_data(tmp, 'A', "HelloWorl5", -1, time(NULL));
		btree_get_data(tmp, 'A', &data_idx, (void **)&data, &size, &ts);
		printf("%s %zd\n", data, *size);
		btree_data_unlock(tmp, data_idx);
	}

	if (btree_insert(tmp, 'C')) {
		btree_set_data(tmp, 'C', "HelloWorl6", 0, time(NULL));
		btree_get_data(tmp, 'C', &data_idx, (void **)&data, &size, &ts);
		printf("%s %zd\n", data, *size);
		btree_data_unlock(tmp, data_idx);
	}

	if (btree_insert(tmp, 'G')) {
		btree_set_data(tmp, 'G', "TooMany1", 8, time(NULL));
		btree_get_data(tmp, 'G', &data_idx, (void **)&data, &size, &ts);
		printf("%s %zd\n", data, *size);
		btree_data_unlock(tmp, data_idx);
	}

	btree_dump(tmp);
	btree_close(tmp);

	return 0;
}
