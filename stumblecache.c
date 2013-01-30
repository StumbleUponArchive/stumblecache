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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <time.h>

#include "php.h"
#include "zend.h"
#include "zend_alloc.h"
#include "php_globals.h"
#include "ext/standard/info.h"

#include "php_stumblecache.h"
#include "php_stumblecache_internal.h"

#include "ext/igbinary/igbinary.h"

zend_function_entry stumblecache_functions[] = {
	{NULL, NULL, NULL}
};

static const zend_module_dep stumblecache_module_deps[] = {
	ZEND_MOD_REQUIRED("igbinary")
	{NULL, NULL, NULL}
};

zend_module_entry stumblecache_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER_EX, NULL,
	stumblecache_module_deps,
#endif
	"stumblecache",
	stumblecache_functions,
	PHP_MINIT(stumblecache),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(stumblecache),
#if ZEND_MODULE_API_NO >= 20010901
	"0.0.1",
#endif
	STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_STUMBLECACHE
ZEND_GET_MODULE(stumblecache)
#endif

ZEND_DECLARE_MODULE_GLOBALS(stumblecache)

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("stumblecache.default_cache_dir", "/tmp", PHP_INI_ALL, OnUpdateString, default_cache_dir, zend_stumblecache_globals, stumblecache_globals)
	STD_PHP_INI_ENTRY("stumblecache.default_ttl", "18000", PHP_INI_ALL, OnUpdateLong, default_ttl, zend_stumblecache_globals, stumblecache_globals)
PHP_INI_END()

 
static void stumblecache_init_globals(zend_stumblecache_globals *stumblecache_globals)
{
	stumblecache_globals->default_cache_dir = "/tmp";
}

/* Variable declarations */
zend_class_entry *stumblecache_ce;
zend_object_handlers stumblecache_object_handlers;

/* Forward method declarations */
PHP_METHOD(StumbleCache, __construct);
PHP_METHOD(StumbleCache, getInfo);
PHP_METHOD(StumbleCache, getPath);
PHP_METHOD(StumbleCache, dump);
PHP_METHOD(StumbleCache, add);
PHP_METHOD(StumbleCache, remove);
PHP_METHOD(StumbleCache, fetch);

/* Reflection information */
ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_construct, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_getinfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_getpath, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_dump, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_add, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_remove, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_fetch, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, ttl)
ZEND_END_ARG_INFO()

/* StumbleCache methods */
zend_function_entry stumblecache_funcs[] = {
	PHP_ME(StumbleCache, __construct,     arginfo_stumblecache_construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, getInfo,         arginfo_stumblecache_getinfo,   ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, getPath,         arginfo_stumblecache_getpath,   ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, dump,            arginfo_stumblecache_dump,      ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, add,             arginfo_stumblecache_add,       ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, remove,          arginfo_stumblecache_remove,    ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, fetch,           arginfo_stumblecache_fetch,     ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* Class helper functions */
zval *stumblecache_instantiate(zend_class_entry *pce, zval *object TSRMLS_DC)
{
	Z_TYPE_P(object) = IS_OBJECT;
	object_init_ex(object, pce);
	Z_SET_REFCOUNT_P(object, 1);
	Z_UNSET_ISREF_P(object);
	return object;
}

static void stumblecache_object_free_storage(void *object TSRMLS_DC)
{
	php_stumblecache_obj *intern = (php_stumblecache_obj *) object;

	if (intern->cache) {
		btree_close(intern->cache);
		btree_free(intern->cache);
		intern->cache = NULL;
	}

	if (intern->path) {
		efree(intern->path);
		intern->path = NULL;
	}

	zend_object_std_dtor(&intern->std TSRMLS_CC);
	efree(object);
}

static inline zend_object_value stumblecache_object_new_ex(zend_class_entry *class_type, php_stumblecache_obj **ptr TSRMLS_DC)
{
	php_stumblecache_obj *intern;
	zend_object_value retval;
	zval *tmp;

	intern = emalloc(sizeof(php_stumblecache_obj));
	memset(intern, 0, sizeof(php_stumblecache_obj));
	if (ptr) {
		*ptr = intern;
	}

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
#if PHP_MINOR_VERSION > 3
	object_properties_init(&intern->std, class_type);
#else
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
#endif
	
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) stumblecache_object_free_storage, NULL TSRMLS_CC);
	retval.handlers = &stumblecache_object_handlers;
	
	return retval;
}

