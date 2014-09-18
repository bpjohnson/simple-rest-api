<?

include_once(__DIR__.'/inflect.inc');

/**
 * Smart Objects
 *
 * Smart objects are objects tied directly to a single record in a db.
 */

abstract class smartObject extends anObject {
  public $table = FALSE;
  public $idField = FALSE;
  public $db;
  public $error;
  public $errno;
  public $insertID;
  public $sql;
  protected $foreignKeys = array();
  protected $serializedKeys = array();
  protected $serializeMethod = 'json'; // json or php
  protected $unserialized = array();
  /*
    protected $format = array(
      'string' => '', // a sprintf style format string
      'vars' => array() // class variables;
    );
  */
  protected $format = 'json'; // interpret as a json string when called as a string.
  public $vars = array();


  public function __construct($id = NULL, $mydb = NULL) {
    if ($this->idField === FALSE) {
      $this->idField = get_class($this)."ID";
    }
    if ($this->table === FALSE) {
      $this->table = Inflect::pluralize(strtolower(get_class($this)));
    }
    if (is_a($mydb, 'idb')) {
      $this->db = $mydb;
    } else {
      global $db;
      if (is_a($db, 'idb')) {
        $this->db = $db;
      }
    }
    if (is_array($id)) {
      foreach ($id as $key => $val) {
        $this->{$key} = $val;
        if (in_array($key, $this->serializedKeys)) {
          $this->unserialized[$key] = FALSE;
        }
      }
    } else if ($id) {
      $this->{$this->idField} = $id;
      $this->read();
    }
  }

    public function __set($name, $value)
      {
        if ($name == "sql") {
          $this->sql = $value;
          return;
        }
        if ($name == 'id' && $this->idField != 'id') {
          $this->vars[$this->idField] = $value;
        } else if (in_array($name, $this->serializedKeys)) {
            $this->vars[$name] = $value;
            $this->unserialized[$name] = TRUE;
        } else if (in_array($name, array_keys($this->foreignKeys))) {
          if (is_object($value) && ($value instanceof $this->foreignKeys[$name]['type'])) {
            $this->vars[$this->foreignKeys[$name]['key']] = $value->id;
            $this->foreignKeys[$name]['obj'] = $value;
          }
        } else {
          $this->vars[$name] = $value;
        }
      }


  public function __get($name)
      {
        if ($name == 'id' && $this->idField != 'id') {
          return $this->vars[$this->idField];
      } else if (in_array($name, $this->serializedKeys)) {
          if (is_array($this->vars[$name]) || is_object($this->vars[$name])) {
              return $this->vars[$name];
          }
          if ($this->serializeMethod == 'php') {
            $retval = unserialize($this->vars[$name]);
          } else {
            $retval = json_decode($this->vars[$name], TRUE);
            if (is_array($retval) && count($retval) == 0) {
                return null;
            }
          }
          return $retval;
        } else if (($name == 'errno' || $name == 'error') && ! in_array($name, array_keys($this->vars))) {
          return $this->$name;
        } else if (!is_array($this->foreignKeys[$name])) {
          return $this->vars[$name];
        } else {
          if (!is_a($this->foreignKeys[$name]['obj'], 'smartObject')) {
            $type = $this->foreignKeys[$name]['type'];
            $fKey = $this->foreignKeys[$name]['key'];
            $this->foreignKeys[$name]['obj'] = new $type($this->{$fKey}, $this->db);
          }
          return $this->foreignKeys[$name]['obj'];
        }
      }


  public function __toString() {
    if ($this->format != 'json') {
      $vars = array($this->format['string']);
      foreach ($this->format['vars'] as $var) {
        $vars[] = $this->vars[$var];
      }
      return call_user_func_array('sprintf', $vars);
    } else {
      
      $type = get_class($this);
      return json_encode(array($type => $this->toArray()));
    }
  }

 public function toArray() {
     $retval = array();
     foreach (array_keys($this->vars) as $key) {
         $retval[$key] = $this->__get($key);
         if (is_a($retval[$key], 'smartObject')) {
	 $retval[$key] = $retval[$key]->toArray();
	}
     }
     return $retval;
 }

  public function reset() {
    foreach ($this->vars as $key => $val) {
      $this->vars[$key] = NULL;
    }
    foreach ($this->foreignKeys as $name => $arr) {
      unset($arr['obj']);
        $arr['obj'] = NULL;
    }
  }


