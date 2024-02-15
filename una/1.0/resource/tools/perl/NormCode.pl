#!/usr/bin/perl
# 
# Copyright (c) 2023 Ricoh Company, Ltd.
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
# NormCode.pl
#
# 機能: 記号類の全半角変換
# ・コード変換テーブルからASCII→全角記号への変換データを抽出し、UCS2に変換(map0)
# ・map0の文字と１対１で置換可能な文字をUnicodeDataから抽出(map1)
# ・map1の文字と１対１で置換可能な文字をUnicodeDataから抽出(map2)
#
# 実行方法:
#   NormCode.pl -s DIR -d DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -d DIR : 中間データの所在 ($datadir) [required]
#    -x     : デバッグ出力
#    -h     : ヘルプ
#
# 入力ファイル:
#   $datadir/euc2ucs.dat    - EUC/UCS変換テーブル
#   $datadir/euc2ucs-jj.dat - EUC/UCS変換テーブル（jj仕様）
#   $srcdir/ambDatCode.c - LIMEDIO内部コード変換テーブル
#   $srcdir/UnicodeData-1.1.5.txt
#
# 出力ファイルのフォーマット: code1;code2
#     code1: 変換前コード
#     code2: 変換後コード（削除の場合は0000）
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
Usage: NormCode.pl -s DIR -d DIR
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
# EUC/UCS変換テーブルの読み込み
$srcfile = "$datadir/euc2ucs.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    # コードの切り出し
    $euc = $work[0];
    $ucs = $work[1];

    # 10進化EUCコードの配列値にUCSコードを格納する
    @euc2ucs[hex($euc)] = $ucs;
}
close(FIN);

##########
# EUC/UCS変換テーブル（jj仕様）の読み込み
$srcfile = "$datadir/euc2ucs-jj.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    # コードの切り出し
    $euc = $work[0];
    $ucs = $work[1];

    # 10進化EUCコードの配列値にUCSコードを格納する
    @jj[hex($euc)] = $ucs;
}
close(FIN);

##########
# ambDatCode.cからASCII記号の変換データを抽出
$srcfile = "$srcdir/ambDatCode.c";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;

    # 対象範囲外になったらループを抜ける
    last if ($_ =~ /ambCnvTbl J201ToNormTbl/);

    y/[a-f]/[A-F]/;		# コード表記を大文字化

    if ($_ =~ /^\/\* \((..)\) +(.),? .+{ *0x(....).*,.*}/) {
	
	# 記号のみを抽出
	if ($2 !~ /^[0-9A-Za-z]$/ && $3 !~ /^0000$/) {

	    # 全角→ASCIIにする
	    $org = $3;
	    $rep = $1;
	    
	    # Unicode変換によるUCS変換
	    $org_ucs = @euc2ucs[hex($org)];
	    $rep_ucs = @euc2ucs[hex($rep)];

	    print "$org_ucs;$rep_ucs\n";

	    # 変換パターンをmap0に格納
	    @map0[hex($org_ucs)] = $rep_ucs;
	    @map0[hex($rep_ucs)] = $rep_ucs;

	    # JJ仕様によるUCS変換
	    if (($org_jj = @jj[hex($org)]) !~ /^$/) {

		@map0[hex($org_jj)] = $rep_ucs;
		if ($org_jj !~ "$rep_ucs") {
		    print "$org_jj;$rep_ucs\n";
		}

	    }
	}
    }
}
close(FIN);

##########
# UnicodeData-1.1.5.txtからmap0のターゲットと1対1のマップ関係にある
# コードを抽出し、map1に格納する。
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

$count_decomp = 0;		# 配列@code2decomp要素数

while (<FIN>) {
    chop;
    @work = split(";");

    $code   = $work[0];		# コード
    $decomp = $work[5];		# 文字構成

    # 文字構成からタグを削除
    $decomp =~ s/^<[^>]+> +//;
    $decomp =~ s/ +<[^>]+>$//;
    $decomp =~ s/^0020 //;	# 先頭のスペースは除く

    # コードと文字構成が1対1の関係にあるもの
    if ($decomp =~ /^....$/) {
	# マップ関係を配列に格納
	@code2decomp[$count_decomp] = "$code;$decomp";
	$count_decomp ++;

	# map0のターゲットが$codeの場合
	if (($rep = @map0[hex($code)]) !~ /^$/ && 
	    @map0[hex($decomp)] =~ /^$/) { # map0との重複は除く

	    if ($DEBUG == 0) {
		print "$decomp;$rep\n";
	    } else {
		print "$decomp;$rep;map1\n";
	    }
	    @map1[hex($decomp)] = $rep;
	}

	# map0のターゲットが$decompの場合
        if (($rep = @map0[hex($decomp)]) !~ /^$/ && 
	    @map0[hex($code)] =~ /^$/) { # map0との重複は除く

	    if ($DEBUG == 0) {
		print "$code;$rep\n";
	    } else {
		print "$code;$rep;map1\n";
	    }
	    @map1[hex($code)] = $rep;
	}
    }
}
close(FIN);

##########
# @code2decompからmap1のターゲットと1対1のマップ関係にあるコードを抽出
for ($i = 0; $i < $count_decomp; $i ++) {

    @codes = split(";", @code2decomp[$i]);

    $code = @codes[0];		# コード
    $decomp = @codes[1];	# 文字構成

    # map1のターゲットが$codeの場合
    if (($rep = @map1[hex($code)]) !~ /^$/ &&
	@map0[hex($decomp)] =~ /^$/ &&
	@map1[hex($decomp)] =~ /^$/) { # map0,1との重複は除く

	if ($DEBUG == 0) {
	    print "$decomp;$rep\n";
	} else {
	    print "$decomp;$rep;map2\n";
	}
    }

    # map1のターゲットが$decompの場合
    if (($rep = @map1[hex($decomp)]) !~ /^$/ &&
	@map0[hex($code)] =~ /^$/ &&
	@map1[hex($code)] =~ /^$/) { # map0,1との重複は除く

	if ($DEBUG == 0) {
	    print "$code;$rep\n";
	} else {
	    print "$code;$rep;map2\n";
	}
    }
}

