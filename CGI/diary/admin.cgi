#!/usr/bin/perl

# Black Diary Write CGI
use strict;
use warnings;
require 'jcode.pl';

# ProtoType - Function
sub get_param();
sub read_file($);
sub check_accept($$);
sub error($);

# Config
my $template_dir = 'template';
my $pass_file = 'diary_pass.txt';
my $diary_file = 'diary.txt';

&Main();
exit();

sub Main()
{
	my $html;

	# CGI���������줿�ǡ������������
	my %in = get_param();

	# ���å�����Check
	if($in{'mode'} eq 'login'){

		if(check_accept($in{'user'},$in{'password'}) != 0){
			# ǧ�ڼ���
			error("�ѥ���ɤޤ��ϡ��桼����̾���㤤�ޤ���");
		}else{
			# ǧ������
			$html = read_file("$template_dir/header.html");
			$html =~ s/=TITLE=/Admin Mode/g;
			$html =~ s/=CHARSET=/euc-jp/g;
			
			$html .= read_file("$template_dir/form.html");
		}
	}elsif($in{'mode'} eq 'write'){

		# �񤭹��ߥ⡼��
		open(my $out,">> $diary_file") or die("open error:$diary_file");
		my $time = localtime(time);

		jcode::convert(\$in{'message'},'euc');
		$in{'message'} =~ s/\r|\n|\r\n/<br>/g;

		print $out "$time,$in{'title'},$in{'message'}\n";
		close($out);

		error("�񤭹����������ޤ�����");

	}elsif($in{'mode'} eq 'help'){
		
		# help
		$html .= read_file("$template_dir/header.html");
		$html =~ s/=TITLE=/Help/g;
		$html =~ s/=CHARSET=/euc-jp/g;

		$html .= read_file("$template_dir/help.html");
		$html =~ s/=HELP=/������Ǥ������Ф餯���Ԥ���������/g;

	}else{
		# ������ɽ���������
		$html .= read_file("$template_dir/header.html");
		$html =~ s/=TITLE=/Login/g;
		$html =~ s/=CHARSET=/euc-jp/g;
	
		$html .= read_file("$template_dir/login.html");
		$html =~ s/=CGIPATH=/$ENV{SCRIPT_NAME}/g;
	}

	# ��˥塼�κ���
	$html .= read_file("$template_dir/menu.html");
	$html =~ s/=CGIPATH=/$ENV{SCRIPT_NAME}/g;
	$html =~ s/=DIARYPATH=/.\//g;

	# �������κ���
	$html .= read_file("$template_dir/copyright.html");

	$html .= "</body>\n</html>\n";

	# ���ܸ���Ѵ�
	jcode::convert(\$html,'euc');

	print "Content-Type: text/html;\n\n";
	print $html;

	return;
}

sub read_file($)
{
	my $file_name = shift;
	my $buf;

	open(my $in,"$file_name") or die("open error: $file_name");
	for my $line(<$in>){
		$buf .= $line;
	}
	close($in);
	return $buf;
}

sub get_param()
{
	my %in;
	my $query;

	if($ENV{REQUEST_METHOD} eq 'GET' || $ENV{REQUEST_METHOD} eq 'HEAD'){
		$query = $ENV{QUERY_STRING};
	}elsif($ENV{REQUEST_METHOD} eq 'POST'){
		read(STDIN,$query,$ENV{CONTENT_LENGTH});
	}

	foreach my $temp(split(/[;&]/,$query)){
		my($key,$val) = split(/=/,$temp,2);

		$val =~ tr/+/ /;
		$val =~ s/%([0-9a-fA-F]{2})/chr(hex($1))/eg;
		jcode::convert(\$val,'euc');
		$val =~ s/&/&amp;/g;
		$val =~ s/</&lt;/g;
		$val =~ s/>/&gt;/g;
		$val =~ s/"/&quot;/g;
		$val =~ s/'/&#39;/g;
		$val =~ s/,/&#44;/g;
		$val =~ s/\n|\r\n|\r|\r\r\n/<br>/g;

		$in{$key} = $val;
	}

	return %in;
}

# ����������0���֤������Ԥ�1
sub check_accept($$)
{
	my $user = shift;
	my $pass = shift;

	open(my $in,"$pass_file") or die("open error:$pass_file");
	for my $line(<$in>){
		chomp($line);

		my($u,$p) = split(/,/,$line,2);
		if($u eq $user){
			if($p eq crypt($pass,$p)){
				close($in);
				return 0;
			}
		}
	}
	close($in);

	return 1;
}

sub error($)
{
	my $msg = shift;

	my $template = read_file("$template_dir/error.html");
	$template =~ s/=ERROR_MESSAGE=/$msg/;

	print "Content-Type: text/html;\n\n";

	my $buf =  read_file("$template_dir/header.html");
	$buf =~ s/=TITLE=/Error/g;
	$buf =~ s/=CHARSET=/euc-jp/g;

	print $buf;
	print $template;

	$buf = read_file("$template_dir/menu.html");
	$buf =~ s/=CGIPATH=/$ENV{SCRIPT_NAME}/g;
	$buf =~ s/=DIARYPATH=/\//g;
	print $buf;

	print read_file("$template_dir/copyright.html");

	print "</body>\n</html>\n";

	exit;
}