  public function read() {
    if (is_a($this->db, 'idb')) {
      $sql = "select * from `".$this->table."` where `".$this->idField."` = '".$this->db->real_escape_string($this->vars[$this->idField])."'";
      $this->reset();
      $this->sql = $sql;
      $res = $this->db->query($sql);
      if (is_object($res)) {
        if ($res->num_rows == 1) {
          $row = $res->fetch_assoc();
          foreach ($row as $key => $val) {
            $this->vars[$key] = $val;
          }
          $this->errno = FALSE;
          return TRUE;
        } else {
          $this->errno = TRUE;
          $this->error = "Query returned ".$res->num_rows." rows. smartObject queries should return only one row.";
          return FALSE;
        }
      } else {
        $this->error = $db->error;
        $this->errno = $db->errno;
        return $db->error;
      }
    } else {
      return FALSE;
    }
  }

  public function insert() {
    if (is_a($this->db, 'idb')) {
      $sql = "insert into `".$this->table."` (";
      foreach ($this->foreignKeys as $fkey => $farray) {
        $this->vars[$farray['key']] = $farray['obj']->id;
      }
      $keys = array_keys($this->vars);
      $sql .= '`'.preg_replace('/,/','` , `',implode(',', $keys)).'`';
      $sql .= ") values (";
      foreach ($this->vars as $key => $val) {
          if (in_array($key, $this->serializedKeys)) {
            if (!$this->unserialized[$key] || (!is_array($val) && !is_object($val))) {
              if ($this->serializeMethod == 'php') {
                $val = unserialize($val);
              } else {
                $val = json_decode($val, TRUE);
              }
            }
            if ($this->serializeMethod == 'php') {
              $val = serialize($val);
            } else {
              $val = json_encode($val);
            }
            if ($val == "" || $val == '""' || $val == "null" || $val == '"null"') {
              $val = NULL;
            }
          }
        $sql .= "'".$this->db->real_escape_string($val)."', ";
      }
      $sql = substr($sql, 0, -2);
      $sql .= ")";
      $this->sql = $sql;
      $this->db->query($sql);
      $this->errno = $this->db->errno;
      $this->error = $this->db->error;
      if ($this->db->insert_id > 0) {
        $this->vars[$this->idField] = $this->db->insert_id;
        return $this->vars[$this->idField];
      } else if ($this->errno == 0) {
        return TRUE;
      } else {
        return FALSE;
      }
    } else {
      return FALSE;
    }
  }

  public function update() {
    if (is_a($this->db, 'idb')) {
      $sql = "update `".$this->table."` set ";
      foreach ($this->foreignKeys as $fkey => $farray) {
        if (is_object($farray['obj']) && $farray['obj']->id) {
            $this->vars[$farray['key']] = $farray['obj']->id;
        }
      }
      foreach ($this->vars as $key => $val) {
        if ($key != $this->idField) {
          if (in_array($key, $this->serializedKeys)) {
            if (!$this->unserialized[$key] || (!is_array($val) && !is_object($val))) {
              if ($this->serializeMethod == 'php') {
                $val = unserialize($val);
              } else {
                $val = json_decode($val, TRUE);
              }
            }
            if ($this->serializeMethod == 'php') {
              $val = serialize($val);
            } else {
              $val = json_encode($val);
            }
            if ($val == "" || $val == '""' || $val == "null" || $val == '"null"') {
              $val = NULL;
            }
            $sql .= "`$key` = '".$this->db->real_escape_string($val)."', ";
          } else if ($val == "null") {
            $sql .= "`$key` = NULL, ";
          } else {
            $sql .= "`$key` = '".$this->db->real_escape_string($val)."', ";
          }
        }
      }
      $sql = substr($sql, 0, -2);
      $sql .= " where `".$this->idField."` = '".$this->db->real_escape_string($this->vars[$this->idField])."'";
      $this->sql = $sql;
      $this->db->query($sql);
      if ($this->db->errno == 0) {
        return TRUE;
      } else {
          $this->error = 'MySQL Error: '.$this->db->errno.' '.$this->db->error;
        return FALSE;
      }
    }
    return FALSE;
  }

  public function delete() {
    if (is_a($this->db, 'idb')) {
      $sql = "delete from `".$this->table."` where `".$this->idField."` = '".$this->db->real_escape_string($this->vars[$this->idField])."' limit 1";
      $res = $this->db->query($sql);
      $this->reset();
      if ($this->db->errno == 0) {
        return TRUE;
      } else {
        return FALSE;
      }
    } else {
      return FALSE;
    }
  }



