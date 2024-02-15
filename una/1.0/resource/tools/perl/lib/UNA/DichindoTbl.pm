#!/usr/bin/perl
# 
# Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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
# DichindoTbl.pm - use Dictionary Hindo Table
#

package UNA::DichindoTbl;

=head1 NAME

 UNA::DichindoTbl - 辞書頻度テーブルを扱う

=head1 SYNOPSIS

 use UNA::DichindoTbl;
 $dichindotbl = new UNA::DichindoTbl($dichindoTbl_file);
 $dichindotbl->load($dichindoTbl_file);
 @hins = @{$dichindotbl->{hins}};
 @hin_codes = @{$dichindotbl->{hin_codes}}
 $hin_wnum = $dichindotbl->{hin_wnum}{$hin}
 $hin_hindo = $dichindotbl->{hin_hindo}{$hin};

=head1 DESCRIPTION

 品詞リストを扱うモジュールである。
 loadメソッドで、品詞リストファイルを読みこんだ後は、
 以下のようにして、品詞名の配列とアプリ品詞コード(UNA品詞番号)の配列
 を得ることができる。

 @hins = @{$dichindotbl->{hins}};
 @hin_codes = @{$dichindotbl->{hin_codes}};

 また以下のように、品詞内単語数と品詞内単語頻度累計を得ることもできる。

 $hin_wnum = $dichindotbl->{hin_wnum}{$hin}
 $hin_hindo = $dichindotbl->{hin_hindo}{$hin};

=head1 HISTORY

 Ver.1.1 2018/01/16 初版作成

=head1 METHODS

=cut

$DEBUG = 0;

use utf8;
use open IN => ":utf8";

=head2 new

 $dichindotbl = new UNA::DichindoTbl($dichindotbl_file);

 UNA::DichindoTblのオブジェクトを作成する。
 $hinlst_fileを指定した場合、品詞リストファイルを
 loadメソッドを使って、読みこむ。

=cut

sub new {
    my($class, $file) = @_;
    my $self =
	{
	 file => $file,		# 辞書頻度テーブルのファイル名
	 hins => [],		# 品詞
	 hin_codes => [],	# 品詞コード
	 hin_wnum => {},	# その品詞に属する単語数
	 hin_hindo => {},	# 品詞毎の頻度(単語頻度の測定値の和)
	 hindo => {},		# 品詞毎の頻度(単語頻度の推定値の和)
	};
    bless $self, $class;
    $self->load($file) if $file;
    $self;
}

=head2 load

 $dichindotbl->load($dichindotbl_file);

 $dichindotbl_fileで指定された品詞リストファイルを読みこむ。

=cut

sub load {
    my($self, $file) = @_;
    open(DICHINDOTBL, $file) || die "can't read $file: $!\n";
    my @a;
    while (<DICHINDOTBL>) {
		chomp;
		my $line = $_;
		s/^\s+//;		# 行頭の空白を削除
		s/\s+$//;		# 行の後ろの空白を削除
		s/(^|\s+)\#.*$//;	# コメントを除去
		next if !$_;		# 空行は無視
		# 行のパース
		if (!/^(\S+)\s{1}(\d+)\s{1}(\S+)\s{1}(\d+)\s{1}(\d+\.*\d*)\s{1}(\d+\.*\d*)$/) {
		    die "ERROR: bad format: $file($.): $line\n";
		}
		my($hin, $hno, $code, $wnum, $hin_hindo, $hindo) = ($1, $2, $3, $4, $5, $6);
		# 一旦配列に登録
		push(@a, [$hin, $hno, $code, $wnum, $hin_hindo, $hindo]);
		for (sort {$a->[1] <=> $b->[1]} @a) { # $hnoでソート
			# 単に配列に登録
			my($hin, $hno, $code, $wnum, $hin_hindo, $hindo) = @$_;
			push(@{$self->{hins}}, $hin);
			push(@{$self->{hin_codes}}, $code);
			$self->{hin_wnum}{$hin} = $wnum;
			$self->{hin_hindo}{$hin} = $hin_hindo;
			$self->{hindo}{$hin} = $hindo;
		}
    }
    close DICHINDOTBL;
}

=head1 COPYRIGHT

 Copyright 2000, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

 UNA::Cmn::Conn(3), UNA::Cmn::HinGrp(3), UNA::Cmn::HinMap(3)

=cut

1;
