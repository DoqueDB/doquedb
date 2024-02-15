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
# ungroup.pl
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDIN,  ":utf8");
binmode(STDOUT, ":utf8");

$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

while (<>) {
    ($Fld1,$Fld2) = split(' ', $_, 9999);

    for ($i = 0; $i < ($N = (@S = split(/,/, $Fld2, 9999))); $i++) {
		for ($j = 0; $j < $N; $j++) {
		    if ($i == $j) {	#???
				print $S[$i] . ' 0:' . $S[$j];
		    }
		    else {
				print $S[$i] . ' 1:' . $S[$j];
		    }
		}
    }
}
