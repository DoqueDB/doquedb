#!/usr/bin/perl
# 
# Copyright (c) 2000,2002, 2023 Ricoh Company, Ltd.
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
#
# HindoMap.pm - use HindoMap Table
#

package UNA::HindoMap;

=head1 NAME

 UNA::HindoMap - 頻度マップを扱う

=head1 SYNOPSIS

 use UNA::HindoMap;
 $hindomap = new UNA::HindoMap($hindomap_file);
 $hindomap->load($hindomap_file);
 $new_hindo = $hindomap->estimate($hindo);
 $new_hindo = $hindomap->estimate($hindo_l, $hindo_r)

=head1 DESCRIPTION

 頻度マップファイル(hindomap.tbl)を扱うモジュールである。
 頻度マップは、測定された頻度から推定される頻度への変換表である。
 頻度マップ中、測定された頻度が存在しないものについては、
 測定された頻度＝推定される頻度とみなす。

=head1 HISTORY

 Ver.5.00 2002/04/30 UNA-CmnToolsとしてRCS管理に
 Ver.5.01 2002/08/20 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.dist
                     に変更

=head1 METHODS

=cut

$DEBUG = 0;

use utf8;
use open IN => ":utf8";

=head2 new

 $hindomap = new UNA::HindoMap($hindomap_file);

 UNA::HindoMapのオブジェクトを返す。
 $hindomap_fileが指定された場合は、頻度マップファイルを読みこむ。

=cut

sub new {
	my($class, $file) = @_;
	my $self =
	{
	 file => $file,		# 品詞グループ定義のファイル名
	 hindo2new => {},	# 測定された頻度→推定される頻度の変換表
	};
	bless $self, $class;
	$self->load($file) if $file;
	$self;
}

=head2 load

 $hindomap->load($hindomap_file);

 $hindomap_fileで指定された頻度マップを読みこむ。

=cut

sub load {
	my($self, $file) = @_;
	open(HINDOMAP, $file) || die "can't read $file: $!\n";
	while (<HINDOMAP>) {
		chomp;
		s/^\s+//;		# 行頭の空白を削除
		s/\s+$//;		# 行の後ろの空白を削除
		s/(^|\s+)\#.*$//;	# コメントを除去
		next if !$_;		# 空行は無視
		if (!/^(\S+)\s+(\S+)$/) {
		    die "ERROR: bad format: $file($.): $_\n";
		}
		my($old, $new) = ($1, $2);
		$old += 0;		# 念のため数値にする
		$new += 0;		# 念のため数値にする
		$self->{hindo2new}{$old} = $new;
	}
	close HINDOMAP;
}

=head2 estimate

 $new_hindo = $hindomap->estimate($hindo);
 $new_hindo = $hindomap->estimate($hindo1, $hindo2, ...)

 測定された頻度$hindoから、推定される頻度$new_hindoを得る。
 頻度マップ中に測定された頻度が存在しない場合は、
 推定される頻度＝測定された頻度
 となる。

 引数に2つ以上の頻度を指定した場合には、それぞれを推定される頻度に
 変換した後、平均値を返す

=cut

sub estimate {
	my($self, @olds) = @_;
	if (!@olds) {
		die "HindoMap.pm: no argument to estimate()\n";
	}
	my $sum = 0;
	my $n = 0;
	for $old (@olds) {
		$old += 0;			# 念のため数値にする
		if (defined $self->{hindo2new}{$old}) {
			$sum += $self->{hindo2new}{$old}
		}
		else {
			$sum += $old;
		}
		$n++;
	}
	$sum / $n;			# 平均値を返す
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::MkMoDic>

=cut

1;
