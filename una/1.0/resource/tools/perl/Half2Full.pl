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
# Half2Full.pl
#
# 機能: 半角文字から全角文字への変換テーブルを作成する。
# ・以下の条件をいずれも満たす場合
#   −Halfwidth and Fullwidth Formsブロックに属する
#   −文字名が /^HALFWIDTH/ に合致する
#   −decomposition が１文字
#
# 実行方法:
#   Half2Full.pl -s DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
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

##########
# パラメータ評価
use Getopt::Std;
getopts('s:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: Half2Full.pl -s DIR
   -s DIR : Source data directory [required]
   -h     : help
EOU
}
$srcdir = $opt_s if defined $opt_s;

##########
# 入力ファイルの設定
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    $code   = $work[0];	# コード
    $name   = $work[1];	# 文字名
    $decomp = $work[5];	# 文字構成

    if ($code =~ /^FF[0-9A-E]/ && 
	$name =~ /^HALFWIDTH/ && 
	$decomp =~ /^....$/) {
	print "$code;$decomp\n";
    }
}
close(FIN);

