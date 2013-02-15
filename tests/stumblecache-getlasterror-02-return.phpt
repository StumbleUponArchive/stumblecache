--TEST--
Stumblecache->getLastError() return values
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
var_dump($cache->getLastError());
$cache->replace(1,1);
var_dump($cache->getLastError());
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-error.scache');
?>
--EXPECTF--
Warning: StumbleCache::getLastError() expects exactly 0 parameters, 1 given in %s on line %d
NULL
array(5) {
  ["code"]=>
  int(0)
  ["file"]=>
  string(%d) "%s"
  ["line"]=>
  int(%d)
  ["message"]=>
  string(8) "No Error"
  ["method"]=>
  string(0) ""
}
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
  string(3) "add"
}
