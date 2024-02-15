#!perl.exe
# 
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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
#check the test output log ,find if there is "not ok"
# usage:
#	> perl	 check_test_output.pl	 output.txt
open FILE ,"< $ARGV[0]";
print "Begin check\n";
while(<FILE>)
{
	if (/^not ok/)
	{
		print $_;
	}
	elsif(/^\[WARNING\]/)
	{
		print $_;
	}
}
print "End check\n";
close FILE;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
