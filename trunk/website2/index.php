<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
  <title>GuacoSoft.com - software for everyone</title>
  <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
  <meta name="KEYWORDS" content="home inventory, space, trading, shooter, assets, borrow, loan, free, download, linux, windows">
  <meta name="description" content="Home inventory software, space trading and shooter game, and other software products.">
  <link rel="stylesheet" href="common.css" type="text/css">
</head>

<body style="background: #fff url(gradient.jpg) repeat-x; margin: 0; padding: 0;">
<center>
<div class="container">
    <div id="header">
        <p class="links"><a href="index.php">Home</a> | <a
            href="index.php?page=Home">Products</a> | <a
            href="index.php?page=Shop">Shop</a> | <a
            href="index.php?page=About">About us</a>
        </p>
        <p><img src="guacosoft.gif" alt="Guacosoft logo"><br />SOFTWARE FOR EVERYONE</p>
    </div>
</div>

<br><br>
<?
    $pages = array('Home', 'Products', 'Shop', 'About');
    if (empty($page) && isset($_GET['page']))
        $page = $_GET['page'];
    if (empty($page))
        $page = 'Home';
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
        require('Home.inc');
?>
</center>
<br>

<div class="footer">
Copyright &copy; 2006, 2007 GuacoSoft.com
</div>

</body>
</html>
