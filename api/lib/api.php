<?php
error_reporting(E_ALL ^ E_NOTICE ^ E_STRICT ^ E_WARNING ^ E_DEPRECATED);
ini_set('display_errors', 1);
ini_set( 'always_populate_raw_post_data', 1 );

$configFile = '../config/config.inc';
include_once ("./alib/alib.inc");
global $debug, $config;

addIncludePath( './alib' );
addIncludePath( './' );
addIncludePath( '../..' );
addIncludePath( '../api', true);
addIncludePath( '../config', true);
include_once ('./alib/iuser.inc');
include_once ('./functions.inc');
include_once ('./login.inc');
include_once ('../config/smartObjectDefs.inc');

include_once ('./alib/aWidget.inc');

include_once ('./alib/anAPI.inc');

#date_default_timezone_set('America/New_York');
try {
// Connect to the db:
if ( !is_object( $db ) ) {
  $db = new idb( $config->mainDB );
  if ($config->mainDB != $config->userDB) {
      $userdb = new idb( $config->userDB );
  } else {
      $userdb = $db;
  }
}

$login = new $config->loginModule();

if ( stristr( $_SERVER[ 'REQUEST_URI' ], 'api' ) && $login->loggedIn ) {

    $api = new api();

} else if (stristr( $_SERVER[ 'REQUEST_URI' ], 'api/loginStatus') || !stristr( $_SERVER[ 'REQUEST_URI' ], 'login')){
    include_once '../loginStatus.inc';
    $nli = new loginStatusAPI();
    $nli->notLoggedIn();
}
} catch (Exception $e) {
  $output = array(
    'success' => false,
    'exception' => array(
      'code' => $e->getCode(),
      'message' => $e->getMessage(),
      'file' => $e->getFile(),
      'line' => $e->getLine(),
      'trace' => $e->getTrace()
    )
  );
  $header = sprintf('%s %d %s', $_SERVER["SERVER_PROTOCOL"], 500, $e->getMessage());
  header($header);
  header("Content-type: application/json");
  print json_encode($output);
}
