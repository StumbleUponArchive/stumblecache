--TEST--
clone Stumblecache class
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
$cache = new StumbleCache( dirname(__FILE__) . '/tests-add', $options );

clone $cache;
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-add.scache');
?>
--EXPECTF--
Fatal error: Trying to clone an uncloneable object of class StumbleCache in %s on line %d