  public function load() {
    return $this->read();
  }
  public function create() {
    return $this->insert();
  }
  public function save() {
    return $this->update();
  }
  public function nuke() {
    return $this->delete();
  }

  public function setDB($db) {
    $this->db = $db;
  }

}

/* You should never have to extend this. Use the factory below. */
class smartResult extends MySQLi_Result implements IteratorAggregate {
  public $resultType;
  public $resultParams;
  public $link;

  public function getIterator()
  {
    return new smartResultIterator($this);
  }

  function fetch($type = NULL, $params = NULL) {
    if (!$type) { $type = $this->resultType; }
    if (!$params) { $params = $this->resultParams; }
    if ($type) {
      $obj =  $this->fetch_object($type, $params);
      if ($this->link && $this->link instanceof idb) {
        //      $obj->setDB($this->link);
      }
      return $obj;
    }
  }

  public function __destruct() {
    $this->free();
  }

}

class smartResultIterator implements Iterator
{
  protected $result;
  protected $position;
  protected $currentRow;
  public $link;

  /**
   * Constructor
   * @param MySQLi_Result $result
   */
  public function __construct($result)
  {
    $result->link = $this->link;
    $this->result = $result;
  }

  /**
   * Destructor
   * Frees the result object
   */
  public function __destruct()
  {
    $this->result->free();
  }

  /**
   * Rewinds the internal pointer
   */
  public function rewind()
  {
    // data_seek moves the results internal pointer
    $this->result->data_seek($this->position = 0);
    // prefetch the current row
    // note that this advances the results internal pointer.
    $this->currentRow = $this->result->fetch();
  }

  /**
   * Moves the internal pointer one step forward
   */
  public function next()
  {
    // prefetch the current row
    $this->currentRow = $this->result->fetch();

    // and increment internal pointer
    ++$this->position;
  }

  /**
   * Returns true if the current position is valid, false otherwise.
   * @return bool
   */
  public function valid()
  {
    return $this->position < $this->result->num_rows;
  }

  /**
   * Returns the row that matches the current position
   * @return array
   */
  public function current()
  {
    return $this->currentRow;
  }

  /**
   * Returns the current position
   * @return int
   */
  public function key()
  {
    return $this->position;
  }
}


class smartObjectFactory {
  private static $instance = NULL;
  private $objType = NULL;
  private $objArgs = NULL;
  public $queryString = "";
  public $error = "";
  public $db;
  private $numWheres = 0;

  private function __construct($db = FALSE) {
    if (!$db) { $this->db = $GLOBALS['db']; };
  }

  public function getInstance() {
    if (!self::$instance) {
      self::$instance = new smartObjectFactory();
    }
    self::$instance->reset();
    return self::$instance;
  }

  public function __call($method, $args) {
    if ($this->queryString == "") {
      if (count($args) == 1 && !is_array($args[0])) {
        return new $method($args[0]);
      } else if (count($args) == 0) {
        $tmp = new $method();
        $this->objType = $method;
        $this->objArgs = $args;
        $this->queryString = "select * from `".$tmp->table."`";
        $this->numWheres = 0;
      }
    } else if (strtolower($method) == "where") {
      $this->queryString .= " where ".$args[0];
      $this->numWheres++;
    } else if (strtolower($method) == "limit") {
      $this->queryString .= " limit ".$args[0].",".$args[1];
    } else if (strtolower($method) == "groupby") {
      $this->queryString .= " group by ";
      if (is_array($args)) {
        $this->queryString .= implode(', ',$args);
      } else {
        $this->queryString .= $args;
      }
    } else if (strtolower($method) == "orderby") {
      $this->queryString .= " order by ";
      if (is_array($args)) {
        $this->queryString .= implode(' ',$args);
      } else {
        $this->queryString .= $args;
      }
    } else if (strtolower($method) == "or") {
      if (! stristr($this->queryString, ' where ')) {
        $this->queryString .= " where";
      } else if ($this->numWheres > 0) {
        $this->queryString .= " or";
      }
      $this->queryString .= " ".$args[0];
      $this->numWheres++;
    } else if (strtolower($method) == "and") {
      if (! stristr($this->queryString, ' where ')) {
        $this->queryString .= " where";
      } else if ($this->numWheres > 0) {
        $this->queryString .= " and";
      }
      $this->queryString .= " ".$args[0];
      $this->numWheres++;
    } else if (strtolower($method) == "match") {
      if (! stristr($this->queryString, ' where ')) {
        $this->queryString .= " where";
      } else if ($this->numWheres > 0) {
        $this->queryString .= " and";
      }
      $this->queryString .= " MATCH (".$args[0].") AGAINST ('".$args[1]."'";
      if ($args[2]) {
        $this->queryString .= "IN BOOLEAN MODE)";
      } else {
        $this->queryString .= ")";
      }
      $this->numWheres++;
    } else {
      if (! stristr($this->queryString, ' where ')) {
        $this->queryString .= " where";
      } else if ($this->numWheres > 0) {
        $this->queryString .= " and";
      }
      $this->queryString .= " `".$method."` = '".$this->db->real_escape_string($args[0])."'";
      $this->numWheres++;
    }
    return self::$instance;
  }

