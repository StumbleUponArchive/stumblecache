--TEST--
Stumblecache->replace() return values
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
var_dump($cache->replace(10, "some data"));

$cache->add(10, "some data");
var_dump($cache->replace(10, "my data"));

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-replace.scache');
unlink(dirname(__FILE__) . '/tests-replace.scstats');
?>
--EXPECTF--
Warning: StumbleCache::replace() expects exactly 2 parameters, 0 given in %s on line %d
NULL
bool(false)
bool(true)
