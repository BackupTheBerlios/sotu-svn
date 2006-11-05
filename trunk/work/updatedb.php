<?
// we'll do our own error handling
error_reporting(0);
if ($argc == 3)
{
  echo "Connect...";
  if (!ibase_connect($argv[1], 'SYSDBA', $argv[2], 'WIN1250'))
  {
    echo "ERROR: ".ibase_errmsg();
    exit("\n");
  }
  $res = ibase_query('select current_version from database_version');
  if (!$res)
  {
    echo "ERROR: ".ibase_errmsg();
    exit("\n");
  }
  while ($row = ibase_fetch_row($res))
    echo "Version = ".$row[0]."\n";
  exit();
}
if ($argc != 4)
{
  echo "Usage: ";
  echo "updatedb.php database_path sysdba_password [start|SET=version]\n";
  exit();
}
echo "Updating database: ".$argv[1]."\n";

if (strpos($argv[3], '='))
{
  $p = explode('=', $argv[3]);
  if ($p[0] != "SET")
    exit("Error. Unknown option\n");

  echo "Connect...";
  if (!ibase_connect($argv[1], 'SYSDBA', $argv[2], 'WIN1250'))
  {
    echo "ERROR: ".ibase_errmsg();
    exit("\n");
  }

  if (!ibase_query("update database_version set current_version = "
    .$p[1].", last_change = current_timestamp") || !ibase_commit())
  {
    echo "\nVersion number not written, error:\n\n".ibase_errmsg();
    exit("\n");
  }
}

$i = $argv[3];
while (true)
{
  $fn = sprintf("%04d.sql", $i);
  if (!file_exists($fn))
    break;

  echo "Connect...";
  if (!ibase_connect($argv[1], 'SYSDBA', $argv[2], 'WIN1250'))
  {
    echo "ERROR: ".ibase_errmsg();
    exit("\n");
  }

  echo "Loading script: $fn.";
  $fp = fopen($fn, 'r');
  if (!$fp)
    exit("cannot open file");
  $sql = fread ($fp, filesize ($fn));
  fclose($fp);
  if (trim($sql) == "")
    exit("ERROR: empty script\n");

  echo "...RUNNING...";
  if (!ibase_query($sql))
  {
    echo "ERROR.\n\n".ibase_errmsg();
    exit("\n");
  }

  echo "Commit...";
  if (!ibase_commit())
  {
    echo "ERROR.\n\n".ibase_errmsg();
    exit("\n");
  }

  if (!ibase_query("update database_version set current_version = $i"
    .", last_change = current_timestamp") || !ibase_commit())
  {
    echo "\nVersion number not written, error:\n\n".ibase_errmsg();
    exit("\n");
  }

  echo "OK.\n";
  ibase_close();
  $i++;
}
echo "Done.\n";
?>
