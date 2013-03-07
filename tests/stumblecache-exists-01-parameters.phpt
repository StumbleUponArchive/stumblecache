--TEST--
Stumblecache->exists() parameters
--EXTENSIONS--
igbinary
--FILE--
<?php
// Initialise the cache
$options = array(
	'order' => 3,
	'max_items' => 1024,
	'max_datasize' => 32,
);
$cache = new StumbleCache(dirname(__FILE__) . '/tests-exists', $options);

var_dump($cache->exists());
var_dump($cache->exists(array()));
var_dump($cache->exists('10'));
var_dump($cache->exists('10', 51));

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-exists.scache');
unlink(dirname(__FILE__) . '/tests-exists.scstats');
?>
--EXPECTF--
Warning: StumbleCache::exists() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: StumbleCache::exists() expects parameter 1 to be long, array given in %s on line %d
NULL
bool(false)

Warning: StumbleCache::exists() expects exactly 1 parameter, 2 given in %s on line %d
NULL
