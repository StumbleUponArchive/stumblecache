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
	char data[1024];

	memset(data, 4, 1023);
	memset(data+1023, 0, 1);

	tmp = btree_create("test.mmap", 3, 400, 1024);
	if (!tmp) {
		printf("Couldn't create tree from disk image.\n");
		exit(1);
	}
	btree_insert(tmp, 'A', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'L', &data_idx); printf("%u\n", data_idx);
	memcpy(tmp->data + data_idx * 1024, data, 1024);
	btree_insert(tmp, 'D', &data_idx); printf("%u\n", data_idx);
	memcpy(tmp->data + data_idx * 1024, data, 1024);
	btree_insert(tmp, 'F', &data_idx); printf("%u\n", data_idx);

	btree_insert(tmp, '4', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, '2', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, '3', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, '5', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, '1', &data_idx); printf("%u\n", data_idx);

	btree_insert(tmp, 'N', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'P', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'd', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'f', &data_idx); printf("%u\n", data_idx); /* */
	btree_insert(tmp, 'n', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'p', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'H', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'C', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'B', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'E', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'G', &data_idx); printf("%u\n", data_idx); /* */

	btree_insert(tmp, 'I', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'K', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'J', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'M', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'o', &data_idx); printf("%u\n", data_idx); /* */

	btree_insert(tmp, 'q', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'r', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'i', &data_idx); printf("%u\n", data_idx);

	btree_insert(tmp, 'j', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'k', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 's', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 't', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'm', &data_idx); printf("%u\n", data_idx);

	btree_insert(tmp, 'O', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'Q', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'R', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'S', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'T', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'U', &data_idx); printf("%u\n", data_idx);

	btree_insert(tmp, 'x', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'w', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'y', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'u', &data_idx); printf("%u\n", data_idx);
	btree_insert(tmp, 'v', &data_idx); printf("%u\n", data_idx);

	btree_dump_dot(tmp);
	btree_free(tmp);

	return 0;
}
