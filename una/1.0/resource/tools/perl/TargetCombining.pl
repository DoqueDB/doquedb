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
# TargetCombining.pl
#
# 機能: 結合文字を収集する。
# ・狭義の結合文字
#   −Combining Diacritical Marksブロックに属し、ラテン/キリル/ギリシャ文字の
#     decomposition に使用されている33文字
#   −結合文字の濁点(3099)、半濁点(309A)
#   −ギリシャ文字のTONOS(0384)、DIALYTIKA TONOS(0395)
# ・広義の結合文字
#   −decomposition が「スペース(0020)＋結合文字」である文字のうち、JISx0201,
#     JISx0208, JISx0212からマッピングされる14文字（準結合文字１）
#   −Halfwidth and Fullwidth Formsブロックに属し、decomposition が準結合文字１の
#     いずれか１文字である文字のうち、FFE3(FULLWIDTH MACRON)を除く4文字
#     （準結合文字２） 
#
# 実行方法: 
#   TargetCombining.pl -s DIR -d DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -d DIR : 中間データの所在 ($datadir) [required]
#    -x     : デバッグ出力
#    -h     : ヘルプ
#
# 入力ファイル: 
#   $srcdir/UnicodeData-1.1.5.txt
#   $datadir/euc2ucs.dat - EUC/UCS2変換テーブル
#
# 出力ファイルのフォーマット: code
#   code: コード値
#
################################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$DEBUG = 0;

##########
# パラメータ評価
use Getopt::Std;
getopts('s:d:xh');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: TargetCombining.pl -s DIR -d DIR
   -s DIR : Source data directory [required]
   -d DIR : Intermediate data directory [required]
   -x     : debug
   -h     : help
EOU
}
$srcdir = $opt_s if defined $opt_s;
$datadir = $opt_d if defined $opt_d;
$DEBUG = 1 if $opt_x;

##########
# ラテン文字の読み込み
$srcfile = "$datadir/TargetLatin.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";
while (<FIN>) {
    chop;
    @target[hex($_)] = $_;
}

##########
# キリル文字の読み込み
$srcfile = "$datadir/TargetCyrillic.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";
while (<FIN>) {
    chop;
    @target[hex($_)] = $_;
}

###########
# ギリシャ文字の読み込み
$srcfile = "$datadir/TargetGreek.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";
while (<FIN>) {
    chop;
    @target[hex($_)] = $_;
}

##########
# UCS2/EUC変換テーブルを作成する
$srcfile = "$datadir/euc2ucs.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";
while (<FIN>) {
    chop;
    @work = split(";");

    $euc = $work[0];
    $ucs = $work[1];

    @ucs2euc[hex($ucs)] = $euc;
}
close(FIN);

##########
# UnicodeData.txtから結合文字対象コードを抽出する
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

$combining = "03[0-6][0-9A-F]";

$count_decomp0 = 0;
$count_decomp1 = 0;

while (<FIN>) {
    chop;
    @work = split(";");		

    $code   = $work[0];	# コード値
    $name   = $work[1];	# 文字名
    $decomp = $work[5];	# 文字構成

    # targetの文字構成から結合文字を切り出す
    if (@target[hex($code)] !~ /^$/ &&
	$decomp =~ / $combining/) {
	@decomp = split(" ", $decomp);
	foreach $decomp_code (@decomp) {
	    if ($decomp_code =~ /^$combining$/) {

		# 結合文字配列に自身を格納
		@combining[hex($decomp_code)] = $decomp_code;

		if ($DEBUG == 0) {
		    print "$decomp_code\n";
		} else {
		    print "0:$decomp_code\n";
		}
	    }
	}

    } elsif ($code =~ /^309[9A]$/ || # 結合文字の濁点、半濁点
	     $code =~ /^038[45]$/) { # GREEK TONOS/DIALYTIKA TONOS
	
	@combining[hex($code)] = $code;	# 結合文字配列に自身を格納

	if ($DEBUG == 0) {
	    print "$code\n";
	} else {
	    print "0:$code\n";
	}

    } elsif ($decomp =~ /^0020 ....$/) {
	# 文字構成がスペース＋１文字のコードを格納する
	$decomp =~ s/^0020 //;	# スペースを削除
	@code2decomp0[$count_decomp0] = "$code;$decomp";
	$count_decomp0 ++;

    } elsif ($name =~ /^(FULLWIDTH|HALFWIDTH)/ && 
	     $decomp =~ /^....$/) {
	# 文字構成が１文字の半角／全角文字を格納する
	@code2decomp1[$count_decomp1] = "$code;$decomp";
	$count_decomp1 ++;
    }
}
close(FIN);

##########
# @code2decomp0から準結合文字１を抽出する
for ($i = 0; $i < $count_decomp0; $i ++) {
    @codes = split(";", @code2decomp0[$i]);

    $code = @codes[0];
    $decomp = @codes[1];

    # 文字構成が結合文字と一致し、EUCとの対応があれば出力
    if (@combining[hex($decomp)] !~ /^$/ && 
	@ucs2euc[hex($code)] !~ /^$/) {
	
        @combining[hex($code)] = $code;	# 結合文字配列に自身を格納
	
        if ($DEBUG == 0) {
	    print "$code\n";
	} else {
	    print "1:$code;$decomp\n";
	}
    }
}

##########
# @code2decomp1から準結合文字２を抽出する
for ($i = 0; $i < $count_decomp1; $i ++) {
    @codes = split(";", @code2decomp1[$i]);

    $code = @codes[0];
    $decomp = @codes[1];

    if (@combining[hex($decomp)] !~ /^$/ &&
	$code !~ /^FFE3$/) {	# FULLWIDTH MACRONは除く

        @combining[hex($code)] = $code;	# 結合文字配列に自身を格納

        if ($DEBUG == 0) {
	    print "$code\n";
	} else {
	    print "2:$code;@combining[hex($decomp)]\n";
	}
    }
}
