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
# group.pl
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDIN,  ":utf8");
binmode(STDOUT, ":utf8");

$FS = ' ';		# set field separator
$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

$, = $FS;

while (<>) {
    chomp;	# strip record separator
    @Fld = split($FS, $_, 9999);
    if ($. == 1) {
	$R{0} = $Fld[0];
	for ($i = 1; $i <= $#Fld; $i++) {
	    $R{$i} = $Fld[$i];
	}
	$N = $#Fld;
    }
    if ($. > 1) {
	if ($Fld[0] eq $R{0}) {
	    for ($i = 1; $i <= $N; $i++) {
		$R{$i} = $R{$i} . ',' . $Fld[$i];
	    }
	} else {
	    printf '%s', $R{0};
	    for ($i = 1; $i <= $N; $i++) {
		printf '%s%s', $,, $R{$i};
	    }
	    print '';

	    $R{0} = $Fld[0];
	    for ($i = 1; $i <= $#Fld; $i++) {
		$R{$i} = $Fld[$i];
	    }
	    $N = $#Fld;
	}
    }
}

printf '%s', $R{0};
for ($i = 1; $i <= $N; $i++) {
    printf '%s%s', $,, $R{$i};
}
print '';
