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
# UniHinMap.pm - use Unified Hinshi Map Definition
#

package UNA::UniHinMap;

=head1 NAME

 UNA::UniHinMap - 統合品詞マップ定義を扱う

=head1 SYNOPSIS

 use UNA::UniHinMap;
 $unihinmap = new UNA::UniHinMap($unihinmap_file);
 $unihinmap->load($unihinmap_file);
 @defs = @{$unihinmap->{defs}};
 ($rexpr, $uni_hin_code, $level) = @{$defs[$i]};

=head1 DESCRIPTION

 統合品詞マップ定義を扱うモジュールである。
 統合品詞マップ定義は、形態素品詞を指定する正規表現と、
 正規表現マッチを行うレベル、マッチしたときの統合品詞番号からなる。
 loadメソッドで読みこんだ後は、以下の方法で、値を得ることができる。
    
 @defs = @{$unihinmap->{defs}};
 ($rexpr, $uni_hin_code, $level) = @{$defs[$i]};

=head1 HISTORY

 Ver.5.00 2014/11/21 HinMap.pmを流用して作成

=head1 METHODS

=cut

$DEBUG = 0;

use utf8;
use open IN => ":utf8";

=head2 new

 $hinmap = new UNA::UniHinMap($hinmap_file);

 UNA::UniHinMapのオブジェクトを返す。
 $hinmap_fileが指定された場合は、品詞マップ定義を読みこむ。

=cut

sub new {
    my($class, $file) = @_;
    my $self =
	{
	 file => $file,		# 統合品詞マップ定義のファイル名
	 defs => [],		# 定義そのもの
	};
    bless $self, $class;
    $self->load($file) if $file;
    $self;
}

=head2 load

 $unihinmap->load($unihinmap_file);

 $unihinmap_fileで指定された統合品詞マップ定義を読みこむ。

=cut

sub load {
    my($self, $file) = @_;
    open(UNIHINMAP, $file) || die "can't read $file: $!\n";
    while (<UNIHINMAP>) {
	chomp;
	my $line = $_;
	s/^\s+//;		# 行頭の空白を削除
	s/\s+$//;		# 行の後ろの空白を削除
	s/(^|\s+)\#.*$//;	# コメントを除去
	next if !$_;		# 空行は無視
	# 行のパース
	my $level = 0;
	while (s/^\+\s+//) {
	    $level++;
	}
	if (!/^(\S+)\s*(\S*)$/) {
	    die "ERROR: bad format: $file($.): $line\n";
	}
	my($rexpr, $uni_hin_code) = ($1, $2);
	push(@{$self->{defs}}, [$rexpr, $uni_hin_code, $level]);
    }
    close UNIHINMAP;
}

=head1 COPYRIGHT

 Copyright 2000,2014, 2023 Ricoh Company, Ltd. All rights reserved.

=head1 SEE ALSO

 UNA::Cmn::Dic(3), UNA::Cmn::HinGrp(3), UNA::Cmn::HinLst(3), UNA::Cmn::HinMap(3)

=cut

1;
