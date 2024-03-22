#!/usr/local/bin/perl5 -- -*-perl-*-
#	for ariel
#
#	mongo:/usr/local/bin/perl5
#
# mkconfdir --- perl5版 Makefile 生成プログラム
#
# Copyright (c) 1996, 2003, 2024 Ricoh Company, Ltd.
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
#	mkconfdir [-f] conf ...
#

#
# 1996/2/28
# 基本的に、使い方は以前のシェルスクリプト版Mkconfdirと同じ。
# 相違点は以下のとおり。
# ・confの下にMakefile.tmplというファイルがあると、それを
#   テンプレートとして使う。(imakeのImake.tmplにあたる)
#   通常、このテンプレートからMakefile.cをインクルードする。
# ・"@@"を見つけると、"\n"に置き換える。(imakeと同じ)
# ・2行以上連続する空行は1行の空行に置き換える。→空行はすべて削除
# ・カレントディレクトリが……/cの場合にはエラーになる。ただし、
#   -fオプションがついたときにはエラーにせず、../c.CONFを作る。
#   カレントディレクトリが……/c以外の場合には、./c.CONFを作る。
#

#
# 設定
#
$ostype = $ENV{OS} || $ENV{OSTYPE};
$conf =
{
    Windows_NT =>
    {
	os => "windows",
	cpp => 'cl /E /EP',
	perl => 'perl',
	pwd => 'cd',
	flagfunc => sub () {
	    my ($x) = @_;
	    # ファイルの前につけるオプションを追加する
	    $x .= " /Tc";
	    $x;
	},
    },
    solaris =>
    {
	os => "solaris",
	cpp => 'gcc -x c -E -traditional-cpp',
	perl => 'perl5',
	pwd => 'pwd',
	flagfunc => sub () {
	    my ($x) = @_;
	    # NTのよけいなオプションを抜いたものを作成する。/XXXや/D"XXX"など。
	    $x =~ s!/([0-9A-Za-z_\"]*)!!g;
	    # cc として SUN のコンパイラ、cpp として gcc を使うときは
	    # gcc の cpp が解釈できないオプションを抜く
	    $x =~ s!-xO[0-9]+!!g;
	    $x =~ s!-mt!!g;
	    $x =~ s!-KPIC!!g;
	    $x =~ s!-features=[a-z_%,]*!!g;
	    $x =~ s!-xipo!!g;
	    $x =~ s!-xs!!g;
	    $x =~ s!-xarch=[a-z0-9]*!!g;
	    $x =~ s!-xmemalign=[a-z0-9]*!!g;
	    $x =~ s!-xopenmp!!g;
	    $x =~ s!-xqopenmp!!g;
	    $x;
	},
    },
    linux =>
    {
	os => "linux",
	cpp => 'gcc -x c -E -traditional-cpp',
	perl => 'perl',
	pwd => 'pwd',
	flagfunc => sub () {
	    my ($x) = @_;
	    # NTのよけいなオプションを抜いたものを作成する。\XXXや\D"XXX"など。
	    $x =~ s!/([0-9A-Za-z_\"]*)!!g;
	    # gcc の cpp が解釈できないオプションを抜く
	    $x =~ s!-ipo!!g;
	    $x =~ s!-parallel!!g;
	    $x =~ s!-par-report3!!g;
	    $x =~ s!-par-threshold0!!g;
	    $x =~ s!-fwritable-strings!!g;
	    $x =~ s!-fpermissive!!g;
	    $x =~ s!-openmp!!g;
	    $x =~ s!-qopenmp!!g;
	    $x;
	},
    },
};

$os = $conf->{$ostype}->{os};
$cpp = $conf->{$ostype}->{cpp};
$perl = $conf->{$ostype}->{perl};
$pwd = $conf->{$ostype}->{pwd};
$flagfunc = $conf->{$ostype}->{flagfunc};

#
# コマンド名 (basename)
#
$0 =~ s|^.*/([^/][^/]*)$|$1| if ($0 =~ m|/|);

#
# オプションの処理
#
use Getopt::Std;
&getopts("f");
$do_force = $opt_f;

#
# 引数リストのチェック
#
if ($#ARGV < 0) {
    die "Usage: $0 conf...\n";
}

#
# umaskのセット
#
umask(002);

#
# カレントディレクトリ
#
$cwd = `$pwd`;
chop($cwd);

#
# ベースディレクトリの設定
#
if ($cwd =~ m|^(.*)/c$|) {
    if ($do_force) {
	$basedir = $1;
    } else {
	# カレントディレクトリが……/cのときにはエラーにする
	die "$0: wrong working directory\n";
    }
} else {
    $basedir = $cwd;
}

#
# confディレクトリを探す
#
$confdir = $cwd;
while (! (-d "$confdir/conf/$os")) {
    $confdir =~ s|^(.*)[/\\][^/\\]*$|$1|;

    # ルートディレクトリまで探しても…/confディレクトリが無ければエラー
    if ($confdir eq "") {
	die "$0: no configuration database for os='$os'.\n";
    }
}

#
# ディレクトリセパレータ '/' or '\' を $(S) に変更する
$conftop = $confdir;
if ($os eq "windows") {
    $conftop =~ s![/\\]!\$(S)!g;
}

my $processed_files = {};
my $defined = {};
my $ifdef = [];

sub read_conf($)
{
    my ($conffile) = @_;

    return () if $processed_files->{$conffile};

    open(CONF, "$conffile")
	|| die "$0: cannot open $conffile\n";
    my @include = ();
    my @flags = ();
    while (<CONF>) {
	if (/^\#\s*endif/) {
	    if (@$ifdef == 0) {
		die "$0: unmatched ifdef-endif in $conffile\n";
	    }
	    shift(@$ifdef);
	} elsif (/^#\s*else/) {
	    if (@$ifdef == 0) {
		die "$0: unmatched ifdef-else in $conffile\n";
	    }
	    if (@$ifdef == 0 || $ifdef->[0] != 2) {
		$ifdef->[0] = 1 - $ifdef->[0];
	    }
	} elsif (/^\#\s*ifdef\s+([a-zA-Z0-9_]+)/) {
	    if (@$ifdef == 0 || $ifdef->[0] == 1) {
		if ($defined->{$1}) {
		    unshift(@$ifdef, 1);
		} else {
		    unshift(@$ifdef, 0);
		}
	    } else {
		unshift(@$ifdef, 2);
	    }
	} elsif (/^\#\s*ifndef\s+([a-zA-Z0-9_]+)/) {
	    if (@$ifdef == 0 || $ifdef->[0] == 1) {
		if ($defined->{$1}) {
		    unshift(@$ifdef, 0);
		} else {
		    unshift(@$ifdef, 1);
		}
	    } else {
		unshift(@$ifdef, 2);
	    }
	} elsif (@$ifdef > 0 && $ifdef->[0] != 1) {
	    next;
	} elsif (/^\#\s*include\s+\"([^\"]+)\"/) {
	    push(@include, $1);
	} elsif (/^\#\s*define\s+([a-zA-Z0-9_]+)/) {
	    $defined->{$1} = 1;
	} elsif (/^\#/) {
	    next;
	} else {
	    chomp;
	    push(@flags, $_);
	}
    }
    close(CONF);

    foreach $file (@include) {
	if (-r "$confdir/conf/include/$file") {
	    push(@flags, read_conf("$confdir/conf/include/$file"));
	} elsif (-r "$confdir/conf/$os/$file") {
	    push(@flags, read_conf("$confdir/conf/$os/$file"));
	} else {
	    die "$0: no include file: $file in $conffile\n";
	}
    }

    $processed_files->{$conffile} = 1;

    return @flags;
}

#
# c.CONFディレクトリとMakefileの作成
#
foreach $i (0 .. $#ARGV) {
    # ディレクトリとファイル名の設定
    $conf = $ARGV[$i];
    $conffile = "$confdir/conf/$os/$conf";
    if (! (-r $conffile)) {
	die "$0: no configuration: $conffile\n";
    }

    # flagsの作成
    @flags = read_conf($conffile);
    $flags = join(' ', @flags);

    $mkconfflags = &$flagfunc($flags);

    # ソースファイルのリスト
    opendir(SRCDIR, "$basedir/c") || die "$0: cannot read $basedir/c\n";
    @srcfiles = grep(/\.c$/, readdir(SRCDIR));
    closedir(SRCDIR);

    # c.CONFディレクトリの作成
    $configdir = "$basedir/c.$conf";
    if (! (-d $configdir)) {
	mkdir($configdir, 0775) || die "$0: cannot mkdir $configdir\n";
    }

    # ターゲットファイルの作成
    foreach $src (@srcfiles) {
	# 生成ファイル名
	($target = $src) =~ s/\.c$//;

	# テンプレートファイル
	$tmpl = "$confdir/conf/$target.tmpl";
	$tmpl = "" unless (-r $tmpl);

	# ターゲットファイルのフルパス名
	$src = "$basedir/c/$src";
	$target = "$basedir/c.$conf/$target";

	# ターゲットファイルの生成
	if ($tmpl) {
	    # テンプレートがある場合
	    open(TMPL, "$cpp -I. -I./c -DCONF=$conf -DFLAGS=\"$flags\" -DCONFTOP=\"$conftop\" -DCONFOS=\"$os\" $mkconfflags $tmpl |")
		|| die "$0: cannot execute cpp to $tmpl";
	} else {
	    open(TMPL, "$cpp -I. -I./c -DCONF=$conf -DFLAGS=\"$flags\" -DCONFTOP=\"$conftop\" -DCONFOS=\"$os\" $mkconfflags $src |")
		|| die "$0: cannot execute cpp to $src";
	}
	open(TARGET, ">$target") || die "$0: cannot open $target\n";
	while (<TMPL>) {
	    next if /^\#.*$/;
	    s!\s*//.*$!!g;
	    s/[\r\n]+//g;
	    s/\s*@@/\n/g;
	    s/\s+$//g;
	    next if /^$/;
	    print TARGET $_, "\n";
	}
	close TARGET;
	close TMPL;
    }
}

# 正常終了
exit;

# Copyright (c) 1996, 2003, 2024 Ricoh Company, Ltd.
# All rights reserved.
