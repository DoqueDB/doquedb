#!/usr/bin/perl
# 
# Copyright (c) 2000,2014, 2023 Ricoh Company, Ltd.
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
# TermTypeMap.pm - use TermTypeMap Definition
#

package UNA::TermTypeMap;

=head1 NAME

 UNA::TermTypeMap - 検索語タイプマップを扱う

=head1 SYNOPSIS

 use UNA::TermTypeMap;
 $termtypemap = new UNA::TermTypeMap($termtypemap_file);
 $termtypemap->load($termtypemap_file);
 $hin = $termtypemap->{hin}[$hin_no];
 $term = $termtypemap->{term}[$hin_no];

=head1 DESCRIPTION

 検索語タイプマップファイル(termtypemap.lst)を扱うモジュールである。
 検索語タイプマップを読み込むと2つの配列が作られる。
 一つは、品詞名の配列であり、もうひとつは検索語タイプの配列である。
 添字が同じ要素どうしが対応づいているが、添字そのものは意味をもたない。

 $hin = $termtypemap->{hin}[$no];
 $term = $termtypemap->{term}[$no];

=head1 HISTORY

 Ver.5.00 2014/12/05 FixHin.pmを流用して作成

=head1 METHODS

=cut

$DEBUG = 0;

use utf8;
use open IN => ":utf8";

=head2 new

 $termtypemap = new UNA::TermTypeMap($termtypemap_file);

 UNA::TermTypeMapのオブジェクトを返す。
 $termtypemap_fileが指定された場合は、検索語タイプマップリストファイルを読みこむ。

=cut

sub new {
    my($class, $file) = @_;
    my $self =
	{
	 file => $file,		# 検索語タイプマップのファイル名
	 hin => [],		# 品詞名のリスト
	 term => [],		# 検索語タイプのリスト
	};
    bless $self, $class;
    $self->load($file) if $file;
    $self;
}

=head2 load

 $termtypemap->load($termtypemap_file);

 $termtypemap_fileで指定された検索語タイプマップを読みこむ。

=cut

sub load {
    my($self, $file) = @_;
    open(TERMTYPEMAP, $file) || die "can't read $file: $!\n";
    while (<TERMTYPEMAP>) {
	chomp;
	s/^\s+//;		# 行頭の空白を削除
	s/\s+$//;		# 行の後ろの空白を削除
	s/(^|\s+)\#.*$//;	# コメントを除去
	next if !$_;		# 空行は無視
	if (!/^(\S+)\s+(\S+)/) {
	    die "ERROR: bad format: $file($.): $_\n";
	}
	my($hin, $term) = ($1, $2);
	push(@{$self->{hin}}, $hin);
	push(@{$self->{term}}, $term);
    }
    close TERMTYPEMAP;
}

=head1 COPYRIGHT

 Copyright 2000,2014, 2023 Ricoh Company, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::MkMoDic>

=cut

1;
