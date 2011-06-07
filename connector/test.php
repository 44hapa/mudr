<?php

$key    = 'fromBalder';

$m = new Memcache();

$m->addServer('localhost', 11211);
$m->delete($key);
$m->set($key, $argv[1]);
var_dump($m->get($key));

?>
