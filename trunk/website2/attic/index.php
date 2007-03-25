<html>
<head>
  <meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">
  <title>Home Inventory Software</title>
    <meta name="description" content="Attic Manager is home inventory software used to build and maintain a database of your assets. You can keep track of all the stuff you have, what you have loaned to others, and all the maintenance costs.">
    <meta name="keywords" content="free, download, attic, manager, home, inventory, software, assets, business, small, medium, database, stuff, things, own, load, borrow, costs, planning, insurance">
<style>
td { font-family: Sans-serif; }

div.menu   { width: 190px; margin-top: 0px; }
div.menu a { text-decoration: none; }

a       { text-decoration: none; }
a:hover { text-decoration: underline; }

a.menuitem:hover    { color: black; background-color: #AACCFF; }
a.menuitem          {
    font-size: 18px;         font-family: Sans-Serif;
    text-align: center;          border-color: black;
        border-style: solid;         border-width: thin;
        background-color: #336699;   background-repeat: repeat-y;
        color: white;                padding: 5px 0px 5px 0px;
        display: block;              margin: 0px 20px 10px 0px;
}

div.glavni { border-style: solid; border-width: thin; border-color: black; }

</style>
</head>

<body bgcolor="#F0F0FF">
<table border="0" cellpadding="10" cellspacing="0" width="100%">
    <tr>
      <td valign="top"><img alt="Logo" src="logo.gif" align=left>
    <FONT face="Helvetica, Arial, sans-serif" size="+3"><B>Attic Manager&#8482;</B></font><br>
    <font face="Helvetica, Arial, sans-serif">HOME INVENTORY SOFTWARE</font>
      </td>
      <TD><font color="#F0F0FF"><H1>Home Inventory Software</H1></font></td>
    </tr>
</table>
<br>

<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td valign="top">
    <div class="menu">
<?
    $pages = array('Home', 'Features', 'Download', 'Manual', 'Shop', 'Support', 'Links');
    foreach ($pages as $p)
    {?>
        <div><a class="menuitem" href="index.php?page=<? echo $p; ?>"><? echo $p; ?></a></div><?
    }?>
    </div>

    </td>
    <td valign="top">

<?
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
        <center>Copyright &copy; 2007 GuacoSoft.com</center>

    </td>
  </tr>
  <tr>
  </tr>
</table>
<br>
</body>
</html>
