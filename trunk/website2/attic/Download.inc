<table cellspacing="1" cellpadding="20" border="0" bgcolor="black">
<TR bgcolor="white"><TD width="550">

<B>Download</B>

<P>Attic Manager is available for various operating systems. The database file itself is 
platform-independent, so you can load databases created on Linux into Windows copy of Attic 
Manager and vice versa (simply by using backup/restore options).</P>

<center>
<table border="0" cellpadding="20" cellspacing="0">
	<TR>
		<TD align="center" nowrap><a href="attic-1.0-setup.exe"><IMG 
			src="windows_logo.jpg" alt="Windows" border="0"><br>Download for Windows</a><br>3.4MB</TD>
		<TD align="center" nowrap><a href="attic-1.0-i386.tar.gz"><IMG 
			src="linux_logo.jpg"   alt="Linux"   border="0"><br>Download for Linux</a><br>4.4MB</TD>
	</TR>
</table>
</center>

<br>
<B>System requirements for Windows version</B>

<UL>
	<LI>233MHz processor</LI>
	<LI>64 MB of RAM</LI>
	<LI>50 MB of hard disk space</LI>
	<LI>Windows 2000 or Windows XP</LI>
</UL>

<br>
<B>System requirements for Linux version</B>

<UL>
	<LI>233MHz processor</LI>
	<LI>64 MB of RAM</LI>
	<LI>50 MB of hard disk space</LI>
	<LI>glibc 2.3.2, gtk+2.0, fontconfig/xft, libstdc++6</LI>
</UL>

<br>
<B>Minimal versions of popular Linux distributions are:</B>

<UL>
	<LI>Ubuntu 5.10, Debian Etch, Mepis 3.4-3</LI>
	<LI>OpenSUSE 10.0, Fedora Core 4, CentOS 4.4, Mandriva 10.1</LI>
	<LI>Slackware 11.0, Gentoo 2005.0</LI>
</UL>

<style>
PRE { background-color: #dddddd }
</style>

<br><br>
<IMG src="linux_logor.jpg" alt="Linux" border="0"><br>
<B>Linux notes</B>

<P>The provided .tar.gz file is a simple zipped package. Unpack it to some directory, 
and run the <B>run-attic.sh</B> script. If you do it from shell, it would go something like this:</P>

<PRE>
tar -xzf attic-1.0-i386.tar.gz
cd attic-1.0-i386
./run-attic.sh
</PRE>

<P>You don't need to install it, and <u>you don't need to be root</u>. If you are
not root, you might see the following message when starting program:</P>

<PRE>
lock manager: couldn't set uid to superuser
</PRE>

<P>This is just a harmless warning message printed by program's database engine. You can safely ignore it.</P>

</TD></TR>
</table>