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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "main/php_open_temporary_file.h"

#include "btree/btree.h"
#include "php_stumblecache.h"

#include "ext/igbinary/igbinary.h"

/* ----------------------------------------------------------------
	Globals and INI Management
------------------------------------------------------------------*/

ZEND_DECLARE_MODULE_GLOBALS(stumblecache)

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("stumblecache.default_cache_dir", "/tmp", PHP_INI_ALL, OnUpdateString, default_cache_dir, zend_stumblecache_globals, stumblecache_globals)
	STD_PHP_INI_ENTRY("stumblecache.default_ttl", "18000", PHP_INI_ALL, OnUpdateLong, default_ttl, zend_stumblecache_globals, stumblecache_globals)
PHP_INI_END()

/* {{{ default for cache dir is php temp dir */
static void stumblecache_init_globals(zend_stumblecache_globals *stumblecache_globals TSRMLS_DC)
{
#if PHP_VERSION_ID >= 50500 
	const char* temp_dir = php_get_temporary_directory(TSRMLS_C);
#else
	const char* temp_dir = php_get_temporary_directory();
#endif
	if (temp_dir && *temp_dir != '\0' && !php_check_open_basedir(temp_dir TSRMLS_CC)) {
		stumblecache_globals->default_cache_dir = estrdup(temp_dir);
	} else {
	       stumblecache_globals->default_cache_dir = "/tmp";
	}
}
/* }}} */

/* ----------------------------------------------------------------
	Extension Definition
------------------------------------------------------------------*/

/* Forward declaration of code to register class */
void stumblecache_register_class(TSRMLS_D);

/* {{{ stumblecache requires igbinary for serialization */
static const zend_module_dep stumblecache_module_deps[] = {
	ZEND_MOD_REQUIRED("igbinary")
	ZEND_MOD_END
};
/* }}} */

/* {{{ module entry has only minit, minfo, deps and version */
zend_module_entry stumblecache_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	stumblecache_module_deps,
	"stumblecache",
	NULL,
	PHP_MINIT(stumblecache),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(stumblecache),
	PHP_STUMBLECACHE_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_STUMBLECACHE
ZEND_GET_MODULE(stumblecache)
#endif

/* {{{ init our globals and register our class */
PHP_MINIT_FUNCTION(stumblecache)
{
	ZEND_INIT_MODULE_GLOBALS(stumblecache, stumblecache_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	stumblecache_register_class(TSRMLS_C);

	return SUCCESS;
}
/* }}} */

/* {{{ show version, ini entries and enabled for phpinfo */
PHP_MINFO_FUNCTION(stumblecache)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "stumblecache support", "enabled");
	php_info_print_table_header(2, "version", PHP_STUMBLECACHE_VERSION);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* ----------------------------------------------------------------
	StumbleCache Class Definition
------------------------------------------------------------------*/

/* class entry, object handlers, object struct */
struct _php_stumblecache_obj {
	zend_object   std;
	btree_tree   *cache;
	char         *path;
	char	     *error_method;
        int           error_line;
        char         *error_file;
        int           error_code;
	zend_bool     is_constructed;
};
static zend_class_entry *stumblecache_ce;
static zend_object_handlers stumblecache_object_handlers;
static zend_function stumblecache_ctor_wrapper_func;

/* memory management struct and helpers for igbinary serialization */
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

/* Forward method declarations */
PHP_METHOD(StumbleCache, __construct);
PHP_METHOD(StumbleCache, getInfo);
PHP_METHOD(StumbleCache, getPath);
PHP_METHOD(StumbleCache, dump);
PHP_METHOD(StumbleCache, add);
PHP_METHOD(StumbleCache, replace);
PHP_METHOD(StumbleCache, set);
PHP_METHOD(StumbleCache, remove);
PHP_METHOD(StumbleCache, fetch);
PHP_METHOD(StumbleCache, exists);
PHP_METHOD(StumbleCache, getLastError);

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

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_replace, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_set, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_exists, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_remove, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_fetch, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, ttl)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_stumblecache_getlasterror, 0, 0, 0)
ZEND_END_ARG_INFO()

