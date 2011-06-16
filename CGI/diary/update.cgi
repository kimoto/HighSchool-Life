#!/usr/bin/perl

# Black Diary
# Charset -> EUC-JP
use strict;
use warnings;
require 'jcode.pl';

# ProtoType - Function
sub read_file($);
sub get_diary($);

# Config
my $template_dir = 'template';
my $title = 'Diary';
my $charset = 'euc-jp';
my $diary_file = 'diary.txt';

# Main Routine
MAIN:{
	my $html;

	# ヘッダー部分の作成
	$html = read_file("$template_dir/header.html");
	$html =~ s/=TITLE=/$title/g;
	$html =~ s/=CHARSET=/$charset/;

	# 日記dataを取得
	GET_DATE:{
		my $diary = get_diary($diary_file);
		chomp($diary);

		my $template = read_file("$template_dir/diary.html");
	
		$template =~ s/=DIARY=/$diary/g;
		
		# URLを変換する
		$template =~ s#(http://[\w\d\+\-\=\/\~\%\@\&\#\.\?]+)#<a href="$1">$1</a>#g;
		$html .= $template;
	}

	# メニュー画面
	MENU:{
		my $template = read_file("$template_dir/menu.html");
		$template =~ s/=CGIPATH=/admin.cgi/g;
		$template =~ s/=DIARYPATH=/$ENV{SCRIPT_PATH}/g;

		$html .= $template;
	}

	# 著作権情報部の作成
	$html .= read_file("$template_dir/copyright.html");

	# 終了部分
	$html .=  "</body>\n</html>\n";

	# 日本語に変換
	jcode::convert(\$html,'euc');

	# 結果を表示
	open(my $fh=>'> index.html') or die $!;
	print $fh $html;
	close($fh);

	print "Content-Type: text/html;\n\n";
	print "Success!!";

	exit;
}

# 指定したファイルを読み込んで返す
sub read_file($)
{
	my $file_name = shift;
	my $file_size;
	my $buf;

	open(my $in,"$file_name") or die("open error: $file_name");

	$file_size = -s $file_name;
	read($in,$buf,$file_size);

	close($in);
	return $buf;
}

# 指定した日記ファイルのデータを取得
# HTMLに変換して返す
sub get_diary($)
{
	my $file_name = shift;
	my $result = '';

	# ログファイルからデータを取得
	open(my $in,"$file_name") or die("open error: $file_name");

	# 最新のが上にくるように
	my @buf = <$in>;
	@buf = reverse @buf;

	my $count = 0;
	foreach my $line(@buf){
		if($count > 10 ){
			last;
		}
		# Date,Title,Messageの順
		my($date,$title,$msg) = split(/,/,$line,3);

		# htmlを形成する
		my $day = "<h2>$date</h2>\n";
		$day .= "<h3>$title</h3>\n";
		$day .= "<p>$msg</p>\n";
	
		my $html = read_file("$template_dir/day.html");
		$html =~ s/=DAY=/$day/g;

		$result .= $html;
		$count++;
	}
	
	close($in);
	return $result;
}
