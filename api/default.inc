<?php

class defaultAPI extends simpleAPI {
  
  // True for 'id', false for 'singluarTableNameID' and string for exceptions.
  // If the object type is not specified, 'id' is assumed. So you
  // don't have to enter anything here if you use 'id' for your id.

  public $idMap = array(
    'user' => true,
  );

}
?>
