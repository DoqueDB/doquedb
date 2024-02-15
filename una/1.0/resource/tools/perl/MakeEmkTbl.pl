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
# MakeEmkTbl.pl
#
# 機能: UNA用英語トークン文字種テーブルの作成
# ・UnicodeDataの文字コードに以下の文字種を付与する
#    0: 英語トークン対象外
#       −該当なし
#    1: 数字
#       −文字名が /(^|^FULLWIDTH )DIGIT [A-Z]+$/ に合致する
#    2: 英大文字母音
#       −異表記正規化の規定によるラテン文字もしくはキリル文字に該当する文字で、
#       −文字名が /CAPITAL (LETTER|LIGATURE)/ に合致し、
#       −LIMEDIO向け前処理データにより a,e,i,o,u のいずれかに変換される
#    3: 英大文字子音
#       −異表記正規化の規定によるラテン文字もしくはキリル文字に該当する文字で、
#       −文字名が /CAPITAL (LETTER|LIGATURE)/ に合致し、
#       −英大文字母音、Y に該当しない
#    4: Y
#       −異表記正規化の規定によるラテン文字もしくはキリル文字に該当する文字で、
#       −文字名が /CAPITAL (LETTER|LIGATURE)/ に合致し、
#       −LIMEDIO向け前処理データにより y に変換される
#    5: 英小文字母音
#       −異表記正規化の規定によるラテン文字もしくはキリル文字に該当する文字で、
#       −文字名が /SMALL (LETTER|LIGATURE)/ に合致し、
#       −LIMEDIO向け前処理データにより a,e,i,o,u のいずれかに変換される
#    6: 英小文字子音
#       −異表記正規化の規定によるラテン文字もしくはキリル文字に該当する文字で、
#       −文字名が /CAPITAL (LETTER|LIGATURE)/ に合致し、
#       −英小文字母音、y に該当しない
#    7: y
#       −異表記正規化の規定によるラテン文字もしくはキリル文字に該当する文字で、
#       −文字名が /SMALL (LETTER|LIGATURE)/ に合致し、
#       −LIMEDIO向け前処理データにより y に変換される
#    8: 音標
#       −異表記正規化の規定による結合文字(準結合文字を含む)に該当する文字で、
#       −文字名が /(GREEK|KATAKANA)/ に該当しない
#    9: ピリオド
#       −002E
#       −もしくはDM向け正規化データにより 002E に変換される文字
#   10: ハイフン
#       −002D
#       −もしくはDM向け正規化データにより 002D に変換される文字
#   11: 空白
#       −0020、0009(TAB)
#       −もしくはDM向け正規化データにより 0020 に変換される文字
#   12: 復帰(CR)
#       −該当なし
#   13: 改行(LF)
#       −該当なし
#
# 実行方法:
#   MakeEmkTbl.pl -s DIR -d DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -d DIR : 中間データの所在 ($datadir) [required]
#    -h     : ヘルプ
#
# 入力ファイル:
#   $srcdir/UnicodeData-1.1.5.txt
#   $datadir/TargetLatin.dat     - ラテン文字テーブル
#   $datadir/TargetCyrillic.dat  - キリル文字テーブル
#   $datadir/TargetCombining.dat - 結合文字テーブル
#   $datadir/preMap.dat          - DM向け前処理テーブル
#   $datadir/postMap.dat         - DM向け後処理テーブル
#   $datadir/preMapLime.dat      - LIMEDIO向け前処理テーブル
#
# 出力ファイルのフォーマット: code<\t>class<\t># name
#   code: 文字コード
#   class: 文字種
#   name: 文字名（コメント）
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
Usage: MakeEmkTbl.pl -s DIR -d DIR
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

$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    $code   = $work[0];    # コード値
    $name   = $work[1];    # 文字名

    if (@latin[hex($code)] !~ /^$/) {
	@latin[hex($code)] = $name;
    }
}
close(FIN);

##########
# キリル文字コードの読み込み
$srcfile = "$datadir/TargetCyrillic.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @cyrillic[hex($_)] = $_;
}
close(FIN);

##########
# 結合文字コードの読み込み
$srcfile = "$datadir/TargetCombining.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @combining[hex($_)] = $_;
}
close(FIN);

##########
# DM向け前処理データの読み込み
$srcfile = "$datadir/preMap.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    $org = $work[0];
    $rep = $work[1];

    @pre[hex($org)] = $rep;
}
close(FIN);

##########
# DM向け後処理データの読み込み
$srcfile = "$datadir/postMap.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    $org = $work[0];
    $rep = $work[1];

    @post[hex($org)] = $rep;
}
close(FIN);

##########
# LIMEDIO向け前処理データの読み込み
$srcfile = "$datadir/preMapLime.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    $org = $work[0];
    $rep = $work[1];

    @pre_lime[hex($org)] = $rep;
}
close(FIN);

##########
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");

    $code   = $work[0];    # コード値
    $name   = $work[1];    # 文字名

    if ($name =~ /(^|^FULLWIDTH )DIGIT [A-Z]+$/) {
	print "$code\t1\t# $name\n"; # 数字

    } elsif (@latin[hex($code)] !~ /^$/ || @cyrillic[hex($code)] !~ /^$/) {

	# LIMEDIO向け前処理結果の取得
	$rep_name = $name;
	if (($rep = @pre_lime[hex($code)]) !~ /^$/) {
	    $rep_name = @latin[hex($rep)];
	}

	if ($rep_name =~ /^LATIN SMALL LETTER Y$/) {
	    if ($name =~ /CAPITAL (LETTER|LIGATURE)/) {
		print "$code\t4\t# $name\n"; # 英字Y
	    } else {
		print "$code\t7\t# $name\n"; # 英字y
	    }

	} elsif ($rep_name =~ /^LATIN SMALL LETTER [AEIOU]$/) {
	    if ($name =~ /CAPITAL (LETTER|LIGATURE)/) {
		print "$code\t2\t# $name\n"; # 英大文字母音
	    } else {
		print "$code\t5\t# $name\n"; # 英小文字母音
	    }

	} elsif ($name =~ /CAPITAL (LETTER|LIGATURE)/) {
	    print "$code\t3\t# $name\n"; # 英大文字子音
	} else {
	    print "$code\t6\t# $name\n"; # 英小文字子音
	}

    } elsif (@combining[hex($code)] !~ /^$/ && $name !~ /(GREEK|KATAKANA)/) {
	print "$code\t8\t# $name\n"; # 音標

    } elsif ($code =~ /^002E$/ || get_rep($code) =~ /^002E$/) {
	print "$code\t9\t# $name\n"; # ピリオド

    } elsif ($code =~ /^002D$/ || get_rep($code) =~ /^002D$/) {
	print "$code\t10\t# $name\n"; # ハイフン

    } elsif ($code =~ /^(0009|0020)$/ || get_rep($code) =~ /^(0009|0020)$/) {
	print "$code\t11\t# $name\n"; # 空白
    }	
}
close(FIN);

##########
# DM向け正規化結果の取得
sub get_rep {
    if (@pre[hex($_)] !~ /^$/) {
	$_ = @pre[hex($_)];
    }
    if (@post[hex($_)] !~ /^$/) {
	$_ = @post[hex($_)];
    }
    return $_;
}

