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
# mk_index.pl: ステマー用のインデックスファイルを作る
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

$KEY_FILE = $STEM_FILE = $EXP_FILE = '';

# パラメータ評価
use Getopt::Std;
getopts('d:k:s:e:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: mk_index.pl -d FILE -k FILE -s FILE -e FILE
   -d FILE : dic_stem.src
   -k FILE : ruleKey.map/dictKey.map [required]
   -s FILE : ruleStem.map
   -e FILE : ruleIndex.src
   -h      : help
EOU
}
$DIC_FILE = $opt_d;
$KEY_FILE = $opt_k;
$STEM_FILE = $opt_s;
$EXP_FILE = $opt_e;

$N_KEY = 0;
open(FIN1, $KEY_FILE) || die "ERROR: cannot open $KEY_FILE\n";
while (<FIN1>) {
    chomp;
    @S = split(' ', $_, 9999);
    $N_KEY++;
    $KEY[$N_KEY-1][0] = $S[0];
    $KEY[$N_KEY-1][1] = $S[1];
}
close(FIN1);

if($STEM_FILE ne '') {
    $N_STEM = 0;
    open(FIN2, $STEM_FILE) || die "ERROR: cannot open $STEM_FILE\n";
    while (<FIN2>) {
	chomp;
	@S = split(' ', $_, 9999);
	$N_STEM++;
	$STEM[$N_STEM-1][0] = $S[0];
	$STEM[$N_STEM-1][1] = $S[1];
    }
    close(FIN2);
}

if($EXP_FILE ne '') {
    $N_EXP = 0;
    open(FIN3, $EXP_FILE) || die "ERROR: cannot open $EXP_FILE\n";
    while (<FIN3>) {
	chomp;
	@S = split(' ', $_, 9999);
	$N_EXP++;
	$EXP[$N_EXP-1][0] = $S[0];
	$EXP[$N_EXP-1][1] = $S[1];
    }
    close(FIN3);
}

if($DIC_FILE ne '') {
    open(FIN4, $DIC_FILE) || die "ERROR: cannot open $DIC_FILE\n";
    while (<FIN4>) {
        chomp;	# strip record separator
        @Fld = split(' ', $_, 9999);

        if ($#Fld == 1 || $#Fld == 2) {
	    $KEY_OFFSET = $STEM_OFFSET = 0;
	    if (($F_KEY = &find($Fld[0], *KEY, $N_KEY)) >= 0) {
	        $KEY_OFFSET = $KEY[$F_KEY][1];
	    }

	    if ($STEM_FILE eq '') {
	        if (($F_KEY = &find($Fld[1], *KEY, $N_KEY)) >= 0) {
		    $STEM_OFFSET = $KEY[$F_KEY][1];
	        }
	    }
	    else {
	        if ($Fld[0] ne $Fld[1] && ($F_STEM = &find($Fld[1], *STEM, $N_STEM)) >= 0) {
		    $STEM_OFFSET = $STEM[$F_STEM][1];
	        }
	    }

	    if ($EXP_FILE eq '') {
	        print $KEY_OFFSET, $STEM_OFFSET;
	    }
	    else {
	        $EXP_OFFSET = 0;
	        if ($Fld[1] ne $Fld[2] && ($F_EXP = &find($Fld[2], *EXP, $N_EXP)) >= 0) {
		    $EXP_OFFSET = $EXP[$F_EXP][1];
	        }
	        print $KEY_OFFSET, $STEM_OFFSET, $EXP_OFFSET;
	    }
        }
    }
    close(FIN4);
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
