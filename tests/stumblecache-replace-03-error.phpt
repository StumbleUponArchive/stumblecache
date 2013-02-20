--TEST--
Stumblecache->replace() error handling
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

var_dump($cache->replace(10, "my data"));
var_dump($cache->getLastError());

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-replace.scache');
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
  string(7) "replace"
}
