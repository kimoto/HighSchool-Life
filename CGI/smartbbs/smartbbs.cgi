#!/usr/bin/perl

#smart bbs

#��̩��Check
use strict;
use warnings;
require "jcode.pl";

#�ᥤ��ؿ�
&main;
exit;

#===============================#
#	�ᥤ��ؿ�		#
#===============================#
sub main()
{
	#����ɽ�����
	my $max_view = 10;
	#�Ǽ��Ĥκ�ԤΥ᡼�륢�ɥ쥹
	my $creater_addr = 'kimoto@example.com';
	#����CGI��̾��
	my $cgi_name = "smartbbs.cgi";
	#���
	my $copyright = "2004 kimoto";
	#BBS��̾��
	my $bbs_title = "Black Earth BBS";
	#���ηǼ��Ĥ��鸫���ȥåץڡ���
	my $top_page = "../";
	#�ǥ����󤬵��Ҥ��Ƥ���CSS�ե�����Υѥ�
	my $css_dir = "/css/layout.css";
	#���ե������̾��
	my $log_file = "./smartbbs.txt";
	
	my $newst_num;
	my %post;
	my $session;

	#POST���줿�ǡ��������,�ǥ����ɤ���
	%post = get_post();
	foreach my $temp (%post){
		$temp =~ tr/+/ /;
		$temp =~ s/%([0-9a-fA-F]{2})/pack("C",hex($1))/eg;
		jcode::convert(\$temp,'euc');
		$temp =~ s/&/&amp;/g;
		$temp =~ s/</&lt;/g;
		$temp =~ s/>/&gt;/g;
		$temp =~ s/"/&quot;/g;
		$temp =~ s/'/&#39;/g;
		$temp =~ s/,/&#44;/g;
		$temp =~ s/\n|\r\n|\r|\r\r\n/<br>/g;
	}

	#POST���줿�ǡ������饻�å�����Ƚ�̤���
	#���줾��ν�����Ԥ�,����ʤ��Ȥ������̤�ɽ����������ˤʤ롣
	$session = $post{'session'} || '';
	if($session eq 'write'){
		#̾���ޤ�����ʸ���ʤ��ä����Ͻ񤭹���ޤ����
		if($post{'name'} eq '' or $post{'message'} eq ''){
			print "Content-Type: text/html;\n\n";
			show_html_header($creater_addr,$top_page,
				$css_dir,$bbs_title);
			show_html_message("Error","̾���ޤ�����ʸ�����Ϥ��Ƥ���������",$cgi_name);
			show_html_close($copyright);
			return;
		}else{
			#���å����˽񤭹��फ���񤭹��ޤʤ���
			if($post{'cookie'} eq 'save'){
				#�񤭹�����֤������ν���
				set_cookie('DATA',$post{'name'},$post{'mail'});
			}
			write_log($log_file,%post);
			print "Content-Type: text/html;\n\n";
			show_html_header($creater_addr,$top_page,
				$css_dir,$bbs_title);
			show_html_message("Success","�񤭹��ߤ��������ޤ�����",$cgi_name);
			show_html_close($copyright);
			return;
		}
	}elsif($session eq 'move'){
		
		#���ʤꤳ���վ�Ĺ
		print "Content-Type: text/html;\n\n";
		show_html_header($creater_addr,
			$top_page,$css_dir,$bbs_title);
		show_post_message_form($cgi_name,(get_cookie()));

		$newst_num = log_get_newst_num($log_file);
		my $page = $post{'page'};

		#���ɤ�
		if($page - 1 == 0){
			show_log_num($log_file,$newst_num,$max_view);
			show_next_prev_button($cgi_name,"next",-1,2);
		}else{	
			my $start_num = $newst_num - ($page - 1) * $max_view;
			show_log_num($log_file,$start_num,$max_view);

			#���ɤ�	
			if(($page) * $max_view >= $newst_num){
				show_next_prev_button($cgi_name,"prev",$page - 1);
			}else{
				show_next_prev_button($cgi_name,"nextprev",$page - 1,$page + 1);
			}
		}

		show_html_close($copyright);
		return;
	}

	print "Content-Type: text/html;\n\n";
	#HTML�����
	show_html_header($creater_addr,
		$top_page,$css_dir,$bbs_title);
	show_post_message_form($cgi_name,(get_cookie()));

	#���ꤷ���ϰϤΥ���ɽ������
	$newst_num = log_get_newst_num($log_file);
	show_log_num($log_file,$newst_num,$max_view);
	if($newst_num <= $max_view){
		show_next_prev_button($cgi_name,"",0,2);
	}else{
		show_next_prev_button($cgi_name,"next",0,2);
	}

	#�������ȡ���λ����
	show_html_close($copyright);

	print %post;
	return;
}