static zend_object_value stumblecache_object_new(zend_class_entry *class_type TSRMLS_DC)
{
	return stumblecache_object_new_ex(class_type, NULL TSRMLS_CC);
}

static int scache_parse_options(zval *options, uint32_t *order, uint32_t *max_items, uint32_t *max_datasize TSRMLS_DC)
{
	zval **dummy;
	int    set_count = 0;

	if (Z_TYPE_P(options) == IS_ARRAY) {
		if (zend_hash_find(HASH_OF(options), "order", 6, (void**) &dummy) == SUCCESS) {
			convert_to_long(*dummy);
			*order = Z_LVAL_PP(dummy);
			if (*order < 3 || *order > BTREE_MAX_ORDER) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "The order should be in between 3 and %d, %d was requested.", BTREE_MAX_ORDER, *order);
				return 0;
			}
			set_count++;
		}
		if (zend_hash_find(HASH_OF(options), "max_items", 10, (void**) &dummy) == SUCCESS) {
			convert_to_long(*dummy);
			*max_items = Z_LVAL_PP(dummy);
			if (*max_items < 1 || *max_items > 1024 * 1048576) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "The max_items should setting be in between 1 and %d, %ld was requested.", 1024 * 1048576, *max_items);
				return 0;
			}
			set_count++;
		}
		if (zend_hash_find(HASH_OF(options), "max_datasize", 13, (void**) &dummy) == SUCCESS) {
			convert_to_long(*dummy);
			*max_datasize = Z_LVAL_PP(dummy);
			if (*max_datasize < 1 || *max_datasize > 1048576) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "The max_datasize setting should be in between 1 and %d, %ld was requested.", 1048576, *max_datasize);
				return 0;
			}
			set_count++;
		}
		if (set_count != 3) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not all three options are set (need: 'order', 'max_items' and 'max_datasize').");
		} else {
			return 1;
		}
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The options should be passed in as an array.");
	}
	return 0;
}

static int stumblecache_initialize(php_stumblecache_obj *obj, char *cache_id, zval *options TSRMLS_DC)
{
	char *path;
	uint32_t order, max_items, max_datasize;

	/* Create filepath */
	if (cache_id[0] != '/') {
		spprintf(&path, 128, "%s/%s.scache", STUMBLECACHE_G(default_cache_dir), cache_id);
	} else {
		spprintf(&path, 128, "%s.scache", cache_id);
	}

	obj->cache = btree_open(path);
	obj->path  = NULL;
	if (!obj->cache) {
		if (!scache_parse_options(options, &order, &max_items, &max_datasize TSRMLS_CC)) {
			efree(path);
			return 0;
		}
		obj->cache = btree_create(path, order, max_items, max_datasize);
		if (!obj->cache) {
			efree(path);
			return 0;
		}
	}
	obj->path = path;
	return 1;
}

void stumblecache_register_class(TSRMLS_D)
{
	zend_class_entry ce_stumblecache;

	INIT_CLASS_ENTRY(ce_stumblecache, "StumbleCache", stumblecache_funcs);
	ce_stumblecache.create_object = stumblecache_object_new;
	stumblecache_ce = zend_register_internal_class_ex(&ce_stumblecache, NULL, NULL TSRMLS_CC);

	memcpy(&stumblecache_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
}

PHP_METHOD(StumbleCache, __construct)
{
	char *cache_id;
	int   cache_id_len;
	zval *options = NULL;

	php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &cache_id, &cache_id_len, &options) == SUCCESS) {
		if (!stumblecache_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), cache_id, options TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not initialize cache.");
		}
	}
	php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}

PHP_METHOD(StumbleCache, getPath)
{
	zval *object;
	php_stumblecache_obj *scache_obj;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, stumblecache_ce) == FAILURE) {
		return;
	}

	php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(object TSRMLS_CC);
	php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);

	RETURN_STRING(scache_obj->path, 1);
}

/* Returns some simple statistics */
PHP_METHOD(StumbleCache, getInfo)
{
	zval *object;
	php_stumblecache_obj *scache_obj;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, stumblecache_ce) == FAILURE) {
		return;
	}

	php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(object TSRMLS_CC);
	php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);

	array_init(return_value);
	add_assoc_long_ex(return_value, "version", sizeof("version"), scache_obj->cache->header->version);
	add_assoc_long_ex(return_value, "order", sizeof("order"), scache_obj->cache->header->order);
	add_assoc_long_ex(return_value, "max_items", sizeof("max_items"), scache_obj->cache->header->max_items);
	add_assoc_long_ex(return_value, "item_count", sizeof("item_count"), scache_obj->cache->header->item_count);
	add_assoc_long_ex(return_value, "item_size", sizeof("item_size"), scache_obj->cache->header->item_size);
	add_assoc_long_ex(return_value, "node_count", sizeof("node_count"), scache_obj->cache->header->node_count);
}

