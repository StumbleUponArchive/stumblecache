--TEST--
Stumblecache->replace() parameters
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
$cache = new StumbleCache(dirname(__FILE__) . '/tests-replace', $options);

var_dump($cache->replace());
var_dump($cache->replace(10));
var_dump($cache->replace(array(), 10));
var_dump($cache->replace(10, 10, 10));

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-replace.scache');
?>
--EXPECTF--
Warning: StumbleCache::replace() expects exactly 2 parameters, 0 given in %s on line %d
NULL

Warning: StumbleCache::replace() expects exactly 2 parameters, 1 given in %s on line %d
NULL

Warning: StumbleCache::replace() expects parameter 1 to be long, array given in %s on line %d
NULL

Warning: StumbleCache::replace() expects exactly 2 parameters, 3 given in %s on line %d
NULL
