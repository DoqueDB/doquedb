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
# Fixhin.pm - use Fixhin Definition
#

package UNA::FixHin;

=head1 NAME

 UNA::FixHin - 固定品詞リストを扱う

=head1 SYNOPSIS

 use UNA::FixHin;
 $fixhin = new UNA::FixHin($fixhin_file);
 $fixhin->load($fixhin_file);
 $hin = $fixhin->{hin}[$hin_no];
 $label = $fixhin->{label}[$hin_no];

=head1 DESCRIPTION

 固定品詞リストファイル(fixhin.lst)を扱うモジュールである。
 固定品詞リストを読み込むと、品詞番号(0以上)を添字とする、
 2つの配列が作られる。
 一つは、品詞名の配列であり、もうひとつはラベルの配列である。

 $hin = $fixhin->{hin}[$hin_no];
 $label = $fixhin->{label}[$hin_no];

 ラベルは、その品詞にあたるC言語の変数名などを定義するために用いられる。

=head1 HISTORY

 Ver.5.00 2002/04/30 UNA-CmnToolsとしてRCS管理に
 Ver.5.01 2002/08/20 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.dist
                     に変更

=head1 METHODS

=cut

$DEBUG = 0;
#$DEBUG = 1;

use utf8;
use open IN => ":utf8";

=head2 new

 $fixhin = new UNA::FixHin($fixhin_file);

 UNA::FixHinのオブジェクトを返す。
 $fixhin_fileが指定された場合は、固定品詞リストファイルを読みこむ。

=cut

sub new {
    my($class, $file) = @_;
    my $self =
	{
	 file => $file,		# 品詞グループ定義のファイル名
	 hin => [],		# 固定品詞のリスト
	 label => [],		# 固定品詞のラベルのリスト
	};
    bless $self, $class;
    $self->load($file) if $file;
    $self;
}

=head2 load

 $fixhin->load($fixhin_file);

 $fixhin_fileで指定された固定品詞リストを読みこむ。

=cut

sub load {
    my($self, $file) = @_;
    open(FIXHIN, $file) || die "can't read $file: $!\n";
    while (<FIXHIN>) {
	chomp;
	s/^\s+//;		# 行頭の空白を削除
	s/\s+$//;		# 行の後ろの空白を削除
	s/(^|\s+)\#.*$//;	# コメントを除去
	next if !$_;		# 空行は無視
	if (!/^(\S+)\s+(\S+)/) {
	    die "ERROR: bad format: $file($.): $_\n";
	}
	my($hin, $label) = ($1, $2);
	push(@{$self->{hin}}, $hin);
	push(@{$self->{label}}, $label);
    }
    close FIXHIN;
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::MkMoDic>

=cut

1;
