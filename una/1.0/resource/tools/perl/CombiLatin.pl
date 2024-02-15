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
# CombiLatin.pl
#
# 機能: 以下を対象としてラテン文字の合字データを作成する
# ・UnicodeDataのdecomposition mappingが以下のパターンに合致する
#   −１文字目がラテン文字かギリシャ文字で、２文字目以降が結合文字
#   −１文字目と２文字目がラテン文字で、３文字目以降が結合文字。
#     この場合、１文字目と２文字目のラテン文字同士の合字規則は作成せず、
#     １文字目と２文字目の合字結果と３文字目以降との合字規則を作成する。
# ・上記結合文字を「空白＋結合文字」(準結合文字１)に置き換えたもの
# ・上記準結合文字１を全角変換(準結合文字２)し、更に１文字目のアルファベットを
#   全角変換したもの
# ・以下については、中間の文字がUnicodeDataで定義されていないため、
#   ダミーコード(FFFE,FFFF)を用いる
#   - 0041 0307 0304: 0041+0307->FFFE, FFFE+0304->01E0
#   - 0061 0307 0304: 0041+0307->FFFF, FFFF+0304->01E0
#   - 0045 0327 0306: 0045+0327->FFFE, FFFE+0306->1E1C
#   - 0065 0327 0306: 0045+0327->FFFF, FFFF+0306->1E1C
#
# 実行方法:
#   CombiLatin.pl -s DIR -d DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -d DIR : 中間データの所在 ($datadir) [required]
#    -h     : ヘルプ
#
# 入力ファイル:
#   $srcdir/UnicodeData-1.1.5.txt
#   $datadir/TargetLatin.dat     - ラテン文字コード
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
Usage: CombiLatin.pl -s DIR -d DIR
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
    @latin[hex($_)] = $_;
}
close(FIN);

##########
# ギリシャ文字コードの読み込み
$srcfile = "$datadir/TargetGreek.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @greek[hex($_)] = $_;
}
close(FIN);

##########
# 結合文字コードの読み込み
$srcfile = "$datadir/TargetCombining.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @combi[hex($_)] = $_;
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

    @h2f[hex($rep)] = $org;
}
close(FIN);

##########
# 変換データの作成、格納
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

$count = 0;
$count_lig = 0;

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

    } elsif (@latin[hex($code)] !~ /^$/) {

        # 文字構成のチェック
	$i = 0;
	@comps = split(/ /, $decomp);
	foreach $comp (@comps) {
	    if ($i == 0 && 
		@latin[hex($comp)] =~ /^$/ && @greek[hex($comp)] =~ /^$/) {
		# １文字目がラテン文字かギリシャ文字でなければ終了
		# ＊019B はヘッドがギリシャ文字(03BB)
		last;
	    }
	    if ($i == 1 && 
		@latin[hex($comp)] !~ /^$/ && @comps == 2) {
		# ２文字目がラテン文字で、構成文字数が２なら、
		# 構成パターンを @lig に格納して終了
		@lig[$count_lig ++] = "$decomp;$code";
		last;
	    }
	    if ($i == 1 && 
		@latin[hex($comp)] =~ /^$/ && @combi[hex($comp)] =~ /^$/) {
		# ２文字目がラテン文字か結合文字でなければ終了
		last;
	    }
	    if ($i > 1 && @combi[hex($comp)] =~ /^$/) {
                # ３文字目以降が結合文字でなければ終了
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

    if (@chars == 2) { # ヘッド＋結合文字１文字
	print "$org;$rep\n";

        $head = @chars[0];
        $combi = @chars[1];

	if (($combi1 = @combi1[hex($combi)]) !~ /^$/) { # 準結合文字１
	    print "$head $combi1;$rep\n";

	    if (($combi2 = @h2f[hex($combi1)]) !~ /^$/ && # 準結合文字２
		($head2 = @h2f[hex($head)]) !~ /^$/) { # ヘッドも全角に変換
		print "$head2 $combi2;$rep\n";
	    }
	}

    } elsif ($org =~ /^0041 0307 0304$/) { # A WITH DOT ABOVE AND MACRON
	print "0041 0307;FFFE\n";
	print "FFFE 0304;01E0\n";

    } elsif ($org =~ /^0061 0307 0304$/) { # a WITH DOT ABOVE AND MACRON
	print "0061 0307;FFFF\n";
	print "FFFF 0304;01E1\n";

    } elsif ($org =~ /^0045 0327 0306$/) { # E WITH CEDILLA AND BREVE
	print "0045 0327;FFFE\n";
	print "FFFE 0306;1E1C\n";

    } elsif ($org =~ /^0065 0327 0306$/) { # e WITH CEDILLA AND BREVE
	print "0065 0327;FFFF\n";
	print "FFFF 0306;1E1D\n";

	# 0304,0307,0306,0327に対する準結合文字１は定義されていない

    } else { # ヘッド＋結合文字２文字以上（ラテン文字は２文字まで）

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
			# 結合文字２文字目以降の場合、ヘッドは音標
			# 符号付きになるので、全角変換の必要はない
			print "$rep2 $combi2;$rep\n";
		    }
		}
		break;
	    }
	} 

	for ($k = 0; $k < $count_lig; $k ++){

	    ($org3 = @lig[$k]) =~ s/;.+$//;
	    ($rep3 = @lig[$k]) =~ s/^.+;//;

	    if ($head =~ /^$org3$/) {
		print "$rep3 $combi;$rep\n";

		if (($combi1 = @combi1[hex($combi)]) !~ /^$/) { # 準結合文字１
		    print "$rep3 $combi1;$rep\n";

		    if (($combi2 = @h2f[hex($combi1)]) !~ /^$/) { # 準結合文字２
			print "$rep3 $combi2;$rep\n";
		    }
		}
		break;
	    }
	}
    }
}
