<?php

/* Application name (used for cookies) */
$appName = 'simplerest';

$defaultAPI = 'default';


$environment = "dev"; //dev or test or prod

if ($environment == "dev") {

  include_once('dev.dbinfo.inc');
  include_once('dev.pathinfo.inc');

    /* Required Variables used by cookieLogin: */

    /* Use cookieLogin */
    $loginModule = "cookieLogin";
    //$loginModule = "dummyLogin";
    $loginSafetyLevel = 2; //iform style safety level

    /* User Login Expiration Time */
    $loginExpires = 60*60*8; //8 hours

    /* Cookie User Key (name of cookie to store loginName in) */
    $cookieUserKey = $appName.'User';

    /* Cookie Token Key (name of cookie to store auth token in) */
    $cookieTokenKey = $appName.'Token';

    /* Optional Variables used by cookieLogin */

    /* Cookie Path Var (see php function set_cookie) */
    $cookiePath = $self;

    /* Cookie Domain Var (see php function set_cookie) */
    $cookieDomain = ''; // '' signifies current URL domain

    /* Use ssl only? */
    $cookieSecure = FALSE;


} else if ($environment == "test") {

  include_once('test.dbinfo.inc');
  include_once('test.pathinfo.inc');

  $loginModule = "dummyLogin"; //allows anyone to log in as user 1, with any username/password

} else {

  include_once('prod.dbinfo.inc');
  include_once('prod.pathinfo.inc');

    
  /* Required Variables used by cookieLogin: */

    /* Use cookieLogin */
    $loginModule = "cookieLogin";
    $loginSafetyLevel = 2; //iform style safety level

    /* User Login Expiration Time */
    $loginExpires = 60*60*8; //8 hours

    /* Cookie User Key (name of cookie to store loginName in) */
    $cookieUserKey = $appName.'User';

    /* Cookie Token Key (name of cookie to store auth token in) */
    $cookieTokenKey = $appName.'Token';

    /* Optional Variables used by cookieLogin */

    /* Cookie Path Var (see php function set_cookie) */
    $cookiePath = $self;

    /* Cookie Domain Var (see php function set_cookie) */
    $cookieDomain = ''; // '' signifies current URL domain

    /* Use ssl only? */
    $cookieSecure = FALSE;
}




?>
