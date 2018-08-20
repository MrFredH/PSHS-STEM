<?php
// No bells and whistles, this is only for the Arduino talking to the server to send data.
//$query = "insert into sensor_data ("..") values ("..")";
error_reporting(E_ALL);
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(-1);
header ("Cache-Control: no-cache, must-revalidate");
header ("Pragma: no-cache");
$servername = "localhost";
$username = "Sensors";
$password = "Password1";
$dbname = "Sensors";
try {
    $conn = new PDO("pgsql:host=$servername;dbname=$dbname", $username, $password);
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
?>
<?php
  $post_data = file_get_contents("php://input");
// if (!empty($_POST)) {
// if (!empty($post_data)) {
 	    echo "Got new data:<br>\n";
 	    $conn->beginTransaction();
 	    $stmt = $conn->prepare("insert into newdata_log (data) values (?)");
 	    $stmt->execute(array($conn->quote(json_encode($post_data))));
 	    $conn->commit();
//}
$conn = null;
}
catch(PDOException $e) {
	error_log($e->getMessage());
    echo "Error: " . $e->getMessage();
}
?>
