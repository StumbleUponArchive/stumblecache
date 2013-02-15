--TEST--
Stumblecache->set() insert or change data in tree
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

var_dump($cache->set(10, "some data"));
var_dump($cache->fetch(10));

var_dump($cache->set(10, "my data"));
var_dump($cache->fetch(10));

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-set.scache');
?>
--EXPECT--
bool(true)
string(9) "some data"
bool(true)
string(7) "my data"
