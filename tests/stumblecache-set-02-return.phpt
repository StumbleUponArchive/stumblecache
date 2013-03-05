--TEST--
Stumblecache->set() return values
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
var_dump($cache->set(10, "some data"));
$cache->add(30, "some data");
var_dump($cache->set(30, "my data"));

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-set.scache');
unlink(dirname(__FILE__) . '/tests-set.scstats');
?>
--EXPECTF--
Warning: StumbleCache::set() expects exactly 2 parameters, 0 given in %s on line %d
NULL
bool(true)
bool(true)
