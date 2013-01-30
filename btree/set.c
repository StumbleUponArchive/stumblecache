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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "set.h"

dr_set *dr_set_create(unsigned int size)
{
	dr_set *tmp;

	if (size < 1) {
		return NULL;
	}

	tmp = calloc(1, sizeof(dr_set));
	tmp->size = size;
	tmp->setinfo = calloc(1, DR_SET_SIZE(size));

	dr_set_init(tmp);

	return tmp;
}

inline void dr_set_init(dr_set *set)
{
	unsigned int i;

	// mass unset everything but the last byte
	memset(set->setinfo, 0xff, set->size / 8);

	// unset bits in the last byte
	for (i = 0; i < set->size % 8; i++) {
		dr_set_remove(set, i + (set->size / 8) * 8);
	}
}

void dr_set_free(dr_set *set)
{
	free(set->setinfo);
	free(set);
}


void dr_set_dump(dr_set *set)
{
	unsigned int byte, bit; 

	printf("SIZE: %d\n", set->size);
	for (byte = 0; byte < ceil((set->size + 7) / 8); byte++) {
		printf("BYTE: %d (%0x)\n", byte, set->setinfo[byte]);
		for (bit = 0; bit < 8; bit++) {
			printf(" %c", set->setinfo[byte] & (1 << bit) ? '1' : '0');
		}
		printf("\n");
	}
	printf("\n");
}



inline void dr_set_add(dr_set *set, unsigned int position)
{
	unsigned char *byte;
	unsigned int   bit;

	if (position > set->size) {
		return;
	}

	byte = &(set->setinfo[position / 8]);
	bit  = position % 8;

	*byte = *byte & ~(1 << bit);
}

inline void dr_set_remove(dr_set *set, unsigned int position)
{
	unsigned char *byte;
	unsigned int   bit;

	if (position > set->size) {
		return;
	}

	byte = &(set->setinfo[position / 8]);
	bit  = position % 8;

	*byte = *byte | (1 << bit);
}


inline int dr_set_in(dr_set *set, unsigned int position)
{
	unsigned char *byte;
	unsigned int   bit;

	if (position > set->size) {
		return 0;
	}

	byte = &(set->setinfo[position / 8]);
	bit  = position % 8;

	return (*byte & (1 << bit));
}

inline int dr_set_find_first(dr_set *set, unsigned int *position)
{
	unsigned int i, j;

	for (i = 0; i < ((set->size + 7) / 8); i++) {
		if (set->setinfo[i] != 0x0) {
			for (j = 0; j < 8; j++) {
				if (dr_set_in(set, i * 8 + j)) {
					*position = i * 8 + j;
					return 1;
				}
			}
		}
	}

	return 0;
}
