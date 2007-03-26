<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
    <title>Home Inventory Software</title>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
    <meta name="description" content="Home inventory software used to build and maintain a database of your assets. You can keep track of all the stuff you have, what you have loaned to others, and all the maintenance costs.">
    <meta name="keywords" content="free, download, attic, manager, home, inventory, software, assets, business, small, medium, database, stuff, things, own, load, borrow, costs, planning, insurance">

<style type="text/css">
td { font-family: Sans-serif; font-size: 14px; }
body {
    background: #fff url(../gradient.jpg) repeat-x;
    margin: 0; padding: 0; }

h1 {
    font-size: 16px;
    margin: 0 0 4px 0;
    font-family: verdana;
}

h2 {
    font-size: 14px;
    margin: 0 0 3px 0;
    font-family: verdana;
    line-height: 1.2em;
}

div.footer {
    text-align: center;
    background-color: #b4d8ec;
    font-family: verdana;
    color: #000;
    padding: 15px 0;
    font-size: 11px;
    line-height: 1.4em;
    margin-top: 25px;
    border-top: 1px solid #6666AA;
    border-bottom: 1px solid #6666AA;
}

div.container {
    width: 765px;
    margin: 0 auto;
}

div#header {
    height: 20px;
    margin: 15px auto;
    width: 750px;
    text-align: left;
}

div#header p {
    padding-top: 2px;
    font-size: 12px;
    font-family: verdana;
}

div#header p.links {
    float: right;
}

.spacer:after {
    content: ".";
    display: block;
    height: 0;
    clear: both;
    visibility: hidden;
}

.spacer {display: inline-block;}

/* Hides from IE-mac \*/
* html .spacer {height: 1%;}
.spacer {display: block;}
/* End hide from IE-mac */

a       { text-decoration: none; }
a:link  { text-decoration: none; }
a:hover { text-decoration: underline; }
a img, a:link img {
    border: 0px;
}


a.menuitem          {
    font-size: 12px;                 font-family: Sans-serif;
    text-align: center;
    border-width: 0;
    background-color: #FFFFFF;       color: black;
    padding: 2px 15px 2px 15px;      text-decoration: none;
    display: block;                  margin: 0px 5px 0px 5px;
}
a.menuitem:hover    {
    color: black;
    background-color: #AACCFF;
    text-decoration: none;
}

</style>
</head>

<body>
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
<table cellspacing="0" cellpadding="0">
    <tr>
<?
    if (empty($page) && isset($_GET['page']))
        $page = $_GET['page'];
    if (empty($page))
        $page = 'About';

    $pages = array('About', 'Features', 'Download', 'Manual', 'Shop', 'Support');
    foreach ($pages as $p)
    {?>
        <td><a class="menuitem" href="index.php?page=<? echo $p; ?>"><?
        if ($p == $page)
            echo "<B>";
        echo $p;
        if ($p == $page)
            echo "</B>";
        ?></a></td><?
    }
    echo "</tr></table>";

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
