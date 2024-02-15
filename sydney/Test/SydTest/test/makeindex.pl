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
# *.$suffixから見出しを作る

$suffix = "txt";
$dir =$ARGV[0] ? $ARGV[0] : ".";
opendir (DIR, $dir);
@dirs = readdir(DIR);
@test = grep (/^\d{4}\.$suffix$/, @dirs);
foreach $file (@test)
{
    open FILE, "<$file";
    while(<FILE>)
    {	   
	if (/^#\s*(.+)$/)
	{
	    $desc = $1;
	    $base = $file; $base =~ s/\.$suffix$//;
	    print "$base\t$desc\n";
	    last;
	}
    }
    close FILE;
}
closedir DIR;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
