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

void setup(btree_tree *tmp)
{
	/* Testing with full root node */
	btree_insert(tmp, 'F');
	btree_insert(tmp, 'Q');
	btree_insert(tmp, 'Z');
	btree_insert(tmp, 'E');
	btree_insert(tmp, 'A');
	btree_insert(tmp, 'B');
	btree_insert(tmp, 'N');
	btree_insert(tmp, 'G');
	btree_insert(tmp, 'R');
	btree_insert(tmp, 'Y');
	btree_insert(tmp, 'H');
	btree_insert(tmp, 'C');
	btree_insert(tmp, 'D');
	btree_insert(tmp, 'I');
	btree_insert(tmp, 'L');
	btree_insert(tmp, 'T');

	btree_insert(tmp, 'q');
	btree_insert(tmp, 'w');
	btree_insert(tmp, 'e');
	btree_insert(tmp, 'r');
	btree_insert(tmp, 't');
	btree_insert(tmp, 'y');
	btree_insert(tmp, 'u');
	btree_insert(tmp, 'i');
	btree_insert(tmp, 'o');
	btree_insert(tmp, 'p');

	btree_insert(tmp, 'S');
	btree_insert(tmp, 'W');
	btree_insert(tmp, 'U');
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
	btree_insert(tmp, 'd');
	btree_insert(tmp, 'f');
	btree_insert(tmp, 'v');

	btree_delete(tmp, 'i');
	btree_delete(tmp, 'f');
	btree_delete(tmp, 'e');
	btree_delete(tmp, 'y');
	btree_delete(tmp, 't');
	btree_dump_dot(tmp);

	btree_close(tmp);

	return 0;
}
