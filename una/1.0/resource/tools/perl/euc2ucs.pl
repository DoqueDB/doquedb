#!/usr/bin/perl
# 
# Copyright (c) 2002, 2023 Ricoh Company, Ltd.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 
################################################################################
#
# euc2ucs.pl
#
# 機能: Unicode準拠のEUC/UCS2変換テーブルを作成する
#
# 実行方法: 
#   euc2ucs.pl -s DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -x     : デバッグ出力
#    -h     : ヘルプ
#
# 入力ファイル:
#   $srcdir/{JIS0201,JIS0208,JIS0212}.TXT - unicode.orgのJIS/UCS2対応表
#
# 出力ファイルのフォーマット: code1;code2
#   code1: EUCコード
#   code2: UCS2コード
#
################################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$DEBUG = 0;

##########
# パラメータ評価
use Getopt::Std;
getopts('s:xh');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: euc2ucs.pl -s DIR
   -s DIR : Source data directory [required]
   -x     : debug
   -h     : help
EOU
}
$srcdir = $opt_s if defined $opt_s;
$DEBUG = 1 if $opt_x;

##########
# JIS0201からの変換
$srcfile = "$srcdir/JIS0201.TXT";
open (FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    
    if ($_ =~ /^0x(..)\t0x(....)/) {
	# JIS, UCS2コードの切り出し
	$jis = $1;
	$ucs = $2;

	@jis[hex($jis)] = $jis;
	
	if ($DEBUG == 0) {
	    if (hex($jis) <= hex("7E")) { # ASCII
		print "$jis;$ucs\n";
	    } else {		# 半角カナ
		print "8E$jis;$ucs\n";
	    }

	} else {
	    if (hex($jis) <= hex("7E")) { # ASCII
		print "$jis;$jis;$ucs\n";
	    } else {		# 半角カナ
		print "$jis;8E$jis;$ucs\n";
	    }
	}
    }
}
close (FIN);

##########
# JIS0208からの変換
$srcfile = "$srcdir/JIS0208.TXT";
open (FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;

    if ($_ =~ /^0x(....)\t0x(....)\t0x(....)/) {
	# JIS, UCS2コードの切り出し
	$jis = $2;
	$ucs = $3;

	@jis[hex($jis)] = $jis;

	# JISコードをEUCに変換
	$jis1 = substr($jis,0,2);
	$jis1 = "0x$jis1";
	$jis2 = substr($jis,2);
	$jis2 = "0x$jis2";
	($euc1, $euc2) = &jis2euc($jis1, $jis2);

	if ($DEBUG == 0) {
	    printf("%X%X;%s\n", $euc1,$euc2,$ucs);

	} else {
	    printf("%s;%X%X;%s\n", $jis,$euc1,$euc2,$ucs);
	}
    }
}
close (FIN);

##########
# JIS0212からの変換
$srcfile = "$srcdir/JIS0212.TXT";
open (FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;

    if ($_ =~ /^0x(....)\t0x(....)/) {
	# JIS, UCS2コードの切り出し
	$jis = $1;
	$ucs = $2;

	if (@jis[hex($jis)] =~ /^$/) { # 出力済みのコードは除く
	    # JISコードをEUCに変換
	    $jis1 = substr($jis,0,2);
	    $jis1 = "0x$jis1";
	    $jis2 = substr($jis,2);
	    $jis2 = "0x$jis2";
	    ($euc1, $euc2) = &jis2euc($jis1, $jis2);

	    if ($DEBUG == 0) {
		printf("%X%X;%s\n", $euc1,$euc2,$ucs);
	    } else {
		printf("%s;%X%X;%s\n", $jis,$euc1,$euc2,$ucs);
	    }
	}
    }
}
close (FIN);

##########
# JISからEUCへの変換
sub jis2euc {
    return((hex $_[0]) | 0x80, (hex $_[1]) | 0x80);
}
