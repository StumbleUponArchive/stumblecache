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

#include "../set.h"
#include <stdio.h>

int main(void)
{
	dr_set *set;

	set = dr_set_create(33);

	printf("IN SET: %s\n", dr_set_in(set, 4) ? "YES" : "NO");
	dr_set_add(set, 4);
	printf("IN SET: %s\n", dr_set_in(set, 4) ? "YES" : "NO");
	dr_set_remove(set, 4);
	printf("IN SET: %s\n", dr_set_in(set, 4) ? "YES" : "NO");

	dr_set_dump(set);
	dr_set_free(set);

	return 0;
}
