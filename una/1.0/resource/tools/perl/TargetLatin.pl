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
# TargetLatin.pl
#
# 機能: 正規化対象のラテン文字を収集する。
# ・以下のブロックで、文字名が下記パターンに合致する文字
#     /(^|^FULLWIDTH )LATIN.*(LETTER|LIGATURE)/
#   - Basic Latin
#   - Latin-1 Supplement
#   - Latin Extended-A
#   - Latin Extended-B
#   - IPA Extensions
#   - Latin Extended Additional
#   - Halfwidth and Fullwidth Forms
#
# 実行方法: 
#   TargetLatin.pl -s DIR
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
Usage: TargetLatin.pl -s DIR
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

    $code   = $work[0];	# コード値
    $name   = $work[1];	# 文字名

    if (($code =~ /^00([2-6]|7[^F])/ || # Basic Latin
	 $code =~ /^00[A-F]/ || # Latin-1 Supplement
	 $code =~ /^01[0-7]/ || # Latin Extended-A
	 $code =~ /^(01[89A-F]|02[0-4])/ || # Latin Extended-B
	 $code =~ /^02[5-9A]/ || # IPA Extensions
	 $code =~ /^1E/ ||	# Latin Extended Additional
	 $code =~ /^FF[0-9A-E]/) && # Halfwidth and Fullwidth Forms
	$name =~ /(^|^FULLWIDTH )LATIN.*(LETTER|LIGATURE)/) {

	print "$code\n";
    }
}
close(FIN);

