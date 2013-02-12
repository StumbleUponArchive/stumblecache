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
	int i, tmp_key;
	char testdata[11];
	int error = 0;

	/* testing too large data */
	tmp = btree_create("test.mmap", 3, 6, 10, &error);
	if (!tmp) {
		printf("Couldn't create tree from disk image, errno %d.\n", error);
		exit(1);
	}

	for (i = 0; i < 205; i++) {
		tmp_key = i * 3;
		if (0 == btree_insert(tmp, tmp_key)) {
			sprintf(testdata, "H: %07d", i);
			btree_set_data(tmp, tmp_key, testdata, 10, time(NULL));
			btree_get_data(tmp, tmp_key, &data_idx, (void **)&data, &size, &ts);
			printf("%s %zd\n", data, *size);
			btree_data_unlock(tmp, data_idx);
		}
	}

	btree_dump(tmp);
	btree_close(tmp);

	return 0;
}
