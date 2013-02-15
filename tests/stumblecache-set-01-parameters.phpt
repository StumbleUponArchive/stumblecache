--TEST--
Stumblecache->set() parameters
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
$cache = new StumbleCache(dirname(__FILE__) . '/tests-set', $options);

var_dump($cache->set());
var_dump($cache->set(10));
var_dump($cache->set(array(), 10));
var_dump($cache->set(10, 10, 10));

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-set.scache');
?>
--EXPECTF--
Warning: StumbleCache::set() expects exactly 2 parameters, 0 given in %s on line %d
NULL

Warning: StumbleCache::set() expects exactly 2 parameters, 1 given in %s on line %d
NULL

Warning: StumbleCache::set() expects parameter 1 to be long, array given in %s on line %d
NULL

Warning: StumbleCache::set() expects exactly 2 parameters, 3 given in %s on line %d
NULL