/* StumbleCache methods */
zend_function_entry stumblecache_methods[] = {
	PHP_ME(StumbleCache, __construct,     arginfo_stumblecache_construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, getInfo,         arginfo_stumblecache_getinfo,   ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, getPath,         arginfo_stumblecache_getpath,   ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, dump,            arginfo_stumblecache_dump,      ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, add,             arginfo_stumblecache_add,       ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, replace,         arginfo_stumblecache_replace,   ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, set,             arginfo_stumblecache_set,       ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, exists,          arginfo_stumblecache_exists,    ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, remove,          arginfo_stumblecache_remove,    ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, fetch,           arginfo_stumblecache_fetch,     ZEND_ACC_PUBLIC)
	PHP_ME(StumbleCache, getLastError,    arginfo_stumblecache_getlasterror,ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

/* Class helper functions */

/* {{{ free information from object struct */
static void stumblecache_object_free_storage(void *object TSRMLS_DC)
{
	php_stumblecache_obj *intern = (php_stumblecache_obj *) object;

	intern->is_constructed = 0;

	if (intern->cache) {
		btree_close(intern->cache);
		intern->cache = NULL;
	}

	if (intern->path) {
		efree(intern->path);
		intern->path = NULL;
	}
	if (intern->error_file) {
		efree(intern->error_file);
	}
	if (intern->error_method) {
		efree(intern->error_method);
	}

	zend_object_std_dtor(&intern->std TSRMLS_CC);
	efree(object);
}
/* }}} */

/* {{{ set up internal information for the object struct */
static inline zend_object_value stumblecache_object_new(zend_class_entry *class_type TSRMLS_DC)
{
	php_stumblecache_obj *intern;
	zend_object_value retval;
	zval *tmp;

	intern = ecalloc(1, sizeof(php_stumblecache_obj));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
#if PHP_VERSION_ID >= 50400 
	object_properties_init(&intern->std, class_type);
#else
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
#endif
	intern->cache = NULL;
	intern->path = NULL;
	intern->is_constructed = 0;
	
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) stumblecache_object_free_storage, NULL TSRMLS_CC);
	retval.handlers = &stumblecache_object_handlers;
	
	return retval;
}
/* }}} */

/* {{{ make our object subclassable */
static zend_function *stumblecache_get_constructor(zval *object TSRMLS_DC)
{
	if (Z_OBJCE_P(object) == stumblecache_ce) {
		return zend_get_std_object_handlers()->get_constructor(object TSRMLS_CC);\
	} else {
        	return &stumblecache_ctor_wrapper_func;
 	}
}
/* }}} */

/* {{{ our constructor wrapper */
static void stumblecache_constructor_wrapper(INTERNAL_FUNCTION_PARAMETERS) {
    zend_fcall_info_cache fci_cache = {0};
    zend_fcall_info fci = {0};
    zend_class_entry *this_ce;
    zend_function *zf;
    php_stumblecache_obj *obj;
    zval *_this = getThis(), *retval_ptr = NULL;

    obj = zend_object_store_get_object(_this TSRMLS_CC);
    zf = zend_get_std_object_handlers()->get_constructor(_this TSRMLS_CC);
    this_ce = Z_OBJCE_P(_this);

    fci.size = sizeof(fci);
    fci.function_table = &this_ce->function_table;
    fci.retval_ptr_ptr = &retval_ptr;
    fci.object_ptr = _this;
    fci.param_count = ZEND_NUM_ARGS();
    fci.params = emalloc(fci.param_count * sizeof *fci.params);
    fci.no_separation = 0;
    _zend_get_parameters_array_ex(fci.param_count, fci.params TSRMLS_CC);

    fci_cache.initialized = 1;
    fci_cache.called_scope = EG(current_execute_data)->called_scope;
    fci_cache.calling_scope = EG(current_execute_data)->current_scope;
    fci_cache.function_handler = zf;
    fci_cache.object_ptr = _this;

    zend_call_function(&fci, &fci_cache TSRMLS_CC);

    if (!EG(exception) && obj->is_constructed == 0)
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
            "parent::__construct() must be called in %s::__construct()", this_ce->name);

    efree(fci.params);
    zval_ptr_dtor(&retval_ptr);
}
/* }}} */

