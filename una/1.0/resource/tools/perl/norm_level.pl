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
# norm_level.pl: 不完全な正規化語形がないように平準化する
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

# パラメータ評価
use Getopt::Std;
getopts('h');
if ($opt_h || $#ARGV > 1) {
    die <<EOU;
Usage: norm_level.pl FILE
   FILE    : norm_var.dat [required]
   -h      : help
EOU
}

# 正規化データの指定
$SRC_FILE = $ARGV[$#ARGV];

# 正規化データの読み込み
$N_NORM = 0;
open(FIN1, $SRC_FILE) || die "ERROR: cannot open $SRC_FILE\n";
while (<FIN1>) {
    chomp;
    @S = split(' ', $_, 9999);
    if ($S[0] ne $S[1]) {
	$N_NORM++;
	$NORM[$N_NORM-1][0] = $S[0];
	$NORM[$N_NORM-1][1] = $S[1];
    }
}
close(FIN1);

open(FIN1, $SRC_FILE) || die "ERROR: cannot open $SRC_FILE\n";
while (<FIN1>) {
    chomp;	# strip record separator
    @Fld = split(' ', $_, 9999);

    if (($F_NORM = &find($Fld[1], *NORM, $N_NORM)) >= 0) {
	if ($Fld[1] ne $NORM[0][1]) {
	    $Fld[1] = $NORM[$F_NORM][1];
	}
    }
    print join($,,@Fld);
}
close(FIN1);

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
