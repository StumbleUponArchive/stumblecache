--TEST--
Stumblecache->exists() error handling
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

var_dump($cache->exists(10));
var_dump($cache->getLastError());

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-exists.scache');
?>
--EXPECTF--
bool(false)
array(5) {
  ["code"]=>
  int(404)
  ["message"]=>
  string(14) "Item not found"
  ["file"]=>
  string(%d) "%s"
  ["line"]=>
  int(%d)
  ["method"]=>
  string(6) "exists"
}
