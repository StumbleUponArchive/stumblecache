--TEST--
Adding items.
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

var_dump( $cache->add( 50, "some data" ) );
var_dump( $cache->add( 50, "some data" ) );
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-add.scache');
unlink(dirname(__FILE__) . '/tests-add.scstats');
?>
--EXPECT--
bool(true)
bool(false)
