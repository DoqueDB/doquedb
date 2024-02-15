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
# src_var_check.pl: 異表記ペアのチェック
# ・削除ファイルに記載されているペアは削除
# ・パターンファイルに合致するペアはOK
# ・語末の差異が以下のいずれかならOK
#   -末尾１文字の有無による差異
#   -差異が"or", "er"
# ・語中の差異が以下のいずれかならOK
#   -語中の1文字の有無による差異
#   -差異部分が1文字
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

# パラメータ評価
use Getopt::Std;
getopts('p:q:d:l:h');
if ($opt_h || $#ARGV > 1) {
    die <<EOU;
Usage: src_var_check.pl -p FILE -q FILE -d FILE -l FILE FILE
   -p FILE : var1.pat [required]
   -q FILE : var2.pat [required]
   -d FILE : ver_del.dat [required]
   -l FILE : src_lemma.dat [required]
   FILE    : var file [required]
   -h      : help
EOU
}

# パターンファイル、削除データの指定
$PAT1_FILE = $opt_p if defined $opt_p; 
$PAT2_FILE = $opt_q if defined $opt_q; 
$DEL_FILE = $opt_d if defined $opt_d; 
$LEM_FILE = $opt_l if defined $opt_l;
$VAR_FILE = $ARGV[$#ARGV];

# パターンファイルの読み込み
$N_PAT1 = 0;
open(FIN1, $PAT1_FILE) || die "ERROR: cannot open $PAT1_FILE\n";
while (<FIN1>) {
    chomp;
    if ($_ !~ /^\#/) {
	@s = split(/:/, $_, 9999);
	$N_PAT1++;
	$PAT1[$N_PAT1-1][0] = $s[0];
	$PAT1[$N_PAT1-1][1] = $s[1];
	$PAT1[$N_PAT1-1][2] = $s[2];
    }
}
close(FIN1);

$N_PAT2 = 0;
open(FIN2, $PAT2_FILE) || die "ERROR: cannot open $PAT2_FILE\n";
while (<FIN2>) {
    chomp;
    if ($_ !~ /^\#/) {
	@s = split(/:/, $_, 9999);
	$N_PAT2++;
	$PAT2[$N_PAT2-1][0] = $s[0];
	$PAT2[$N_PAT2-1][1] = $s[1];
	$PAT2[$N_PAT2-1][2] = $s[2];
    }
}
close(FIN2);

# 削除対象データの読み込み
$n_line = 0;
open(FIN3, $DEL_FILE) || die "ERROR: cannot open $DEL_FILE\n";
while (<FIN3>) {
    chomp;
    if (/^[^\\#]/) {
        ($Fld1,$Fld2) = split(' ', $_, 9999);
        $n_line++;
        $lines[$n_line-1] = $Fld1 . " " . $Fld2;
        $n_line++;
        $lines[$n_line-1] = $Fld2 . " " . $Fld1;
    }
}
close(FIN3);

@sortedlines = sort @lines;
$N_DEL = 0;
$DEL[$N_DEL][0] = $sortedlines[0];
for ($i = 1; $i <= $#sortedlines; $i++){
    if($sortedlines[$i] ne $sortedlines[$i-1]){
        $N_DEL++;
        $DEL[$N_DEL][0] = $sortedlines[$i];
    }
}

# 語幹データの読み込み
$N_LEM = 0;
open(FIN4, $LEM_FILE) || die "ERROR: cannot open $LEM_FILE\n";
while (<FIN4>) {
    chomp;
    @s = split(' ', $_, 9999);
    $N_LEM++;
    $LEM[$N_LEM-1][0] = $s[0];
}
close(FIN4);

open(FIN5, $VAR_FILE) || die "ERROR: cannot open $VAR_FILE\n";
while (<FIN5>) {
    ($Fld1,$Fld2) = split(' ', $_, 9999);

    # 降順に並べ替える
    $H1 = $Fld1;
    $H2 = $Fld2;
    if (length($Fld2) > length($Fld1) || (length($Fld2) == length($Fld1) && $Fld2 gt $Fld1)) {
	$H1 = $Fld2;
	$H2 = $Fld1;
    }

    if (&findPair($H1, $H2, *DEL, $N_DEL+1) == 0 && &find($H1, *LEM, $N_LEM) >= 0 && &find($H2, *LEM, $N_LEM) >= 0 && &checkVar($H1, $H2) == 1) {
	print $Fld1, $Fld1;
	print $Fld1, $Fld2;
	print $Fld2, $Fld2;
	print $Fld2, $Fld1;
    }
}
close(FIN5);

sub findPair {
    local($str1, $str2, *array, $size) = @_;
    if (&find($str1 . ' ' . $str2, *array, $size) >= 0) {
	return 1;
    }
    if (&find($str2 . ' ' . $str2, *array, $size) >= 0) {
	return 1;
    }
    0;
}

sub checkVar {
    local($str1, $str2) = @_;

    # パターンにあっていればOKとする
    for ($C = 0; $C <= $N_PAT1-1; $C++) {
	$pat = $PAT1[$C][0] . $PAT1[$C][1] . "\$";
	$var = substr($str1, 0, length($str1) - length($PAT1[$C][1])) . $PAT1[$C][2];
	if ($str1 =~ $pat && $str2 eq $var) {
	    return 1;
	}
    }

    for ($C = 0; $C <= $N_PAT2-1; $C++) {
	$pat = $PAT2[$C][0] . $PAT2[$C][1];
	$var = $str1;
        $var =~ s/$PAT2[$C][1]/$PAT2[$C][2]/;

	if ($str1 =~ $pat && $str2 eq $var) {
	    return 1;
	}
    }
    $sub1 = $str1;
    $sub2 = $str2;

    # 語頭からの共通文字列を抽出
    $COMM = '';
    while (substr($sub1, 0, 1) eq substr($sub2, 0, 1)) {
	$COMM = $COMM . substr($sub1, 0, 1);
	$sub1 =~ s/^.//;
	$sub2 =~ s/^.//;
    }

    # 以下はOK
    # ・末尾1文字の有無による差異
    # ・差異が"or", "er"
    if ($sub1 =~ /^.$/ && $sub2 eq '' || $sub1 eq 'or' && $sub2 eq 'er') {
	return 1;
    }

    # 差異文字列から末尾からの共通部分を削除
    while (substr($sub1, length($sub1)-1) eq substr($sub2, length($sub2)-1)) {
	$sub1 =~ s/.$//;
	$sub2 =~ s/.$//;
    }

    # 以下はOK
    # ・語中1文字の有無による差異
    # ・差異部分が１文字
    if ($sub1 =~ /^.$/ && $sub2 eq '' || $sub1 =~ /^.$/ && $sub2 =~ /^.$/) {
	return 1;
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
