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
# mk_norm.pl
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDIN,  ":utf8");
binmode(STDOUT, ":utf8");

# パラメータ評価
use Getopt::Std;
getopts('h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: mk_norm.pl
   -h         : help
EOU
}

while (<>) {
    chomp;	# strip record separator
    @Fld = split(' ', $_, 9999);

    if ($_ =~ /^[^\# 	]/) {
	# 行末のコメントを削除
	s/[ 	]*\#.*$//;

	$POS = '名詞.サ変';

	if ($Fld[0] =~ /^.$/ || $Fld[0] =~ /^\\u....$/) {
	    printf "%s %s %f %s\n", $Fld[0], $POS, 1, &mergeDiff($Fld[0],$Fld[1]);
	}
	else {
	    printf "%s %s %f %s\n", $Fld[0], $POS, 0, &mergeDiff($Fld[0],$Fld[1]);
	}
    }
}

sub mergeDiff {
    local($s1, $s2) = @_;
    if ($s1 eq $s2) {
	return $s1;
    }

    $head = $tail = '';

    while (substr($s1, 0, 1) eq substr($s2, 0, 1)) {
	$head = $head . substr($s1, 0, 1);
	$s1 =~ s/^.//;
	$s2 =~ s/^.//;
    }

    while (substr($s1, length($s1) - 1, 999999) eq substr($s2, length($s2) - 1, 999999)) {
	$tail = substr($s1, length($s1) - 1, 999999) . $tail;
	$s1 =~ s/.$//;
	$s2 =~ s/.$//;
    }

    if ($s1 eq '') {
	if ($head ne '') {
	    $s1 = substr($head, length($head) - 1, 999999) . $s1;
	    $s2 = substr($head, length($head) - 1, 999999) . $s2;
	    $head =~ s/.$//;
	}
	else {
	    $s1 = $s1 . substr($tail, 0, 1);
	    $s2 = $s2 . substr($tail, 0, 1);
	    $tail =~ s/^.//;
	}
    }

    $res = $head . '{' . $s1 . ',' . $s2 . '}' . $tail;
    $res;
}
