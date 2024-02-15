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
# norm_var.pl: 
# ・屈折形に対応する原形が異表記の場合、以下の要領でいずれかに正規化する
#   -長さが異なる場合は短い方に正規化
#   -長さが同じ場合はアルファベット順の若い方に正規化
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

# パラメータ評価
use Getopt::Std;
getopts('s:h');
if ($opt_h || $#ARGV > 1) {
    die <<EOU;
Usage: norm_var.pl -s FILE FILE
   -s FILE : src_var.dat [required]
   FILE    : norm_inf.dat [required]
   -h      : help
EOU
}

# 異表記データの指定
$VAR_FILE = $opt_s if defined $opt_s;
$INF_FILE = $ARGV[$#ARGV];

# 異表記データの読み込み
$N_VAR = 0;
open(FIN1, $VAR_FILE) || die "ERROR: cannot open $VAR_FILE\n";
while (<FIN1>) {
    @s = split(' ', $_, 9999);
    $N_VAR++;
    $VAR[$N_VAR-1][0] = $s[0];
    $VAR[$N_VAR-1][1] = $s[1];
}
close(FIN1);

open(FIN2, $INF_FILE) || die "ERROR: cannot open $INF_FILE\n";
while (<FIN2>) {
    chomp;	# strip record separator
    @Fld = split(' ', $_, 9999);

    if ($Fld[1] =~ /,/) {
	$N = (@S = split(/,/, $Fld[1], 9999));

	if (($F = &find($S[0], *VAR, $N_VAR)) >= 0) {
	    $V1 = $VAR[$F][1];

	    # 原形同士が異表記関係になければ、カウンタを-1にして終了
	    for ($i = 1; $i < $N; $i++) {
		if (($F = &find($S[$i], *VAR, $N_VAR)) == -1 || $VAR[$F][1] ne $V1) {
		    $i = -1;
		    last;
		}
	    }
	    if ($i >= 0) {
		# 異表記の場合は、指定の表記に正規化する
		print $Fld[0], &getVar($Fld[1]);
	    }
	    # 異表記でなければ、正規化しない
	    else {
		print $Fld[0], $Fld[0];
	    }
	}
	else {
	    print $Fld[0], $Fld[0];
	}
    }
    else {
	print $_;
    }
}
close(FIN2);

sub getVar {
    local($str) = @_;
    $num = (@SPL = split(/,/, $str, 9999));
    $var = $SPL[0];
    $len = length($var);

    for ($i = 1; $i < $num; $i++) {
	$comm = &getComm($var, $SPL[$i]);
	$sub1 = substr($var, length($comm), 999999);
	$sub2 = substr($SPL[$i], length($comm), 999999);

	# 長さが同じ
	if ($len == length($SPL[$i])) {
	    # realise -> realize
	    if ($comm =~ /[iy]$/ && $sub1 =~ /^z/ && $sub2 =~ /^s/) {
		;
	    }
	    elsif ($comm =~ /[iy]$/ && $sub1 =~ /^s/ && $sub2 =~ /^z/) {
		$var = $SPL[$i];
		$len = length($SPL[$i]);
	    }
	    elsif ($var gt $SPL[$i]) {
		# デフォルトは昇順に正規化
		$var = $SPL[$i];
		$len = length($SPL[$i]);
	    }
	}

	# 長さが異なる
	else {
	    # centring -> centering
	    # connexion -> connection
	    if (($sub1 =~ /^ering/ && $sub2 =~ /^ring/) || ($sub1 =~ /^ction/ && $sub2 =~ /^xion/)) {
		;
	    }
	    elsif (($sub1 =~ /^ring/ && $sub2 =~ /^ering/) || ($sub1 =~ /^xion/ && $sub2 =~ /^ction/)) {
		$var = $SPL[$i];
		$len = length($SPL[$i]);
	    }
	    elsif ($len > length($SPL[$i])) {
		# デフォルトは短い方に正規化
		$var = $SPL[$i];
		$len = length($SPL[$i]);
	    }
	}
    }
    $var;
}

sub getComm {
    local($str1, $str2) = @_;
    $ret = '';
    while (substr($str1, 0, 1) eq substr($str2, 0, 1) && $str1 ne '' &&	$str2 ne '') {
	$ret = $ret . substr($str1, 1, 1);
	$str1 =~ s/^.//;
	$str2 =~ s/^.//;
    }
    $ret;
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
