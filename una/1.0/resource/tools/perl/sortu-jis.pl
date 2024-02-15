#!/usr/bin/perl
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
###############################################################################
#
# sortu-jis.pl: "sort -u"して出力する
#               ただしJIS X0208にある文字→ない文字の順にソートする
#
###############################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDIN,  ":utf8");
binmode(STDOUT, ":utf8");

my %uni2jis = ();
my $jis0208 = "../src-data/norm/JIS0208.TXT";
open(IN, $jis0208) || die "ERROR: cannot open $jis0208\n";
while (<IN>) {
	chomp;
	next if /^#/;
	# 書式は <SJIS>\t<JISX0208>\t<Unicode>\t#<コメント>
	# 例：0x8140  0x2121  0x3000  # IDEOGRAPHIC SPACE
	/^0x[0-9A-Za-z]{4}\t0x([0-9A-Za-z]{4})\t(0x[0-9A-Za-z]{4})\t/;
	$uni2jis{chr(eval $2)} = $1;
}
close(IN);

my @lines = ();
while(<>) {
	chomp;
	my $orig = $_;
	my @data = split(//, $orig);
	my $line = "";
	for (my $i = 0; $i <= $#data; $i++) {
		$line .= "," if $i > 0;
		if (exists($uni2jis{$data[$i]})) {
			$line .= "0:$uni2jis{$data[$i]}";
		} else {
			$line .= "1:" . sprintf("%04x", ord($data[$i]));
		}
	}
	$line .= " ORIG=$orig";
	push(@lines, $line);
}

@lines = sort @lines;
&output($lines[0]);
for (my $i=1; $i < $#lines; $i++) {
    if ($lines[$i] ne $lines[$i-1]) {
		&output($lines[$i]);
    }
}
exit;

sub output
{
	my ($line) = @_;
	$line =~ /ORIG=(.*)$/;
	print "$1\n";
}
