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

int main(int argc, char *argv[])
{
	btree_tree *tmp;
	FILE *f;
	char urlBuffer[2048], *data;
	uint64_t id;
	uint64_t i = 0;
	uint32_t data_idx;

	tmp = btree_create("test.mmap", 128, 700000, 2048);
	if (!tmp) {
		printf("Couldn't create tree from disk image.\n");
		exit(1);
	}

	f = fopen(argv[1], "r");
	while (!feof(f)) {
		fgets(urlBuffer, 2048, f);
		data = strchr(urlBuffer, ' ');
		if (data) {
			data++;
			data[-1] = '\0';
			id = atoll(urlBuffer);
			if (btree_insert(tmp, id, &data_idx) == 1) {
				btree_set_data(tmp, data_idx, data, strlen(data), time(NULL));
			}
		}
		i++;
	}

	btree_free(tmp);

	return 0;
}
