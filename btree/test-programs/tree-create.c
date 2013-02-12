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
	uint32_t data_idx;
	char data[1024];
	int error = 0;

	memset(data, 4, 1023);
	memset(data+1023, 0, 1);

	tmp = btree_create("test.mmap", 3, 400, 1024, &error);
	if (!tmp) {
		printf("Couldn't create tree from disk image, errno %d.\n", error);
		exit(1);
	}

	insert_item(tmp, 'A');
	data_idx = insert_item(tmp, 'L');
	memcpy(tmp->data + data_idx * 1024, data, 1024);
	data_idx = insert_item(tmp, 'D');
	memcpy(tmp->data + data_idx * 1024, data, 1024);
	insert_item(tmp, 'F');

	insert_item(tmp, '4');
	insert_item(tmp, '2');
	insert_item(tmp, '3');
	insert_item(tmp, '5');
	insert_item(tmp, '1');

	insert_item(tmp, 'N');
	insert_item(tmp, 'P');
	insert_item(tmp, 'd');
	insert_item(tmp, 'f');
	insert_item(tmp, 'n');
	insert_item(tmp, 'p');
	insert_item(tmp, 'H');
	insert_item(tmp, 'C');
	insert_item(tmp, 'B');
	insert_item(tmp, 'E');
	insert_item(tmp, 'G');

	insert_item(tmp, 'I');
	insert_item(tmp, 'K');
	insert_item(tmp, 'J');
	insert_item(tmp, 'M');
	insert_item(tmp, 'o');

	insert_item(tmp, 'q');
	insert_item(tmp, 'r');
	insert_item(tmp, 'i');

	insert_item(tmp, 'j');
	insert_item(tmp, 'k');
	insert_item(tmp, 'd');
	insert_item(tmp, 't');
	insert_item(tmp, 'm');

	insert_item(tmp, 'O');
	insert_item(tmp, 'Q');
	insert_item(tmp, 'R');
	insert_item(tmp, 'S');
	insert_item(tmp, 'T');
	insert_item(tmp, 'U');

	insert_item(tmp, 'x');
	insert_item(tmp, 'w');
	insert_item(tmp, 'y');
	insert_item(tmp, 'u');
	insert_item(tmp, 'v');

	btree_dump_dot(tmp);
	btree_close(tmp);

	return 0;
}
