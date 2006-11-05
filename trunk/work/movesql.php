<?
if ($argc != 6)
{
  echo "Usage:   movesql.php in_pattern out_pattern start end moveto\n";
  echo "Example: movesql.php log%5d.sql %03d.sql    1     145 346\n";
  echo "         log00001.sql -> 346.sql\n";
  echo "         log00002.sql -> 347.sql\n";
  exit();
}

$inp = $argv[1];
$outp = $argv[2];
$start = $argv[3];
$end = $argv[4];
$moveto = $argv[5];
echo "MOVE: (". sprintf($inp, $start) . " - " . sprintf($inp, $end)
  . ") TO: (". sprintf($outp, $moveto) . " - "
  . sprintf($outp, $moveto + $end - $start) . ")\n";

for ($i = $start; $i <= $end; $i++)
{
  $from = sprintf($inp, $i);
  if (!file_exists($from))
  {
    echo "FILE: $from does not exists, skipping.\n";
    continue;
  }
  $to = sprintf($outp, $moveto++);
  if ($from == $to)
  {
    echo "SAME FILES: $from, skipping\n";
    continue;
  }
  echo "MOVE: $from -> $to\n";
  if (!rename($from, $to))
  {
    echo "ERROR!!!";
    break;
  }
}
?>
