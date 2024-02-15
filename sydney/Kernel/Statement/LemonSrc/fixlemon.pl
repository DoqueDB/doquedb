# 
# Copyright (c) 2023 Ricoh Company, Ltd.
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
if (@ARGV != 2) {
    print STDERR "Usage: fixlemon source dest\n";
    exit(1);
}

my ($src, $dst) = @ARGV;

open(IN, "<$src") or die "$src: $!";
open(OUT, ">$dst") or die "$dst: $!";

while (<IN>) {
    if (s[^void \*(\w+)\QAlloc(void *(*mallocProc)(size_t))]
	 [void *$1Alloc(void *(*mallocProc)(ModSize n, const char* file, int line))]) {
	$engineName = $1;
    }
    s[^\Q  void (*freeProc)(void*)]
	[  void (*freeProc)(void* pMem, const char* file, int)];
    s[^\Qstatic void yy_accept();]
	[static void yy_accept(yyParser* yypParser $engineName ANSIARGDECL);] &&
	    s/ ANSIARG/ANSIARG/; # to keep emacs happy
    s[^\Qstatic char *yyTokenName[]][static const char *yyTokenName[]];
	s[\\(\w+)\\][/$1/];

} continue {
    print OUT;
}
