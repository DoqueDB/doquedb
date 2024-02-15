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
# src_var2.pl: 表記パターンによる異表記の収集２
# ・屈折形、派生形に影響する異形表記を収集
# ・語中もチェックする
# ・品詞のチェックはなし
# 
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

# パラメータ評価
use Getopt::Std;
getopts('p:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: src_var2.pl -p FILE FILE
   -p FILE : var2.pat [required]
   -h      : help
EOU
}

# パターンファイル、処理対象ファイルの指定
$PAT_FILE = $opt_p if defined $opt_p;
$SRC_FILE = $ARGV[$#ARGV];

# パターンファイルを読み込む
$N_PAT = 0;
open(FIN1, $PAT_FILE) || die "ERROR: cannot open $PAT_FILE\n";
while (<FIN1>) {
    chomp;
    if ($_ !~ /^\#/) {
	@s = split(/:/, $_, 9999);
	$N_PAT++;
	$PAT[$N_PAT-1][0] = $s[0];
	$PAT[$N_PAT-1][1] = $s[1];
	$PAT[$N_PAT-1][2] = $s[2];
    }
}
close(FIN1);

# 処理対象ファイルを読み込む
$N_TARGET = 0;
open(FIN2, $SRC_FILE) || die "ERROR: cannot open $SRC_FILE\n";
while (<FIN2>) {
    chomp;
    @s = split(' ', $_, 9999);
    $N_TARGET++;
    $TARGET[$N_TARGET-1][0] = $s[0];
    $TARGET[$N_TARGET-1][1] = $s[1];
}
close(FIN2);

open(FIN2, $SRC_FILE) || die "ERROR: cannot open $SRC_FILE\n";
while (<FIN2>) {
    ($Fld1,$Fld2) = split(' ', $_, 9999);

    for ($i = 0; $i <= $N_PAT-1; $i++) {
	$pat = $PAT[$i][0] . $PAT[$i][1];
	if ($Fld1 =~ $pat) {
	    $var = $Fld1;
            $var =~ s/$PAT[$i][1]/$PAT[$i][2]/;

	    if (&checkDiff($Fld1, $var, $PAT[$i][1], $PAT[$i][2]) == 1) {
		&printVar($Fld1, $var, $Fld2);
	    }
	}
    }
}

sub printVar {
    local($s1, $s2, $s3) = @_;

    if ($s2 ne '' && ($ID = &find($s2, *TARGET, $N_TARGET)) >= 0 &&  $s3 ne '' && $TARGET[$ID][1] =~ $s3) {

	# 異表記ペアを降順に出力
	if (length($s2) > length($s1) || (length($s2) == length($s1) && $s2 gt $s1)) {
	    print $s2, $s1;
	}
	else {
	    print $s1, $s2;
	}
    }
    return;
}

sub checkDiff {
    local($s1, $s2, $s3, $s4) = @_;
    $len_s3 = length($s3);
    $len_s4 = length($s4);
    $len = $len_s1 = length($s1);
    $len_s2 = length($s2);
    if ($len_s2 < $len_s1) {
	$len = $len_s2;
    }
    while ($len > 0) {
	$sub1 = substr($s1, 0, $len_s3);
	$sub2 = substr($s2, 0, $len_s4);
	if ($sub1 eq $s3 && $sub2 eq $s4) {
	    last;
	}
	$s1 =~ s/^.//;
	$s2 =~ s/^.//;
	$len--;
    }
    while ($len > 0) {
	if ($s1 eq $s3 && $s2 eq $s4) {
	    return 1;
	    last;
	}
	$s1 =~ s/.$//;
	$s2 =~ s/.$//;
	$len--;
    }
    0;
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
