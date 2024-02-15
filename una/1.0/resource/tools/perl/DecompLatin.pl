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
# DecompLatin.pl
#
# 機能: 音標符号付きラテン文字および合字を構成文字に分解する（結合文字は削除）
#
# 実行方法:
#   DecompLatin.pl -s DIR -d DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -d DIR : 中間データの所在 ($datadir) [required]
#    -h     : ヘルプ
#
# 入力ファイル:
#   $srcdir/UnicodeData-1.1.5.txt
#   $datadir/TargetLatin.dat - ラテン文字のコード
#
# 出力ファイルのフォーマット: code1;code2
#   code1: 変換前コード
#   code2: 変換後コード列
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
Usage: DecompLatin.pl -s DIR -d DIR
   -s DIR : Source data directory [required]
   -d DIR : Intermediate data directory [required]
   -h     : help
EOU
}
$srcdir = $opt_s if defined $opt_s;
$datadir = $opt_d if defined $opt_d;

##########
# ラテン文字コードの読み込み
$srcfile = "$datadir/TargetLatin.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    if ($_ !~ /^FF/) {		# 全角文字は除く
	@target[hex($_)] = $_;
    }
}

##########
# 変換データの作成
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

# 基底文字（Basic Latinの英字）と結合文字の定義
$base      = "00(4[1-9A-F]|5[0-9A]|6[1-9A-F]|7[0-9A])";
$combining = "03[0-6][0-9A-F]";

$count_decomp = 0;

while (<FIN>) {
    chop;
    @work = split(";");

    $code   = $work[0];	# コード
    $decomp = $work[5];	# 文字構成

    if (@target[hex($code)] !~ /^$/) {
	
	if ($decomp =~ /^$base( $base)*( $combining)*$/) { # 文字構成による変換
	    
	    $decomp =~ s/ $combining.*$//; # 結合文字を削除
	    @decomp_code[hex($code)] = $decomp;	# 変換結果を配列に格納
	    print "$code;$decomp\n";

	} elsif ($decomp =~ /^....( $combining)+$/) {
	    # 基底文字がBasic Latinではない
	    #  → 文字構成を配列に格納して後で再帰的な処理を行なう
	    @code2decomp[$count_decomp] = "$code;$decomp";
	    $count_decomp ++;
	}
    }
}
close(FIN);

##########
# 変換できなかったコードに対して再帰的に変換を行なう
for ($i = 0; $i < $count_decomp; $i ++) {
    @codes = split(";", @code2decomp[$i]);

    $code   = @codes[0];
    $decomp = @codes[1];

    $head   = $decomp;
    $head   =~ s/ .*$//;
    $rep = @decomp_code[hex($head)];

    if ($rep !~ /^$/) {	# 変換結果あり
	print "$code;$rep\n";	# 01F[CD] (AE WITH ACUTE)
	# 同様の文字で、01E[23](AE WITH MACRON)はLIMEDIO仕様から
	# 直接AEに変換される

    } else {	# 変換結果なし→Basic Latin以外への変換
	print "$code;$head\n";	# 019B, 01EE, 01EF
    }
}
