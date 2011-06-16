#!/usr/bin/perl

=info
	Uploader v1.0b

	created by kimoto.
	E-Mail: kimoto@example.com
=cut

$ENV{'TZ'} = 'JST-09';
$ENV{'PATH'} = '/bin/:/usr/bin';

# pragma
use strict;
use warnings;
use Jcode;
use CGI::Carp qw(fatalsToBrowser);

# config
our $Title   = 'Uploader v1.0b';
our $CGIPath = $ENV{'SCRIPT_NAME'} || 'uploader.cgi';
our $SaveDir = 'data/';
our $LogFile = 'log.cgi';
our $MaxView = 10;
our $MaxComments = 20;			# byte
our $CharSet = 'euc-jp';
our $CSSDir = './design.css';
our @Permission = (
	'txt','lzh','zip','rar','gca','mpg',
	'mp3','avi','swf','bmp','jpg','jpeg',
	'gif','png','wmv','rm','ram',
	'c','c++','cxx','h','hxx','asm',
	'pdf',
);

&main;
exit;

# メイン関数
sub main()
{
	my %param = get_param();

	# 基本的な処理
	my $cmd = $param{'cmd'} || '';
	if( $cmd eq 'upload' ){
		# アップロード処理
		do_upload(%param);
	}elsif( $cmd eq 'delete' ){
		# 削除処理
		do_delete(%param);
	}

	# create html
	my $html = '';
	my $page = $param{'page'} || 1;
	my $newst = get_newst_number($LogFile);
	$html .= html_header($Title);
	$html .= html_postform();
	$html .= show_pageindex($newst);
	$html .= show_upfiles($page,$newst);
	$html .= show_permission();
	$html .= html_deleteform();
	$html .= html_footer();

	print "Content-Type: text/html;charset=$CharSet;\n\n";
	print $html;
	return;
}

# [1][2][3]みたいな,ページのインデックスを表示
sub show_pageindex($)
{
	my $newst = shift;
	my $html = '';

	# create page index
	$html .= qq(<!-- Page Index -->\n);
	$html .= qq(<p id="page_index">\n);
	$html .= qq(\t<a href="$CGIPath?page=all">[ALL]</a>\n);

	# 切り上げ
	my $num = $newst/$MaxView;
	$num = ($num == int($num) ? $num : int($num+1));
	for(1 .. $num){
		$html .= qq(\t<a href="$CGIPath?page=$_">[$_]</a>\n);
	}
	$html .= qq(</p>\n\n);
	
	return $html;
}

# ファイルの削除処理
sub do_delete(%)
{
	my %param = @_;

	# initialize
	$param{'num'} || error('ファイルが指定されていません');
	$param{'delete_key'} || error('Delete Keyが設定されていません');

	# ログを削除するときに必要になる
	# パスワードを生成する
	my $passwd = create_password( $param{'delete_key'} );
	
	# ログの対象lineを削除
	delete_log($param{'num'},$passwd) || error('Delete Keyが違います');

	return;
}

# 指定した番号のログを削除する
# ミスったらundefが返る
# うまくいったらtrue
sub delete_log($$)
{
	my $target_num = shift;
	my $target_passwd = shift;
	my $error = undef;

	# create tempfile
	my $tempfile = create_tempfile('./');

	# delete target line
	open(my $in=>$LogFile) or die $!;
	open(my $out=>"> $tempfile") or die $!;
	while(my $line = <$in>){
		my($num,undef,undef,undef,undef,undef,$passwd)
			= split(/<>/,$line);

		if($num == $target_num){
			if($passwd eq $target_passwd){
				next;
			}else{
				$error = 1;
				last;
			}
		}
		print $out $line;
	}
	close($out);
	close($in);

	# error check
	if($error == 1){
		unlink($tempfile) or die $!;
		return undef;
	}
	
	rename($tempfile=>$LogFile) or die $!;
	return 1;
}

