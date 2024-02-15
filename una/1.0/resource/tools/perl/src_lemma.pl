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
# src_lemma.pl: 語幹データの抽出
# ・名詞、形容詞、副詞、動詞のベースフォーム
# ・語長が３文字以上
# ・音節数が１以上
# ・出力形式:語形" "品詞（屈折情報なし）
# 
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDIN,  ":utf8");
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

while (<>) {
    ($Fld1,$Fld2,$Fld3,$Fld4) = split(' ', $_, 9999);

    if ($Fld4 =~ /^[NADV]/ && $Fld3 eq '@' && length($Fld1) > 2 && &countV($Fld1) > 0) {
	print $Fld1, substr($Fld4, 0, 1);
    }
}

sub countV {
    local($str) = @_;
    $str = &analyzeCV($str);
    $res = 0;
    for ($count = 0; $count < length($str); $count++) {
	if (substr($str, $count, 1) eq 'V') {
	    $res++;
	}
    }
    $res;
}

sub analyzeCV {
    local($str) = @_;
    $buf = $str;
    $buf =~ s/^y[aeiou]+/CV/;
    $buf =~ s/^y+[^aeiouy]+/VC/;
    $buf =~ s/^[aeiou]+/V/;
    $buf =~ s/^[^CVaeiouy]+/C/;

    while ($buf =~ /[a-z]/) {
	$buf =~ s/Ce$/C/;
	$buf =~ s/C[aeiou]*y+$/CVC/;
	$buf =~ s/V[aeiou]*y+$/VC/;

	if ($buf =~ /V[^CV]/) {
	    $buf =~ s/V[^CVaeiouy]+/VC/;
	    $buf =~ s/Vy+[aeiou]+/VCV/;
	    $buf =~ s/Vy+[^aeiouy]+/VC/;
	}
	else {
	    $buf =~ s/C[aeiou]+/CV/;
	    $buf =~ s/Cy+[aeiou]+/CV/;
	    $buf =~ s/Cy+[^CVaeiouy]+/CVC/;
	}
    }
    $buf;
}
