--TEST--
Adding an item with too much data.
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

var_dump( $cache->add( 50, "This text field has quite a little bit too much data. Oh no!" . str_repeat( 'xxxxx', 60 ) ) );
var_dump( $cache->fetch( 50 ) );
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-add.scache');
unlink(dirname(__FILE__) . '/tests-add.scstats');
?>
--EXPECT--
bool(false)
NULL