#===============================#
#	HTML�إå���ɽ��	#
#===============================#
#$creater_mail_addr - ��ԤΥ᡼�륢�ɥ쥹
#$index_dir - ���Υ����ȤΥȥåץڡ���
#$css_dir - �ǥ�����򵭽Ҥ���CSS�ե�����κ߽�
#$title - ���ηǼ��ĤΥ����ȥ�,ɽ��
sub show_html_header($$$$)
{
	my $creater_mail_addr	= shift @_;
	my $index_dir		= shift @_;
	my $css_dir		= shift @_;
	my $title		= shift @_;

	print <<HTML_HEADER
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
	"http://www.w3.org/TR/html4/strict.dtd">
<html lang="ja">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=EUC-JP">
<meta http-equiv="Content-Style-Type" content="text/css">
<link rev="made" href="mailto:$creater_mail_addr">
<link rel="index" href="$index_dir">
<link rel="stylesheet" href="$css_dir" type="text/css">
<title>$title</title>
</head>

<body>
<h1>$title</h1>

<p>
	<a href="$index_dir">�ȥåפ����</a>
</p>
HTML_HEADER
;

	return;
}

#=======================================#
#	�ʤ󤫥桼���������������Ȥ�	#
#=======================================#
sub show_html_message($$$)
{
	my $title = shift @_;
	my $msg = shift @_;
	my $bbs_page = shift @_;

	print <<HTML_MSG

<!-- Message -->
<div class="message">
	<h2>$title</h2>
	<p>$msg</p>
</div>

<!-- Return TopPage -->
<div class="message">
	<h2><a href="$bbs_page">�Ǽ��Ĥ����</a></h2>
</div>
HTML_MSG
;
	return;
}

#===============================#
#	HTML��λ��ʬ�����	#
#===============================#
#$copyright - ����ݻ��ԡ��Ĥޤ���̾
sub show_html_close($)
{
	my $copyright = shift @_;

	print <<HTML_CLOSE

<!-- CopyRight -->
<div class="copyright">
CopyRight &copy; $copyright All Rights Reserved.
</div>

</body>
</html>
HTML_CLOSE
;
	return;
}

#=======================================#
#	����ѥե���������		#
#=======================================#
#$cgi_name -	����CGI�Υѥ�
#		���Хѥ��Ǥ�ե�ѥ��Ǥ��
sub show_post_message_form($$$)
{
	my $cgi_path = shift @_ || $ENV{'SCRIPT_NAME'};
	my $name = shift @_ || '';
	my $mail = shift @_ || '';

	print <<"POST_FORM";
<!-- Post a Message Form -->
<form method="post" action="$cgi_path">
<div><input type="hidden" name="session" value="write"></div>
	<table class="post" summary="post_form">
		<tr>
			<td>Name</td>
			<td><input type="text" name="name" size="20" value="$name" maxlength="100" tabindex="1" accesskey="n"></td>
		</tr>
		<tr>
			<td>Mail</td>
			<td><input type="text" name="mail" size="40" value="$mail" maxlength="100" tabindex="2" accesskey="a"></td>
		</tr>
		<tr>
			<td>Title</td>
			<td><input type="text" name="title" size="40" value="" maxlength="100" tabindex="3" accesskey="t"></td>
		</tr>
		<tr>
			<td>Message</td>
			<td><textarea rows="5" cols="40" name="message" tabindex="4" accesskey="m"></textarea></td>
		</tr>
		<tr>
			<td colspan="3">
				<input type="Submit" value="Submit" tabindex="5" accesskey="s">
				<input type="Reset" value="Reset" tabindex="6" accesskey="r">
				<input type="checkbox" name="cookie" value="save" tabindex="7" checked>Save Information
			</td>
		</tr>
	</table>
</form>
POST_FORM
	return;
}

