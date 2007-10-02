<?
    //echo "IP: " . $_SERVER["REMOTE_ADDR"] . "\n";
    //echo "REQUEST_URI: " . $_SERVER["REQUEST_URI"] . "\n";
    //echo "HTTP_USER_AGENT: "  . $_SERVER["HTTP_USER_AGENT"] . "\n";
    // _SERVER["HTTP_REFERER"]

    if (isset($showdata))
        echo "<html><body><style>\ntd { font-family: sans-serif; font-size: 12px }\n</style>";

    $refer = $_SERVER["HTTP_REFERER"];
    //addCount('/home2/duliodd/public_html/guacosoft/stats.txt', $_SERVER["REQUEST_URI"], $refer);
    //addCount('/home2/duliodd/public_html/guacosoft/refer.txt', $refer, '[none]');

    if (isset($showdata) ||
        strstr($_SERVER["REQUEST_URI"], 'njam/Shop.php')
        && FALSE === strstr($refer, 'ttp://www.guacosoft.com/'))
    {
        //addCount('/home2/duliodd/public_html/guacosoft/plays.txt', $_SERVER["REMOTE_ADDR"], '[none]');
        addCount('/home2/duliodd/public_html/guacosoft/ppd.txt', date('Y-m-d'), '[none]');
    }

    if (isset($showdata))
        echo "</body></html>";

function addCount($file, $variable, $refer)
{
    global $showdata;

    $total1 = $total2 = 0;
    if (isset($showdata))   // render a nice table
    {
        echo "<table border=0 bgcolor=0 cellspacing=1 cellpadding=2>";
        echo "<tr bgcolor=silver><td>URL</td><td align=right>Hits</td>";
        if ($refer != '[none]')
            echo '<td align=right>Direct</td>';
        echo '<td>Visit</td></tr>';
    }

    // load info
    $fp = fopen($file, 'r');
    $uri = array();
    if ($fp)
    {
        while (true)
        {
            $line = fgets($fp);
            if (feof($fp))
                break;
            if (strstr($line, '|'))
            {
                $split = explode('|', $line);
                $uri[$split[0]] = (int)$split[1];
                $total1 += (int)$split[1];
                if ($refer != '[none]')
                {
                    $uri2[$split[0]] = (int)$split[2];
                    $total2 += (int)$split[2];
                }
                if (isset($showdata))
                {
                    $s = $split[0];
                    if (strlen($s) > 80)
                        $s = substr($s, 0, 80);
                    echo '<tr bgcolor=#E0E0E0><td>'.htmlspecialchars($s).
                        '</td><td align=right>'.$split[1].'</td>';
                    if ($refer != '[none]')
                        echo '<td align=right>'.$split[2].'</td>';
                    echo '<td><a href="'.$split[0].'">...</a></td></tr>';
                }
            }
        }
        fclose($fp);
    }

    if (isset($showdata))
    {
        echo "<tr bgcolor=silver><td>Total</td><td align=right>$total1</td>\n";
        if ($refer != '[none]')
            echo "<td align=right>$total2</td>";
        echo "<td></td></tr></table><br><br>\n";
        return;
    }

    if ($refer == '[none]' || strstr($refer, 'ttp://www.guacosoft.com/'))
    {
        if (isset($uri[$variable]))
            $uri[$variable]++;
        else
            $uri[$variable] = 1;

        if (empty($uri2[$variable]))
            $uri2[$variable] = 0;
    }
    else
    {
        if (isset($uri2[$variable]))
            $uri2[$variable]++;
        else
            $uri2[$variable] = 1;

        if (empty($uri[$variable]))
            $uri[$variable] = 0;
    }

    ksort($uri);

    $fp = fopen($file, 'w+');
    if ($fp)
    {
        foreach($uri as $a => $b)
        {
            $c = "$a|$b";
            if ($refer != '[none]')
                $c .= '|'.$uri2[$a];
            fputs($fp, $c."\n");
        }
        fclose($fp);
    }
}
?>
