#!/usr/bin/perl
# 
# Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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
# NormKmap.pl
#
# 機能: EUC版norm準拠の漢字異体字変換テーブルを作成する
#
# 実行方法:
#   NormKmap.pl -s DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -h     : ヘルプ
#
# 入力ファイル:
#   $srcdir/NormKmap.srt - EUC版normの漢字異体字テーブル(UTF8版)
#
# 出力ファイルのフォーマット: code1;code2
#   code1: 変換前コード (UCS2)
#   code2: 変換後コード列 (UCS2)
#
################################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

$DEBUG = 0;

##########
# パラメータ評価
use Getopt::Std;
getopts('s:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: NormKmap.pl -s DIR
   -s DIR : Source data directory [required]
   -h     : help
EOU
}
$srcdir = $opt_s if defined $opt_s;

##########
# 異体字テーブルの文字をUTF8からUCS2の16進表記に変換する
$srcfile = "$srcdir/NormKmap.txt";
open (FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;

    if (/^(.+)→(.+)$/) {
	# 文字の切り出し
	$org_char = $1;
	$rep_char = $2;

	# UCS2 16進表記に変換
	$org_ucs = sprintf("%04X", ord($1));
	$rep_ucs = sprintf("%04X", ord($2));

	print "$org_ucs;$rep_ucs\n";
    }
}
close(FIN);
