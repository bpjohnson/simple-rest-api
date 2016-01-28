<?php

define("FILTER_VALIDATE_JSON", "JSON");

class anAPI extends aWidget {

  public $response = array();
  public $debug = array();
  public $filters = array();
  public $f;
  public $request;
  public $smartClass = FALSE;
  public $defaultErrorCode = 418;
  public $defaultErrorMessage = 'Oh crap.';
  public $globalFilters = array();
  public $useRemoteFilters = false;

  public function json($output, $utfSafe = FALSE, $exit = TRUE) {
    global $config;
    if ($config->debugConfig['debugLevel'] != DEBUG_OFF) {
      $this->response['debug'] = $this->debug;
    }
    if (!$this->response['success']) {
      if (!is_string($this->response['message'])) {
	$this->response['message'] = 'Unidentified Error.';
      }
      $this->error(500);
    } else {
      parent::json($this->response,$utfSafe,$exit);
    }
  }

  public function check($var, $allowBlank = FALSE) {
    if (is_array($var)) {
      foreach ($var as $val) {
	$this->check($val, $allowBlank);
      }
    } else {
      $this->debug['unusedFilters'] = array_diff($this->debug['unusedFilters'], array($var));

      if ($this->request->$var) {
	$this->$var = $this->request->$var;
      } else if (!array_key_exists($var, $this->filters)) {
	$this->debug['filters'] = $this->filters;

	$this->response = array(
	  'success' => false,
	  'message' => 'Variable Filter Does Not Exist: '.$var
	);
	$this->error(404);
      } else if (!array_key_exists($var, $_REQUEST) && !$allowBlank) {
	$this->response = array(
	  'success' => false,
	  'message' => 'Variable not included in request: '.$var
	);
	$this->debug = array(
	  'request' => array(
	    'request' => $_REQUEST,
	    'get' => $_GET,
	    'post' => $_POST
	  )
	);
	$this->error(420);
      } else if (!array_key_exists($var, $_REQUEST) && $allowBlank) {
	$this->$var = FALSE;
      } else {
	$this->response = array(
	  'success' => false,
	  'message' => 'Malformed variable: '.$var
	);
	$this->error(400);
      }
    }
  }

  public function error($code) {
    $message = ($this->response['message'])?$this->response['message']:$this->defaultErrorMessage;
    $code = ($code)?$code:$this->defaultErrorCode;
    $header = sprintf('%s %d %s', $_SERVER["SERVER_PROTOCOL"], $code, $message);
    header($header);
    $this->response['success'] = false;
    $this->response['code'] = $code;
    $this->response['message'] = $message;


    global $config;
    if ($config->debugConfig['debugLevel'] != DEBUG_OFF) {
      $this->response['debug'] = $this->debug;
    }

    parent::json($this->response);
  }

  public function __construct() {
    //ini_set('display_errors', 1);
    //error_reporting(E_ALL);

    $this->request =  new iForm(2); //Strict Form
    $this->request->addGet();
    $this->request->addPost();
    $this->request->addJSON();
    if ($this->useRemoteFilters) {
      $this->remoteFilters = $this->request->addRemoteFilters();
      $this->debug['remoteFilters'] = $this->remoteFilters;
    }

    $this->request->filter->action( FILTER_SANITIZE_STRING, FILTER_FLAG_ENCODE_AMP );


    if (count($this->filters) > 0) {
      $this->debug['unusedFilters'] = array_keys($this->filters);
    }
    $this->filters = array_merge($this->globalFilters, $this->filters);
    foreach ($this->filters as $filter => $args) {
      if ($args[0] == FILTER_VALIDATE_JSON) {
        $this->request->filter->$filter(FILTER_UNSAFE_RAW);
        $json = json_decode($this->request->$filter, TRUE);
        $this->request->$filter = $json;
      } else {
        call_user_func_array(array($this->request->filter, $filter), $args);
      }
    }

    if ($this->smartClass) {
      $name = $this->smartClass;
    } else {
      $name = strtolower(substr(get_class($this),0,-3));
    }

    if ($this->smartClass !== false) {
      $f = smartObjectFactory::getInstance();
      $this->f = $f->$name();
    }
    if (method_exists($this, $this->request->action)) {
      $method = $this->request->action;
    } else {
      $method = strtolower($_SERVER['REQUEST_METHOD']);
    }
    $this->$method();
  }


}
?>
