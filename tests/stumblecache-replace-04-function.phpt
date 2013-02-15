--TEST--
Stumblecache->replace() change data in btree
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

$cache->add(10, "some data");
var_dump($cache->fetch(10));

var_dump($cache->replace(10, "my data"));
var_dump($cache->fetch(10));

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-replace.scache');
?>
--EXPECT--
string(9) "some data"
bool(true)
string(7) "my data"
