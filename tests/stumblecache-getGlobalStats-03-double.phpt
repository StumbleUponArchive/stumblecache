--TEST--
Stumblecache->getGlobalStats() second object open
--EXTENSIONS--
igbinary
--FILE--
<?php
// function to fill caches
function fill_cache($name, $options) {
	$cache = new StumbleCache(dirname(__FILE__) . '/' . $name, $options);

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
	
	// add 100 fetch timeouts
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
}
// files to fill
$files = array('tests-01',
               'tests-02',
               'tests-03',
               'tests-04'
);

// options for all of them
$options = array(
    'order' => 3,
    'max_items' => 1024,
    'max_datasize' => 32,
);

// fill our global stats
foreach($files as $name) {
	fill_cache($name, $options);
}
var_dump(StumbleCache::getGlobalStats());
?>
--CLEAN--
<?php
$files = array('tests-01',
               'tests-02',
               'tests-03',
               'tests-04');

foreach($files as $name) {
	unlink(dirname(__FILE__) . '/' . $name . '.scache');
	unlink(dirname(__FILE__) . '/' . $name . '.scstats');
}
unlink(ini_get('stumblecache.default_cache_dir') . '/globals.scstats');
?>
--EXPECT--
array(19) {
  ["fetch"]=>
  int(2400)
  ["fetch_hit"]=>
  int(800)
  ["fetch_miss"]=>
  int(800)
  ["fetch_timeout"]=>
  int(800)
  ["replace"]=>
  int(1600)
  ["replace_write"]=>
  int(800)
  ["replace_miss"]=>
  int(800)
  ["set"]=>
  int(1600)
  ["set_insert"]=>
  int(800)
  ["set_write"]=>
  int(800)
  ["delete"]=>
  int(1600)
  ["delete_hit"]=>
  int(800)
  ["delete_miss"]=>
  int(800)
  ["exists"]=>
  int(1600)
  ["exists_hit"]=>
  int(800)
  ["exists_miss"]=>
  int(800)
  ["add"]=>
  int(1600)
  ["add_exists"]=>
  int(800)
  ["add_insert"]=>
  int(800)
}