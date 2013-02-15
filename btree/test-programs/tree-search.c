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

#include <stdio.h>
#include <stdlib.h>
#include "btree.h"

int main(void)
{
	btree_tree *tmp;
	uint64_t i;
	int error;

	tmp = btree_open("./test.mmap", &error);
	if (!tmp) {
		printf("Couldn't open tree, errno %d.\n", error);
		exit(1);
	}
	btree_dump(tmp);

	for (i = 0; i < 125000000; i++) {
		btree_search(tmp, tmp->root, 'F');
		btree_search(tmp, tmp->root, 'y');
		btree_search(tmp, tmp->root, '0');
		btree_search(tmp, tmp->root, '1');
	}
	btree_close(tmp);

	return 0;
}
