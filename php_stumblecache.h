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

#ifndef PHP_STUMBLECACHE_H
#define PHP_STUMBLECACHE_H

extern zend_module_entry stumblecache_module_entry;
#define phpext_stumblecache_ptr &stumblecache_module_entry

#define PHP_STUMBLECACHE_VERSION "1.0.0-dev"

#ifdef ZTS
#include "TSRM.h"
#endif

/* forward declaration of btree_tree struct */
typedef struct _btree_tree btree_tree;

ZEND_BEGIN_MODULE_GLOBALS(stumblecache)
	char      *default_cache_dir;
	long       default_ttl;
	btree_tree *global_stat_file;
ZEND_END_MODULE_GLOBALS(stumblecache) 

typedef struct _php_stumblecache_obj php_stumblecache_obj;

PHP_MINIT_FUNCTION(stumblecache);
PHP_MINFO_FUNCTION(stumblecache);
PHP_GINIT_FUNCTION(stumblecache);
PHP_GSHUTDOWN_FUNCTION(stumblecache);

#ifdef ZTS
# define STUMBLECACHE_G(v) TSRMG(stumblecache_globals_id, zend_stumblecache_globals *, v)
#else
# define STUMBLECACHE_G(v) (stumblecache_globals.v)
#endif

#endif