# ファイルのアップロード処理
sub do_upload(%)
{
	my %param = @_;

	# 投稿者の入力データチェック
	$param{'upfile'} || error('ファイルが選択されていません');
	$param{'comments'} || error('コメント欄が未記入です');
	$param{'delete_key'} || error('Delete Keyが設定されていません');

	length $param{'comments'} > $MaxComments && error('コメントが長すぎます');

	# ファイル名から拡張子の取得、チェック
	my $msg = 0;
	my ($ext) = $param{'upfile'}{'filename'} =~ m/[^.]*\.(.*)/;
	unless(check_extension($ext)){
		$msg = '許可されていない拡張子です';
	}

	# ファイルサイズの取得、チェック
	my $filesize = -s $param{'upfile'}{'tempfile'};
	unless($filesize){
		$msg = '不正なファイルがアップロードされました';
	}
	
	# ここから上までにエラーがあった場合
	if($msg != 0){
		unlink($param{'upfile'}{'tempfile'}) or die $!;
		error($msg);
	}

	# 一時ファイル名をきちんとしたアップファイル名に変更
	my $num = get_newst_number($LogFile) + 1;
	my $upname = sprintf('up%04d',$num);
	rename($param{'upfile'}{'tempfile'},"$SaveDir$upname.$ext")
			or die $!;

	# delete keyの生成
	my $delete_key = $param{'delete_key'};
	my $passwd = create_password($delete_key);

	# ログファイルに書き込む
	write_log(
		"$upname.$ext",
		$param{'upfile'}{'filename'},
		$param{'upfile'}{'content_type'},
		$param{'comments'},
		$filesize,
		$passwd,
		$num
	);
	return;
}

# パスワード生成
sub create_password()
{
	my $str = shift;
	return substr(crypt($str,'tx'),3,10);
}
		

# 許可されている拡張子のリストを表示する
sub show_permission()
{
	my $html = '';

	$html .= qq(<!-- Permission Extension -->\n);
	$html .= qq(<p id="permission">\n\t);
	my @temp = sort @Permission;
	while(my $ext = shift @temp){
		$html .= "$ext,";
	}
	$html .= qq(\n</p>\n\n);
	return $html;
}

# 与えられた数字に3桁ごとにカンマを打つ,
# そして、その結果を返す関数
sub format_three_comma($)
{
	my $value = shift;
	1 while $value =~ s/^([-+]?\d+)(\d\d\d)/$1,$2/;
	return $value;
}

# 許可されている拡張子かチェックする
sub check_extension($)
{
	my $ext = shift;
	my @Temp = @Permission;
	while(my $per = shift @Temp){
		return 1 if $per eq $ext;
	}
	return undef;
}

# アップロードされたファイルを表示する
sub show_upfiles($$)
{
	# 表示オプション
	my $page = shift;
	my $newst = shift;

	my $min = 0;
	my $max = 0;

	if($page eq 'all'){
		$min = 0;
		$max = $newst;
	}else{
		$page = 1 if $page <= 0;
		$page--;
		$max = $newst - $page * $MaxView;
		$min = $max - $MaxView + 1;
	}

	my $html = <<'TABLE';
<!-- Upfile List -->
<table id="upfile" summary="upfiles">
	<tr>
		<td>File Name</td>
		<td>Comments</td>
		<td>File Size</td>
		<td>Content-Type</td>
		<td>Original Name</td>
	</tr>

TABLE

	open(my $fh=>$LogFile) or die $!;
	my @data = <$fh>;
	@data = reverse @data;
	while(my $line = shift @data){
		#chomp $line;	# 行末の改行コードを取り除く
		my($num,$upname,$truename,$ctype,$size,$comments,$ip)
			= split(/<>/,$line);

		# 指定された範囲ではない
		next if($max < $num);
		last if($min > $num);

		# ファイルサイズをKB単位に変換する,一応コメントを日本語に変換
		$size = int($size / 1024 + 1) . 'KB';# 切り上げ
		$size = format_three_comma( $size );
		Jcode::convert( \$comments, 'euc' );
		
		$html .= <<"HTML";
	<tr>
		<td><a href="$SaveDir$upname">$upname</a></td>
		<td>$comments</td>
		<td>$size</td>
		<td>$ctype</td>
		<td>$truename</td>
	</tr>

HTML
	}
	close($fh);

	$html .= qq(</table>\n\n);
	return $html;
}

