--TEST--
Stumblecache->exists() return values
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
var_dump($cache->exists(10));

$cache->add(10, "some data");
var_dump($cache->exists(10));

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-exists.scache');
unlink(dirname(__FILE__) . '/tests-exists.scstats');
?>
--EXPECTF--
Warning: StumbleCache::exists() expects exactly 1 parameter, 0 given in %s on line %d
NULL
bool(false)
bool(true)
