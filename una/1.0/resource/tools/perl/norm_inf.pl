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
# norm_inf.pl: 屈折形正規化
# ・以下は正規化しない。
#   -自分自身が形容詞/副詞の屈折形、不規則屈折形
#   -自身と原形の屈折パターンが一致しない
#   -原形が、形容詞/副詞の屈折形もしくは不規則屈折形と曖昧な場合
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

# パラメータ評価
use Getopt::Std;
getopts('e:s:i:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: norm_inf.pl -e FILE -s FILE -i FILE
   -e FILE : form_exc.dat [required]
   -s FILE : form.dat [required]
   -i FILE : inf.dat [required]
   -h      : help
EOU
}
$EXC_FILE = $opt_e if defined $opt_e;
$SRC_FILE = $opt_s if defined $opt_s;
$INF_FILE = $opt_i if defined $opt_i;

# 強制的に正規化する語形リスト
$N_EXC = 0;
open(FIN1, $EXC_FILE) || die "ERROR: cannot open $EXC_FILE\n";
while (<FIN1>) {
    chomp;
    $N_EXC++;
    $EXC[$N_EXC - 1][0] = $_;
}
close(FIN1);

# 入力データ
$N_SRC = 0;
open(FIN2, $SRC_FILE) || die "ERROR: cannot open $SRC_FILE\n";
while (<FIN2>) {
    chomp;
    @s = split(' ', $_, 9999);
    if (&find($s[0], *EXC, $N_EXC) >= 0) {
	$s[1] =~ s/^[a-z]+:\@,//;
	$s[1] =~ s/,[a-z]+:\@$//;
	$s[1] =~ s/,[a-z]+:\@,/,/;
    }
    $N_SRC++;
    $SRC[$N_SRC - 1][0] = $s[0];
    $SRC[$N_SRC - 1][1] = $s[1];
}
close(FIN2);

# 屈折パターン
$N_INF = 0;
open(FIN3, $INF_FILE) || die "ERROR: cannot open $INF_FILE\n";
while (<FIN3>) {
    chomp;
    @s = split(' ', $_, 9999);
    $N_INF++;
    $INF[$N_INF - 1][0] = $s[0];
    $INF[$N_INF - 1][1] = $s[1];
}
close(FIN3);

# 強制的に原形に正規化する語形
open(FIN2, $SRC_FILE) || die "ERROR: cannot open $SRC_FILE\n";
while (<FIN2>) {
    chomp;
    ($Fld1,$Fld2) = split(' ', $_, 9999);
    if (&find($Fld1, *EXC, $N_EXC) >= 0) {
	# 正規化語形の曖昧性を解消
	$Fld2 =~ s/^[a-z]+:\@,//;
	$Fld2 =~ s/,[a-z]+:\@$//;
	$Fld2 =~ s/,[a-z]+:\@,/;/;
    }
    if ($Fld2 =~ /[*+]/) {
	# 形容詞/副詞の屈折形、不規則屈折形は正規化しない
	if ($Fld2 =~ /(\*|\+(er|est)(,|$))/) {
	    $Fld2 = $Fld1 . ':@';
	}
	else {
	    for ($i = 0; $i < (@S = split(/,/, $Fld2, 9999)); $i++) {
		$BASE = $PAT = $S[$i];
		$BASE =~ s/:.+$//;
		$PAT =~ s/^.+://;

		$F1 = &find($Fld1 . ':' . $PAT, *INF, $N_INF);
		$F2 = &find($BASE . ':@', *INF, $N_INF);

		if ($F1 >= 0 && &find($BASE, *EXC, $N_EXC) == -1) {
		    # 自身と原形の屈折パターンが一致しない場合は正規化しない
		    if ($INF[$F1][1] ne $INF[$F2][1]) {
			$Fld2 = $Fld1 . ':@';
			last;
		    }
      		    # 原形が形容詞/副詞の屈折形もしくは不規則屈折形と曖昧な場合は正規化しない
		    elsif (($F = &find($BASE, *SRC, $N_SRC)) >= 0 && $SRC[$F][1] =~ /(\*|\+(er|est)(,|$))/) {
			$Fld2 = $Fld1 . ':@';
			last;
		    }
		}
	    }
	}
    }
    for ($i = 0; $i < (@S = split(/,/, $Fld2, 9999)); $i++) {
	$S[$i] =~ s/:.+$//;
	print $Fld1, $S[$i];
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
