--TEST--
Deletion with existing and non-existing items.
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
$cache = new StumbleCache( dirname(__FILE__) . '/tests-delete', $options );

var_dump( $cache->remove( 50 ) );
var_dump( $cache->add( 50, "some data" ) );
var_dump( $cache->remove( 50 ) );
var_dump( $cache->remove( 50 ) );
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-delete.scache');
unlink(dirname(__FILE__) . '/tests-delete.scstats');
?>
--EXPECT--
bool(false)
bool(true)
bool(true)
bool(false)
