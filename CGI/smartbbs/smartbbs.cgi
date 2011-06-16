#!/usr/bin/perl

#smart bbs

#厳密なCheck
use strict;
use warnings;
require "jcode.pl";

#メイン関数
&main;
exit;

#===============================#
#	メイン関数		#
#===============================#
sub main()
{
	#最大表示件数
	my $max_view = 10;
	#掲示板の作者のメールアドレス
	my $creater_addr = 'kimoto@example.com';
	#このCGIの名前
	my $cgi_name = "smartbbs.cgi";
	#著作権
	my $copyright = "2004 kimoto";
	#BBSの名前
	my $bbs_title = "Black Earth BBS";
	#この掲示板から見たトップページ
	my $top_page = "../";
	#デザインが記述してあるCSSファイルのパス
	my $css_dir = "/css/layout.css";
	#ログファイルの名前
	my $log_file = "./smartbbs.txt";
	
	my $newst_num;
	my %post;
	my $session;

	#POSTされたデータを取得,デコードする
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

	#POSTされたデータからセッションを判別し、
	#それぞれの処理を行う,何もないときが普通に表示する処理になる。
	$session = $post{'session'} || '';
	if($session eq 'write'){
		#名前または本文がなかった時は書き込めませんよ
		if($post{'name'} eq '' or $post{'message'} eq ''){
			print "Content-Type: text/html;\n\n";
			show_html_header($creater_addr,$top_page,
				$css_dir,$bbs_title);
			show_html_message("Error","名前または本文を入力してください。",$cgi_name);
			show_html_close($copyright);
			return;
		}else{
			#クッキーに書き込むか、書き込まないか
			if($post{'cookie'} eq 'save'){
				#書き込む時間だけこの処理
				set_cookie('DATA',$post{'name'},$post{'mail'});
			}
			write_log($log_file,%post);
			print "Content-Type: text/html;\n\n";
			show_html_header($creater_addr,$top_page,
				$css_dir,$bbs_title);
			show_html_message("Success","書き込みが成功しました。",$cgi_name);
			show_html_close($copyright);
			return;
		}
	}elsif($session eq 'move'){
		
		#かなりこの辺冗長
		print "Content-Type: text/html;\n\n";
		show_html_header($creater_addr,
			$top_page,$css_dir,$bbs_title);
		show_post_message_form($cgi_name,(get_cookie()));

		$newst_num = log_get_newst_num($log_file);
		my $page = $post{'page'};

		#前読み
		if($page - 1 == 0){
			show_log_num($log_file,$newst_num,$max_view);
			show_next_prev_button($cgi_name,"next",-1,2);
		}else{	
			my $start_num = $newst_num - ($page - 1) * $max_view;
			show_log_num($log_file,$start_num,$max_view);

			#先読み	
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
	#HTMLを出力
	show_html_header($creater_addr,
		$top_page,$css_dir,$bbs_title);
	show_post_message_form($cgi_name,(get_cookie()));

	#指定した範囲のログを表示する
	$newst_num = log_get_newst_num($log_file);
	show_log_num($log_file,$newst_num,$max_view);
	if($newst_num <= $max_view){
		show_next_prev_button($cgi_name,"",0,2);
	}else{
		show_next_prev_button($cgi_name,"next",0,2);
	}

	#著作権情報と、終了タグ
	show_html_close($copyright);

	print %post;
	return;
}

#===============================#
#	HTMLヘッダを表示	#
#===============================#
#$creater_mail_addr - 作者のメールアドレス
#$index_dir - そのサイトのトップページ
#$css_dir - デザインを記述したCSSファイルの在処
#$title - この掲示板のタイトル,表題
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
	<a href="$index_dir">トップに戻る</a>
</p>
HTML_HEADER
;

	return;
}

#=======================================#
#	なんかユーザーに伝えたいとき	#
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
	<h2><a href="$bbs_page">掲示板へ戻る</a></h2>
</div>
HTML_MSG
;
	return;
}

#===============================#
#	HTML終了部分を出力	#
#===============================#
#$copyright - 著作権保持者、つまり作者名
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
#	投稿用フォームを出力		#
#=======================================#
#$cgi_name -	このCGIのパス
#		相対パスでもフルパスでも可
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
#	Next,Prevボタンを出力する	#
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
#	指定したデータログを		#
#	整形して出力する		#
#=======================================#
# 第1引数-$title=タイトル
# 第2引数-$no	=番号
# 第3引数-$name	=名前
# 第4引数-$mail =アドレス
# 第5引数-$time =時間
# 第6引数-$msg  =本文
sub show_log($$$$$$)
{
	my $title	= shift @_;
	my $no		= shift @_;
	my $name 	= shift @_;
	my $mail	= shift @_;
	my $time	= shift @_;
	my $msg		= shift @_;

	#メールアドレスが空だったときと
	#そうでなかったときの処理
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
	<h3>[$no] 投稿者 : $name_and_mail 投稿日 : $time</h3>
	<p>$msg</p>
</div>
EOT
;
	return;
}

