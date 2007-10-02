<HTML>
<HEAD>
    <TITLE>Moj broj - Slagalica</TITLE>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
    <meta name="description" content="A number calculating puzzle game like game Moj Broj (my number) from TV quiz show Slagalica">
    <meta name="keywords" content="moj, broj, slagalica, kviz, quiz, free, game, puzzle, calculate, numbers, operations, le, compte, est, bon">
    <link rel="shortcut icon" href="/favicon.ico" />
</HEAD>
<STYLE>
.tekst
{
    color: #FFFF66; font-family: verdana,sans-serif; font-size: 14px
}

.blok
{
    float: left;
    background-color: #0000ff;
    padding: 15px 15px 15px 15px;
    font-size: 24px; font-family: verdana,sans-serif;
    color: #FFFFaa;
    text-align: center;
    width: 60px;
    border: 2px solid #6666ff;
}

.long
{
    width: 475px;
}

.container
{
    width: 765px;
    padding: 5px;
    margin: 0 auto;
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

.clickable
{
    cursor: pointer
}

.title
{
    padding: 15px 15px 10px 10px;
    font-size: 32px; font-family: verdana,sans-serif;
    color: #FFFF66;
}

div.footer {
    text-align: center;
    font-family: verdana,sans-serif;
    color: #FFFF99;
    padding: 15px 0;
    font-size: 11px;
    line-height: 1.4em;
    margin-top: 25px;
    border-top: 1px solid #6666AA;
}

div.header {
    text-align: center;
    font-family: verdana,sans-serif;
    color: #FFFF99;
    padding: 15px 0;
    font-size: 14px;
    line-height: 1.4em;
    margin-bottom: 25px;
    border-bottom: 1px solid #6666AA;
}

</STYLE>
<?
if (isset($play))
{
    include('compte.php');
    do
    {
        $target = rand(200, 999);
        $numbers = array(
            rand(1, 9),
            rand(1, 9),
            rand(1, 9),
            rand(1, 9),
            5 * rand(2, 4),
            25 * rand(1, 4));
        $steps = '';
        $rezult = getResult($numbers, $target, $steps);
    }
    while ($rezult != $target); // only solvable ones
}
?>
<SCRIPT>
var tokenList = new Array();
var tokenPos  = new Array();
var pokusaja = <?
    if (empty($_GET['play']))
        echo 0;
    else
    {
        echo (4 - (int)$_GET['play']);
    }
    ?>;

// obisi poslednji element
function brisanje()
{
    var sz = tokenList.length;
    if (sz == 0)
        return;
    var x = tokenPos[sz - 1];
    if (x > -1)
    {
        if (document.all)
        {
            var ename = 'number'+x;
            elem = document.all[ename];
        }
        else
            elem = document.getElementById("number" + x);
        elem.style.color = '#FFFFaa';
    }
    tokenList.length = sz - 1;
    tokenPos.length = sz - 1;
    napraviFormulu();
}

function napraviFormulu()
{
    if (document.all)
        frml = document.all['formula'];
    else
        frml = document.getElementById("formula");

    var f = '';
    for (i = 0; i < tokenList.length; i++)
    {
        f += tokenList[i];
    }
    if (f == '')
        frml.innerHTML = '&nbsp;';
    else
        frml.innerHTML = f;
}

function dodaj(elem, remove, pos)
{
    if (elem.style.color == 'black')
    {
        alert('You already used that number');
        return;
    }
    if (pos > -1 && tokenPos.length > 0 && tokenPos[tokenPos.length - 1] > -1)
    {
        alert('You must put some operation between numbers');
        return;
    }
    tokenList.length = tokenList.length + 1;
    tokenList[tokenList.length - 1] = elem.innerHTML;
    tokenPos.length = tokenList.length;
    tokenPos[tokenPos.length - 1] = pos;
    napraviFormulu();
    if (remove)
        elem.style.color = 'black';
}

function provera()
{
    if (document.all)
        frml = document.all['formula'];
    else
        frml = document.getElementById("formula");

    if (formula == "")
    {
        alert('Nothing to calculate');
        return;
    }

    try
    {
        var result = eval(frml.innerHTML);
    }
    catch(e)
    {
        alert('Invalid expression!');
        return;
    }

    if (result != <? echo $target; ?>)
    {
        alert('Result is '+result);
        pokusaja--;
        if (pokusaja < 1)
        {
            zaustaviSat();
            alert('This was your last attempt. You failed.\n\nSolution:\n'
                + '<? echo str_replace("\n", '\n', $steps); ?>');
            window.location.reload();
        }
    }
    else
    {
        alert('Well done.');
        window.location.reload();
    }
}


var sat = 0;
var sekundi = 91;
function pomeriSat()
{
    zaustaviSat();
    sekundi--;
    if (document.all)
        elem = document.all.tajmer;
    else
        elem = document.getElementById('tajmer');
    elem.innerHTML = sekundi;
    if (sekundi <= 0)
    {
        alert('The time has ran out. You failed.\n\nSolution:\n'
            + '<? echo str_replace("\n", '\n', $steps); ?>');
        window.location.reload();
    }
    else
        pokreniSat();
}

function pokreniSat()
{
    sat = setTimeout("pomeriSat()", 1000);
}

function zaustaviSat()
{
    if (sat)
    {
        clearTimeout(sat);
        sat  = 0;
    }
}
</SCRIPT>

<BODY BGCOLOR="#3333cc" style="margin: 0; padding: 0;"<?
    if (isset($play) && $play == 3)
        echo ' onLoad="pokreniSat()" onUnload="zaustaviSat()"';
?>>
<div class="header spacer" style="width: 750px;">
    <div style="float: left; text-align: left; padding: 5px;">
        <h1>Moj broj</h1>
        as seen on TV show Slagalica
    </div>
    <div style="float: right">
        <a style="color: white" href="../index.php">Home</a> | <a style="color: white"
        href="../index.php?page=Products">Products</a> | <a style="color: white"
        href="../index.php?page=Shop">Shop</a> | <a style="color: white"
        href="../index.php?page=About">About us</a>
    </div>
</div>

<div class="spacer" style="float: left; margin-left:10px;">
<script type="text/javascript"><!--
google_ad_client = "pub-2072430600789119";
google_ad_width = 120;
google_ad_height = 600;
google_ad_format = "120x600_as";
google_ad_type = "text";
//2007-06-21: Slagalica
google_ad_channel = "2764297059";
//-->
</script>
<script type="text/javascript"
 src="http://pagead2.googlesyndication.com/pagead/show_ads.js">
</script>
</div>';


<?
if (isset($play))
{
?>
    <br>
<div class="container spacer" style="float: left">
    <div class="container spacer">
        <div class="title" style="float: left">Target number:</div>
        <div style="width: 10px; float:left">&nbsp;</div>
        <div class="blok"><? echo $target; ?></div>
        <?
        if ($play == 3) { ?>
        <div class="blok" style="float: right" id="tajmer">90</div>
        <div class="title" style="float: right">Time:</div>
        <?
        } ?>
    </div>
    <div class="container spacer">
        <div class="title">Available numbers to use:</div>
    </div>
    <div class="container spacer">
    <?
        for ($n=0; $n < 6; $n++)
        {?>
            <div class="blok clickable" id="number<? echo $n; ?>" onClick="dodaj(this, true, <? echo $n; ?>);">
            <? echo $numbers[$n]; ?>
            </div> <div style="width: 10px; float:left">&nbsp;</div><?
        }
    ?>
    </div>
    <div class="container spacer">
        <div class="title">Available operations:</div>
    </div>
    <div class="container spacer">
    <?
        $ops = array('+', '-', '*', '/', '(', ')');
        foreach ($ops as $o)
        {?>
            <div class="blok clickable" onClick="dodaj(this, false, -1);">
            <? echo $o; ?>
            </div> <div style="width: 10px; float:left">&nbsp;</div><?
        }
    ?>
    </div>
    <div class="container spacer">
        <div class="title">Your formula:</div>
    </div>
    <div class="container spacer">
            <div class="blok long" id="formula" style="float: left">&nbsp;</div>
            <div style="width: 10px; float:left">&nbsp;</div>
            <div class="blok clickable" id="del" style="float: left" onClick="brisanje();">DEL</div>
            <div style="width: 10px; float:left">&nbsp;</div>
            <div class="blok clickable" id="del" style="float: left" onClick="provera();">DONE</div>
    </div>
    <div class="container spacer">
    <p class="tekst">
        Click on numbers and operations to create the formula.<br>
        Click <a href="#" onClick="window.location.reload(); return false;" style="color: white">here</a> to give up and pick another number.<br>
    </p>
    <p class="tekst">
        The game is
        written in pure DHTML, you don't need to install anything (no Java, no Flash, nothing).
    </p>
    <p class="tekst">
        Copyright
        &copy; 2007. Milan Babuskov (<a href="mailto:mbabuskov@yahoo.com" style="color: white">e-mail</a>)
    </p>
    </div>
</div>
    <?
}   // end: if isset($play)
else
{?>
    <div class="container spacer" style="float: left; width: 600px">
    <p class="tekst">
    <B>Moj broj</B> (my number) is one of the most popular games in top-rated TV quiz show <B>Slagalica</B>.
    The goal of the game is to to combine 6 numbers and basic mathematical operations (add,
    subtract, multiply, divide) to get the target number. Four of those numbers are single
    digit, one is either of 10, 15 or 20 and sixth is either of 25, 50, 75 and 100. The
    number of operations is not limited, but each number can be used only once. The game is
    written in pure DHTML, you don't need to install anything (no Java, no Flash, nothing).
    Have fun playing...</P>
    <br>
    <form action="mojbroj.php" method="GET">
    <P class="tekst"><B>Select difficulty level:</B></P>
    <label class="tekst"><input type="radio" name="play" value="1" checked>Beginner - unlimited time and 3 attempts allowed</label><br>
    <label class="tekst"><input type="radio" name="play" value="2">Advanced - unlimited time and 2 attempts</label><br>
    <label class="tekst"><input type="radio" name="play" value="3">Pro - time limited to 90 seconds and only 1 attempt</label><br>
    <P class="tekst">In the TV show, the game is played at <B>Pro</B> setting.</P><br>
    <input type=submit value="Start the game">
    </FORM>
    </div>
<?
}
?>

</BODY>
</HTML>
