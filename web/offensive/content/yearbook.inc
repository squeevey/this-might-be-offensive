<?
	// Include, and check we've got a connection to the database.
	require_once( 'admin/mysqlConnectionInfo.inc' );
	if(!isset($link) || !$link) $link = openDbConnection();

	require_once( 'offensive/tabs.inc' );
	
	function start() {
		if( ! is_numeric( $_SESSION['userid'] ) ) {
			session_unset();
			header( "Location: ./" );
		}
	}

	function body() {
?>
	<div class="heading">the beautiful people.</div>
<?
		tabs();
?>
		<div class="bluebox">
			<? yearbook(); ?>
		</div>
<?
	}

	function yearbook() {

		$order = array_key_exists("sort", $_REQUEST) && $_REQUEST['sort'] == 'date' ? 
		         'up.timestamp DESC' :
			 'username';

		if( $order == 'username' ) {
			?><b><a href="./?c=yearbook&sort=date">sort by date</a></b><?
		} else {
			?><b><a href="./?c=yearbook">sort by username</a></b><?
		}
?>



<table width="100%" class="thumbnails">

<?

		$numPerPage = 100;
		$page = array_key_exists("p", $_REQUEST) ? $_REQUEST['p'] : 0;

		if( ! is_numeric( $page ) ) {
			$page = 0;
		}
		$start = ($page * $numPerPage);

		$sql = "SELECT users.userid, username, account_status, filename, up.id
					FROM users, offensive_uploads up
					WHERE users.userid=up.userid
						AND up.type='avatar'
						AND account_status != 'locked'
						AND up.id = (SELECT MAX( up.id) FROM offensive_uploads up where type='avatar' AND userid=users.userid)
					ORDER BY $order, up.id DESC";

		$result = mysql_query($sql) or trigger_error(mysql_error(), E_USER_ERROR);

		echo mysql_error();

		$THUMBS_PER_ROW = 4;
		$count = 0;

		while( $row = mysql_fetch_assoc( $result ) ) {
		
		
			if( (($count++) % $THUMBS_PER_ROW) == 0 ) {
				$css = ($css == "even_row") ? "odd_row" : "even_row";
				?><tr class="<?= $css ?>"><?
			}
		
			?>
				
					<td>
						<?
							$thumbnail = file_exists( "images/users/thumbs/th-" . $row['filename'] ) ? "th-" . $row['filename'] : "th-unavailable.gif"
						?>
						<a href="images/users/<?= $row['filename'] ?>"><img src="images/users/thumbs/<?= $thumbnail ?>"/></a>
						<br/>
						<a href="./?c=user&userid=<?= $row['userid'] ?>"><?= $row['username'] ?></a>
						<a href="./?c=comments&fileid=<?= $row['id'] ?>">[c]</a>						
					</td>

			<?
			
			if( (($count) % $THUMBS_PER_ROW) == 0 ) {
				?></tr><?
			}
		}

		if( ($count % $THUMBS_PER_ROW) != 0 ) {
			?></tr><?
		}
		?>

	</table>


<?
	}
?>