# ログファイルに書き込む
# 第一引数は、アップされたファイル名
# 第二引数は、本当のファイル名
# 第三引数は、Content-Type
# 第四引数は、comments
# 第五引数は、ファイルサイズ
# 第六引数は、Delete Pass
# 第七引数は、ログ番号
sub write_log($$$$$$)
{
	my $upfile_name = shift;
	my $orig_name = shift;
	my $ctype = shift;
	my $comments = shift;
	my $filesize = shift;
	my $passwd = shift;
	my $num = shift;

	# 投稿者のIPアドレスを取得
	my $ip = $ENV{'REMOTE_ADDR'} || 'Unknown';

	# ログファイルに書き込む
	open(my $fh=>">> $LogFile") or die $!;
	print $fh "$num<>$upfile_name<>$orig_name<>$ctype<>";
	print $fh "$filesize<>$comments<>$passwd<>$ip<>\n";
	close($fh);
	return;
}

sub get_newst_number($)
{
	my $file = shift;
	my $num = 0;

	open(my $fh=>$file) or die $!;
	my $line = '';
	while(<$fh>){
		$line = $_;
	}
	close($fh);

	($num,undef) = split(/<>/,$line);
	return 0 unless $num;
	$num =~ m/(\d+)/;
	return $1;
}

# 指定したファイルの行数を取得する
sub number_of_lines($)
{
	my $file = shift;
	my $num = 0;

	open(my $fh=>$file) or die $!;
	while(<$fh>){
		$num++;
	}
	close($fh);
	return $num;
}

# CGIに渡されたデータを取得する
sub get_param()
{
	my $query = '';
	my $method = $ENV{'REQUEST_METHOD'} || '';
	my $length = $ENV{'CONTENT_LENGTH'} || 0;
	my $ctype = $ENV{'CONTENT_TYPE'} || '';

	# リクエストメソッド別に QUERY_STRING を取得
	if( $ctype =~ m/multipart/i ){
		return read_multipart();
	}else{
		if($method eq 'GET'){
			$query = $ENV{'QUERY_STRING'};
		}elsif($method eq 'POST'){
			read(STDIN,$query,$length);
		}
	}

	my %param = ();
	my @data = split(/&/,$query);
	while(my $line = shift @data){
		my($key,$val) = split(/=/,$line);

		url_decode(\$val);
		Jcode::convert(\$val,'euc');
		escape_html(\$val);

		$param{$key} = $val;
	}
	return %param;
}

sub url_decode($)
{
	my $str = shift;

	$$str =~ tr/+/ /;
	$$str =~ s/%([0-9a-fA-F]{2})/chr(hex($1))/eg;
	return;
}

sub escape_html($)
{
	my $str = shift;

	$$str =~ s/&/&amp;/g;
	$$str =~ s/</&lt;/g;
	$$str =~ s/>/&gt;/g;
	$$str =~ s/"/&quot;/g;
	$$str =~ s/'/&#39;/g;
	return;
}

sub html_header($)
{
	my $title = shift;

	return <<"HEADER"
<?xml version="1.0" encoding="$CharSet"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
	"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
	
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="ja">
<head>
	<link rel="stylesheet" href="$CSSDir" type="text/css" />
	<link rev="made" href="mailto:kimoto\@example.com" />
	<link rel="index" href="$CGIPath" />
	<title>$title</title>
</head>
<body>

<!-- Title -->
<h1>$title</h1>

HEADER
;
}

sub html_postform()
{
	return <<"POSTFORM"
<!-- Post Form -->
<form id="post_form" method="post" action="$CGIPath" enctype="multipart/form-data">
<p>
	File:<br />
	<input type="file" name="upfile" tabindex="1" accesskey="f" />
	DelKey:<input type="password" name="delete_key" tabindex="3" accesskey="d" /><br />
	Comments:<br />
	<input type="text" name="comments" value="" tabindex="2" accesskey="c" />
	<input type="submit" value="upload" tabindex="4" accesskey="s" />
	<input type="reset" value="clear" tabindex="5" accesskey="r" />
	<input type="hidden" name="cmd" value="upload" />
</p>
</form>

POSTFORM
;
}

sub html_deleteform()
{
	return <<"DELETE"
<!-- Delete Form -->
<form id="delete_form" method="post" action="$CGIPath" enctype="multipart/form-data">
<p>
	No.<input type="text" name="num" value="" tabindex="5" accesskey="n" />
	Key<input type="password" name="delete_key" tabindex="6" accesskey="k" />
	<input type="submit" value="delete" tabindex="7" accesskey="d" />
	<input type="reset" value="clear" tabindex="8" accesskey="c" />
	<input type="hidden" name="cmd" value="delete" />
</p>
</form>

DELETE
}

