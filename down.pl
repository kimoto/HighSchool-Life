#!/usr/bin/perl

=info
 	 画像連番生成スクリプト

	http://test.com/image[01-100].jpg 形式のアドレスから
	http://test.com/image/1.jpg形式のアドレスを出力

	http://test.com/image/01.jpg形式でも出力できるように改良するべきかも

	Created by kimoto.
=cut

while($url = shift @ARGV){
	if( $url =~ m#(.*)\[(.*)-(.*)\](.*)# ){
		for($2 .. $3){
			$url = $1.$_.$4;
			push(@url_list,$url);
		}
	}
}

# result
while($url = shift @url_list){
	print "$url\n";
}


# 以下出力サンプル
#./down.pl http://test.com/image/[01-10].png

__DATA__
http://test.com/image/1.png
http://test.com/image/2.png
http://test.com/image/3.png
http://test.com/image/4.png
http://test.com/image/5.png
http://test.com/image/6.png
http://test.com/image/7.png
http://test.com/image/8.png
http://test.com/image/9.png
http://test.com/image/10.png
