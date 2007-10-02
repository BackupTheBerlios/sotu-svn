<?
require_once('class.phpmailer.php');

$validation = 'none';
if (isset($_POST['ime']))    // send message
{
    require_once('recaptchalib.php');
    $privatekey = "6LeAQgAAAAAAAFspmacUj8qEgkhJ2sBODFf5OV3-";
    $resp = recaptcha_check_answer ($privatekey,
                                    $_SERVER["REMOTE_ADDR"],
                                    $_POST["recaptcha_challenge_field"],
                                    $_POST["recaptcha_response_field"]);
    if (!$resp->is_valid)
        $validation = $resp->error;
    else
    {
        $validation = 'ok';
        // send e-mail...
        $mail = new PHPMailer();
        $mail->IsSMTP();
        $mail->Port = 2525;
        $mail->Host = "mail.guacosoft.com";
        $mail->From = "office@guacosoft.com";
        $mail->FromName = "Website feedback";
        $mail->AddReplyTo($mejl);
        $mail->WordWrap = 72;
        $mail->IsHTML(false);
        $mail->CharSet = 'windows-1252';
        $mail->AddAddress('office@guacosoft.com', 'Office');
        $mail->Subject = 'Message from '.$ime;
        $mail->Body = $pitanje;
        $mail->Username = 'office@guacosoft.com';
        $mail->Password = 'nalimsoft';
        if (!$mail->Send())
            $validation = $mail->ErrorInfo;
    }
}
?>
<table cellspacing="0" cellpadding="20" border="0">
<TR bgcolor="white"><TD width="550" align="left">

<B>E-mail</B>

<P>If you have any questions, problems or suggestions, please e-mail us
at: <font color=blue>office&#064;guacosoft.com</font>.
Make sure you write in the version of Attic Manager you're using and full version of operating
system on which you run it, for example:</P>

<UL>
    <LI>Windows XP Professional Edition - Service Pack 2</LI>
    <LI>Ubuntu Linux 6.10</LI>
</UL>


<br>
<?
if ($validation == 'ok')
    echo '<B>Your message has been sent.</B>';
else
{   ?>
<B>Direct contact</B><br><br>
<FORM action="index.php?page=Support" method="post">
<table cellpadding="5" cellspacing="0" border="0" bgcolor="#EEEEEE">
<TR>
    <TD>Your name:</TD>
    <TD><INPUT type=text name=ime size="30" maxlength="255" value="<?
        if (isset($ime) && $validation != 'ok')
            echo htmlspecialchars($ime); ?>"></TD>
</TR>
<TR>
    <TD>E-mail address:</TD>
    <TD><INPUT type="text" name="mejl" size="30" maxlength="255" value="<?
        if (isset($mejl) && $validation != 'ok')
            echo htmlspecialchars($mejl); ?>"></TD>
</TR>
<TR>
    <TD></TD>
    <TD></TD>
</TR>
<TR>
    <TD valign="top">Your question:</TD>
    <TD><TEXTAREA name=pitanje cols=48 rows=7><?
        if (isset($pitanje) && $validation != 'ok')
            echo htmlspecialchars($pitanje); ?></TEXTAREA></TD>
</TR>
<TR>
    <TD valign="top">Anti-spam test:</TD>
    <TD>
    <?
        require_once('recaptchalib.php');
        $publickey = "6LeAQgAAAAAAAPwe2nLih4aDg83RRQRx-QG1LpQs";
        echo recaptcha_get_html($publickey);
    ?>
    </TD>
</TR>
<TR>
    <TD><?
        if ($validation != 'none')
            echo "<B>Error: $validation</B>";
    ?></TD>
    <TD align="right">
    <input type="submit" name="submit" value="Send message">
    </TD>
</TR>
</TABLE>
</FORM>

<P>Please make sure you leave the correct e-mail address, otherwise we won't be able to contact you back.</P>
<?
}
?>
</TD></TR>
</table>