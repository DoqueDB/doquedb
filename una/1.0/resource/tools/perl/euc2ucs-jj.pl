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
# euc2ucs-jj.pl
#
# 機能: jj準拠のEUC/UCS2変換テーブルを作成する（Unicode準拠との差異のみ）
#
# 実行方法: 
#   euc2ucs-jj.pl -s DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -h     : ヘルプ
#
# 入力ファイル:
#   $srcdir/JIS.uni.jj - unicodeとjjとのJIS/UCS2対応の差異
#
# 出力ファイルのフォーマット: code1;code2
#   code1: EUCコード
#   code2: UCS2コード
#
################################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

##########
# パラメータ評価
use Getopt::Std;
getopts('s:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: euc2ucs-jj.pl -s DIR
   -s DIR : Source data directory [required]
   -h     : help
EOU
}
$srcdir = $opt_s if defined $opt_s;

##########
$srcfile = "$srcdir/JIS.uni.jj";
open (FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    
    if ($_ =~ /^0x(.*) U\+(....) U\+(....)$/) {
	$jis = $1;
	$ucs = $3;
	$jis =~ y/a-z/A-Z/;
	$ucs =~ y/a-z/A-Z/;

	# JISコードをEUCに変換して出力
	if ($jis =~ /^..$/) {
	    print "$jis;$ucs\n";

	} else {
	    $jis1 = substr($jis,0,2);
	    $jis1 = "0x$jis1";
	    $jis2 = substr($jis,2);
	    $jis2 = "0x$jis2";
	    ($euc1, $euc2) = &jis2euc($jis1, $jis2);

	    printf("%X%X;%s\n", $euc1,$euc2,$ucs);
	}
    }
}
close (FIN);

##########
# JISからEUCへの変換
sub jis2euc {
    return((hex $_[0]) | 0x80, (hex $_[1]) | 0x80);
}
