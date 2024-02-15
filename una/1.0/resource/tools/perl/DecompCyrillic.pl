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
################################################################################
#
# DecompCyrillic.pl
#
# 機能: キリル文字の音標付きおよび合字を構成文字に分解する（結合文字は削除）
#
# 実行方法:
#   DecompCyrillic.pl -s DIR -d DIR [OPTIONS]
#    -s DIR  : 元データの所在 ($srcdir) [required]
#    -d DIR  : 中間データの所在 ($datadir) [required]
#    -h      : ヘルプ
#
# 入力ファイル:
#   $srcdir/UnicodeData-1.1.5.txt
#   $datadir/TargetCyrillic.dat - キリル文字のコード
#
# 出力ファイルのフォーマット: code1;code2
#   code1: 変換前コード
#   code2: 変換後コード列
#
################################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

##########
# パラメータ評価
use Getopt::Std;
getopts('s:d:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: DecompCyrillic.pl -s DIR -d DIR [OPTIONS]
   -s DIR  : Source data directory [required]
   -d DIR  : Intermediate data directory [required]
   -h      : help
EOU
}
$srcdir = $opt_s if defined $opt_s;
$datadir = $opt_d if defined $opt_d;

##########
# 対象コードの読み込み
$srcfile = "$datadir/TargetCyrillic.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";
while (<FIN>) {
    chop;
    @target[hex($_)] = $_;
}

##########
# 変換データの作成
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

# 基底文字と結合文字の定義
$base      = "04[0-9A-F][0-9A-F]";
$combining = "03[0-6][0-9A-F]";

while (<FIN>) {
    chop;
    @work = split(";");

    $code   = $work[0];		# コード
    $decomp = $work[5];		# 文字構成

    if (@target[hex($code)] !~ /^$/) {
	if ($decomp =~ /^$base( $combining)+$/) { # 文字構成による変換
	    $decomp =~ s/ $combining//g;
	    print "$code;$decomp\n";
	}
    }
}
close(FIN);

