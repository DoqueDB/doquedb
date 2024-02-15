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
# Old2New.pl
#
# 機能: かなの旧字体から新字体への変換テーブルを作成する
#
# 実行方法:
#   Old2New.pl -s DIR
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
Usage: Old2New.pl -s DIR
   -s DIR : Source data directory [required]
   -h     : help
EOU
}
$srcdir = $opt_s if defined $opt_s;

##########
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

$count_kana = 0;

while (<FIN>) {
    chop;
    @work = split(";");

    $code = $work[0];      # コード
    $name = $work[1];      # 文字名

    # 仮名のコードと文字名を配列に格納
    if ($name =~ /^(HIRAGANA|KATAKANA) LETTER/) {
	@kana[$count_kana] = "$code;$name";
	$count_kana ++;
    }
}
close(FIN);

# 文字名から旧字/新字の対応を抽出
for ($i = 0; $i < $count_kana; $i ++) {

    if (@kana[$i] =~ /;HIRAGANA LETTER WI$/) { # ゐ→い
	for ($j = 0; $j < $count_kana; $j ++) {
	    if (@kana[$j] =~ /;HIRAGANA LETTER I$/) {
		@kana[$i] =~ s/;.+$//;
		@kana[$j] =~ s/;.+$//;
		print "@kana[$i];@kana[$j]\n";
	    }
	}
    } elsif (@kana[$i] =~ /;HIRAGANA LETTER WE$/) { # ゑ→え
	for ($j = 0; $j < $count_kana; $j ++) {
	    if (@kana[$j] =~ /;HIRAGANA LETTER E$/) {
		@kana[$i] =~ s/;.+$//;
		@kana[$j] =~ s/;.+$//;
		print "@kana[$i];@kana[$j]\n";
	    }
	}
    } elsif (@kana[$i] =~ /;KATAKANA LETTER WI$/) { # ヰ→イ
	for ($j = 0; $j < $count_kana; $j ++) {
	    if (@kana[$j] =~ /;KATAKANA LETTER I$/) {
		@kana[$i] =~ s/;.+$//;
		@kana[$j] =~ s/;.+$//;
		print "@kana[$i];@kana[$j]\n";
	    }
	}
    } elsif (@kana[$i] =~ /;KATAKANA LETTER WE$/) { # ヱ→エ
	for ($j = 0; $j < $count_kana; $j ++) {
	    if (@kana[$j] =~ /;KATAKANA LETTER E$/) {
		@kana[$i] =~ s/;.+$//;
		@kana[$j] =~ s/;.+$//;
		print "@kana[$i];@kana[$j]\n";
	    }
	}
    } elsif (@kana[$i] =~ /;KATAKANA LETTER VA$/) { # ワ゛→ヴァ
	for ($j = 0; $j < $count_kana; $j ++) {
	    if (@kana[$j] =~ /;KATAKANA LETTER SMALL A$/) {
		@kana[$i] =~ s/;.+$//;
		@kana[$j] =~ s/;.+$//;
		print "@kana[$i];30F4 @kana[$j]\n";
	    }
	}
    } elsif (@kana[$i] =~ /;KATAKANA LETTER VI$/) { # ヰ゛→ヴィ
	for ($j = 0; $j < $count_kana; $j ++) {
	    if (@kana[$j] =~ /;KATAKANA LETTER SMALL I$/) {
		@kana[$i] =~ s/;.+$//;
		@kana[$j] =~ s/;.+$//;
		print "@kana[$i];30F4 @kana[$j]\n";
	    }
	}
    } elsif (@kana[$i] =~ /;KATAKANA LETTER VE$/) { # ヱ゛→ヴェ
	for ($j = 0; $j < $count_kana; $j ++) {
	    if (@kana[$j] =~ /;KATAKANA LETTER SMALL E$/) {
		@kana[$i] =~ s/;.+$//;
		@kana[$j] =~ s/;.+$//;
		print "@kana[$i];30F4 @kana[$j]\n";
	    }
	}
    } elsif (@kana[$i] =~ /;KATAKANA LETTER VO$/) { # ヲ゛→ヴォ
	for ($j = 0; $j < $count_kana; $j ++) {
	    if (@kana[$j] =~ /;KATAKANA LETTER SMALL O$/) {
		@kana[$i] =~ s/;.+$//;
		@kana[$j] =~ s/;.+$//;
		print "@kana[$i];30F4 @kana[$j]\n";
	    }
	}
    }
}

