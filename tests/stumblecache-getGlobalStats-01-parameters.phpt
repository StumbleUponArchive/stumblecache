--TEST--
Stumblecache->getGlobalStats() parameters
--EXTENSIONS--
igbinary
--FILE--
<?php
var_dump(StumbleCache::getGlobalStats(1));
?>
--EXPECTF--
Warning: StumbleCache::getGlobalStats() expects exactly 0 parameters, 1 given in %s on line %d
NULL