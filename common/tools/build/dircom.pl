#!/usr/local/bin/perl5 -- -*-perl-*-
#	for ariel
#
#	mongo:/usr/local/bin/perl5
#
# dircom --- 
#
# Copyright (c) 2003, 2024 Ricoh Company, Ltd.
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
# Usage:
#	dircom (mv|cp|rm|mkdir|rmdir|install|cat|touch) ...
#

#
# コマンド名 (basename)
#
$0 =~ s|^.*/([^/][^/]*)$|$1| if ($0 =~ m|/|);

if ($#ARGV < 1) {
    &usage;
}

$command = shift(@ARGV);
@args = @ARGV;

use File::Path;
use File::Basename;
use File::Copy;

if ($command eq "mv") {
    # 引数のファイルを移動
    @args = (map {glob($_)} @args);
    $target = pop(@args);
    die "$0: mv source ... target\n" if !@args;
    die "$0: mv source ... target\n\ttarget must be a directory for multiple sources (" . join(' ', @args) . ")\n"
	if ($#args > 0 && !(-d $target));
    if (-d $target) {
	$gettarget = sub () {$target . "/" . &basename($_);};
    } else {
	$gettarget = sub () {$target;};
    }
    map {
	my $t = &$gettarget($_);
	if (-e $t) {
	    unlink($t) || die "$0: Can't rm $t. $!\n";
	}
	rename($_, $t) || die "$0: Can't mv $_. $!\n";
    } @args;

} elsif ($command eq "cp") {
    # 引数のファイルをコピー
    @args = (map {glob($_)} @args);
    $target = pop(@args);
    die "$0: cp source ... target\n" if !@args;
    die "$0: cp source ... target\n\ttarget must be a directory for multiple sources (" . join(' ', @args) . ")\n"
	if ($#args > 0 && !(-d $target));
    if (-d $target) {
	$gettarget = sub () {$target . "/" . &basename($_);};
    } else {
	$gettarget = sub () {$target;};
    }
    $currenttime = time;
    map {
	my $t = &$gettarget($_);
	copy($_, $t);
	if (!-w $t) {
	    my @stat = stat($t);
	    my $mode = $stat[2];
	    chmod(0644, $t) || die "$0: can't chmod: " . $t . " $!\n";
	    utime($currenttime, $currenttime, $t) || die "$0: can't set time: " . $t . " $!\n";
	    chmod($mode, $t);
	} else {
	    utime($currenttime, $currenttime, $t) || die "$0: can't set time: " . $t . " $!\n";
	}
    } @args;

} elsif ($command eq "rm") {
    # 引数のファイルを削除
    @args = (map {glob($_)} @args);
    unlink(@args);

} elsif ($command eq "rmdir") {
    # 引数のディレクトリー以下を削除
    @args = (map {glob($_)} @args);
    &rmtree(\@args);

} elsif ($command eq "mkdir") {
    # 引数のディレクトリーを作成
    &mkpath(\@args, 0, 0755);

} elsif ($command eq "install") {
    # 引数のファイルのうち新しいものを最後の引数のディレクトリーにコピー
    $mode = shift(@args);
    @args = (map {glob($_)} @args);
    $target = pop(@args);
    die "$0: $target: No such directory.\n"  unless -d $target;
    die "$0: no source files are specified.\n"  unless $#args >= 0;
    map {
	$sourcefile = $_;
	die "$0: $sourcefile: No such source file.\n" unless -e $sourcefile;
	$targetfile = $target . "/" . &basename($sourcefile);
	if (!(-e $targetfile) || (-M $_ < -M $targetfile)) {
	    $currenttime = time;
	    copy($sourcefile, $targetfile);
	    utime($currenttime, $currenttime, $targetfile);
	    print STDERR "$sourcefile -> $target\n";
	}
	chmod(oct($mode), $targetfile);
    } @args;

} elsif ($command eq "cat") {
    # 引数のファイルの内容を出力する
    @args = (map {glob($_)} @args);
    map {
	&cat($_);
    } @args;
} elsif ($command eq "touch") {
    # 引数のファイルのタイムスタンプを更新する
    @args = (map {glob($_)} @args);
    $currenttime = time;
    map {
	if (!-e $_) {
	    open(OUT, ">$_") || die "$0: Can't open $_. $!\n";
	    close(OUT);
	} elsif (!-w $_) {
	    my @stat = stat($_);
	    my $mode = $stat[2];
	    chmod(0644, $_) || die "$0: can't chmod: " . $_ . " $!\n";
	    utime($currenttime, $currenttime, $_) || die "$0: can't set time: " . $_ . " $!\n";
	    chmod($mode, $_);
	} else {
	    utime($currenttime, $currenttime, $_) || die "$0: can't set time: " . $_ . " $!\n";
	}
    } @args;
} else {
    &usage;
}

# 正常終了
1;

sub usage
{
    die "Usage: $0 (mv|cp|rm|mkdir|rmdir|install|cat|touch) ...\n";
}
sub cat
{
    my ($s) = @_;
    open(INPUT, "$s") || die "$0: Can't open $s. $!\n";
    while (<INPUT>) {
	print STDOUT $_;
    }
    close (INPUT);
}

#
# Copyright (c) 2003, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
