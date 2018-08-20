<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(-1);
header ("Cache-Control: no-cache, must-revalidate");
header ("Pragma: no-cache");
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>Aquatics Monitoring System</title>
<meta name="generator" content="Bluefish 2.2.6" >
<meta name="author" content="Year 10 Student" >
<meta name="date" content="2017-08-30T09:19:26+1000" >
<meta name="copyright" content="">
<meta name="keywords" content="">
<meta name="description" content="">
<meta name="ROBOTS" content="NOINDEX, NOFOLLOW">
<meta http-equiv="content-type" content="text/html; charset=UTF-8">
<meta http-equiv="content-type" content="application/xhtml+xml; charset=UTF-8">
<meta http-equiv="content-style-type" content="text/css">
<meta http-equiv="expires" content="0">
<meta http-equiv="refresh" content="1; URL="+<?php htmlspecialchars($_SERVER["PHP_SELF"]); ?>>
<style type="text/css">
      .table_titles, .table_cells_odd, .table_cells_even {
		padding-right: 20px;
		padding-left: 20px;
		color: #000;#
	}
	.table_titles {
		color: #000000;
		background-color: #996DFF;
	}
	.table_cells_odd {
		background-color: #00AAFF;
	}
	.table_cells_even {
		background-color: #D76DF0;
	}
	table {
		border: 2px solid #333;
	}
	body {
		font-family: "Trebuchet MS", Arial;
	}
</style>
</head>
<body>
<h1>Database Records</h1>
This table shows the information gathered by the sensors then transfered to the database.
<?php
echo "<table border=\"0\" cellspacing=\"0\" cellpadding=\"4\">";
//echo "<tr><th>Id</th><th>Firstname</th><th>Lastname</th></tr>";
class TableRows extends RecursiveIteratorIterator {
	public $oddrow = true;
	public $css_class = ' class="table_cells_odd"';
    function __construct($it) {
        parent::__construct($it, self::LEAVES_ONLY);
    }

    function current() {
    	return "<td".$this->css_class.">" . parent::current(). "</td>";
    }

    function beginChildren() {
    	if($this->oddrow)
    	{
    		$this->css_class=' class="table_cells_odd"';
    	
     	}
    	else {
    		$this->css_class=' class="table_cells_even"';
    	}
        echo "<tr>";
    }

    function endChildren() {
        echo "</tr>" . "\n";
        $this->oddrow = !$this->oddrow;      
    }
}

$servername = "localhost";
$username = "Sensors";
$password = "Password1";
$dbname = "Sensors";

try {
    $conn = new PDO("pgsql:host=$servername;dbname=$dbname", $username, $password);
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    //$stmt = $conn->prepare("SELECT *, 'Yes' as \"Mr Houweling Rocks\" FROM prefix order by power asc");
	 //$stmt = $conn->prepare("SELECT * FROM newdata_log last 10");

	 $stmt = $conn->prepare("select * from (SELECT * FROM newdata_log order by rec_id desc limit 6) as SubQuery order by rec_id");
    $stmt->execute();

    // set the resulting array to associative
    $result = $stmt->setFetchMode(PDO::FETCH_ASSOC);
	 $recordSet = $stmt->fetchAll();
       // Get table headers dynamically
	echo '<tr class="table_titles">';
	// Create the header row    
    foreach($recordSet[0] as $k=>$v) {
        echo "<th>$k</th>";
    }    
    echo "</tr>";
    foreach(new TableRows(new RecursiveArrayIterator($recordSet)) as $k=>$v) {
        echo $v;
    }
}
catch(PDOException $e) {
    echo "Error: " . $e->getMessage();
}
$conn = null;
echo "</table>";
?>
<h1>Graph</h1>


</body>
</html>

