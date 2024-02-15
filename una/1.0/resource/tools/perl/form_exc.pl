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
# form_exc.pl: 正規化対象とする語形のチェック
# ・強制的に正規化すると判断された語形データ(check_form.dat)から、
#   正規化前後の語形に対応する屈折パターンが一致しないものを除外する
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

# パラメータ評価
use Getopt::Std;
getopts('t:i:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: form_exc.pl -t FILE -i FILE 
   -t FILE : tmpfile [required]
   -i FILE : inf.dat [required]
   -h      : help
EOU
}

# 屈折パターン
$TMP_FILE = $opt_t if defined $opt_t;
$INF_FILE = $opt_i if defined $opt_i;
$N_INF = 0;
open(FIN1, $INF_FILE) || die "ERROR: cannot open $INF_FILE\n";
while (<FIN1>) {
    chomp;
    @S = split(' ', $_, 9999);
    $N_INF++;
    $INF[$N_INF-1][0] = $S[0];
    $INF[$N_INF-1][1] = $S[1];
}
close(FIN1);

open(FIN2, $TMP_FILE) || die "ERROR: cannot open $TMP_FILE\n";
while (<FIN2>) {
    ($Fld1,$Fld2) = split(' ', $_, 9999);

    for ($i = 0; $i < (@S = split(/,/, $Fld2, 9999)); $i++) {
	$FORM = $BASE = $S[$i];
	($s_ = '"'.($Fld1 . ':').'"') =~ s/&/\$&/g;

	$FORM =~ s/^.+:/eval $s_/e;
        $BASE =~ s/:.+$/:@/;

	$F_FORM = &find($FORM, *INF, $N_INF);
	$F_BASE = &find($BASE, *INF, $N_INF);

	if ($F_FORM == -1 || $F_BASE == -1) {
	    # 屈折パターンが見つからない
	    # (本来はあってはいけないはず?)
	    # 語形パターンから除外する
	    $i = -1;
	    last;
	}

	if ($INF[$F_FORM][1] ne $INF[$F_BASE][1]) {
	    $i = -1;
	    last;
	}
    }
    if ($i >= 0) {
	print $Fld1;
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
