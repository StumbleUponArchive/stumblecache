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

int main(int argc, char *argv[])
{
	btree_tree *tmp;
	char *data;
	uint64_t id;
	uint64_t found = 0;
	uint32_t idx;
	size_t data_size;
	time_t ts;

	tmp = btree_open("test.mmap");
	if (!tmp) {
		printf("Couldn't create tree from disk image.\n");
		exit(1);
	}

	id = atoll(argv[1]);
	found += btree_search(tmp, tmp->root, id, &idx);
	data = (char*) btree_get_data(tmp, idx, &data_size, &ts);
	printf("Found: %lu (%u)\n%s\n", found, idx, data);

	btree_free(tmp);

	return 0;
}
