<?PHP
function htmlutf8entities ($string) {
    #curly quotes
    global $debug;
    $debug->debug("utf8 %s", $string, 1);
    $search = array(chr(145),
                    chr(146),
                    chr(147),
                    chr(148),
                    chr(151));

    $replace = array("'",
                     "'",
                     '"',
                     '"',
                     '-');

    #$string = str_replace($search, $replace, $string);

    $debug->debug("utf8 %s", $string, 1);


    return htmlentities($string, ENT_QUOTES, 'UTF-8');

}


function unhtmlize($string, $jssafe = FALSE) {

    $search = array(chr(145),
                    chr(146),
                    chr(147),
                    chr(148),
                    chr(151),
                    chr( 226 )
                    );

    $replace = array("'",
                     "'",
                     '"',
                     '"',
                     '-',
                     '-');

    $string = str_replace($search, $replace, $string);

    $s = html_entity_decode( $string, ENT_QUOTES, 'UTF-8' );

    $s = preg_replace('/&#(0)*39;/', "'", $s);
    if ($jssafe) {
        $s = preg_replace('/\r\n/',"\\n",$s);
        $s = preg_replace('/\'/', "\'", $s);
    }

    return $s;
}

function truncate($string, $limit = 50, $singleLine=TRUE) {
# simple function to truncate a string to a certain word limit
  $string2 = preg_replace('/\<(br|p)[ \/]*\>/', '\n', $string);
  $string2 = preg_replace('/\&(amp\;)*nbsp\;/', ' ', $string2);

    if ( $singleLine ) {
        $temp = explode( '\n', $string2 );
     $string2 = $temp[0];
    }
	$string = strip_tags($string2);
	$fullArray = explode(" ", $string);
	$array = explode(" ", $string, $limit+1);
#	print count($array)." ".count($fullArray)." ".$limit;
	if (count($fullArray) > $limit) {
	# print "yes";
		$append = "...";
	}
	if(count($array) > $limit) {
		array_pop($array);
	}
	#	pre($array);
	$string2 = implode(" ", $array) . $append;
			  $string = preg_replace('/\\\\n/', '<br />', $string2);

	return $string;
}

function splitURL($string, $chunkLen = 50) {
# simple function used with unhtmlize to truncate a url to a certain limit for display
	$chunked_str = chunk_split($string, $chunkLen);
	$output = str_replace( "\r\n", "&lt;br&gt;", $chunked_str );
	return $output;
}

function multiline($text) {
  $retval = preg_replace("/\n/", "<br />\n", $text);
  return $retval;
}

function formatPhone($number) {
  $number = preg_replace("/[^0-9]/","",$number);
  $npa = substr($number, 0,3);
  $nxx = substr($number, 3,3);
  if (strlen($number) == 10) {
  $rest = substr($number, 6);
  $string = "$npa-$nxx-$rest";
  } else {
    $rest = substr($number,6,4);
    $ext = substr($number,10);
    $string = "$npa-$nxx-$rest x$ext";
  }
  return $string;

}

function formatMoney($text) {
  $retval = "$".number_format($text,2);
  return $retval;
}

function formatSocial($text) {
  $temp = preg_replace("/[^0-9]*/", "", $text);
  $temp = str_pad($temp, 9, "0", STR_PAD_LEFT);
  $one = substr($temp, 0, 3);
  $two = substr($temp, 3, 2);
  $three = substr($temp, 5);
  $retval = $one ."-". $two ."-". $three;
  return $retval;

}

function returnStyleString($styleArray) {
	// this takes a style array like the defaultStyles value in config and
	// returns a formated string for use in an EXT style config or an html style value

	if($styleArray['bold']) {
		$styleString = "font-weight: bold;";
	} else {
		$styleString = "font-weight: normal;";
	}
	if($styleArray['italic']) {
		$styleString .= "font-style: italic;";
	} else {
		$styleString .= "font-style: normal;";
	}
	if($styleArray['underline']) {
		$styleString .= "text-decoration: underline;";
	} else {
		$styleString .= "text-decoration: none;";
	}
	$styleString .= "color:".$styleArray['color']."; font-size: ".$styleArray['size']."; font-family: ".$styleArray['family'].";";

	return $styleString;

}


function doStatusCheck($siteID) {
	// this is currently a placeholder function
	// this should eventually run all the checks for a site and determine what still needs to be done
	// right now it just sets the status of the site to open, closed or ready to launch
	global $db;

	$sql = "SELECT * FROM sites WHERE id = ".$siteID;
	$siteRes = $db->query($sql);
	$site = $siteRes->fetch_object();
	$curTime = time();

	if($curTime > $site->realEndDate) {
		$sql = "UPDATE sites SET status = 0 WHERE id = ".$siteID;
		$siteUpdate = $db->query($sql);
	} elseif($curTime < $site->realEndDate && $curTime > $site->startDate) {
		$sql = "UPDATE sites SET status = 2 WHERE id = ".$siteID;
		$siteUpdate = $db->query($sql);
	} else {
		$sql = "UPDATE sites SET status = 1 WHERE id = ".$siteID;
		$siteUpdate = $db->query($sql);
	}

}

function getSlidesBySectionOrder($index, $siteID) {
	global $db, $debug;
	$sql = "SELECT * FROM sections WHERE sectionOrder = ".$index." AND siteID = ".$siteID;

	$debug->debug('Index is %o and siteID is %o', $index, $siteID, 3);
	$sectionRes = $db->query($sql);
	$section = $sectionRes->fetch_object();

	$sql = "SELECT * FROM slides WHERE sectionID = ".$section->sectionID." AND siteID = ".$siteID." ORDER BY slideIndex DESC LIMIT 0,1";
	$debug->debug('sql is %o', $sql, 3);
	$slideRes = $db->query($sql);

	if($slideRes->num_rows != 0) {
          return $slideRes->fetch_assoc();
	} else if ($index - 1 > 0) {
          return getSlidesBySectionOrder($index - 1, $siteID);
	} else {
          return false;
        }


}

?>
