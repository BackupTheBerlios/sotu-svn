<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
  <title>GuacoSoft.com - software for everyone</title>
  <meta name="GENERATOR" content="Quanta Plus">
  <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
  <meta name="KEYWORDS" content="home inventory, space, trading, shooter, assets, borrow, loan, free, download, linux, windows">
</head>

<style>
td { font-family: Sans-serif; font-size: 11px;}
body { 	background: #fff url(gradient.jpg) repeat-x;
	margin: 0; padding: 0; }


h1 {
	font-size: 16px;
	margin: 0 0 4px 0;
	font-family: verdana;
}

h2 {
	font-family: verdana;
	line-height: 1.2em;
	font-size: 14px;
	margin: 0 0 3px 0;
}

div.footer {
	text-align: center;
	background-color: #b4d8ec;
	font-family: verdana;
	color: #000;
	padding: 15px 0;
	font-size: 10px;
	line-height: 1.4em;
	margin-top: 25px;
}

div.footer a {
	color: #000;
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
	font-size: 11px;
	font-family: verdana;
}

div#header p.logo {
	float: right;
}

div#header a img {
	border: 0;
}

</style>

<body>

<div class="container">
	<div id="header">
		<p class="logo"><a href="index.php">Home</a> | <a
			href="index.php?page=Home">Products</a> | <a
			href="index.php?page=Shop">Shop</a> | <a
			href="index.php?page=About">About us</a>
		</p>
		<p><img src="guacosoft.gif" alt="Guacosoft logo"><br />SOFTWARE FOR EVERYONE</p>
	</div>
</div>

<br><br>
<center>

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


<br>

</center>
<div class="footer">
Copyright &copy; 2006, 2007 GuacoSoft.com
</div>

</body>
</html>