/* {{{ validate our options array items */
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
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "The max_items should setting be in between 1 and %d, %ld was requested.", 1024 * 1048576, (long int) *max_items);
				return 0;
			}
			set_count++;
		}
		if (zend_hash_find(HASH_OF(options), "max_datasize", 13, (void**) &dummy) == SUCCESS) {
			convert_to_long(*dummy);
			*max_datasize = Z_LVAL_PP(dummy);
			if (*max_datasize < 1 || *max_datasize > 1048576) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "The max_datasize setting should be in between 1 and %d, %ld was requested.", 1048576, (long int) *max_datasize);
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
/* }}} */

/* {{{ helper to initialize the internal btree */
static int stumblecache_initialize(php_stumblecache_obj *obj, char *cache_id, zval *options TSRMLS_DC)
{
	char *path;
	uint32_t order, max_items, max_datasize;
	int error;

	/* Create filepath */
	if (cache_id[0] != '/') {
		spprintf(&path, 128, "%s/%s.scache", STUMBLECACHE_G(default_cache_dir), cache_id);
	} else {
		spprintf(&path, 128, "%s.scache", cache_id);
	}

	obj->cache = btree_open(path, &error);
	obj->path  = NULL;
	if (!obj->cache) {
		if (!scache_parse_options(options, &order, &max_items, &max_datasize TSRMLS_CC)) {
			efree(path);
			return 0;
		}
		obj->cache = btree_create(path, order, max_items, max_datasize, &error);
		if (!obj->cache) {
			efree(path);
			return 0;
		}
	}
	obj->path = path;
	obj->error_code = 0;
	obj->error_line = 0;
	obj->error_file = NULL;
	obj->error_method = NULL;
	obj->is_constructed = 1;
	return 1;
}
/* }}} */

