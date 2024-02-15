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
# MakeStdTbl.pl
#
# 機能: UNA用文字列標準化テーブルの作成
# ・仮名合字データ
#   −仮名合字規則の形式変換
#   −合字規則から濁点類削除規則を生成
# ・半角/全角変換データ
#   −半角/全角変換規則の形式変換
#   −全角/半角変換規則の入出力入れ換えと形式変換
#   −ただし、元規則の入力コードの文字名が以下に合致する場合
#       /(^|^HALFWIDTH )KATAKANA/
#
# 実行方法:
#   MakeStdTbl.pl -s DIR -d DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -d DIR : 中間データの所在 ($datadir) [required]
#    -h     : ヘルプ
#
# 入力ファイル:
#   $srcdir/UnicodeData-1.1.5.txt
#   $datadir/CombiKana.dat - 仮名合字テーブル
#   $datadir/Half2Full.dat - 半角/全角変換テーブル
#   $datadir/Full2Half.dat - 全角/半角変換テーブル
#
# 出力ファイルのフォーマット: "\u"code1("\u"code1)* ("\u"code2)
#   code1: 変換前コード(列)
#   code2: 変換後コード(削除規則の場合は空文字)
#
################################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

##########
# パラメータ評価
use Getopt::Std;
getopts('s:d:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: MakeStdTbl.pl -s DIR -d DIR
   -s DIR : Source data directory [required]
   -d DIR : Intermediate data directory [required]
   -h     : help
EOU
}
$srcdir = $opt_s if defined $opt_s;
$datadir = $opt_d if defined $opt_d;

##########
# 半角/全角変換テーブルの読み込み
$srcfile = "$datadir/Half2Full.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    $org = $work[0];
    $rep = $work[1];

    @h2f[hex($org)] = $rep;
}
close(FIN);

##########
# 全角/半角変換テーブルの読み込み
$srcfile = "$datadir/Full2Half.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    $org = $work[0];
    $rep = $work[1];

    @f2h[hex($org)] = $rep;
}
close(FIN);

##########
# 半角/全角変換データの出力
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    $code   = $work[0];    # コード値
    $name   = $work[1];    # 文字名

    if ($name =~ /(^|^HALFWIDTH )KATAKANA/) {

	if (@h2f[hex($code)] !~ /^$/) {	# 半角/全角変換テーブルの形式変換
	    print "\\u$code \\u@h2f[hex($code)]\n";

	} elsif (@f2h[hex($code)] !~ /^$/) { # 全角/半角変換テーブルの形式変換
	    print "\\u@f2h[hex($code)] \\u$code\n"; # 全半角を入れ換え
	}
    }
}
close(FIN);

##########
# 仮名合字データの出力
$srcfile = "$datadir/CombiKana.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    # 仮名合字規則の形式変換
    ($org = $work[0]) =~ s/^/\\u/;  $org =~ s/ /\\u/;
    ($rep = $work[1]) =~ s/^/\\u/;
    print "$org $rep\n";

    # 濁点類削除規則の生成
    $org =~ s/^.+\\u/\\u/;
    print "$org \n";
}
close(FIN);

