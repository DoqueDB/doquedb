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
# src_var.pl: 抽出した異表記を統合して出力する
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDIN,  ":utf8");
binmode(STDOUT, ":utf8");

while (<>) {
    chomp;
    @Fld = split(/\,/, $_);
    for($i = 0; $i <= $#Fld; $i++){
        for($j = 0; $j <= $#Fld; $j++){
            print "$Fld[$i] $Fld[$j]\n";
            print "$Fld[$j] $Fld[$i]\n";
        }
    }
}
