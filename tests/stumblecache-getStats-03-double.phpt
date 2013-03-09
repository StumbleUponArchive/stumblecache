--TEST--
Stumblecache->getStats() second object open
--EXTENSIONS--
igbinary
--FILE--
<?php
/* NOTE: this relies on an earlier run of another test! */
// Initialise the cache
$options = array(
	'order' => 3,
	'max_items' => 1024,
	'max_datasize' => 32,
);
$cache = new StumbleCache(dirname(__FILE__) . '/tests-stats', $options);

// add 100 items - add_insert = 100
$s = 'A';
for ($i = 1; $i <= 100; $i++) {
    $cache->add($i, $s);
    $s++;
}

// add 100 items - add_exists
$s = 'A';
for ($i = 1; $i <= 100; $i++) {
    $cache->add($i, $s);
    $s++;
}

// add 100 fetch hits fetch_hit = 100
for ($i = 1; $i <= 100; $i++) {
    $cache->fetch($i);
}

// add 100 fetch misses fetch_miss = 100
for ($i = 101; $i <= 200; $i++) {
    $cache->fetch($i);
}

// guarantee our ttl will time it out
sleep(2);

/* add 100 fetch timeouts*/
for ($i = 100; $i >= 1; $i--) {
    $cache->fetch($i, 1);
}

// fetch miss deleted all our items!
// use set - set_miss = 100
$s = 'A';
for ($i = 1; $i <= 100; $i++) {
    $cache->set($i, $s);
    $s++;
}

// use set - set_h = 100
$s = 'A';
for ($i = 1; $i <= 100; $i++) {
    $cache->set($i, $s);
    $s--;
}

// do 100 replace writes
for ($i = 1; $i <= 100; $i++) {
    $cache->replace($i, "foo");
}

// do 100 replace misses
for ($i = 101; $i <= 200; $i++) {
    $cache->replace($i, "foo");
}

// do 100 exists checks for true
for ($i = 1; $i <= 100; $i++) {
    $cache->exists($i);
}

// do 100 exists checks for true
for ($i = 401; $i <= 500; $i++) {
    $cache->exists($i);
}

// delete all the things!
for ($i = 100; $i >= 1; $i--) {
    $cache->remove($i);
}

// delete all the things that do not exist!
for ($i = 401; $i <= 500; $i++) {
    $cache->remove($i);
}

var_dump($cache->getStats());
?>
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/tests-stats.scache');
unlink(dirname(__FILE__) . '/tests-stats.scstats');
?>
--EXPECT--
array(19) {
  ["fetch"]=>
  int(600)
  ["fetch_hit"]=>
  int(200)
  ["fetch_miss"]=>
  int(200)
  ["fetch_timeout"]=>
  int(200)
  ["replace"]=>
  int(400)
  ["replace_write"]=>
  int(200)
  ["replace_miss"]=>
  int(200)
  ["set"]=>
  int(400)
  ["set_insert"]=>
  int(200)
  ["set_write"]=>
  int(200)
  ["delete"]=>
  int(400)
  ["delete_hit"]=>
  int(200)
  ["delete_miss"]=>
  int(200)
  ["exists"]=>
  int(400)
  ["exists_hit"]=>
  int(200)
  ["exists_miss"]=>
  int(200)
  ["add"]=>
  int(400)
  ["add_exists"]=>
  int(200)
  ["add_insert"]=>
  int(200)
}