sub html_return()
{
	return <<"RETURN";
<!-- Return -->
<p id="return">
	<a href="$CGIPath">Return</a>
</p>

RETURN
}

sub html_footer()
{
	return <<"FOOTER"
<!-- Valid -->
<p id="valid">
	<!-- XHTML 1.1 -->
	<a href="http://validator.w3.org/check?uri=referer"><img
		src="http://www.w3.org/Icons/valid-xhtml11"
		alt="Valid XHTML 1.1!" height="31" width="88" /></a>
	<!-- CSS -->
	<a href="http://jigsaw.w3.org/css-validator/"><img
		src="http://jigsaw.w3.org/css-validator/images/vcss"
		alt="Valid CSS!" height="31" width="88" /></a>
</p>
	
<!-- Copyright -->
<p id="copyright">
	Copyright(C) 2004 kimoto All Rights Reserved.
	<a href="http://black-earth.cun.jp/">&gt;&gt;</a>
</p>

</body>
</html>
FOOTER
;
}

# デバッグ時に使用
sub debug_print(%)
{
	my %param = @_;

	my $html = '<h2>Debug Information</h2>';
	foreach my $temp(sort keys %param){
		if( ref($param{$temp}) eq 'HASH' ){
			foreach my $tmp( sort keys %{$param{$temp}} ){
				$html .= "$temp=>$tmp=>$param{$temp}{$tmp}<br />\n";
			}
		}else{
			$html .= "$temp=>$param{$temp}<br />\n";
		}
	}
	$html .= "<br />\n\n";
	return $html;
}

# Multipart/form-dataの解析
sub read_multipart()
{
	my $boundary = <STDIN>;
	$boundary =~ s/\r\n/\n/;
	$boundary =~ s/\n//;
	my $in_header = 'true';
	my $tempdir = '/tmp/';
	my $tempfile = create_tempfile($tempdir);
	my %header = ();
	my %param = ();

	while(my $line = <STDIN>){
		$line =~ s/\r\n/\n/;
		$line =~ s/\n//;

		# escape
		#last unless $line;
		last if $line eq "$boundary--";
	
		# headerの終わり
		if($line eq ''){
			if($header{'filename'}){
				# add tempfile
				open(my $fh=>"> $tempfile") or die $!;
				my $buf = '';
				while(my $line = <STDIN>){
					last if $line =~ m/$boundary/;
					print $fh $buf;
					$buf = $line;
				} 
				$buf =~ s/\r\n$//;
				print $fh $buf;
				close($fh);

				my %tmp = ();
				$tmp{'tempfile'} = $tempfile;
				$tmp{'filename'} = $header{'filename'};
				$tmp{'content_type'} = $header{'CONTENT_TYPE'};

				$param{ $header{'name'} } = \%tmp;
			}else{
				# get parameters
				while(my $line = <STDIN>){
					$line =~ s/\r\n/\n/;
					$line =~ s/\n//;
					last if $line =~ m/$boundary/;
					$param{ $header{'name'} } .= $line;
				}
			}
			$in_header = 'true';
			$tempfile = create_tempfile($tempdir);
			%header = ();
			next;
		}
		# analysis
		my($key,$val) = split(/\:\s*/,$line,2);
		$key =~ s/-/_/g;
		$key = uc $key;
		$header{$key} = $val;

		# case Content-Disposition
		if($key eq 'CONTENT_DISPOSITION'){
			$val =~ s/.*?;//;

			my @pairs = split(/;/,$val);
			while(my $pair = shift @pairs){
				my($key,$val) = split(/=/,$pair);
				$key =~ s/\s//g;
				$val =~ s/"(.*)"/$1/;

				$header{$key} = $val;
			}
		}
	}
	return %param;
}

# 指定したディレクトリに
# ユニークなファイル名を捜し出して返す関数
sub create_tempfile($)
{
	my $dir = shift;
	my $str = time;

	while( -e $dir . $str ){
		$str = time;	
	}
	return $dir . $str;
}

# エラーメッセージを出力して
# スクリプトを強制終了する
sub error($)
{
	my $msg = shift;

	my $html = '';
	$html .= html_header('Error');
	$html .= "<h2>$msg</h2>";
	$html .= html_return();
	$html .= html_footer();

	print "Content-Type: text/html;\n\n";
	print $html;
	exit;
}
