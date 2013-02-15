--TEST--
Stumblecache->getLastError() parameters
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
$cache = new StumbleCache(dirname(__FILE__) . '/tests-error', $options);

var_dump($cache->getLastError(10));
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-error.scache');
?>
--EXPECTF--
Warning: StumbleCache::getLastError() expects exactly 0 parameters, 1 given in %s on line %d
NULL
