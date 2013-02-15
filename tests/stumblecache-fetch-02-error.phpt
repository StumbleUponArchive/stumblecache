--TEST--
Stumblecache->fetch() errors after failed fetch
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
$cache = new StumbleCache( dirname(__FILE__) . '/tests-fetch', $options );

var_dump($cache->fetch(50));
var_dump($cache->getLastError());

var_dump($cache->add(50, "some data"));
sleep(3);
var_dump($cache->fetch(50, 2));
var_dump($cache->getLastError());
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-fetch.scache');
?>
--EXPECTF--
NULL
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
  string(5) "fetch"
}
bool(true)
NULL
array(5) {
  ["code"]=>
  int(410)
  ["message"]=>
  string(48) "Gone, item ttl %d is past ttl %d"
  ["file"]=>
  string(%d) "%s"
  ["line"]=>
  int(%d)
  ["method"]=>
  string(5) "fetch"
}
