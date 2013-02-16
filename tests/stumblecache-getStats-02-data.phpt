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

// add 100 items
$s = 'A';
for ($i = 1; $i <= 100; $i++) {
    $cache->add($i, $s);
    $s++;
}

// add 100 fetch hits
for ($i = 1; $i <= 100; $i++) {
    $cache->fetch($i);
}

// add 100 fetch misses
for ($i = 101; $i <= 200; $i++) {
    $cache->fetch($i);
}

// add 100 items
$s = 'A';
for ($i = 1; $i <= 100; $i++) {
    $cache->add($i, $s);
    $s++;
}

// do 100 replace writes
for ($i = 1; $i <= 100; $i++) {
    $cache->replace($i, "foo");
}

// do 100 replace misses
for ($i = 101; $i <= 200; $i++) {
    $cache->replace($i, "foo");
}
var_dump($cache->getStats());
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-stats.scache');
unlink(dirname(__FILE__) . '/tests-stats.scstats');
?>
--EXPECTF--
Warning: StumbleCache::exists() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: StumbleCache::exists() expects parameter 1 to be long, array given in %s on line %d
NULL
bool(false)

Warning: StumbleCache::exists() expects exactly 1 parameter, 2 given in %s on line %d
NULL
