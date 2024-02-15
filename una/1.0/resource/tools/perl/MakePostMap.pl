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
# MakePostMap.pl
#
# 機能: 後処理テーブルの作成
#
# 実行方法:
#   MakePostMap.pl -d DIR [OPTIONS] | sort -u > 出力ファイル名
#    -d DIR  : 中間データの所在 ($datadir) [required]
#    -h      : ヘルプ
#
# 入力ファイル:
#   $datadir/preMap.dat       - 前処理テーブル
#   $datadir/NormCode.dat - コード変換テーブル
#
# 出力ファイルのフォーマット: code1;code2
#     code1: 変換前コード
#     code2: 変換後コード
#
################################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDOUT, ":utf8");

##########
# パラメータ評価
use Getopt::Std;
getopts('d:h');
if ($opt_h || $#ARGV > 0) {
    die <<EOU;
Usage: MakePostMap.pl -d DIR [OPTIONS]
   -d DIR  : Intermediate data directory [required]
   -h      : help
EOU
}
$datadir = $opt_d if defined $opt_d;

##########
# 前処理テーブルの読み込み
$srcfile = "$datadir/preMap.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");
    $org = $work[0];
    $rep = $work[1];
    @pre_map[hex($org)] = $rep;
}
close(FIN);

# コード変換テーブルの読み込み
$srcfile = "$datadir/NormCode.dat";
open(FIN, $srcfile) || die "ERROR: cannot open $srcfile\n";

while (<FIN>) {
    chop;
    @work = split(";");
    $org = $work[0];	# 変換前コード
    $rep = $work[1];	# 変換後コード

    &chrep($rep);

    # 変換前コードを前処理結果に合わせて修正
    if (($org_pre = @pre_map[hex($org)]) !~ /^$/) {
	$org = $org_pre if ($org_pre !~/^077F/); # see MakePreMap.pl
    }
    # 変換後コードを前処理結果に合わせて修正
    if (($rep_pre = @pre_map[hex($rep)]) !~ /^$/) {
	$rep = $rep_pre if ($rep_pre !~/^077F/); # see MakePreMap.pl
    }
    # 変換前コードが変換後コードと異なる場合は出力
    if ($org !~ /^0000$/ && $org !~ "$rep") {
	print "$org;$rep\n";
    }
}
close(FIN);

sub chrep{
    my($rep) = @_;
    if ($rep !~ /^\S{4}$/){
        die "ERROR: MakePostMap.pl: $rep format error\n";
    }
}
