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
# exp_merge_inf.pl: 
# ・異表記パターンに屈折形展開パターンをマージする
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

# パラメータ評価
use Getopt::Std;
getopts('i:h');
if ($opt_h || $#ARGV > 1) {
    die <<EOU;
Usage: exp_merge_inf.pl -i FILE FILE
   -i FILE : exp_inf_var.dat [required]
   FILE    : src_var.dat [required]
   -h      : help
EOU
}
$INF_FILE = $opt_i if defined $opt_i;
$SRC_FILE = $ARGV[$#ARGV];

# 屈折形展開データ
$N_INF = 0;
open(FIN1, $INF_FILE) || die "ERROR: cannot open $INF_FILE\n";
while (<FIN1>) {
    chomp;
    @s = split(' ', $_, 9999);
    $N_INF++;
    $INF[$N_INF-1][0] = $s[0];
    $INF[$N_INF-1][1] = $s[1];
}
close(FIN1);

open(FIN2, $SRC_FILE) || die "ERROR: cannot open $SRC_FILE\n";
while (<FIN2>) {
    chomp;
    ($Fld1,$Fld2) = split(' ', $_, 9999);

    for ($i = 0; $i < (@S1 = split(/,/, $Fld2, 9999)); $i++) {
	if (($F = &find($S1[$i], *INF, $N_INF)) >= 0) {
	    for ($j = 0; $j < (@S2 = split(/,/, $INF[$F][1], 9999)); $j++) {
		print $Fld1, $S2[$j];
	    }
	}
	else {
	    print $Fld1, $S1[$i];
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
