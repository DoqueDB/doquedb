#!/usr/bin/perl
# 
# Copyright (c) 2004, 2023 Ricoh Company, Ltd.
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
# MakeCombiMap.pl
#
# 機能: 合字テーブルの作成
#
# 実行方法:
#   MakeCombiMap.pl -d DIR [OPTIONS]
#    -d DIR  : 中間データの所在 ($datadir) [required]
#    -m MODE : モード指定 (MODE = dm[ja, euro, zh])
#    -h      : ヘルプ
#
# 入力ファイル:
#   $datadir/CombiKana.dat  - かな合字テーブル
#   $datadir/CombiLatin.dat - ラテン文字合字テーブル
#   $datadir/CombiCyrillic.dat - キリル文字合字テーブル
#   $datadir/CombiGreek.dat - ギリシャ文字合字テーブル
#
# 出力ファイルのフォーマット: code1;code2
#   code1: 変換前コード列
#   code2: 変換後コード
#
################################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$count_combi = 0;	# 規則数

##########
# パラメータ評価
use Getopt::Std;
getopts('d:m:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: MakeCombiMap.pl -d DIR [OPTIONS]
   -d DIR  : Intermediate $datadir directory [required]
   -m MODE : Mode specification (MODE = dm[ja, euro, zh])
   -h      : help
EOU
}
$datadir = $opt_d if defined $opt_d;
$mode = $opt_m if defined $opt_m;
if ($mode && $mode !~ /^(dmja|dmeuro|dmzh)$/) {
    die <<EOU;
Illegal value for -m option
Usage: MakePreMap.pl -d DIR [OPTIONS]
   -d DIR  : Intermediate data directory
   -m MODE : Mode specification (MODE = dm[ja, euro, zh])
   -h      : help
EOU
}

##########
# かな合字テーブルの読み込み
if ($mode !~ /^(dmeuro|dmzh)$/) {
    $srcfile = "$datadir/CombiKana.dat";
    open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

    while (<FIN>) {
        chop;
        print "$_\n";
        $count_combi ++;
    }
    close(FIN);
}

##########
# ラテン合字テーブルの読み込み
if ($mode =~ /^(dmja|dmeuro|dmzh)$/) {
    $srcfile = "$datadir/CombiLatin.dat";
    open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";
    
    while (<FIN>) {
	chop;
	print "$_\n";
	$count_combi ++;
    }
    close(FIN);
}

##########
# キリル合字テーブルの読み込み
if ($mode =~ /^(dmja|dmeuro|dmzh)$/) {
    $srcfile = "$datadir/CombiCyrillic.dat";
    open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";
    
    while (<FIN>) {
	chop;
	print "$_\n";
	$count_combi ++;
    }
    close(FIN);
}

##########
# ギリシャ合字テーブルの読み込み
if ($mode =~ /^(dmja|dmeuro|dmzh)$/) {
    $srcfile = "$datadir/CombiGreek.dat";
    open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";
    
    while (<FIN>) {
	chop;
	print "$_\n";
	$count_combi ++;
    }
    close(FIN);
}

##########
# 規則数を出力
# ＊後でソートする関係上、規則数が先頭行になるように、スペースを入れる。
#   スペースはソートした後で削除する必要がある。
print " $count_combi\n";

