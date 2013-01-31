--TEST--
Extend Stumblecache class
--EXTENSIONS--
igbinary
--FILE--
<?php

// extend class, do not override constructor
class GoodCache extends StumbleCache {}

// extend class, override constructor, do not call parent
class BadCache extends StumbleCache
{
	public function __construct() {}
}

// extend class, override constructor, call parent
class WorksCache extends StumbleCache
{
	public function __construct($path, $options) {
		parent::__construct($path, $options);
	}
}

// options for all classes
$options = array(
	'order' => 3,
	'max_items' => 1024,
	'max_datasize' => 32,
);

// path to use
$path = dirname(__FILE__) . '/';

$cache = new GoodCache($path . 'good-cache' , $options );
var_dump( $cache->add( 50, "some data" ) );

try {
	$cache = new BadCache($path . 'bad-cache' , $options );
} catch (Exception $e) {
	echo $e->getMessage(), PHP_EOL;
}

$cache = new WorksCache($path . 'works-cache' , $options );
var_dump( $cache->add( 50, "some data" ) );
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/good-cache.scache');
unlink(dirname(__FILE__) . '/bad-cache.scache');
unlink(dirname(__FILE__) . '/works-cache.scache');
?>
--EXPECT--
bool(true)
parent::__construct() must be called in BadCache::__construct()
bool(true)
