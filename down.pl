#!/usr/bin/perl

=info
 	 $B2hA|O"HV@8@.%9%/%j%W%H(B

	http://test.com/image[01-100].jpg $B7A<0$N%"%I%l%9$+$i(B
	http://test.com/image/1.jpg$B7A<0$N%"%I%l%9$r=PNO(B

	http://test.com/image/01.jpg$B7A<0$G$b=PNO$G$-$k$h$&$K2~NI$9$k$Y$-$+$b(B

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


# $B0J2<=PNO%5%s%W%k(B
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
