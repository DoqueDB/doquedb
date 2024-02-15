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
# exp_merge_var.pl: 
# ・屈折形展開パターンに異表記パターンをマージする
#
###############################################################################
use utf8;
use open IN  => ":utf8";
use open OUT => ":utf8";
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

# パラメータ評価
use Getopt::Std;
getopts('s:i:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: exp_merge_var.pl -s FILE -i FILE
   -s FILE : src_var.dat [required]
   -i FILE : exp_inf.dat [required]
   -h      : help
EOU
}
$VAR_FILE = $opt_s if defined $opt_s;
$INF_FILE = $opt_i if defined $opt_i;

# 異表記データ
$N_VAR = 0;
open(FIN1, $VAR_FILE) || die "ERROR: cannot open $VAR_FILE\n";
while (<FIN1>) {
    chomp;
    @s = split(' ', $_, 9999);
    $N_VAR++;
    $VAR[$N_VAR-1][0] = $s[0];
    $VAR[$N_VAR-1][1] = $s[1];
}
close(FIN1);

# 屈折形展開データ
$N_INF = 0;
open(FIN2, $INF_FILE) || die "ERROR: cannot open $INF_FILE\n";
while (<FIN2>) {
    chomp;
    @s = split(' ', $_, 9999);
    $N_INF++;
    $INF[$N_INF-1][0] = $s[0];
    $INF[$N_INF-1][1] = $s[1];
}
close(FIN2);

open(FIN2, $INF_FILE) || die "ERROR: cannot open $INF_FILE\n";
while (<FIN2>) {
    chomp;
    ($Fld1,$Fld2) = split(' ', $_, 9999);

    $Fld1 =~ s/:.*$//;

    for ($i = 0; $i < (@S = split(/,/, $Fld2, 9999)); $i++) {
	$FORM = $PAT = $S[$i];
	$FORM =~ s/:.*$//;
	$PAT =~ s/^.*://;

	# 異表記が存在する
	if (($F_VAR = &find($FORM, *VAR, $N_VAR)) >= 0) {
	    for ($j = 0; $j < (@S_VAR = split(/,/, $VAR[$F_VAR][1], 9999)); $j++) {
		print $Fld1, $S_VAR[$j]; # 自分自身も含まれる
		# 異表記に対する屈折形展開パターンも出力する
		$KEY = $S_VAR[$j] . ':' . $PAT;
		if (($F_INF = &find($KEY, *INF, $N_INF)) >= 0) {
		    for ($k = 0; $k < (@S_INF = split(/,/, $INF[$F_INF][1], 9999)); $k++) {
			$FORM_INF = $PAT_INF = $S_INF[$k];
			$FORM_INF =~ s/:.*$//;
			$PAT_INF =~ s/^.*://;

			# 屈折パターンが異なる語形は排除する
                        $PAT1 = $Fld2 . ",";
                        $PAT2 = ":" . $PAT_INF . ",";
			if ($PAT1 =~ $PAT2) {
			    print $Fld1, $FORM_INF;
			}
		    }
		}
	    }
	}
        # 異表記が存在しない → 自身との対応のみ出力
	else {
	    print $Fld1, $FORM;
	}
    }
}
close(FIN2);

sub find {
    local($string, *array, $size) = @_;
    $L = 0;
    $H = $size - 1;

    while ($L <= $H) {
	$M = int(($L + $H) / 2);
	if ($string eq $array[$M][0]) {
	    return $M;
	}

	if ($string lt $array[$M][0]) {
	    $H = $M - 1;
	}
	else {
	    $L = $M + 1;
	}
    }
    -1;
}
