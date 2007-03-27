<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
    <title>Home Inventory Software</title>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
    <meta name="description" content="Home inventory software used to build and maintain a database of your assets. You can keep track of all the stuff you have, what you have loaned to others, and all the maintenance costs.">
    <meta name="keywords" content="free, download, attic, manager, home, inventory, software, assets, business, small, medium, database, stuff, things, own, load, borrow, costs, planning, insurance">
    <link rel="stylesheet" href="../common.css" type="text/css">
</head>

<body style="background: #fff url(../gradient.jpg) repeat-x; margin: 0; padding: 0;">
<center>
<div class="container">
    <div id="header">
        <p class="links"><a href="../index.php">Home</a> | <a
            href="../index.php?page=Home">Products</a> | <a
            href="../index.php?page=Shop">Shop</a> | <a
            href="../index.php?page=About">About us</a>
        </p>
        <p><img src="attic.gif" alt="Guacosoft logo"><br />HOME INVENTORY SOFTWARE</p>
    </div>
</div>

<br><br>
<div class="atticmenu">
<?
    if (empty($page) && isset($_GET['page']))
        $page = $_GET['page'];
    if (empty($page))
        $page = 'About';

    $pages = array('About', 'Features', 'Download', 'Manual', 'Shop', 'Support');
    foreach ($pages as $p)
    {
        if ($p == $page)
            echo '<B style="margin: 0px 15px 0px 15px; color:black;">'.$p.'</B>';
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
        require('About.inc');
?>
</center>


<div class="footer">
Copyright &copy; 2006, 2007 GuacoSoft.com
</div>

</body>
</html>
