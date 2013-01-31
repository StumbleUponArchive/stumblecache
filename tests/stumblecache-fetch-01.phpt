--TEST--
Fetching items with expired TTL.
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
var_dump( $cache->fetch( 50 ) );
var_dump( $cache->fetch( 50, 2 ) );
sleep( 3 );
var_dump( $cache->fetch( 50, 2 ) );

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-add.scache');
?>
--EXPECT--
bool(true)
string(9) "some data"
string(9) "some data"
NULL
