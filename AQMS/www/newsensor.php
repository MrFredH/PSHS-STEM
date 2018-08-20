 <?php
error_reporting(E_ALL);
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(-1);
header ("Cache-Control: no-cache, must-revalidate");
header ("Pragma: no-cache");

$hereText= <<<end_delimiter
This table shows the information gathered by the sensors then transfered to the database.
echo($hereText);

$servername = "localhost";
$username = "Sensors";
$password = "Password1";
$dbname = "Sensors";
try {
    $conn = new PDO("pgsql:host=$servername;dbname=$dbname", $username, $password);
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    //$stmt = $conn->prepare("SELECT *, 'Yes' as \"Mr Houweling Rocks\" FROM prefix order by power asc");
?>
<?php
 if (!empty($_POST)) {
 	    echo "Got new sensor request:<br>\n";
 	    echo "name={".htmlspecialchars($_POST["name"]) . "}<br>\n";
 	    echo "location={".htmlspecialchars($_POST["location"]) . "}<br>\n";
 	    $query = 'insert into sensors (name, location) values ('.$_POST["name"].', '.$_POST["location"].');';
 	    echo "You get the idea, now format the returned data into a [".$query."] and send it to the database<br>";
 	    //$stmt = $conn->prepare($query);
       //$stmt->execute();
 }
 ?>
<h1>Create a New Sensor</h1><br>
<form action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]); ?>" method="post" enctype="multipart/form-data">
<input type="text" name="name" value="Name your sensor"><br>
<input type="text" name="location" value="Where is this sensor?"><br>
<input type="text" name="project" value="What project is this for?"><br>
<input type="text" name="address" value="Unique hardware identifier"><br>
<input type="text" name="contact" value="Who made this sensor?"><br>
<input type="hidden" name="action" value="Submit">
<select name="sensorType">
<?php
    $stmt = $conn->prepare("SELECT * FROM sensor_type");
    $stmt->execute();

    // set the resulting array to associative
    $result = $stmt->setFetchMode(PDO::FETCH_ASSOC);
	 while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
	 	echo '<option value="' . $row['name'] . '" label="'. $row['rec_id'] .'">'.$row['name']. '</option>'."\n";
		}
}



catch(PDOException $e) {
    echo "Error: " . $e->getMessage();
}
$conn = null;
?>
</select>

<?php

/*
name		text,
		location	text,
		project		text,
		sensor_address  text,
		sensor_contact  text,
		sensor_type	integer REFERENCES sensor_type (rec_id)
*/
?>
<input type="submit" name="Create Sensor"/>
</form>