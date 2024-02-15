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
# exp_inf.pl: 
# ・正規化されていない屈折パターンを展開データとして抽出する
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

# パラメータ評価
use Getopt::Std;
getopts('n:i:h');
if ($opt_h || $#ARGV > 1) {
    die <<EOU;
Usage: exp_inf.pl -n FILE FILE
   -n FILE : norm.dat [required]
   FILE    : inf.dat [required]
   -h      : help
EOU
}
$NORM_FILE = $opt_n if defined $opt_n;
$INF_FILE = $ARGV[$#ARGV];

# 正規化データの読み込み
$N_NORM = 0;
open(FIN1, $NORM_FILE) || die "ERROR: cannot open $NORM_FILE\n";
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

open(FIN2, $INF_FILE) || die "ERROR: cannot open $INF_FILE\n";
while (<FIN2>) {
    chomp;
    ($Fld1,$Fld2) = split(' ', $_, 9999);
    if (&isStemmed($Fld1) == -1) {
	for ($i = 0; $i < (@S = split(/,/, $Fld2, 9999)); $i++) {
	    if (&isStemmed($S[$i]) == -1) {
		$Fld1 =~ s/\@.*\+/\@/;
		$S[$i] =~ s/\@.*\+/\@/;
		print $Fld1, $S[$i];
	    }
	}
    }
}
close(FIN2);

sub isStemmed {
    local($str) = @_;
    $str =~ s/:.+$//;
    &find($str, *NORM, $N_NORM);
}

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
