<?

$validation_errors = array();

/*
 * Checks an incoming argument (usually in $_REQUEST) for type and validity.
 * Arguments can be optional, or enforced to be one of a set of possibilities.
 * If an argument fails to check, an error message is added to the global
 * $validation_errors array, and false is returned.  A false return value 
 * can be returned if an argument is optional and not set (but no error 
 * will be registered).
 * Note that all arguments returned by this function that are not type "string"
 * (not including the false return value) are safe to use in a SQL query.
 */
function check_arg($key, $type, $method=null, $required=true, $set=null) {
	global $validation_errors;

	// make sure a valid method is being used.
	if($method == null) $method = $_REQUEST;
	// if this is triggered, there is a code problem.  kaboom.
	if(!is_array($method)) {
		trigger_error("ASSERT: invalid method", E_USER_ERROR);
		exit;
	}
	
	// optional vs required variables
	if(!array_key_exists($key, $method) || $method[$key] == '') {
		if($required) {
			$validation_errors[] = "required parameter '$key' is not set (needs: $type).";
		}
		if($type == "limit") return "0,200";
		return false;
	}
	
	// currently we don't accept arrays, which you can submit through POST
	if(!is_string($method[$key])) {
		$validation_errors[] = "parameter '$key' must be of type $type.";
	}
	// this might change someday.  but not today.
	
	// general type enforcement
	switch($type) {
		case "string":
			if(strlen($method[$key]) == 0 && $required) {
				$validation_errors[] = "parameter '$key' cannot be zero-length.";
				return false;
			}
			break;
		case "integer":
			if(!is_numeric($method[$key]) || strpos($method[$key], ".") !== false) {
				$validation_errors[] = "parameter '$key' must be of type $type (was: ".$method[$key].").";
				return false;
			}					
			break;
		case "float":
			if(!is_numeric($method[$key])) {
				$validation_errors[] = "parameter '$key' must be of type $type (was: ".$method[$key].").";
				return false;
			}
			break;
		/*
		 * dates are not set matched (it doesn't make much sense to), so
		 * we return the properly-formatted date representation of the argument
		 * that can be used immediately in a query.
		 */
		case "date":
			// unix timestamps, stop here.
			if(is_numeric($method[$key]) && strpos($method[$key], ".") === false) {
				return date("Y-m-d H:i:s", (int)$method[$key]);
			}
			// other date formats that we recognize, stop here.
			if(strtotime($method[$key]) !== false) {
				return date("Y-m-d H:i:s", strtotime($method[$key]));
			}
			$validation_errors[] = "parameter '$key' is not a recognizable date string.";				
			return;
		/*
		 * Limits are special types.  They are in the form %d or %d,%d, matching the MySQL syntax
		 * for limits on queries.  This function detects if the argument is in the correct form, 
		 * and returns a strictly-formatted %d,%d string back to the caller, after enforcing 
		 * syntax and a maximum limit.
		 */
		case "limit":
			// %d,%d or %d only.
			$limit = $offset = false;

			/*
			 * Either the limit is a standalone integer...
			 */
			if(is_numeric($method[$key]) && strpos($method[$key], ".") === false) {
				$limit = (int)$method[$key];
				$offset = 0;
			/*
			 * Or it is in the format %d,%d, MySQL style.
			 */
			} else if(strpos($method[$key], ",") !== false) {
				$arr = explode(",", $method[$key]);

				/* only accept if there are two elements in the explosion, 
				 * both of which are integers.
				 */
				if(count($arr) == 2 &&
				   is_numeric($arr[0]) && strpos($arr[0], ".") === false &&
				   is_numeric($arr[1]) && strpos($arr[1], ".") === false ) {
					list($offset, $limit) = $arr;
				}
			}

			// did we make it?  is there a limit and an offset?
			if($limit === false || $offset === false) {
				$validation_errors[] = "parameter '$key' is not in the correct format.  ".
				            "Accepted formats: '%d' and '%d,%d'.";
				return false;
			}

			/* force-coerce in case there are chars that are non-numeric, 
			 * but acceptable for php soft-coercion.
			 */
			$limit = (int)$limit;
			$offset = (int)$offset;

			// currently we enforce a limit of 200 elements per request.  just in case.
			if($limit > 200) {
				$validation_errors[] = "the limit of parameter '$key' cannot exceed 200.  you requested: $limit.";
				return false;
			}

			return "$offset,$limit";
	}
	
	// check if a value is only allowed to be one in a limited set
	if(is_array($set)) {
		foreach($set as $example) {
			if($example == $method[$key]) {
				return $method[$key];
			}
		}
		$err = "parameter '$key' must be one of: { ";
		foreach($set as $example) {
			$err .= "\"$example\" ";
		}
		$validation_errors[] = $err."}, was ".$method[$key];
		return false;
	}
	
	// coerce values to the appropriate php type on return.
	switch($type) {
		case "float":
			return (double)$method[$key];
		case "integer":
			return (int)$method[$key];
		default:
			return $method[$key];
	}
}

/*
 * Check the global $validation_errors array for error messages.  
 * If any exist, print them out and exit with an error code.
 * This should always be called after the last argument is validated with
 * check_arg to ensure that execution stops on error.
 */
function handle_errors() {
	global $validation_errors;
	if(count($validation_errors) == 0) return true;
	
	header("400 Bad Request");
	header("Content-type: text/plain");
		
	foreach($validation_errors as $error) {
		trigger_error($error, E_USER_WARNING);
		echo "$error\n";
	}
	
	exit;
}

?>