PHP_METHOD(StumbleCache, dump)
{
	zval *object;
	php_stumblecache_obj *scache_obj;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, stumblecache_ce) == FAILURE) {
		return;
	}

	php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(object TSRMLS_CC);
	php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);

	btree_dump(scache_obj->cache);
}


struct stumblecache_mm_context {
	void *data;
	size_t max_size;
};

static inline void *stumblecache_alloc(size_t size, void *context)
{
	return ((struct stumblecache_mm_context*) context)->data;
}

static inline void *stumblecache_realloc(void *ptr, size_t newsize, void *context)
{
	if (newsize > ((struct stumblecache_mm_context*) context)->max_size) {
		return NULL;
	}
	return ptr;
}

static inline void stumblecache_free(void *ptr, void *context)
{
	return;
}

PHP_METHOD(StumbleCache, add)
{
	zval *object;
	php_stumblecache_obj *scache_obj;
	long  key;
	zval *value;
	uint32_t data_idx;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Olz", &object, stumblecache_ce, &key, &value) == FAILURE) {
		return;
	}

	php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(object TSRMLS_CC);
	php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);

	if (btree_insert(scache_obj->cache, key, &data_idx)) {
		size_t   serialized_len;
		void    *data, *dummy;
		size_t  *data_size;
		time_t  *ts;
		struct igbinary_memory_manager *mm;
		struct stumblecache_mm_context context;

		/* Add data */
		btree_get_data_ptr(scache_obj->cache, data_idx, (void**) &data, (size_t**) &data_size, (time_t**) &ts);

		*ts = time(NULL);

		/* Prepare memory manager for add */
		context.data = data;
		context.max_size = scache_obj->cache->header->item_size;

		mm = malloc(sizeof(struct igbinary_memory_manager));
		mm->alloc = stumblecache_alloc;
		mm->realloc = stumblecache_realloc;
		mm->free = stumblecache_free;
		mm->context = (void*) &context;

		if (igbinary_serialize_ex((uint8_t **) &dummy, data_size, value, mm TSRMLS_CC) == 0) {
			RETVAL_TRUE;
		} else {
			RETVAL_FALSE;
		}
		free(mm);
		btree_data_unlock(scache_obj->cache, data_idx);
		return;
	}

	/* Need to remove the element now */
	RETURN_FALSE;
}

PHP_METHOD(StumbleCache, remove)
{
	zval *object;
	php_stumblecache_obj *scache_obj;
	long  key;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &object, stumblecache_ce, &key) == FAILURE) {
		return;
	}

	php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(object TSRMLS_CC);
	php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);

	if (btree_delete(scache_obj->cache, key)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

PHP_METHOD(StumbleCache, fetch)
{
	zval *object;
	php_stumblecache_obj *scache_obj;
	long  key;
	uint32_t data_idx;
	void    *data;
	size_t data_size;
	time_t ts;
	long   ttl = STUMBLECACHE_G(default_ttl);

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol|l", &object, stumblecache_ce, &key, &ttl) == FAILURE) {
		return;
	}

	php_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(object TSRMLS_CC);
	php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);

	if (btree_search(scache_obj->cache, scache_obj->cache->root, key, &data_idx)) {
		/* Retrieve data */
		data = btree_get_data(scache_obj->cache, data_idx, &data_size, &ts);
		btree_data_unlock(scache_obj->cache, data_idx);

		/* Check whether the data is fresh */
		if (time(NULL) < ts + ttl) {
			if (data_size) {
				igbinary_unserialize((uint8_t *) data, data_size, &return_value TSRMLS_CC);
				return;
			}
		} else {
			btree_delete(scache_obj->cache, key);
		}
	}
	return; /* Implicit return NULL; */
}

PHP_MINIT_FUNCTION(stumblecache)
{
	ZEND_INIT_MODULE_GLOBALS(stumblecache, stumblecache_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	stumblecache_register_class(TSRMLS_C);

	return SUCCESS;
}


PHP_MINFO_FUNCTION(stumblecache)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "stumblecache support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
