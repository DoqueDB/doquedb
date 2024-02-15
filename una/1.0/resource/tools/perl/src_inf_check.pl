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
# src_inf_check.pl: 屈折情報のエラー修正
# ・エントリの異表記数と活用形の異表記数が合わないレコード
# ・Celexとの記述ずれを修正
# ・以下の記述ミスをチェック/
#   -原形末尾"[cs]h","[szx]"に"@+s"を付加
#   -原形末尾"[cs]h","[szx]"以外に"@+es"を付加
#   -原形末尾"[^e]"に"@+d"を付加
#   -原形末尾"e"に"@+ed"を付加
#   -原形末尾"[^e]"に"@-e+ing"が付加
#   -原形末尾"e"に"@+ing"を付加
#   -原形末尾"[^aeiou]y"に"@+s","@+ed"が付加
#   -原形末尾"[aeiou]y"に"@-y+ies","@-y+ied"が付加
# ・出力形式は入力形式と同じ
# 
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDIN,  ":utf8");
binmode(STDOUT, ":utf8");

$FS = ' ';		# set field separator
$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

$FS = $, = '#';

while (<>) {
    chomp;	# strip record separator
    @Fld = split($FS, $_, 9999);

    @BASE = split(/\|/, $Fld[0], 9999);
    $lenB = length($BASE[0]);

    for ($i = 1; $i <= $#Fld; $i++) {
	$N = (@VAR = split(/;/, $Fld[$i], 9999));

	for ($j = 0; $j < $N; $j++) {
	    @SUF = split(/\|/, $VAR[$j], 9999);
	    $lenS = length($SUF[0]);

	    $REPLACED = 0;

	    ##### AC
	    if ($SUF[1] eq 'AC') {
		if ($BASE[0] =~ /ey$/ && $SUF[2] eq '@-y+i+er') {
		    $STEM = substr($BASE[0], 0, $lenB - 2);
		    $PAT = '@-ey+i+er';
		    $VAR[$j] = $STEM . 'ier|' . $SUF[1] . '|' . $PAT;
		    $REPLACED = 1;
		}
	    }
	    ##### AC

	    ##### AS
	    elsif ($SUF[1] eq 'AS') {
		if ($BASE[0] =~ /ey$/ && $SUF[2] eq '@-y+i+est') {
		    $STEM = substr($BASE[0], 0, $lenB - 2);
		    $PAT = '@-ey+i+est';
		    $VAR[$j] = $STEM . 'iest|' . $SUF[1] . '|' . $PAT;
		    $REPLACED = 1;
		}
	    }
	    ##### AS

	    ##### NP/V3
	    elsif ($SUF[1] eq 'NP' || $SUF[1] eq 'V3') {
		if ($SUF[2] eq '@+s' && $BASE[0] =~ /s$/) {
		    $STEM = $BASE[0];
		    $PAT = '@+e+s';
		    $VAR[$j] = $STEM . 'es|' . $SUF[1] . '|' . $PAT;
		    $REPLACED = 1;
		}
		elsif ($SUF[2] eq '@+e+s' && $BASE[0] =~ /e$/) {
		    $STEM = $BASE[0];
		    $PAT = '@+s';
		    $VAR[$j] = $STEM . 's|' . $SUF[1] . '|' . $PAT;
		    $REPLACED = 1;
		}
	    }
	    ##### NP/V3

	    ##### VD/VN
	    elsif ($SUF[1] eq 'VD' || $SUF[1] eq 'VN') {
		if ($SUF[2] eq '@-e+ed' && $BASE[0] !~ /e$/) {
		    $STEM = $BASE[0];
		    $PAT = '@+ed';
		    $VAR[$j] = $STEM . 'ed|' . $SUF[1] . '|' . $PAT;
		    $REPLACED = 1;
		}
	    }
	    ##### VD/VN

	    if ($REPLACED == 1) {
		$Fld[$i] = '';
		for ($k = 0; $k < $N; $k++) {
		    $Fld[$i] = $Fld[$i] . ';' . $VAR[$k];
		}
		$Fld[$i] =~ s/^;//;
	    }
	}
	### for(j=0; j<N; j++)

	;
    }

    print join($,,@Fld);
}
