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
use utf8;
use strict;

print "executing $0 $ARGV[0] $ARGV[1]\n";

if ("$ARGV[1]" eq "") {
	die "USAGE: $0 infile outfile\n";
}
open(IN, $ARGV[0]);
open(OUT, ">" . $ARGV[1]);

my $module = "";
my $prefix = "";
while (<IN>) {
    next if /^#/;
    next if /^\s*$/;
    if (/^module:(\S+)$/) {
	$module = $1;
	last;
    }
}

print OUT "ModExceptionMessageAssoc ${module}MessageArray[] = {\n";

while (<IN>) {
    next if /^#/;
    next if /^\s*$/;
    $prefix = $1, next if /^(\S+):$/;
    if (my ($code, $message) = /^\s*(\S+)\s*(.*)/) {
	print OUT "\t{ $prefix$code, \"$message\" },\n";
    }
}
print OUT "\t{ 0, (char*)0 }\n";
print OUT "};\n";

print OUT "static ModExceptionMessage ${module}Message($module, ${module}MessageArray);\n";
