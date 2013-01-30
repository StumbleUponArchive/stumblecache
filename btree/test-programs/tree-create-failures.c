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
	size_t size;
	time_t ts;
	char *data;

	// testing too large data
	tmp = btree_create("test.mmap", 3, 6, 10);
	if (!tmp) {
		printf("Couldn't create tree from disk image.\n");
		exit(1);
	}
	if (btree_insert(tmp, 'X', &data_idx)) {
		btree_set_data(tmp, data_idx, "HelloWorl1", 10, time(NULL));
		data = (char*) btree_get_data(tmp, data_idx, &size, &ts);
		printf("%s %zd\n", data, size);
	}

	if (btree_insert(tmp, 'Q', &data_idx)) {
		btree_set_data(tmp, data_idx, "HelloWorld2", 10, time(NULL));
		data = (char*) btree_get_data(tmp, data_idx, &size, &ts);
		printf("%s %zd\n", data, size);
	}

	if (btree_insert(tmp, 'D', &data_idx)) {
		btree_set_data(tmp, data_idx, "HelloWorld3", 11, time(NULL));
		data = (char*) btree_get_data(tmp, data_idx, &size, &ts);
		printf("%s %zd\n", data, size);
	}

	if (btree_insert(tmp, 'Z', &data_idx)) {
		btree_set_data(tmp, data_idx, "HelloWorl4", 11, time(NULL));
		data = (char*) btree_get_data(tmp, data_idx, &size, &ts);
		printf("%s %zd\n", data, size);
	}

	if (btree_insert(tmp, 'A', &data_idx)) {
		btree_set_data(tmp, data_idx, "HelloWorl5", -1, time(NULL));
		data = (char*) btree_get_data(tmp, data_idx, &size, &ts);
		printf("%s %zd\n", data, size);
	}

	if (btree_insert(tmp, 'C', &data_idx)) {
		btree_set_data(tmp, data_idx, "HelloWorl6", 0, time(NULL));
		data = (char*) btree_get_data(tmp, data_idx, &size, &ts);
		printf("%s %zd\n", data, size);
	}

	if (btree_insert(tmp, 'G', &data_idx)) {
		btree_set_data(tmp, data_idx, "TooMany1", 8, time(NULL));
		data = (char*) btree_get_data(tmp, data_idx, &size, &ts);
		printf("%s %zd\n", data, size);
	}

	btree_dump(tmp);
	btree_free(tmp);

	return 0;
}
