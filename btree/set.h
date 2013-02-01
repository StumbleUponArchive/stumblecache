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
   | Authors: Derick Rethans    <derick@derickrethans.nl>                 |
   |          Elizabeth M Smith <auroraeosrose@php.net>                   |
   +----------------------------------------------------------------------+
 */
#ifndef __DR_SET_H__
#define __DR_SET_H__

typedef struct _dr_set {
	unsigned int size;
	unsigned char *setinfo;
} dr_set;

#if defined(__GNUC__) && __GNUC__ >= 4
#       define DRSET_API __attribute__ ((visibility("default")))
#   else
#       define DRSET_API
#   endif

DRSET_API dr_set *dr_set_create(unsigned int size);
DRSET_API void dr_set_init(dr_set *set);
DRSET_API void dr_set_free(dr_set *set);

DRSET_API void dr_set_add(dr_set *set, unsigned int position);
DRSET_API void dr_set_remove(dr_set *set, unsigned int position);

DRSET_API int dr_set_in(dr_set *set, unsigned int position);
DRSET_API int dr_set_find_first(dr_set *set, unsigned int *position);

DRSET_API void dr_set_dump(dr_set *set);
	
#define DR_SET_SIZE(size) ceil((size + 7) / 8)

#endif