#=======================================#
#	Next,Prev�ܥ������Ϥ���	#
#=======================================#
sub show_next_prev_button($$$$)
{
	my $cgi_name = shift @_;
	my $next_prev_flag = shift @_;
	my $prev_page = shift @_;
	my $next_page = shift @_;

	print "\n";
	print "<!-- Next Prev Button -->\n";
	print "<table summary=\"next_prev\">\n";
	print "\t<tr>\n";

	if($next_prev_flag =~ /prev/){
		print "\t\t<td>\n";
		print "\t\t\t<form method=\"post\" action=\"$cgi_name\">\n";
		print "\t\t\t\t<div>\n";
		print "\t\t\t\t\t<input type=\"submit\" name=\"prev_button\" value=\"Prev\">\n";
		print "\t\t\t\t\t<input type=\"hidden\" name=\"page\" value=\"$prev_page\">\n";
		print "\t\t\t\t\t<input type=\"hidden\" name=\"session\" value=\"move\">\n";
		print "\t\t\t\t</div>\n";
		print "\t\t\t</form>\n";
		print "\t\t</td>\n";
	}

	if($next_prev_flag =~ /next/){
		print "\t\t<td>\n";
		print "\t\t\t<form method=\"post\" action=\"$cgi_name\">\n";
		print "\t\t\t\t<div>\n";
		print "\t\t\t\t\t<input type=\"submit\" name=\"next_button\" value=\"Next\">\n";
		print "\t\t\t\t\t<input type=\"hidden\" name=\"page\" value=\"$next_page\">\n";
		print "\t\t\t\t\t<input type=\"hidden\" name=\"session\" value=\"move\">\n";
		print "\t\t\t\t</div>\n";
		print "\t\t\t</form>\n";
		print "\t\t</td>\n";
	}

	print "\t</tr>\n";
	print "</table>\n";		
	return;
}

#=======================================#
#	���ꤷ���ǡ�������		#
#	�������ƽ��Ϥ���		#
#=======================================#
# ��1����-$title=�����ȥ�
# ��2����-$no	=�ֹ�
# ��3����-$name	=̾��
# ��4����-$mail =���ɥ쥹
# ��5����-$time =����
# ��6����-$msg  =��ʸ
sub show_log($$$$$$)
{
	my $title	= shift @_;
	my $no		= shift @_;
	my $name 	= shift @_;
	my $mail	= shift @_;
	my $time	= shift @_;
	my $msg		= shift @_;

	#�᡼�륢�ɥ쥹�������ä��Ȥ���
	#�����Ǥʤ��ä��Ȥ��ν���
	my $name_and_mail;
	if($mail eq ''){
		$name_and_mail = $name;
	}else{
		$name_and_mail = "<a href=\"mailto:$mail\">$name</a>";
	}

	if($title ne ''){
		$title = "\n<h2>$title</h2>\n";
	}

	print <<EOT;

<!-- Log Message Number[$no]-->
<div class="message">$title
	<h3>[$no] ��Ƽ� : $name_and_mail ����� : $time</h3>
	<p>$msg</p>
</div>
EOT
;
	return;
}

#=======================================#
#	���˽񤭹���ؿ�		#
#=======================================#
#��1����-$title-�����ȥ�
#��2����-$no-�ֹ�
#��3����-$name-̾��
#��4����-$mail-�᡼�륢�ɥ쥹
#��5����-$time-����
#��6����-$msg-��å�����
sub log_write($$$$$$$)
{
	my $log_file = shift @_;

	my $title	= shift @_;
	my $no		= shift @_;
	my $name	= shift @_;
	my $mail	= shift @_;
	my $time	= shift @_;
	my $msg		= shift @_;

	my $lock_file = "lock";

	#���ե�������å�����
	file_lock($lock_file,0700,5);

	#���ޤޤǤΤ��٤ƤΥ���������롣
	open(IN,"$log_file") or die("file open error");
	my @old_log = <IN>;
	close(IN);

	#����Τ��ɲä�����¸����
	open(OUT,">$log_file") or die("file open error");
	print OUT "$title,$no,$name,$mail,$time,$msg,\n";
	print OUT @old_log;
	close(OUT);

	#��å���������
	file_unlock($lock_file);

	return;
}

#=======================================#
#	�����ɤ߹���			#
#=======================================#
sub log_read($)
{
	my $log_file = shift;
	my @log;
	my $i = 0;

	open(IN,$log_file) or die("file open error");

	#���Ԥ����ɤ߹���
	@log = <IN>;

	close(IN);

	return @log;
}

#=======================================#
#	POST���줿�ǡ��������		#
#=======================================#
sub get_post()
{
	my $post;
	my %list;

	my $length = $ENV{'CONTENT_LENGTH'} || 0;
	read(STDIN,$post,$length);

	#POST���줿�ǡ��������
	foreach my $temp (split(/&/,$post)){
		my($key,$value) = split(/=/,$temp);

		#�ǡ������Ǽ
		$list{$key} = $value;
	}

	#�Ǹ�ˡ����������ǡ������֤�
	return %list;
}

#=======================================#
#	���顼��å�������ɽ������	#
#=======================================#
sub error($)
{
	my $err_msg = shift @_;

	print <<ERROR

<!-- Error Message -->
<h2>Error - $err_msg</h2>
ERROR
;

	exit();
	return;
}

