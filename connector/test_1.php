<?php

require_once ('telnetConnet.php');

$telnetConnect  = telnetConnect::instance();
$telnetConnect->botConnect();
$telnetConnect->socketRead();

?>