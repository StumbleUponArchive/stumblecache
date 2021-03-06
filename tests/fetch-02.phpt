--TEST--
Finding an element with expired TTL.
--EXTENSIONS--
igbinary
--FILE--
<?php
require 'lib/test-runner.php';

$initial = "Sfetch01:3:64:128";
$spec    = 'F65 A65:1 F65 F80 A80:1 F80 R80 F80';
runTest($initial, $spec);
?>
--EXPECT--
65: NOT FOUND
65: FOUND
80: NOT FOUND
80: FOUND
80: NOT FOUND
