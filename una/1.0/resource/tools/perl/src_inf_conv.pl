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
################################################################################
#
# src_inf_conv.pl: 屈折情報の変換
# ・語尾が確定できるような記法にする e.g. @+es -> @+e+s
# ・出力形式は入力形式と同じ
# 
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDIN,  ":utf8");
binmode(STDOUT, ":utf8");

$FS = ' ';		# set field separator
$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

$FS = $, = '#';

while (<>) {
    chomp;	# strip record separator
    @Fld = split($FS, $_, 9999);

    @BASE = split(/\|/, $Fld[0], 9999);
    $lenB = length($BASE[0]);

    for ($i = 1; $i <= $#Fld; $i++) {
	$N = (@VAR = split(/;/, $Fld[$i], 9999));

	for ($j = 0; $j < $N; $j++) {
	    @SUF = split(/\|/, $VAR[$j], 9999);
	    $lenS = length($SUF[0]);

	    if ($SUF[2] =~ /\+/) {
		$ADD = $SUF[2];
		$ADD =~ s/^.*\+//;
		$lenA = length($ADD);

		if ($ADD ne substr($SUF[0], $lenS - $lenA, $lenA)) {
		    last;
		}
		if ($SUF[2] !~ /^@\-/) {
		    if ($BASE[0] ne substr($SUF[0], 0, $lenS - $lenA)) {
			last;
		    }
		}
		else {
		    $DEL = $SUF[2];

		    $DEL =~ s/\+.*$//;

		    $DEL =~ s/^.*\-//;
		    $lenD = length($DEL);
		    if (substr($BASE[0], 0, $lenB - $lenD) ne substr($SUF[0], 0, $lenS - $lenA)) {
			last;
		    }
		}
	    }

	    $REPLACED = 0;

	    ##### same form
	    if ($SUF[0] eq $BASE[0]) {
		$PAT = '@';
		if ($SUF[2] ne $PAT) {
		    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
		    $REPLACED = 1;
		}
	    }

	    ##### AC/DC
	    if ($SUF[1] eq 'AC' || $SUF[1] eq 'DC') {
		if ($SUF[0] =~ /ier$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 3);
		    ######### pricey -> pricier
		    if ($BASE[0] =~ /ey$/) {
			$PAT = '@-ey+i+er';
			if (substr($BASE[0], 0, $lenB - 2) eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		    ######### pricy -> pricier
		    if ($REPLACED == 0 && $BASE[0] =~ /y$/) {
			$PAT = '@-y+i+er';
			if (substr($BASE[0], 0, $lenB - 1) eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		}
		####### big -> bigger
		if ($REPLACED == 0 && $SUF[0] =~ /[bdgklmnprstz]er$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 3);
		    if (substr($STEM, length($STEM) - 1, 1) eq substr($BASE[0], $lenB - 1, 1)) {
			$PAT = '@+' . substr($STEM, length($STEM) - 1, 1) . '+er';
			if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		}
		####### able -> abler
		if ($REPLACED == 0 && $BASE[0] =~ /e$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 1);
		    $PAT = '@-e+er';
		    if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### cheap -> cheaper
		if ($REPLACED == 0 && $SUF[0] =~ /er$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 2);
		    $PAT = '@+er';
		    if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
	    }
	    ##### AC/DC

	    ##### AS/DS
	    elsif ($SUF[1] eq 'AS' || $SUF[1] eq 'DS') {
		if ($SUF[0] =~ /iest$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 4);
		    ######### pricey -> priciest
		    if ($BASE[0] =~ /ey$/) {
			$PAT = '@-ey+i+est';
			if (substr($BASE[0], 0, $lenB - 2) eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		    ######### pricy -> priciest
		    if ($REPLACED == 0 && $BASE[0] =~ /y$/) {
			$PAT = '@-y+i+est';
			if (substr($BASE[0], 0, $lenB - 1) eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		}
		####### big -> biggest
		if ($REPLACED == 0 && $SUF[0] =~ /[bdgklmnprstz]est$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 4);
		    if (substr($STEM, length($STEM) - 1, 1) eq substr($BASE[0], $lenB - 1, 1)) {
			$PAT = '@+' . substr($STEM, length($STEM) - 1, 1) . '+est';
			if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		}
		####### able -> ablest
		if ($REPLACED == 0 && $SUF[0] =~ /est$/ && $BASE[0] =~ /e$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 2);
		    $PAT = '@-e+est';
		    if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### cheap -> cheapest
		if ($REPLACED == 0 && $SUF[0] =~ /est$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 3);
		    $PAT = '@+est';
		    if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
	    }
	    ##### AS/DS

	    ##### NP/V3
	    elsif ($SUF[1] =~ /^(NP|V3)$/) {
		if ($SUF[0] =~ /ies$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 3);
		    ######### chantey -> chanties
		    if ($BASE[0] =~ /ey$/) {
			$PAT = '@-ey+ie+s';
			if (substr($BASE[0], 0, $lenB - 2) eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		    ######### body -> bodies
		    if ($REPLACED == 0 && $BASE[0] =~ /y$/) {
			$PAT = '@-y+ie+s';
			if (substr($BASE[0], 0, $lenB - 1) eq $STEM &&  $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		}
		if ($REPLACED == 0 && $SUF[0] =~ /ves$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 3);
		    ######### life -> lives
		    if ($BASE[0] =~ /fe$/) {
			$PAT = '@-fe+ve+s';
			if (substr($BASE[0], 0, $lenB - 2) eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		    ######### leaf -> leaves
		    if ($REPLACED == 0 && $BASE[0] =~ /f$/) {
			$PAT = '@-f+ve+s';
			if (substr($BASE[0], 0, $lenB - 1) eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		}
		####### appendix -> appendices  規則屈折になってしまうため削除
		#	if (REPLACED==0 && SUF[0]~/ces$/ && BASE[0]~/x$/) {
		#	  STEM = substr(SUF[0],0,lenS-3);
		#	  PAT = "@-x+ce+s";
		#	  if (substr(BASE[0],0,lenB-1) == STEM && SUF[2] != PAT) {
		#	    VAR[j] = SUF[0] "|" SUF[1] "|" PAT;
		#	    REPLACED = 1;
		#	  }
		#	}
		####### formula -> formulae
		if ($REPLACED == 0 && $SUF[1] eq 'NP' && $SUF[0] =~ /ae$/ && $BASE[0] =~ /a$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 1);
		    $PAT = '@-a+ae';
		    if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### addendum -> addenda
		if ($REPLACED == 0 && $SUF[1] eq 'NP' && $SUF[0] =~ /a$/ && $BASE[0] =~ /um$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 1);
		    $PAT = '@-um+a';
		    if (substr($BASE[0], 0, $lenB - 2) eq $STEM && $SUF[2] ne $PAT) {	#???
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### abacus -> abaci
		if ($REPLACED == 0 && $SUF[1] eq 'NP' && $SUF[0] =~ /i$/ && $BASE[0] =~ /us$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 1);
		    $PAT = '@-us+i';
		    if (substr($BASE[0], 0, $lenB - 2) eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### automaton -> automata
		if ($REPLACED == 0 && $SUF[1] eq 'NP' && $SUF[0] =~ /a$/ && $BASE[0] =~ /on$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 1);
		    $PAT = '@-on+a';
		    if (substr($BASE[0], 0, $lenB - 2) eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### miss -> misses
		if ($REPLACED == 0 && $SUF[0] =~ /[sz]es$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 3);
		    if (substr($STEM, length($STEM) - 1, 1) eq substr($BASE[0], $lenB - 1, 1)) {
			$PAT = '@+' . substr($STEM, length($STEM) - 1, 1) . 'e+s';
			if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		}
		####### fish -> fishes
		if ($REPLACED == 0 && $SUF[0] =~ /es$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 2);
		    if ($STEM =~ /([cs]h|[szxo])$/) {
			$PAT = '@+e+s';
		    }
		    else {
			$PAT = '*';
		    }
		    if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### eye -> eyes
		if ($REPLACED == 0 && $SUF[0] =~ /s$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 1);
		    $PAT = '@+s';
		    if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
	    }
	    ##### NP/V3

	    ##### VD/VN
	    elsif ($SUF[1] eq 'VD' || $SUF[1] eq 'VN') {
		if ($SUF[0] =~ /ied$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 3);
		    ######### seems not existing
		    if ($BASE[0] =~ /ey$/) {
			$PAT = '@-ey+i+ed';
			if (substr($BASE[0], 0, $lenB - 2) eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		    ######### worry -> worried
		    if ($REPLACED == 0 && $BASE[0] =~ /y$/) {
			$PAT = '@-y+i+ed';
			if (substr($BASE[0], 0, $lenB - 1) eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		}
		####### panic -> panicked
		if ($REPLACED == 0 && $SUF[0] =~ /cked$/ && $BASE[0] =~ /c$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 4);
		    $PAT = '@+k+ed';
		    if (substr($BASE[0], 0, $lenB - 1) eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### beg -> begged
		if ($REPLACED == 0 && $SUF[0] =~ /[bdgklmnprstz]ed$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 3);
		    if (substr($STEM, length($STEM) - 1, 1) eq substr($BASE[0], $lenB - 1, 1)) {
			$PAT = '@+' . substr($STEM, length($STEM) - 1, 1) . '+ed';
			if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		}
		####### bake -> baked
		if ($REPLACED == 0 && $SUF[0] =~ /ed$/ && $BASE[0] =~ /e$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 1);
		    $PAT = '@-e+ed';
		    if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### fish -> fished
		if ($REPLACED == 0 && $SUF[0] =~ /ed$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 2);
		    $PAT = '@+ed';
		    if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
	    }
	    ##### VD/VN

	    ##### VG
	    elsif ($SUF[1] eq 'VG') {
		####### lie -> lying
		if ($SUF[0] =~ /ying$/ && $BASE[0] =~ /ie$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 4);
		    $PAT = '@-ie+y+ing';
		    if (substr($BASE[0], 0, $lenB - 2) eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### race -> racing
		if ($REPLACED == 0 && $SUF[0] =~ /ing$/ && $BASE[0] =~ /e$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 3);
		    $PAT = '@-e+ing';
		    if (substr($BASE[0], 0, $lenB - 1) eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### panic -> panicking
		if ($REPLACED == 0 && $SUF[0] =~ /cking$/ && $BASE[0] =~ /c$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 5);
		    $PAT = '@+k+ing';
		    if (substr($BASE[0], 0, $lenB - 1) eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
		####### beg -> begging
		if ($REPLACED == 0 && $SUF[0] =~ /[bdgklmnprstz]ing$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 4);
		    if (substr($STEM, length($STEM) - 1, 1) eq substr($BASE[0], $lenB - 1, 1)) {
			$PAT = '@+' . substr($STEM, length($STEM) - 1, 1) . '+ing';
			if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			    $REPLACED = 1;
			}
		    }
		}
		####### fish -> fishing
		if ($REPLACED == 0 && $SUF[0] =~ /ing$/) {
		    $STEM = substr($SUF[0], 0, $lenS - 3);
		    $PAT = '@+ing';
		    if ($BASE[0] eq $STEM && $SUF[2] ne $PAT) {
			$VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
			$REPLACED = 1;
		    }
		}
	    }
	    ##### VG

	    ##### irregular
	    elsif ($SUF[2] !~ /\@/ && $SUF[0] ne $BASE[0]) {
		$PAT = '*';
		if ($SUF[2] ne $PAT) {
		    $VAR[$j] = $SUF[0] . '|' . $SUF[1] . '|' . $PAT;
		    $REPLACED = 1;
		}
	    }
	    ##### irregular

	    if ($REPLACED == 1) {
		$Fld[$i] = '';
		for ($k = 0; $k < $N; $k++) {
		    $Fld[$i] = $Fld[$i] . ';' . $VAR[$k];
		}
		$Fld[$i] =~ s/^;//;
	    }
	}
	### for (j = 0; j < N; j ++)

	;
    }

    print join($,,@Fld);
}
