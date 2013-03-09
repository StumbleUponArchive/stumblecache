--TEST--
Stumblecache->getStats() parameters
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
$cache = new StumbleCache(dirname(__FILE__) . '/tests-stats', $options);

var_dump($cache->getStats(1));

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-stats.scache');
unlink(dirname(__FILE__) . '/tests-stats.scstats');
?>
--EXPECTF--
Warning: StumbleCache::getStats() expects exactly 0 parameters, 1 given in %s on line %d
NULL
