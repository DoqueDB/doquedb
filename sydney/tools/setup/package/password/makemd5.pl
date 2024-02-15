#! /usr/local/bin/perl5
#
# Usage: makemd5.pl <source file>
#
# source file should have following format;
#
#{user name}:{plain password}::

use Digest::MD5 qw(md5_hex);

my $filename = shift @ARGV;
$filename || die "Usage: makemd5.pl <source file>";

open(IN, $filename) || die "Can't open $filename: $!";

while (<IN>) {
    next if /^#/;
    next if /^$/;
    s/^([^:]*):([^:]*):([^:]*):([^:]*)/$1.':'.md5($2).':'.$3.':'.$4/e;
    print;
}
close IN;

sub md5($)
{
    my $v = shift;

    md5_hex $v;
}
