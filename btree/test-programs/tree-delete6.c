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
#include <unistd.h>

void setup(btree_tree *tmp)
{
	uint32_t data_idx;
	int i;

	for (i = 0; i < 80; i++) {
		btree_insert(tmp, i, &data_idx);
	}
}

int main(void)
{
	btree_tree *tmp;
	int i;

	tmp = btree_create("test.mmap", 5, 64, 32);
	if (!tmp) {
		printf("Couldn't create tree from disk image.\n");
		exit(1);
	}

	setup(tmp);
	for (i = 25; i < 31; i++) {
		btree_delete(tmp, i);
	}
	btree_dump(tmp);
	btree_delete(tmp, 31);
	btree_dump(tmp);

	btree_free(tmp);
	unlink("test.mmap");

	return 0;
}
