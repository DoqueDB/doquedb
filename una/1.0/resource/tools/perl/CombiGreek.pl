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
# CombiGreek.pl
#
# 機能: 以下を対象としてギリシャ文字の合字データを作成する
# ・UnicodeDataのdecomposition mappingが以下のパターンに合致する
#   −１文字目がギリシャ文字で、２文字目以降が結合文字か0399(IOTA)
# ・上記結合文字を「空白＋結合文字」(準結合文字１)に置き換えたもの
# ・上記準結合文字１を全角変換(準結合文字２)し、更に１文字目のアルファベットを
#   全角変換したもの
#
# 実行方法:
#   CombiGreek.pl -s DIR -d DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -d DIR : 中間データの所在 ($datadir) [required]
#    -h     : ヘルプ
#
# 入力ファイル:
#   $srcdir/UnicodeData-1.1.5.txt
#   $datadir/TargetGreek.dat     - ギリシャ文字コード
#   $datadir/TargetCombining.dat - 結合文字コード
#   $datadir/Full2Half.dat       - 全角/半角変換テーブル
#
# 出力ファイルのフォーマット: code1;code2
#   code1: 変換前コード列
#   code2: 変換後コード
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
Usage: CombiGreek.pl -s DIR -d DIR
   -s DIR : Source data directory [required]
   -d DIR : Intermediate data directory [required]
   -h     : help
EOU
}
$srcdir = $opt_s if defined $opt_s;
$datadir = $opt_d if defined $opt_d;

##########
# ギリシャ文字コードの読み込み
$srcfile = "$datadir/TargetGreek.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @greek[hex($_)] = $_;
}

##########
# 結合文字コードの読み込み
$srcfile = "$datadir/TargetCombining.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @combi[hex($_)] = $_;
}

##########
# 全角/半角変換テーブルの読み込み
$srcfile = "$datadir/Full2Half.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    $org = $work[0];
    $rep = $work[1];

    @h2f[hex($rep)] = $org;
}

##########
# 変換データの作成、格納
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

$count = 0;

while (<FIN>) {
    chop;
    @work = split(";");

    $code   = $work[0];		# コード
    $name   = $work[1];		# 文字名
    $decomp = $work[5];		# 文字構成

    if (@combi[hex($code)] !~ /^$/ && $decomp =~ /^0020 ....$/) {
        # 結合文字と準結合文字１（空白＋結合文字）の対応を配列に格納
	$decomp =~ s/^0020 //;
	@combi1[hex($decomp)] = $code;

    } elsif (@greek[hex($code)] !~ /^$/) {

        # 文字構成のチェック
	$i = 0;
	@comps = split(/ /, $decomp);
	foreach $comp (@comps) {
	    if ($i == 0 && @greek[hex($comp)] =~ /^$/) {
                # １文字目がギリシャ文字でなければ終了
		last;
	    }
	    if ($i > 0 &&
		@combi[hex($comp)] =~ /^$/ && $comp !~ /^0399$/) {
                # ２文字目以降が結合文字か0399(IOTA)でなければ終了
		last;
	    }
	    $i ++;
	}
        # 構成文字数が２文字以上で、末尾の文字までチェックされている
	if (@comps > 1 && $i == @comps) {
	    @rules[$count ++] = "$decomp;$code";
	}
    }
}
close(FIN);

##########
# 変換データの出力
for ($i = 0; $i < $count; $i ++) {

    ($org = @rules[$i]) =~ s/;.+$//;
    ($rep = @rules[$i]) =~ s/^.+;//;

    @chars = split(/ /, $org);

    if (@chars == 2) {	# ヘッド＋結合文字１文字
	print "$org;$rep\n";

	$head = @chars[0];
        $combi = @chars[1];

	if (($combi1 = @combi1[hex($combi)]) !~ /^$/) { # 準結合文字１
	    print "$head $combi1;$rep\n";

	    if (($combi2 = @h2f[hex($combi1)]) !~ /^$/) { # 準結合文字２
		print "$head $combi2;$rep\n";
	    }
	}

    } else { # ヘッド＋結合文字２文字以上（ギリシャ文字は３文字まで）

	($head = $org) =~ s/ ....$//;
	($combi = $org) =~ s/^.* //;

	for ($j = 0; $j < $count; $j ++) {

	    ($org2 = @rules[$j]) =~ s/;.+$//;
	    ($rep2 = @rules[$j]) =~ s/^.+;//;
	    
	    if ($head =~ /^$org2$/) {
		print "$rep2 $combi;$rep\n";

		if (($combi1 = @combi1[hex($combi)]) !~ /^$/) { # 準結合文字１
		    print "$rep2 $combi1;$rep\n";

		    if (($combi2 = @h2f[hex($combi1)]) !~ /^$/) { # 準結合文字２
			print "$rep2 $combi2;$rep\n";
		    }
		}
		break;
	    }
	}
    } 
}
