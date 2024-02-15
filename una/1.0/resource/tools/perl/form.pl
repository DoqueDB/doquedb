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
# form.pl: 抽出した異表記を統合して出力する
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDIN,  ":utf8");
binmode(STDOUT, ":utf8");

while (<>) {
    chomp;
    @Fld = split(/ /, $_);

    print "$Fld[0] $Fld[1]:$Fld[2]\n";
    if($Fld[2]=~/\+(ing|ed)$/){
        print "$Fld[0]s $Fld[0]:\@\+s\n";
    }
}
