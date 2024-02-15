#!/usr/bin/perl
# 
# Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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
# MakePreMap.pl
#
# 機能: 前処理テーブルの作成
#
# 実行方法:
#   MakePreMap.pl -d DIR [OPTIONS] | sort -u > 出力ファイル名
#    -d DIR  : 中間データの所在 ($datadir) [required]
#    -h      : ヘルプ
#
# 入力ファイル:
#   $datadir/Cap2Small.dat          - 大文字/小文字変換テーブル
#   $datadir/Full2Half.dat          - 全角/半角変換テーブル
#   $datadir/Half2Full.dat          - 半角/全角変換テーブル
#   $datadir/Small2Basic.dat        - 小字形/標準形変換テーブル
#   $datadir/Old2New.dat            - かな旧字体/新字体変換テーブル
#   $datadir/Hira2Kata.dat          - ひらがな/カタカナ変換テーブル
#   $datadir/DecompLatin.dat        - ラテン文字分解テーブル
#   $datadir/DecompCyrillic.dat     - キリル文字分解テーブル
#   $datadir/DecompGreek.dat        - ギリシャ文字分解テーブル
#   $datadir/NormKmap.dat           - 漢字異体字変換テーブル
#   $datadir/TargetLatin.dat        - ラテン文字テーブル
#   $datadir/TargetCyrillic.dat     - キリル文字テーブル
#   $datadir/TargetGreek.dat        - ギリシャ文字テーブル
#   $datadir/TargetKana.dat         - 仮名文字テーブル
#   $datadir/TargetCombining.dat    - 結合文字テーブル
#   $datadir/TargetControl.dat      - 制御文字テーブル
#
# 出力ファイルのフォーマット: code1;code2
#   code1: 変換前コード
#   code2: 変換後コード列
#
################################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$count_misc = 0;

##########
# パラメータ評価
use Getopt::Std;
getopts('d:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: MakePreMap.pl -d DIR [OPTIONS]
   -d DIR  : Intermediate data directory [required]
   -h      : help
EOU
}
$datadir = $opt_d if defined $opt_d;

##########
# 大文字/小文字変換テーブルの読み込み
$srcfile = "$datadir/Cap2Small.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");
    $org = $work[0];
    $rep = $work[1];
    @cap2small[hex($org)] = $rep;
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
    @full2half[hex($org)] = $rep;
    @misc[$count_misc] = "$org;$rep";
    $count_misc ++;
}
close(FIN);

##########
# 半角/全角変換テーブルの読み込み
$srcfile = "$datadir/Half2Full.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");
    $org = $work[0];
    $rep = $work[1];
    if (0) {
		# ※特定モードで使用
        if($org =~/^FFE.$/) {
            @half2full[hex($org)] = $rep;
            @misc[$count_misc] = "$org;$rep";
            $count_misc ++;
        }
    } else {
        @half2full[hex($org)] = $rep;
        @misc[$count_misc] = "$org;$rep";
        $count_misc ++;
    }
}
close(FIN);

##########
# 小字形/標準形変換テーブルの読み込み
$srcfile = "$datadir/Small2Basic.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");
    $org = $work[0];
    $rep = $work[1];
    @small2basic[hex($org)] = $rep;
    @misc[$count_misc] = "$org;$rep";
    $count_misc ++;
}
close(FIN);

##########
# かな旧字体/新字体変換テーブルの読み込み
$srcfile = "$datadir/Old2New.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
	chop;
	@work = split(";");
	$org = $work[0];
	$rep = $work[1];
	@old2new[hex($org)] = $rep;
}
close(FIN);

##########
# ひらがな/カタカナ変換テーブルの読み込み
# ※特定モードで使用
if (0) {
    $srcfile = "$datadir/Hira2Kata.dat";
    open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

    while (<FIN>) {
        chop;
        @work = split(";");
        $org = $work[0];
        $rep = $work[1];
        @hira2kata[hex($org)] = $rep;
    }
    close(FIN);
}

##########
# ラテン文字分解テーブルの読み込み
$srcfile = "$datadir/DecompLatin.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
	chop;
	@work = split(";");
	$org = $work[0];
	$rep = $work[1];
	@decomp_latin[hex($org)] = $rep;
}
close(FIN);

##########
# キリル文字分解テーブルの読み込み
$srcfile = "$datadir/DecompCyrillic.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
	chop;
	@work = split(";");
	$org = $work[0];
	$rep = $work[1];
	@decomp_cyrillic[hex($org)] = $rep;
}
close(FIN);

##########
# ギリシャ文字分解テーブルの読み込み
$srcfile = "$datadir/DecompGreek.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";
while (<FIN>) {
	chop;
	@work = split(";");
	$org = $work[0];
	$rep = $work[1];
	@decomp_greek[hex($org)] = $rep;
}
close(FIN);

##########
# 漢字異体字変換データの出力、格納
$srcfile = "$datadir/NormKmap.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");
    
    $org = $work[0];
    $rep = $work[1];

    if ($org !~ /^$rep$/) {
        print "$org;$rep\n";
        @pre[hex($org)] = $rep;
    }
}
close(FIN);

