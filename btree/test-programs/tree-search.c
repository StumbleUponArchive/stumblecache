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

int main(void)
{
	btree_tree *tmp;
	uint32_t idx;
	uint64_t i;

	tmp = btree_open("test.mmap");
	btree_dump(tmp);

	for (i = 0; i < 125000000; i++) {
		btree_search(tmp, tmp->root, 'F', &idx);
		btree_search(tmp, tmp->root, 'y', &idx);
		btree_search(tmp, tmp->root, '0', &idx);
		btree_search(tmp, tmp->root, '1', &idx);
	}
	btree_free(tmp);

	return 0;
}
