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
# HinMap.pm - use Hinshi Map Definition
#

package UNA::HinMap;

=head1 NAME

 UNA::HinMap - 品詞マップ定義を扱う

=head1 SYNOPSIS

 use UNA::HinMap;
 $hinmap = new UNA::HinMap($hinmap_file);
 $hinmap->load($hinmap_file);
 @defs = @{$hinmap->{defs}};
 ($rexpr, $hin_code, $level) = @{$defs[$i]};

=head1 DESCRIPTION

 品詞マップ定義を扱うモジュールである。
 品詞マップ定義は、形態素品詞を指定する正規表現と、
 正規表現マッチを行うレベル、マッチしたときの品詞コードからなる。
 loadメソッドで読みこんだ後は、以下の方法で、値を得ることができる。
    
 @defs = @{$hinmap->{defs}};
 ($rexpr, $hin_code, $level) = @{$defs[$i]};

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

 $hinmap = new UNA::HinMap($hinmap_file);

 UNA::HinMapのオブジェクトを返す。
 $hinmap_fileが指定された場合は、品詞マップ定義を読みこむ。

=cut

sub new {
    my($class, $file) = @_;
    my $self =
	{
	 file => $file,		# 品詞グループ定義のファイル名
	 defs => [],		# 定義そのもの
	};
    bless $self, $class;
    $self->load($file) if $file;
    $self;
}

=head2 load

 $hinmap->load($hinmap_file);

 $hinmap_fileで指定された品詞マップ定義を読みこむ。

=cut

sub load {
    my($self, $file) = @_;
    open(HINMAP, $file) || die "can't read $file: $!\n";
    while (<HINMAP>) {
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
	my($rexpr, $hin_code) = ($1, $2);
	push(@{$self->{defs}}, [$rexpr, $hin_code, $level]);
    }
    close HINMAP;
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

 UNA::Cmn::Dic(3), UNA::Cmn::HinGrp(3), UNA::Cmn::HinLst(3)

=cut

1;