/* {{{ helper to register the class with php */
void stumblecache_register_class(TSRMLS_D)
{
	zend_class_entry ce_stumblecache;

	INIT_CLASS_ENTRY(ce_stumblecache, "StumbleCache", stumblecache_methods);
	ce_stumblecache.create_object = stumblecache_object_new;
	stumblecache_ce = zend_register_internal_class_ex(&ce_stumblecache, NULL, NULL TSRMLS_CC);

	memcpy(&stumblecache_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	stumblecache_object_handlers.get_constructor = stumblecache_get_constructor;
	stumblecache_object_handlers.clone_obj = NULL;
	
	stumblecache_ctor_wrapper_func.type = ZEND_INTERNAL_FUNCTION;
	stumblecache_ctor_wrapper_func.common.function_name = "internal_construction_wrapper";
	stumblecache_ctor_wrapper_func.common.scope = stumblecache_ce;
	stumblecache_ctor_wrapper_func.common.fn_flags = ZEND_ACC_PROTECTED;
	stumblecache_ctor_wrapper_func.common.prototype = NULL;
	stumblecache_ctor_wrapper_func.common.required_num_args = 0;
	stumblecache_ctor_wrapper_func.common.arg_info = NULL;
#if PHP_VERSION_ID < 50399
	stumblecache_ctor_wrapper_func.common.pass_rest_by_reference = 0;
	stumblecache_ctor_wrapper_func.common.return_reference = 0;
#endif
	stumblecache_ctor_wrapper_func.internal_function.handler = stumblecache_constructor_wrapper;
	stumblecache_ctor_wrapper_func.internal_function.module = EG(current_module);
}
/* }}} */

/* {{{ helper to save error information */
void stumblecache_save_error(php_stumblecache_obj *scache_obj, const char * method, int code TSRMLS_DC)
{
	if (scache_obj->error_file) {
		efree(scache_obj->error_file);
	}
	if (scache_obj->error_method) {
		efree(scache_obj->error_method);
	}

	scache_obj->error_line = zend_get_executed_lineno(TSRMLS_C);
	scache_obj->error_file = estrdup(zend_get_executed_filename(TSRMLS_C));
	scache_obj->error_method = estrdup(method);
	scache_obj->error_code = code;
}

/* ----------------------------------------------------------------
	StumbleCache Public API
------------------------------------------------------------------*/

/* {{{ proto void StumbleCache->__construct(string id, array options)
	options MUST be
        array('order' => integer between 3 and BTREE_MAX_ORDER,
              'max_datasize' => integer between 1 and 1048576,
               'max_items' => integer between 1 and 1024 * 1048576)
        Creates a new stumblecache instance, using the filename provided.

        If an absolute path is given, .scache will be appended to the name
        and the file created or opened.  Otherwise the default cache directory
        will be used - php_get_sys_temp_dir or the value set in php.ini
        and .scache will be appended

        Will throw an exception if there is a problem creating the stumblecache file

	TODO: get backend library errors and provide
        meaningful error messages and check open_base_dir setting!
        also move initialization in here, parsing code can stay separate
*/
PHP_METHOD(StumbleCache, __construct)
{
	char *cache_id;
	int   cache_id_len;
	zval *options = NULL;
	zend_error_handling error_handling;

	zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &cache_id, &cache_id_len, &options) == SUCCESS) {
		if (!stumblecache_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), cache_id, options TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not initialize cache.");
		}
	}
	zend_restore_error_handling(&error_handling TSRMLS_CC);
}
/* }}} */

/* {{{ proto string StumbleCache->getPath(void)
	returns the absolute path and filename (including extension) of the file currently in use
*/
PHP_METHOD(StumbleCache, getPath)
{
	php_stumblecache_obj *scache_obj;

	if (FAILURE == zend_parse_parameters_none()) {
		return;
	}

	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	RETURN_STRING(scache_obj->path, 1);
}
/* }}} */

/* {{{ proto array StumbleCache->getInfo(void)
	returns an array of information about the btree currently in use
	array('version' => integer version #,
		'order' => integer order value (supplied in constructor),
		'max_items' => integer max items (supplied in constructor),
		'item_count' => integer total number of items in tree,
		'item_size' => integer size of items stored,
                'node_count' => number of nodes in tree)
*/
PHP_METHOD(StumbleCache, getInfo)
{
	zval *object;
	php_stumblecache_obj *scache_obj;

	if (FAILURE == zend_parse_parameters_none()) {
		return;
	}

	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	array_init(return_value);
	add_assoc_long_ex(return_value, "version", sizeof("version"), scache_obj->cache->header->version);
	add_assoc_long_ex(return_value, "order", sizeof("order"), scache_obj->cache->header->order);
	add_assoc_long_ex(return_value, "max_items", sizeof("max_items"), scache_obj->cache->header->max_items);
	add_assoc_long_ex(return_value, "item_count", sizeof("item_count"), scache_obj->cache->header->item_count);
	add_assoc_long_ex(return_value, "item_size", sizeof("item_size"), scache_obj->cache->header->item_size);
	add_assoc_long_ex(return_value, "node_count", sizeof("node_count"), scache_obj->cache->header->node_count);
}
/* }}} */

