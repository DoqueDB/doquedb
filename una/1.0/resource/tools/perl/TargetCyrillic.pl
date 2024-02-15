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
# TargetCyrillic.pl
#
# 機能: 正規化対象のキリル文字を収集する。
# ・Cyrillic ブロックで、文字名が /^CYRILLIC.*(LETTER|LIGATURE)/ に合致する文字
#
# 実行方法: 
#   TargetCyrillic.pl -s DIR
#    -s DIR : 元データの所在 ($srcdir) [required]
#    -h     : ヘルプ
#
# 入力ファイル: 
#   $srcdir/UnicodeData-1.1.5.txt
#
# 出力ファイルのフォーマット: code
#   code: コード値
#
################################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

##########
# パラメータ評価
use Getopt::Std;
getopts('s:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: TargetCyrillic.pl -s DIR
   -s DIR : Source data directory [required]
   -h     : help
EOU
}
$srcdir = $opt_s if defined $opt_s;

##########
# UnicodeData.txtから対象コードを抽出する
$srcfile = "$srcdir/UnicodeData-1.1.5.txt";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");		

    $code   = $work[0];		# コード値
    $name   = $work[1];		# 文字名

    # 対象コードを出力
    if ($code =~ /^04/ && $name =~ /^CYRILLIC.*(LETTER|LIGATURE)/) {
	print "$code\n";
    }
}
close(FIN);

