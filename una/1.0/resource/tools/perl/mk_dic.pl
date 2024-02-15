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
# mk_dic.pl: 辞書データを作る
# ・正規化データと展開データを結合する
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

# パラメータ評価
use Getopt::Std;
getopts('e:h');
if ($opt_h || $#ARGV > 1) {
    die <<EOU;
Usage: mk_dic.pl -e FILE FILE
   -e FILE : exp.dat [required]
   FILE    : norm.dat [required]
   -h      : help
EOU
}

# データの指定
$EXP_FILE = $opt_e if defined $opt_e;
$NORM_FILE = $ARGV[$#ARGV];

# 展開データの読み込み
$N_EXP = 0;
open(FIN1, $EXP_FILE) || die "ERROR: cannot open $EXP_FILE\n";
while (<FIN1>) {
    chomp;
    @s = split(' ', $_, 9999);
    $N_EXP++;
    $EXP[$N_EXP-1][0] = $s[0];
    $EXP[$N_EXP-1][1] = $s[1];
}
close(FIN1);

open(FIN2, $NORM_FILE) || die "ERROR: cannot open $NORM_FILE\n";
while (<FIN2>) {
    ($Fld1,$Fld2) = split(' ', $_, 9999);

    $expand = $Fld2;
    if (($F_EXP = &find($Fld2, *EXP, $N_EXP)) >= 0) {
	$expand = $EXP[$F_EXP][1];
    }
    print $Fld1, $Fld2, $expand;
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
