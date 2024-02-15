#!/usr/bin/perl
# 
# Copyright (c) 2004, 2005, 2023 Ricoh Company, Ltd.
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
# HinhindoTbl.pm - use HinhindoTbl Definition
#

package UNA::HinhindoTbl;

=head1 NAME

 UNA::HinhindoTbl - 基本品詞頻度テーブルを扱う

=head1 SYNOPSIS

 use UNA::HinhindoTbl;
 $hinhindotbl = new UNA::HinhindoTbl($hinhindotbl_file);
 $hinhindotbl->load($hinhindotbl_file);

=head1 DESCRIPTION

 基本品詞頻度テーブルファイル(hinhindo.tbl)を扱うモジュールである。
 基本品詞頻度テーブルを読み込む。
 読み込み時にフォーマットのチェックを行い、合わない場合はエラーを出力し
 停止する。問題がない場合は、前接素性付き品詞をキーとしてハッシュに格納する。
 
 $fix_hin = $hinhindotbl->{hinhindo}{hin}{$maefeahin};
 $fix_hindo = $hinhindotbl->{hinhindo}{hindo}[$maefeahin];
 $fix_basehindo = $hinhindotbl->{hinhindo}{basehindo}[$maefeahin];
 $fix_real_hindo = $hinhindotbl->{hinhindo}{real_hindo}{$maefeahin};
 $fix_real_basehindo = $hinhindotbl->{hinhindo}{real_basehindo}{$maefeahin};

=head1 HISTORY

 Ver.5.00 2005/04/14 UNA-CmnToolsとしてRCS管理に
                     最初の動作版

=head1 METHODS

=cut

use utf8;
use open IN => ":utf8";

=head2 new

 $hinhindotbl = new UNA::HinhindoTbl($hinhindotbl_file);

 UNA::HinhindoTblのオブジェクトを返す。
 $hinhindotbl_fileが指定された場合は、基本品詞頻度テーブルファイルを読みこむ。

=cut

sub new {
	my($class, $file) = @_;
	my $self =
	{
		file => $file,		# 基本品詞頻度テーブルのファイル名
		hinhindo => {},		# 前接素性付き品詞もしくは基本品詞に対応する頻度
	};
	bless $self, $class;
	$self->load($file) if $file;
	$self;
}

=head2 load

 $hinhindotbl->load($hinhindotbl_file);

 $hinhindotbl_fileで指定された基本品詞頻度テーブルを読み、記述されている頻度を
 ハッシュに格納する。

=cut

sub load {
	my($self, $hinhindotbl) = @_;

	# 基本品詞頻度テーブルを読み込み配列に格納する
	# 形式は品詞, 前接素性付き品詞頻度, 基本品詞頻度, 前接素性付き品詞real頻度, 基本品詞real頻度
	open(HINHINDOTBL, $hinhindotbl) || die "can't read $hinhindotbl: $!\n";
	while (<HINHINDOTBL>) {
		chomp;
		s/^\s+//;           # 行頭の空白を削除
		s/\s+$//;           # 行の後ろの空白を削除
		s/(^|\s+)\#.*$//;   # コメントを除去
		next if !$_;        # 空行は無視
		if(!/^(\S+),\s{1}(\d+\.*\d*),\s{1}(\d+\.*\d*),\s{1}(\d+\.*\d*),\s{1}(\d+\.*\d*)$/){
		    die "ERROR: bad format: $hinhindotbl($.): $_\n";
		}
		my $maefeahin = $1;
		my $hindo = $2;
		my $basehindo = $3;
		my $real_hindo = $4;
		my $real_basehindo = $5;
		$self->{hinhindo}{hin}{$maefeahin} = 1;
		$self->{hinhindo}{hindo}{$maefeahin} = $hindo;
		$self->{hinhindo}{basehindo}{$maefeahin} = $basehindo;
		$self->{hinhindo}{real_hindo}{$maefeahin} = $real_hindo;
		$self->{hinhindo}{real_basehindo}{$maefeahin} = $real_basehindo;
	}
	close HINHINDOTBL;
}

=head1 COPYRIGHT

 Copyright 2005, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::HinhindoTbl>

=cut

1;
