<?PHP


/**
 * anApp API -- Based on the Ext.Direct API and the php router by Tommy Maintz
 *              http://www.extjs.com/forum/showthread.php?t=68186
 *
 *
 * @category API
 * @package anApp
 * @subpackage API
 * @author Bryan P Johnson <bryan@12ftguru.com>
 * @license http://www.opensource.org/licenses/bsd-license.php BSD
 * @version 1.0
 * @link
 */

 include_once('inflect.inc');
class api {
     private $forbiddenExtensionsRegex = "/\.(inc|php|phps|php5|ihtml|js)$/";
   public function __construct() {
     global $user, $config, $db, $login;

        //    ini_set('display_errors', 1);
        //  error_reporting(E_ALL);



      if (strstr($_SERVER['REQUEST_URI'],'?')) {
        list($uri, $cruft) = explode('?', $_SERVER['REQUEST_URI']);
      } else {
        $uri = $_SERVER['REQUEST_URI'];
      }

      $uri = '/'. preg_replace('/^'. preg_quote($config->self, '/') . '/', '', $uri);
      $fileInfo = explode('/', rtrim($uri, '/'));
      $appdir = realpath(__DIR__.'/../');


         if(file_exists($appdir.$uri) and ($_SERVER['SCRIPT_FILENAME']!=$appdir.$_SERVER['REQUEST_URI']) and ($_SERVER['REQUEST_URI']!="/") and !preg_match($this->forbiddenExtensionsRegex, $_SERVER['REQUEST_URI']) and !is_dir($appdir.$uri)){


            $array = explode("/", $_SERVER['SCRIPT_NAME']);
            $path1 = $appdir.$uri."/".$array[count($array)-1];
            $path2 = preg_replace('/\/\//', '/', $path1);
            $path3 = str_replace("\\", '/', __FILE__);

            if ($path2 != $path3) {
                list($url,$cruft2)=explode('?',$_SERVER['REQUEST_URI']);

                include($appdir.$url);
                exit();
            }
        } else {
            if ($config->defaultAPI != '') {
              $oldEH = set_error_handler('defaultAPI');
              $oldLev = error_reporting(0);
          global $widgetName;
          $widgetName = $fileInfo[count($fileInfo)-1]; // drop the .json
                          #  print('1Attempting to include: '.$widgetName);
         include_once("api/".$widgetName.".inc");
         restore_error_handler();
         error_reporting($oldLev);
         $widgetName = Inflect::singularize($widgetName);
         $widgetName .= "API";
//         print('Attempting to include2: '.$widgetName);

         $widget = new $widgetName;
            $widget->json();
            exit;
        } else {
            global $widgetName;
            $widgetName = $fileInfo[count($fileInfo)-1]; // drop the .json
                          #     print('2Attempting to include: '.$widgetName);
           include_once("api/".$widgetName.".inc");

           $widgetName = Inflect::singularize($widgetName);
           $widgetName .= "API";
         #  print('Attempting to include2: '.$widgetName);

           $widget = new $widgetName;
              $widget->json();
              exit;
        }
       }
     }
        }


function defaultAPI($n, $s, $f, $l, $c) {

  if ($n > E_WARNING) {
    print "anApp PHP Error: ".$s." in $f ($l)<br />";
    return FALSE;
  }
  global $config, $widgetName, $defaultAPI;
  $defaultAPI = $config->defaultAPI;
  include_once("api/".$defaultAPI.".inc");
  $defaultAPI = Inflect::singularize($defaultAPI);
  $defaultAPI .= "API";
  $widget = new $defaultAPI($widgetName);
  $widget->json();
  exit;
}

?>
