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
# Cap2Small.pl
#
# 機能: 大文字から小文字への変換テーブルを作成する
# ・以下の条件をいずれも満たす場合
#   −UnicodeDataに小文字化コードが記述されている
#   −上記小文字化コードに対する大文字化または表題化コードが、自身と一致する
#
# 実行方法:
#   Cap2Small.pl -s DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -x     : デバッグ出力
#    -h     : ヘルプ
#
# 入力ファイル:
#   $srcdir/UnicodeData-1.1.5.txt
#
# 出力ファイルのフォーマット: code1;code2
#   code1: 変換前コード
#   code2: 変換後コード
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
Usage: Cap2Small.pl -s DIR
   -s DIR : Source data directory [required]
   -x     : debug
   -h     : help
EOU
}
$srcdir = $opt_s if defined $opt_s;
$DEBUG = 1 if $opt_x;

##########
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

$count_cap = 0;

while (<FIN>) {
    chop;
    @work = split(";");

    $code = $work[0];		# コード
    $lower_eq = $work[13];	# 小文字化コード
    $upper_eq = $work[12];	# 大文字化コード
    $title_eq = $work[14];	# タイトル化コード

    # 大文字→小文字の対応を配列@cap2smallに格納
    if ($lower_eq !~ /^$/) {
	@cap2small[$count_cap] = "$code;$lower_eq";
	$count_cap ++;
    }
    # 大文字化コードがあれば配列@upperの10進化コードの位置に格納
    if ($upper_eq !~ /^$/) {
	@upper[hex($code)] = $upper_eq;
    }
    # タイトル化コードがあれば配列@titleの10進化コードの位置に格納
    if ($title_eq !~ /^$/) {
	@title[hex($code)] = $title_eq;
    }
}
close(FIN);

##########
# 配列@cap2smallの要素が、大文字化またはタイトル化コードと相互参照関係
# にあるかどうかをチェックする
for ($i = 0; $i < $count_cap; $i ++) {

    @codes = split(";", @cap2small[$i]);

    $cap = @codes[0];		# 大文字
    $small = @codes[1];		# 小文字

    # 相互参照関係であれば出力
    if (@upper[hex($small)] == $cap || @title[hex($small)] == $cap) {
	print "$cap;$small\n";

    } elsif ($DEBUG == 1) {
	print "!$cap;$small\n";
    }
}