##########
# ラテン文字変換データの出力、格納
$srcfile = "$datadir/TargetLatin.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    $org = $rep = $_;

    # 分解
    if (($decomp = @decomp_latin[hex($rep)]) !~ /^$/) {
        $rep = $decomp;
    }

    # 構成文字毎に正規化
    @rep_chars = split(" ", $rep);
    $rep = "";
    foreach $rep_char (@rep_chars) {
    # 全角→半角
        if (($rep2half = @full2half[hex($rep_char)]) !~ /^$/) {
            $rep_char = $rep2half;
        }
    # 大文字→小文字
        if (($rep2small = @cap2small[hex($rep_char)]) !~ /^$/) {
            $rep_char = $rep2small;
        }
    # 再分解（これをしないと00D0が完全に正規化できない）
        if (($rep2decomp = @decomp_latin[hex($rep_char)]) =~ /^....$/) {
            $rep_char = $rep2decomp;
        }
        $rep = "$rep $rep_char";
    }
    $rep =~ s/^ //;

    if ($org !~ /^$rep$/) {
        print "$org;$rep\n";
        @pre[hex($org)] = $rep;
    }
}
close(FIN);

##########
# キリル文字変換データの出力、格納
$srcfile = "$datadir/TargetCyrillic.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    $org = $rep = $_;

    # 分解
    if (($decomp = @decomp_cyrillic[hex($rep)]) !~ /^$/) {
        $rep = $decomp;
    }

    # 構成文字毎に正規化
    @rep_chars = split(" ", $rep);
    $rep = "";
    foreach $rep_char (@rep_chars) {
    # 全角→半角
        if (($rep2half = @full2half[hex($rep_char)]) !~ /^$/) {
            $rep_char = $rep2half;
        }
    # 大文字→小文字
        if (($rep2small = @cap2small[hex($rep_char)]) !~ /^$/) {
            $rep_char = $rep2small;
        }
    # 再分解（影響はない）
        if (($rep2decomp = @decomp_cyrillic[hex($rep_char)]) =~ /^....$/) {
            $rep_char = $rep2decomp;
        }
        $rep = "$rep $rep_char";
    }
    $rep =~ s/^ //;

    if ($org !~ /^$rep$/) {
        print "$org;$rep\n";
        @pre[hex($org)] = $rep;
    }
}
close(FIN);

##########
# ギリシャ文字変換データの出力、格納
$srcfile = "$datadir/TargetGreek.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    $org = $rep = $_;

    # 分解
    if (($decomp = @decomp_greek[hex($rep)]) !~ /^$/) {
        $rep = $decomp;
    }

    # 構成文字毎に正規化
    @rep_chars = split(" ", $rep);
    $rep = "";
    foreach $rep_char (@rep_chars) {
    # 全角→半角
        if (($rep2half = @full2half[hex($rep_char)]) !~ /^$/) {
            $rep_char = $rep2half;
        }
    # 大文字→小文字
        if (($rep2small = @cap2small[hex($rep_char)]) !~ /^$/) {
            $rep_char = $rep2small;
        }
    # 再分解（影響はない）
        if (($rep2decomp = @decomp_greek[hex($rep_char)]) =~ /^....$/) {
            $rep_char = $rep2decomp;
        }
        $rep = "$rep $rep_char";
    }
    $rep =~ s/^ //;

    if ($org !~ /^$rep$/) {
        print "$org;$rep\n";
        @pre[hex($org)] = $rep;
    }
}
close(FIN);

##########
# かな変換データの出力、格納
$srcfile = "$datadir/TargetKana.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    $org = $rep = $_;

    # 半角→全角
	if (($rep2full = @half2full[hex($rep)]) !~ /^$/) {
		$rep = $rep2full;
	}

    # 旧字→新字
	if (($rep2new = @old2new[hex($rep)]) !~ /^$/) {
		$rep = $rep2new;
	}

    # ひらがな→カタカナ
	# ※特定モードで使用
    if (0) {
        if (($rep2kata = @hira2kata[hex($rep)]) !~ /^$/) {
            $rep = $rep2kata;
        }
    }

    if ($org !~ /^$rep$/) {
        print "$org;$rep\n";
        @pre[hex($org)] = $rep;
    }
}
close(FIN);

##########
# 結合文字削除データの出力、格納
$srcfile = "$datadir/TargetCombining.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;

    $org = $_;
    if (1) {
        $rep = "0000";
    } else {
		# 結合文字/合字処理対応のため変換先へunassignedのコードを割り当てる
		# ※特定モードで使用
        $rep = "077F"; 
    }
    print "$org;$rep\n";
    @pre[hex($org)] = $rep;
}
close(FIN);

##########
# 制御文字削除データの出力、格納
$srcfile = "$datadir/TargetControl.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;

    $org = $_;
    $rep = "0000";

    if (0) {
		# 制御文字を削除しない
		# ※特定モードで使用
        ;
    } elsif (0 && $org !~ /^000[AD]$/) {
		# 改行以外の制御文字を削除する
		# ※特定モードで使用
        print "$org;$rep\n";
        @pre[hex($org)] = $rep;
    } else {
        print "$org;$rep\n";
        @pre[hex($org)] = $rep;
    }
}
close(FIN);

##########
# 出力されていない全角、半角、小字形正規化データを出力する
for ($i = 0; $i < $count_misc; $i ++) {
    @codes = split(";", @misc[$i]);

    $org = @codes[0];
    $rep = @codes[1];

    if (@pre[hex($org)] =~ /^$/) {
        # 変換先を処理済みの結果に合わせる（該当データなし）
        if (($rep_pre = @pre[hex($rep)]) !~ /^$/) {
            $rep = $rep_pre;
        }
        print "$org;$rep\n";
    }
}
