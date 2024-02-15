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
###############################################################################
# 
# src_form.pl: 語形単位のレコードに分割
# ・出力形式:スペースをFSとする以下の4フィールド
#   1: 語形
#   2: 原形
#   3: 屈折パターン
#   4: 品詞
# ・"-s","-ed","-ing","-er","-est"以外の屈折は不規則パターンに変換
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDIN,  ":utf8");
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

$FS = '#';

while (<>) {
    chomp;	# strip record separator
    @Fld = split(/[#\n]/, $_, 9999);

    $CAT = $FORM = $STEM = $INF = '';

    # ベースフォームあり
    if ($Fld[0] ne '') {
	@BASE = split(/\|/, $Fld[0], 9999);

	$FORM = $STEM = &lower($BASE[0]);
	$CAT = $BASE[1];
	$INF = &convInf($BASE[2]);

	if ($FORM !~ /[^A-Za-z]/) {
	    print $FORM, $STEM, $INF, $CAT;
	}

	for ($i = 1; $i <= $#Fld; $i++) {
	    if ($Fld[$i] ne '') {
		for ($j = 0; $j < (@PAT = split(/;/, $Fld[$i], 9999)); $j++) {
		    @P = split(/\|/, $PAT[$j], 9999);
		    $P[2] =~ s/^@ //;

		    $FORM = &lower($P[0]);
		    $STEM = &lower($BASE[0]);
		    $CAT = $P[1];
		    $INF = &convInf($P[2]);

		    if ($FORM !~ /[^A-Za-z]/) {
			print $FORM, $STEM, $INF, $CAT;
		    }
		}
	    }
	}
    }

    # ベースフォームなし
    else {
	for ($i = 1; $i <= $#Fld; $i++) {
	    if ($Fld[$i] ne '') {
		for ($j = 0; $j < (@PAT = split(/;/, $Fld[$i], 9999)); $j++) {
		    @P = split(/\|/, $PAT[$j], 9999);
		    $P[2] =~ s/^@ //;

		    $FORM = $STEM = &lower($P[0]);
		    $CAT = $P[1];
		    $INF = &convInf($P[2]);

		    if ($FORM !~ /[^A-Za-z]/) {
			print $FORM, $STEM, $INF, $CAT;
		    }
		}
	    }
	}
    }
}

# 外来屈折コードを不規則屈折コードに変換
sub convInf {
    local($str) = @_;
    if ($str =~ /\+/ && $str !~ /\+(s|ing|ed|er|est)$/) {
	$str = '*';
    }
    $str;
}

# 小文字化
sub lower {
    local($str) = @_;
    $ret = '';
    for ($c = 0; $c < length($str); $c++) {
	$ret = $ret . &tolower(substr($str, $c, 1));
    }
    $ret;
}

#
sub tolower {
  local($str) = @_;
  $str = "\L$str";
  $str;
}

