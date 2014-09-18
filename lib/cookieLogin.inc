<?php

/**
 * cookieLogin provides a method to authenticate someone based on cookies (or alternatively form input)
 */

class cookieLogin extends loginPrototype {
  public function __construct() {
    global $_POST, $user, $config;
    // Set us up some vars:

    $loginSafety = ($config->loginSafetyLevel)?$config->loginSafetyLevel:2; //iform level
    $cookieUser = ($config->cookieUserKey)?$config->cookieUserKey:$config->appName.'User';
    $cookieToken = ($config->cookieTokenKey)?$config->cookieTokenKey:$config->appName.'Token';
    $form = new iForm($loginSafety); //Strict Form
    if ($config->environment == 'dev') {
      $form->addGet();
    }
    $form->addPost(); //We only want posted variables.
    $form->addCookie(); // And cookie vars.
    $form->filter->username( FILTER_SANITIZE_STRING, FILTER_FLAG_ENCODE_AMP );
    $form->filter->password( FILTER_SANITIZE_STRING, FILTER_FLAG_ENCODE_AMP );
    $form->filter->$cookieUser( FILTER_SANITIZE_STRING, FILTER_FLAG_ENCODE_AMP );
    $form->filter->$cookieToken( FILTER_SANITIZE_STRING, FILTER_FLAG_ENCODE_AMP );
    $form->filter->logout( FILTER_VALIDATE_BOOLEAN );
    // Now we check to see what's going on. First, did we just logout?
    // If so, remove the cookies then reload the page.
    if ($form->logout) {
      $this->CookieLogout($cookieUser,$cookieToken);
      $reloadURL = ($config->logoutURL)?$config->logoutURL:$config->appURL;
      header("Location: ".$reloadURL);
      exit;
    }
    $success = false;
    // Next, do we have valid cookies?
    if ($form->$cookieUser !== FALSE && $form->$cookieToken !== FALSE) {
      $success = $this->CookieAuth($form);
    }

    if (!$success && ($form->username !== FALSE && $form->password !== FALSE)) {

      // Else we should check form auth
      $success = $this->FormAuth($form);
    } else if (!$success) {
      // Not logged in and not trying to log in.
      $this->loggedIn = FALSE;
      $this->error = '';
    }
  }

  public function FormAuth($form) {
    global $config, $user, $db;
    $cookieExpiry = ($config->loginExpires)?$config->loginExpires:(8*60*60); //default 8 hours from now.
    $cookieUser = ($config->cookieUserKey)?$config->cookieUserKey:$config->appName.'User';
    $cookieToken = ($config->cookieTokenKey)?$config->cookieTokenKey:$config->appName.'Token';
    $cookiePath = ($config->cookiePath)?$config->cookiePath:"";
    $cookieDomain = ($config->cookieDomain)?$config->cookieDomain:"";
    $cookieSecure = ($config->cookieSecure)?$config->cookieSecure:FALSE;
    $salt = ($config->cookieSalt)?$config->cookieSalt:'anAppLoginCookieSalt';

    $sql = " (`username` = '".$form->username."' OR `email` = '".$form->username."') AND `passwd` = PASSWORD('".$form->password."') AND `status` = 1";
    $factory = smartObjectFactory::getInstance($db);
    $users1 = $factory->user()->where($sql);
    $sql = $users1->queryString;
    $users = $users1->f();
    if ($users->num_rows == 1) {
      $user = $users->fetch();

      $this->loggedIn = TRUE;
      //Set cookies:
      $passwordHash = md5($form->username.$user->passwd.$salt);
      setcookie($cookieUser, $form->username, time() + $cookieExpiry, $cookiePath, $cookieDomain, $cookieSecure);
      setcookie($cookieToken, $passwordHash, time() + $cookieExpiry , $cookiePath, $cookieDomain, $cookieSecure);
      //And reload the page:
#      print "{success: true}";
 #     exit;
      return $this->loggedIn;
    } else {
      $this->loggedIn = FALSE;
      $this->error = 'Invalid login information.';
      header('HTTP/1.1 401 Unauthorized');
      header("Content-type: application/json");
      $output = array(
	'success' => false
      );
      if ($config->environment == 'dev') {
	$output['sql'] = $sql;
	$output['db_connect_error'] = $db->connect_error;
	$output['error'] = $db->error;
      }
      print json_encode($output);
      exit;
    }




    return $this->loggedIn;
  }

  public function CookieAuth($form) {
    global $config, $user, $db;
    $cookieExpiry = ($config->loginExpires)?$config->loginExpires:(8*60*60); //default 8 hours from now.
    $cookieUser = ($config->cookieUserKey)?$config->cookieUserKey:$config->appName.'User';
    $cookieToken = ($config->cookieTokenKey)?$config->cookieTokenKey:$config->appName.'Token';
    $cookiePath = ($config->cookiePath)?$config->cookiePath:"";
    $cookieDomain = ($config->cookieDomain)?$config->cookieDomain:"";
    $cookieSecure = ($config->cookieSecure)?$config->cookieSecure:FALSE;
    $salt = ($config->cookieSalt)?$config->cookieSalt:'anAppLoginCookieSalt';
    $sql = " (`username` = '".$form->$cookieUser."' OR `email` = '".$form->$cookieUser."') AND MD5(CONCAT('".$form->$cookieUser."', `passwd`, '".$salt."')) = '".$form->$cookieToken."' AND `status` = 1";
    $factory = smartObjectFactory::getInstance($db);
    $users = $factory->user()->where($sql)->f();
    if ($users->num_rows == 1) {
        $user = $users->fetch();
      $this->loggedIn = TRUE;
      //Update session expiry:
      setcookie($cookieUser, $form->$cookieUser, time() + $cookieExpiry, $cookiePath, $cookieDomain, $cookieSecure);
      setcookie($cookieToken, $form->$cookieToken, time() + $cookieExpiry , $cookiePath, $cookieDomain, $cookieSecure);
    } else {
      $this->loggedIn = FALSE;
      $this->error = 'Invalid cookie information: '.$form->$cookieUser.' '.$form->$cookieToken;
    }
    return $this->loggedIn;
  }


  public function CookieLogout($cookieUser, $cookieToken) {
    global $config;
    $cookieUser = ($config->cookieUserKey)?$config->cookieUserKey:$config->appName.'User';
    $cookieToken = ($config->cookieTokenKey)?$config->cookieTokenKey:$config->appName.'Token';
    $cookiePath = ($config->cookiePath)?$config->cookiePath:"";
    $cookieDomain = ($config->cookieDomain)?$config->cookieDomain:"";
    $cookieSecure = ($config->cookieSecure)?$config->cookieSecure:FALSE;
    setcookie($cookieUser, "", time() -3600, $cookiePath, $cookieDomain, $cookieSecure);
    setcookie($cookieToken, "", time() -3600, $cookiePath, $cookieDomain, $cookieSecure);
    unset($_COOKIE[$cookieUser]);
    unset($_COOKIE[$cookieToken]);
  }

}
?>