  public function reset() {
    $this->error = "";
    $this->queryString = "";
    $this->numWheres = 0;
  }

  public function query($query = NULL, $db = NULL) {
    if (!$db) { $db = $GLOBALS['db']; };
    if (!$query && $this->queryString != "") {
      $query = $this->queryString;
    }
    if ($db->real_query($query)) {
      $smartResult = new smartResult($db);
      $smartResult->resultType = $this->objType;
      $smartResult->resultParams = $this->objArgs;
      $smartResult->link = $db;
      $this->error = "";
      $this->lastQuery = $query;
      $this->queryString = "";
      $this->numWheres = 0;
      return $smartResult;
    } else {
      $this->error = $db->error;
      $this->errorQuery = $queryString;
    $this->queryString = "";
    $this->numWheres = 0;
      return FALSE;
      // A real programmer would add exceptions here.
    }
  }

  public function count($query = NULL, $db = NULL) {
    if (!$db) { $db = $GLOBALS['db']; };
    if (!$query && $this->queryString != "") {
      $query = preg_replace('/select \*/', 'select count(*) as total', $this->queryString);
    }
    if ($res = $db->query($query)) {
      $row = $res->fetch_assoc();
      return $row['total'];
    } else {
      return FALSE;
    }
  }

  public function f($query = NULL, $db = NULL) {
   return $this->query($query, $db);
  }

}


abstract class smartObjectWithTags extends smartObject {
  public $updateTagCounts = TRUE; // TRUE/FALSE to update a tagCounts table.
  /** Something like:
     CREATE TABLE `tagCounts` (
     `tag` VARCHAR( 255 ) NOT NULL ,
     `total` INT( 64 ) NOT NULL ,
     PRIMARY KEY ( `tag` )
     ) ENGINE = MYISAM ;
  */


  /**
   * Add a tag or an array of tags to this object
   */

  function addTag($tags) {
    if(preg_match('/[^\s]+/', $this->tags)) {
        $array = explode(' | ', $this->tags);
    } else {
     $array = array();
    }

    if (!is_array($tags)) {
      $tag = $tags;
      $tags = array($tag);
    }
    foreach ($tags as $tag) {
      $array[] = $tag;
      if ($this->updateTagCounts) {
        if (is_a($this->db, 'idb')) {
          $res = $this->db->query("insert into tagCounts set `tag` = '$tag', `total` = 1 on duplicate key update `total` = `total` + 1;");
        }
      }
    }
    $this->tags = implode(' | ', array_unique($array));
  }

  /**
   * Remove a tag or an array of tags to this object
   */
  function removeTag($tags) {
    $array = explode(' | ', $this->tags);
    if (!is_array($tags)) {
      $tag = $tags;
      $tags = array($tag);
    }
    foreach ($tags as $tag) {
      $key = array_search( $tag, $array ); // Find key of given value
      if ($key != NULL || $key !== FALSE) {
        unset($array[$key]); // remove key from array
        if ($this->updateTagCounts) {
          if (is_a($this->db, 'idb')) {
            $res = $this->db->query("update tagCounts set `total` = `total` - 1 where `tag` = '$tag'");
          }
        }
      }
    }
    $this->tags = implode(' | ', array_unique($array));
  }

  function clearTags() {
    $array = explode(' | ', $this->tags);
    $this->tags = '';
    if (count($array) > 0 && $this->updateTagCounts) {
      foreach ($array as $tag) {
          if (is_a($this->db, 'idb')) {
            $res = $this->db->query("update tagCounts set `total` = `total` - 1 where `tag` = '$tag'");
          }
      }
    }
  }

}
?>