#===============================================#
#	���줤���������ƥ��˽񤭹���		#
#===============================================#
sub write_log($%)
{
	my $log_file = shift @_;

	my($title,$no,$name,$mail,$time,$msg);
	my %post = @_;

	#�����ʾ��������
	$title = $post{'title'};

	$no = log_get_newst_num($log_file);
	$no++;

	$name = $post{'name'};
	$mail = $post{'mail'};
	$time = get_now_time();
	$msg = $post{'message'};

	#���˽񤭹���
	log_write($log_file,$title,$no,$name,$mail,$time,$msg);

	return;
}

#===============================================#
#	���ߤλ��֤�����������Τ��֤�		#
#===============================================#
sub get_now_time()
{
	my($sec,$min,$hour,$day,$mon,$year,$wday) =
		localtime(time());

	$mon += 1;
	$year += 1900;

	my @youbi = qw(�� �� �� �� �� �� ��);

	my $result =  "$year/$mon/$day($youbi[$wday]) $hour:$min";

	return $result;
}

#=======================================#
#	�ǿ��Υ��ֹ���������	#
#=======================================#
sub log_get_newst_num($)
{
	my $log_file = shift;

	open(IN,$log_file);

	my $newst_line = <IN>;
	my($title,$no) = split(/,/,$newst_line);

	close(IN);
	return $no;
}

#=======================================#
#	���ꤷ���ϰϤΥ���ɽ������	#
#=======================================#
sub show_log_area($$$)
{
	my $log_file = shift;
	my $min_num = shift @_;
	my $max_num = shift @_;

	my @log_list = log_read($log_file);

	foreach my $log (@log_list){
		my($title,$no,$name,$mail,$time,$msg) = split(/,/,$log);

		#���ꤷ���ϰϤ���Υ����ä����
		if($min_num <= $no && $no <= $max_num){
			show_log($title,$no,$name,$mail,$time,$msg);
		}
	}
	return;
}

#=======================================#
#	���ꤷ���ֹ椫����ꤷ��������	#
#	����ɽ������ؿ�		#
#=======================================#
sub show_log_num($$$)
{
	my $log_file = shift;
	my $max_num = shift @_;
	my $count = shift @_;

	my $min_num;
	
	$min_num = $max_num - $count;
	$min_num++;

	show_log_area($log_file,$min_num,$max_num);

	return;
}

#=======================================#
#	Cookie�����ꤹ��ؿ�		#
#=======================================#
sub set_cookie($$$)
{
	my $keyword = shift @_;
	my $name = shift @_;
	my $value = shift @_;

	write_cookie($keyword,"$name&$value");
	return;
}

#=======================================#
#	Cookie�ξ�����������		#
#=======================================#
sub write_cookie($$)
{
	#�����μ���
	my $name = shift @_;
	my $value = shift @_;

	my $now_time = localtime(time());
	my @time = split(/ /,$now_time);

	#ʬ�򤷤ơ���ʸ�Τ褦���¤��ؤ���
	#�˴������Τϡ���ǯ��ˤ���
	$time[4] += 10;

	my $gmt = sprintf("%s, %d-%s-%d %s GMT",
			$time[0],$time[2],$time[1],$time[4],$time[3]);

	print "Set-Cookie: $name=$value; expires=$gmt;\n";

	return;
}

#=======================================#
#	���å������������		#
#=======================================#
sub get_cookie()
{
	#���å��������
	my $cookie = $ENV{'HTTP_COOKIE'} || return;

	my %cookie_list;
	my @cook;

	#�ǡ��������Ƽ��Ф�
	foreach my $temp (split(/;/,$cookie)){
		my ($key,$value) = split(/=/,$temp);
		$key =~ s/\s//g;
		$cookie_list{$key} = $value;
	}

	#���Υǡ�������ɬ�פʤ�Τ�������Ф�
	foreach my $data (split(/&/,$cookie_list{'DATA'})){
		#URL�ǥ�����
		$data =~ s/%([0-9a-fA-F]{2})/pack("C",hex($1))/eg;

		push(@cook,$data);
	}
	return @cook;
}

#=======================================#
#	�ե������å�����ؿ�		#
#=======================================#
sub file_lock()
{
	my $lock_file = shift @_;
	my $access = shift @_;
	my $retry = shift @_;

	#��å�������mkdir����Ѥ�������Ū�ʥ�å�
	while(!mkdir($lock_file,$access)){
		if($retry-- < 0){
			error("CGI��������Ǥ������Ф餯���Ԥ���������");
		}
		sleep(1);
	}

	return;
}

#===============================#
#	��å����		#
#===============================#
sub file_unlock()
{
	my $lock_file = shift @_;

	#��å��β�����ե�����κ���ˤʤ롣
	rmdir($lock_file);

	return;
}
