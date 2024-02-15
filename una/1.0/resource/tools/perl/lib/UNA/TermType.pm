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
# TermType.pm - use TermType Definition
#

package UNA::TermType;

=head1 NAME

 UNA::TermType - 検索語タイプリストを扱う

=head1 SYNOPSIS

 use UNA::TermType;
 $termtype = new UNA::TermType($termtype_file);
 $termtype->load($termtype_file);
 $term = $termtype->{term}[$term_no];
 $val = $termtype->{val}[$term_no];

=head1 DESCRIPTION

 検索語タイプリストファイル(termtype.lst)を扱うモジュールである。
 検索語タイプリストを読み込むと、検索語タイプ名から検索語タイプ値を
 得るための連想配列が作られる。

 $val = $termtype->{val}{$term};

=head1 HISTORY

 Ver.5.00 2014/12/05 FixHin.pmを流用して作成

=head1 METHODS

=cut

$DEBUG = 0;

use utf8;
use open IN => ":utf8";

=head2 new

 $termtype = new UNA::TermType($termtype_file);

 UNA::TermTypeのオブジェクトを返す。
 $termtype_fileが指定された場合は、検索語タイプリストファイルを読みこむ。

=cut

sub new {
    my($class, $file) = @_;
    my $self =
	{
	 file => $file,		# 検索語タイプリストのファイル名
	 val => {},		# 検索語タイプ値のハッシュ
	};
    bless $self, $class;
    $self->load($file) if $file;
    $self;
}

=head2 load

 $termtype->load($termtype_file);

 $termtype_fileで指定された検索語タイプリストを読みこむ。

=cut

sub load {
    my($self, $file) = @_;
    open(TERMTYPE, $file) || die "can't read $file: $!\n";
    while (<TERMTYPE>) {
	chomp;
	s/^\s+//;		# 行頭の空白を削除
	s/\s+$//;		# 行の後ろの空白を削除
	s/(^|\s+)\#.*$//;	# コメントを除去
	next if !$_;		# 空行は無視
	if (!/^(\S+)\s+(\S+)/) {
	    die "ERROR: bad format: $file($.): $_\n";
	}
	my($term, $val) = ($1, $2);
	$self->{val}{$term} = $val;
    }
    close TERMTYPE;
}

=head1 COPYRIGHT

 Copyright 2000,2014, 2023 Ricoh Company, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::MkMoDic>

=cut

1;
