<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
    <title>Scum of the Universe - space shooter and strategy game</title>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
    <meta name="description" content="Space shooter similar to Galaga and Space Invaders. Beside shooting aliens it is an adventure game with strategic elements like trading from Elite.">
    <meta name="keywords" content="space, trading, combat, shooter, arcade, 3D, galaga, space, invaders, adventure, elite, game, trade, galaxy, scum of the universe, free, download">
    <link rel="stylesheet" href="../common.css" type="text/css">

<style>
p { color: #99FF99 }
</style>
</head>

<body style="background: #000 url(gradientscum.jpg) repeat-x; margin: 0; padding: 0;"
link="#66ff66" vlink="#66ff66" alink="#66ff66">
<center>
<div class="container">
    <div id="header">
        <p class="links"><a style="color:#66ff66" href="../index.php">Home</a> | <a
            style="color:#66ff66" href="../index.php?page=Home">Products</a> | <a
            style="color:#66ff66" href="../index.php?page=Shop">Shop</a> | <a
            style="color:#66ff66" href="../index.php?page=About">About us</a>
        </p>
        <p><img src="scum.png" alt="Scum of the Universe"><br />
        <font color="#66ff66">A SPACE ADVENTURE</font></p>
    </div>
</div>

<br><br>
<div class="scummenu">
<?
    if (empty($page) && isset($_GET['page']))
        $page = $_GET['page'];
    if (empty($page))
        $page = 'Story';

    $pages = array('Story', 'Screenshots', 'Manual', 'Download', 'Shop');
    foreach ($pages as $p)
    {
        if ($p == $page)
            echo '<B style="margin: 0px 15px 0px 15px; color: #99FF99;">'.$p.'</B>';
        else
            echo '<a style="margin: 0px 15px 0px 15px;" href="index.php?page='.$p.'">'.$p.'</a>';
    }
    echo "</div>";

    $found = false;
    foreach ($pages as $p)
    {
        if ($p == $page)
        {
            $found = true;
            require($p.'.inc');
        }
    }
    if (!$found)
        require('Story.inc');
?>
</center>


<div class="footer"
    style="
        color: #99FF99;
        background-color: #336633;
        border-top: 1px solid #669966;
        border-bottom: 1px solid #669966;">
Copyright &copy; 2006, 2007 GuacoSoft.com
</div>

</body>
</html>
