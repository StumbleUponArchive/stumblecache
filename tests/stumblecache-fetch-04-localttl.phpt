--TEST--
Stumblecache->fetch() local ttl fails fetch
--EXTENSIONS--
igbinary
--FILE--
<?php
// Initialise the cache
$options = array(
	'order' => 3,
	'max_items' => 1024,
	'max_datasize' => 32,
	'default_ttl' => 2,
);
$cache = new StumbleCache( dirname(__FILE__) . '/tests-fetch', $options );

var_dump($cache->add(50, "some data"));
var_dump($cache->fetch(50));
sleep(3);
var_dump($cache->fetch(50));
var_dump($cache->getLastError());
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-fetch.scache');
?>
--EXPECTF--
bool(true)
string(9) "some data"
NULL
array(5) {
  ["code"]=>
  int(410)
  ["message"]=>
  string(48) "Gone, item ttl %d is past ttl %d"
  ["file"]=>
  string(%d) "%s"
  ["line"]=>
  int(14)
  ["method"]=>
  string(5) "fetch"
}
