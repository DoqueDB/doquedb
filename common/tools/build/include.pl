#! /usr/local/bin/perl
# 
# Copyright (c) 1994, 2024 Ricoh Company, Ltd.
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
# include [-aemh] [-Idir] ... file ...
#
# file から #include されているファイルを出力する。
# -a をつけると /usr/include の下も探す。
# -e をつけるとインクルードファイルがみつからなかったとき警告を表示する。
# -m をつけると Makefile の dependency の形で出力する。
# -m をつけたとき、
# -h をつけるとインクルードファイルの dependency のみを出力し、
# -h をつけないとソースファイルの dependency と生成ルールも出力する。
#
# 修正履歴
#    1997/10/31 WindowsNT用のヘッダ作成部分を追加
#               .o を$Oに変更。
#
#    1997/8/12  WindowsNT用のソース変換部分を追加
#    2000/11/30	Windows98用のシンボルを追加。
#
#
# オプションの処理
#
# @incdirs は -I で指定されたディレクトリのリスト。
#
@incdirs = (".");
while (@ARGV[0] =~ /^-/) {
	$_ = shift;
	s/-//;
	if (/^I(.*)/) {
		push(@incdirs, $1 ? $1 : shift);
		next;
	}
	if (s/a//g) { $aflag = 1; }
	if (s/e//g) { $eflag = 1; }
	if (s/m//g) { $mflag = 1; }
	if (s/h//g) { $hflag = 1; }
	if ($_) {
		die "Unknown option: $_\n";
	}
}
unless (@ARGV) {
	die "Usage: include [-aemh] [-Idir] ... file ...\n";
}
push(@incdirs, "/usr/include");

#
# 引数に与えられたファイルの処理
#
# %depend はインクルードファイル名をキーとする連想配列。
# これは、同じファイルを何度も出力しないためのもの。
# 出力するべきものは 1、出力しないものは 2。
# @pathqueue は処理すべきソースファイル名を入れたキュー。
# これは、ファイルを再帰的にオープンすることを避けるためのもの。
#
foreach $srcfile (map {glob($_)} @ARGV) {
	%depend = ();
	@pathqueue = ($srcfile);
	# / までを削除する。
	$_ = $srcfile;
	s/.*\///;
	# . が含まれていないとエラー。
	unless (/\./) {
		print STDERR "Invalid source file name: $_\n";
		next;
	}
	# .master からは .master をとったものが作られると仮定。
	# .c などからは .o に変えたものが作られると仮定。

	# NTに対応するため、.oから$Oへ変更する。
	#s/\.[^.]*$/.o/ unless s/\.master$//;
	s/\.[^.]*$/\$O/ unless (s/\.master$// || s/\.rc$/.res/);
	$objfile = $_;
	# キューがつきるまで繰り返し処理する。
	while (@pathqueue) {
		&scan(shift @pathqueue);
	}
	# 連想配列のキーをソートして出力する。
	foreach $path (sort(keys %depend)) {
		if ($depend{$path} == 2) {
			next;
		}
		# NT用のヘッダファイルを作成
		# パス はinclude$(SUFFIX)を見るようにした
		$gheader = $path;
		$gheader =~ s|include|include\$(SUFFIX)|g;
		if ($mflag) {
			#print "$objfile: $path\n";
			print "$objfile: $gheader\n";
		}
		else {
			print "$path\n";
		}
	}
	if (!$mflag) {
		next;
	}
	if (!$hflag) {
		$_ = $srcfile;
		# 拡張子としてcppを採用
		# 更にNT対応
		# ../src/file.c のファイル名部分だけをsrcntfileとしてとりだす
		#
		# 2000/05/18	Windows 2000 の del は NT と異なり
		#		ファイルが存在しないと
		#		exit コードが 1 になるので、
		#		del の前に - をつけた
		if (/\.c$/ || /\.cpp$/ ) {
		    print "$objfile : $srcfile\n";
		    print "\#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98) || defined(OS_WINDOWS95)\n";
		    $srcfile =~ s!/!\\!g;	# .NETは'/'だとうまくいかない
		    print "\t\$(CC) \$(CFLAGS) -c $srcfile\n";
		    print "\#else\n";
		    print "\t\$(CC) \$(CFLAGS) -c $_\n";
		    print "\#endif\n";
		}
		elsif (/\.dml$/) {
		    print "$objfile : $srcfile\n";
		    print "\t\$(DMLC) \$(DMLCFLAGS) -c $_\n";
		}
		elsif (/\.master$/) {
		    $perlscript = $objfile;
		    $perlscript =~ s/(.*)\.([^\.]+)/\1.pl/;
		    print "$objfile : $srcfile $perlscript\n";
		    print "\t\$(PERL) \$(PERLFLAGS) $perlscript $_ > \$(\@B).tmp\n";
		    print "\t\$(MV) \$(\@B).tmp $objfile\n";
		}
		elsif (/\.rc$/) {
		    print "$objfile : $srcfile\n";
		    print "\t\$(RC) \$(RFLAGS) /fo \$\@ $_ \n";
		}
		else {
		    print STDERR "Unknown suffix: $_\n";
		}
	}
	print "\n";
}
exit 0;

#
# 一つのソースファイルの処理
#
sub scan {
	local($path) = @_;
	local($ifile, $start, $i, $idir, $ipath);

	unless (open(FILE, $path)) {
		print STDERR "Cannot open $path: $!\n";
		return;
	}
	# ファイル名が〜.cの場合のみ、ソースファイルのある
	# ディレクトリ名を求める。
	if ($path =~ /(.*)\/.*\.(c|cpp)$/) { $incdirs[0] = $1; }
	else { $incdirs[0] = "."; }
	# 行ごとのループ。
	LINE:
	while (<FILE>) {
		# #include の行を探す。
		unless (/^#[ \t]*include/) {
			next;
		}
		if (/^#[ \t]*include[ \t]*"(.*)"/) {
			$ifile = $1;
			$start = 0;
		}
		elsif (/^#[ \t]*include[ \t]*<(.*)>/) {
			$ifile = $1;
			$start = 1;
		}
		else {
			print STDERR "Invalid line in $file: $_";
			next;
		}
		# インクルードファイルを探す。
		for ($i = $start; $i <= $#incdirs; $i++) {
			$idir = $incdirs[$i];
			if ($ifile =~ /^\//) { $ipath = $ifile; }
			elsif ($idir eq ".") { $ipath = $ifile; }
			else { $ipath = "$idir/$ifile"; }
			# すでに登録されているものは飛ばす。
			if ($depend{$ipath}) {
				next LINE;
			}
			unless (-f $ipath) {
				next;
			}
			# -a なしなら /usr/include の下のファイルは処理しない。
			if ($i == $#incdirs && !$aflag) {
				$depend{$ipath} = 2;
				next LINE;
			}
			$depend{$ipath} = 1;
			push(@pathqueue, $ipath);
			next LINE;
		}
		if ($eflag) {
			print STDERR "Cannot find $ifile\n";
		}
	}
	close(FILE);
}

# Copyright (c) 1994, 2024 Ricoh Company, Ltd.
# All rights reserved.
