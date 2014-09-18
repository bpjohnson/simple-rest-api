<?php

class loginStatusAPI extends anAPI {
  public $smartClass = false;
  public $apiName = 'loginStatus';
  public $filters = array();
  public $options = array();

    public function notLoggedIn() {
        $this->response['message'] = sprintf('Not Logged In');
        $this->debug['cookies'] = $_COOKIE;
        $this->error(401);
    }

    public function post() {
      global $user, $login;
      $this->debug['login'] = $login;
      if ($login->loggedIn) {
          $this->response['success'] = true;
          $vars = $user->vars;
          unset($vars['passwd'], $vars['password']);
          $this->response['user'] = $vars;
      } else {
          $this->notLoggedIn();
      }
    }

    public function get() {
      $this->post();
    }
}
?>
