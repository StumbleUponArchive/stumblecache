--TEST--
Creating two instances of the same file
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
$cache1 = new StumbleCache( dirname(__FILE__) . '/tests-two', $options );
$cache2 = new StumbleCache( dirname(__FILE__) . '/tests-two', $options );

var_dump( $cache1->add( 50, "some data" ) );
var_dump( $cache2->add( 60, "some more data" ) );
var_dump( $cache2->add( 70, "some yes more data" ) );

?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-two.scache');
unlink(dirname(__FILE__) . '/tests-two.scstats');
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
