--TEST--
Stumblecache->set() error handling
--EXTENSIONS--
igbinary
--FILE--
<?php
// Initialise the cache
$options = array(
	'order' => 3,
	'max_items' => 2, /* force max items error */
	'max_datasize' => 1, /* force datasize error */
);
$cache = new StumbleCache(dirname(__FILE__) . '/tests-set', $options);

var_dump($cache->set(10, "my data"));
var_dump($cache->set(20, "my data"));
var_dump($cache->set(30, "my data"));
var_dump($cache->getLastError());

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-set.scache');
unlink(dirname(__FILE__) . '/tests-set.scstats');
?>
--EXPECTF--
bool(true)
bool(true)
bool(false)
array(5) {
  ["code"]=>
  int(413)
  ["message"]=>
  string(62) "Maximum number of items reached, could not add additional item"
  ["file"]=>
  string(%d) "%s"
  ["line"]=>
  int(%s)
  ["method"]=>
  string(3) "set"
}
