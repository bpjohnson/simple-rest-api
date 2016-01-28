<?php

class simpleAPI extends anAPI {
  public $smartClass = false;
  public $apiName = '';
  public $filters = array();
  public $options = array();
  public $useRemoteFilters = true;

  public $globalFilters = array(
    'id' => array( FILTER_VALIDATE_INT )
  );


  public function __construct($apiName) {
    $this->apiName = $apiName;
    $this->debug['defaultAPI'] = $this->apiName;

    if (class_exists($apiName)) {
      $this->smartClass = $apiName;
      $this->debug['smartClass'] = $apiName;
      $idProp = $this->getIDProp();

      $this->filters[$idProp] =  array(FILTER_VALIDATE_INT);
    }

    parent::__construct();

    if ($this->smartClass === false) {
      $this->response['message'] = sprintf('Resource %s not found.', $apiName);
      $this->error(404);
    }
  }

  public function post() {

    $idProp = $this->getIDProp();

    $this->check(array($idProp), true);

    if ($this->request->json[$idProp] !== false) {
      $q = $this->f;
      $q = $q->$idProp($this->request->json[$idProp]);
      if ($this->remoteFilters) {
        $this->debug['filtered'] = array();
        foreach ($this->remoteFilters as $filter) {
          if ($filter != $idProp) {
            $q = $q->$filter($this->$filter);
            $this->debug['filtered'][$filter] = $this->$filter;
          }
        }
      }


      $res = $q->query();
      $this->debug['sql'] = $q->lastQuery;

      if (is_object($res)) {
        $this->response['success'] = true;
        $this->response['data'] = array();
        $record = $res->fetch();
        $this->debug['fetchedRecord'] = var_export($record, true);
        $this->debug['idProp'] = $idProp;
        $this->debug['thisIdProp'] = $this->$idProp;
        $this->debug['setKeys'] = array();
        if (is_object($record)) {
          foreach ($this->request->json as $key => $val) {
            if ($key != $this->$idProp && $key != $idProp) {
              $this->debug['setKeys'][] = $key;
              if (is_array($val) && is_array($record->$key)) {
                $newval = array_merge($record->$key, $val);
                $record->$key = $newval;
              } else {
                $record->$key = $val;
              }
            }
          }
          if ($record->update()) {
            $this->response['success'] = true;
            $this->response['data'] = array($record->toArray());
          } else {
            $this->response['success'] = false;
            $this->response['error'] = "Failed to update: " . $record->error;
            $this->debug['record'] = var_export($record, true);
          }
        } else {
          $this->response['success'] = false;
          $this->response['error'] = "Record not found: " . $q->error;
        }
      } else {
        $this->response['success'] = false;
        $this->response['error'] = "DB Error: " . $q->error;
      }
    } else {
      $this->response['success'] = false;
      $this->response['error'] = "Unknown ID";
      $this->debug['req'] = var_export($this->request, true);
    }
  }


  public function get() {
    $idProp = $this->getIDProp();

    $this->check(array($idProp), true);
    $this->debug['class'] = get_class($this);
    if ($this->$idProp === false) {
      $this->findAll();
    } else {
      $this->findOne();
    }
  }

  protected function findAll() {
    $idProp = $this->getIDProp();

    $q = $this->f->$idProp($this->request->id);
    if ($this->remoteFilters) {
      $this->debug['filtered'] = array();
      foreach ($this->remoteFilters as $filter) {
        if ($filter != $idProp) {
          $q = $q->$filter($this->request->$filter);
          $this->debug['filtered'][$filter] = $this->request->$filter;
        }
      }
    }
    $this->debug['form'] = $this->request;
    $res = $q->query();
    $this->debug['sql'] = $q->lastQuery;
    if (is_object($res)) {
      $this->response['success'] = true;
      $this->response['data'] = array();
      while ($row = $res->fetch()) {
        $data = $row->toArray();
        $this->response['data'][] = $data;
      }
    } else {
      $this->response['success'] = false;
      $this->response['error'] = $q->error;
    }

  }

  public function getIDProp() {
    if ($this->idMap[$this->smartClass] === true) {
      return 'id';
    } else if ($this->idMap[$this->smartClass]) {
      return $this->idMap[$this->smartClass];
    } else {
      return 'id';
    }
  }


  
}


?>