/* {{{ proto bool StumbleCache->dump(void)
	outputs a dump (via php_printf) of the current
        contents of the opened cache
	TODO: add different output formats for dump
*/
PHP_METHOD(StumbleCache, dump)
{
	int error = 0;
	php_stumblecache_obj *scache_obj;

	if (FAILURE == zend_parse_parameters_none()) {
		return;
	}

	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	error = btree_dump(scache_obj->cache);
	if (error) {
		stumblecache_save_error(scache_obj, "dump", error TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool StumbleCache->add(integer key, mixed value)
	adds an item to the btree
	if the item already exists then no add takes place
*/
PHP_METHOD(StumbleCache, add)
{
	php_stumblecache_obj *scache_obj;
	long  key;
	zval *value;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz", &key, &value)) {
		return;
	}

	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	if (0 == btree_insert(scache_obj->cache, key)) {
		size_t   serialized_len;
		uint32_t data_idx;
		void    *data, *dummy;
		size_t  *data_size;
		time_t  *ts;
		int error;
		struct igbinary_memory_manager *mm;
		struct stumblecache_mm_context context;

		error = btree_get_data_ptr(scache_obj->cache, key, &data_idx, (void**) &data, (size_t**) &data_size, (time_t**) &ts);
		/* Try to get our ptr */
		if (0 != error) {
			stumblecache_save_error(scache_obj, "add", error TSRMLS_CC);
			RETURN_FALSE;
		} else {

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
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool StumbleCache->replace(integer key, mixed value)
	changes value of item already in btree, does NOT add it if it doesn't exist
*/
PHP_METHOD(StumbleCache, replace)
{
	php_stumblecache_obj *scache_obj;
	long  key;
	zval *value;

	size_t   serialized_len;
	uint32_t data_idx;
	void    *data, *dummy;
	size_t  *data_size;
	time_t  *ts;
	int error;
	struct igbinary_memory_manager *mm;
	struct stumblecache_mm_context context;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz", &key, &value)) {
		return;
	}

	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	error = btree_get_data_ptr(scache_obj->cache, key, &data_idx, (void**) &data, (size_t**) &data_size, (time_t**) &ts);
	
	/* Try to get our ptr */
	if (0 != error) {
		stumblecache_save_error(scache_obj, "add", error TSRMLS_CC);
		RETURN_FALSE;
	} else {

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
			stumblecache_save_error(scache_obj, "add", 406 TSRMLS_CC);
			RETVAL_FALSE;
		}
		free(mm);
		btree_data_unlock(scache_obj->cache, data_idx);
		return;
	}
}
/* }}} */

/* {{{ proto bool StumbleCache->set(integer key, mixed value)
	if the item doesn't exist, adds it
        if it does exist, replace the value
        identical to calling add followed by replace on failure
*/
PHP_METHOD(StumbleCache, set)
{
	php_stumblecache_obj *scache_obj;
	long  key;
	zval *value;
	int error = 0;

	size_t   serialized_len;
	uint32_t data_idx;
	void    *data, *dummy;
	size_t  *data_size;
	time_t  *ts;
	struct igbinary_memory_manager *mm;
	struct stumblecache_mm_context context;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz", &key, &value)) {
		return;
	}

	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	/* we'll attempt an insert - if it's 404 ignore and continue */
	error = btree_insert(scache_obj->cache, key);

	if (error != 409 && error != 0) {
		stumblecache_save_error(scache_obj, "set", error TSRMLS_CC);	
		RETURN_FALSE;
	}

	error = btree_get_data_ptr(scache_obj->cache, key, &data_idx, (void**) &data, (size_t**) &data_size, (time_t**) &ts);
	/* Try to get our ptr */
	if (0 != error) {
		stumblecache_save_error(scache_obj, "set", error TSRMLS_CC);
		RETURN_FALSE;
	} else {

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
}
/* }}} */


/* {{{ proto bool StumbleCache->exists(integer key)
	checks to see if an item exists in the cache
*/
PHP_METHOD(StumbleCache, exists)
{
	php_stumblecache_obj *scache_obj;
	long  key;
	int error;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &key)) {
		return;
	}

	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
	error = btree_search(scache_obj->cache, scache_obj->cache->root, key);

	if (0 == error) {
		RETURN_TRUE;
	} else {
		stumblecache_save_error(scache_obj, "exists", error TSRMLS_CC);
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool StumbleCache->remove(integer key)
	removes an item from the cache
*/
PHP_METHOD(StumbleCache, remove)
{
	long  key;
	int error = 0;
	uint32_t data_idx;
	php_stumblecache_obj *scache_obj;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &key) == FAILURE) {
		return;
	}

	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	error = btree_delete(scache_obj->cache, key);
	if (error == 0) {
		RETURN_TRUE;
	} else {
		stumblecache_save_error(scache_obj, "remove", error TSRMLS_CC);
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto mixed StumbleCache->fetch(integer key)
	fetch's data from the cache
*/
PHP_METHOD(StumbleCache, fetch)
{
	php_stumblecache_obj *scache_obj;
	long  key;
	uint32_t data_idx;
	void    *data;
	size_t *data_size;
	time_t *ts;
	int error;
	long   ttl = STUMBLECACHE_G(default_ttl);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &key, &ttl) == FAILURE) {
		return;
	}

	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	error = btree_get_data(scache_obj->cache, key, &data_idx, (void**) &data, (size_t**) &data_size, (time_t**) &ts);

	if (0 == error) {
		/* unlock, we're good */
		btree_data_unlock(scache_obj->cache, data_idx);

		/* Check whether the data is fresh */
		if (time(NULL) < *ts + ttl) {
			if (*data_size) {
				igbinary_unserialize((uint8_t *) data, *data_size, &return_value TSRMLS_CC);
				return;
			}
		} else {
			btree_delete(scache_obj->cache, key);
		}
	} else {
		stumblecache_save_error(scache_obj, "fetch", error TSRMLS_CC);	
	}
	return; /* Implicit return NULL; */
}
/* }}} */

/* {{{ proto array StumbleCache->getLastError(void)
	returns array of error informaion from the last method reporting errors
	array('code' => error code,
	      'message' => error message,
		'file' => file where it occured,
		'line' => line where it occured,
		'method' => method of class that did it)
*/
PHP_METHOD(StumbleCache, getLastError)
{
	int code;
	char *buf;
	php_stumblecache_obj *scache_obj;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	scache_obj = (php_stumblecache_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	array_init(return_value);

	code = scache_obj->error_code;
	add_assoc_long(return_value, "code", code);
	if (0 != code) {

		switch(code) 
		{
		    case 404:
			add_assoc_string(return_value, "message", "Item not found", 1);
			break;
		    case 406:
			add_assoc_string(return_value, "message", "Error serializing data, not acceptable", 1);
			break;
		    case 409:
			add_assoc_string(return_value, "message", "Conflict, item already exists", 1);
			break;
		    case 413:
			add_assoc_string(return_value, "message", "Maximum number of items reached, could not add additional item", 1);
			break;
		    case 414:
			add_assoc_string(return_value, "message", "Item too large to be stored", 1);
			break;
		    case 500:
			add_assoc_string(return_value, "message", "Internal error, node could not be retrieved", 1);
			break;
		    default :
			buf = emalloc(1024);
			strerror_r(code, buf, 1024);
			add_assoc_string(return_value, "message", buf, 1);
			efree(buf);
		}
		add_assoc_string(return_value, "file", scache_obj->error_file, 1);
		add_assoc_long(return_value, "line", scache_obj->error_line);
		add_assoc_string(return_value, "method", scache_obj->error_method, 1);
		
	} else {
		
		add_assoc_string(return_value, "file", estrdup(zend_get_executed_filename(TSRMLS_C)), 0);
		add_assoc_long(return_value, "line", zend_get_executed_lineno(TSRMLS_C));
		add_assoc_string(return_value, "message", "No Error", 1);
		add_assoc_string(return_value, "method", "", 1);
	}
}
/* }}} */
