--TEST--
Stumblecache->fetch() ttl override order
--EXTENSIONS--
igbinary
--INI--
stumblecache.default_ttl = 1
--FILE--
<?php
// Initialise the cache
$options = array(
	'order' => 3,
	'max_items' => 1024,
	'max_datasize' => 32,
);
$cache = new StumbleCache( dirname(__FILE__) . '/tests-fetch', $options );

var_dump($cache->add(50, "some data"));
var_dump($cache->fetch(50));
sleep(1);
var_dump($cache->fetch(50));

// remake cache - test local overrides ini
$options['default_ttl'] = 3;
$cache = new StumbleCache( dirname(__FILE__) . '/tests-fetch', $options );
var_dump($cache->add(50, "some data"));
var_dump($cache->fetch(50));
sleep(3);
var_dump($cache->fetch(50));

// test passed overrides local and ini
var_dump($cache->add(50, "some data"));
var_dump($cache->fetch(50));
sleep(4);
var_dump($cache->fetch(50, 10));
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-fetch.scache');
?>
--EXPECTF--
bool(true)
string(9) "some data"
NULL
bool(true)
string(9) "some data"
NULL
bool(true)
string(9) "some data"
string(9) "some data"