#=======================================#
#	ログに書き込む関数		#
#=======================================#
#第1引数-$title-タイトル
#第2引数-$no-番号
#第3引数-$name-名前
#第4引数-$mail-メールアドレス
#第5引数-$time-時間
#第6引数-$msg-メッセージ
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

	#ログファイルをロックする
	file_lock($lock_file,0700,5);

	#いままでのすべてのログを取得する。
	open(IN,"$log_file") or die("file open error");
	my @old_log = <IN>;
	close(IN);

	#今回のを追加して保存する
	open(OUT,">$log_file") or die("file open error");
	print OUT "$title,$no,$name,$mail,$time,$msg,\n";
	print OUT @old_log;
	close(OUT);

	#ロックを解除する
	file_unlock($lock_file);

	return;
}

#=======================================#
#	ログを読み込む			#
#=======================================#
sub log_read($)
{
	my $log_file = shift;
	my @log;
	my $i = 0;

	open(IN,$log_file) or die("file open error");

	#１行ずつ読み込む
	@log = <IN>;

	close(IN);

	return @log;
}

#=======================================#
#	POSTされたデータを取得		#
#=======================================#
sub get_post()
{
	my $post;
	my %list;

	my $length = $ENV{'CONTENT_LENGTH'} || 0;
	read(STDIN,$post,$length);

	#POSTされたデータを取得
	foreach my $temp (split(/&/,$post)){
		my($key,$value) = split(/=/,$temp);

		#データを格納
		$list{$key} = $value;
	}

	#最後に、取得したデータを返す
	return %list;
}

#=======================================#
#	エラーメッセージを表示する	#
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
#	きれいに整形してログに書き込む		#
#===============================================#
sub write_log($%)
{
	my $log_file = shift @_;

	my($title,$no,$name,$mail,$time,$msg);
	my %post = @_;

	#いろんな情報の整理
	$title = $post{'title'};

	$no = log_get_newst_num($log_file);
	$no++;

	$name = $post{'name'};
	$mail = $post{'mail'};
	$time = get_now_time();
	$msg = $post{'message'};

	#ログに書き込む
	log_write($log_file,$title,$no,$name,$mail,$time,$msg);

	return;
}

#===============================================#
#	現在の時間を整形したものを返す		#
#===============================================#
sub get_now_time()
{
	my($sec,$min,$hour,$day,$mon,$year,$wday) =
		localtime(time());

	$mon += 1;
	$year += 1900;

	my @youbi = qw(日 月 火 水 木 金 土);

	my $result =  "$year/$mon/$day($youbi[$wday]) $hour:$min";

	return $result;
}

#=======================================#
#	最新のログ番号を取得する	#
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
#	指定した範囲のログを表示する	#
#=======================================#
sub show_log_area($$$)
{
	my $log_file = shift;
	my $min_num = shift @_;
	my $max_num = shift @_;

	my @log_list = log_read($log_file);

	foreach my $log (@log_list){
		my($title,$no,$name,$mail,$time,$msg) = split(/,/,$log);

		#指定した範囲の中のログだった場合
		if($min_num <= $no && $no <= $max_num){
			show_log($title,$no,$name,$mail,$time,$msg);
		}
	}
	return;
}

#=======================================#
#	指定した番号から指定した数だけ	#
#	ログを表示する関数		#
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
#	Cookieを設定する関数		#
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
#	Cookieの情報を取得する		#
#=======================================#
sub write_cookie($$)
{
	#引数の取得
	my $name = shift @_;
	my $value = shift @_;

	my $now_time = localtime(time());
	my @time = split(/ /,$now_time);

	#分解して、構文のように並び替える
	#破棄されるのは、十年後にする
	$time[4] += 10;

	my $gmt = sprintf("%s, %d-%s-%d %s GMT",
			$time[0],$time[2],$time[1],$time[4],$time[3]);

	print "Set-Cookie: $name=$value; expires=$gmt;\n";

	return;
}

#=======================================#
#	クッキーを取得する		#
#=======================================#
sub get_cookie()
{
	#クッキーを取得
	my $cookie = $ENV{'HTTP_COOKIE'} || return;

	my %cookie_list;
	my @cook;

	#データを全て取り出す
	foreach my $temp (split(/;/,$cookie)){
		my ($key,$value) = split(/=/,$temp);
		$key =~ s/\s//g;
		$cookie_list{$key} = $value;
	}

	#そのデータから必要なものだけを取り出す
	foreach my $data (split(/&/,$cookie_list{'DATA'})){
		#URLデコード
		$data =~ s/%([0-9a-fA-F]{2})/pack("C",hex($1))/eg;

		push(@cook,$data);
	}
	return @cook;
}

#=======================================#
#	ファイルロックする関数		#
#=======================================#
sub file_lock()
{
	my $lock_file = shift @_;
	my $access = shift @_;
	my $retry = shift @_;

	#ロック処理、mkdirを使用して汎用的なロック
	while(!mkdir($lock_file,$access)){
		if($retry-- < 0){
			error("CGIが処理中です。しばらくお待ち下さい。");
		}
		sleep(1);
	}

	return;
}

#===============================#
#	ロック解除		#
#===============================#
sub file_unlock()
{
	my $lock_file = shift @_;

	#ロックの解除、フォルダの削除になる。
	rmdir($lock_file);

	return;
}
