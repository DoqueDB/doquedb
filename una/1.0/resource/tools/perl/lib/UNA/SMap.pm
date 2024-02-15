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
# SMap.pm - use SMap Definition
#

package UNA::SMap;

=head1 NAME

 UNA::SMap - シンボルマップを扱う

=head1 SYNOPSIS

 use UNA::SMap;
 $smap = new UNA::SMap($smap_file);
 $smap->load($smap_file);
 ($sym, $hindo) = $smap->get_sym_and_hindo($sym);

=head1 DESCRIPTION

 シンボルマップファイル(smap.lst)を扱うモジュールである。
 シンボルマップは、接続シンボルから頻度への変換表である。

 $hindo = $smap->{sym2hindo}{$sym};

 このように、接続シンボル $sym から 頻度 $hindo への変換ができる。

 $sym = $smap->{default_sym};
  
 これは、接続が定義されていない場合のデフォルトの接続シンボルを返す。
 これは、シンボルマップファイル中で最初に定義される接続シンボルでもある。

 $d = $smap->{sym2hindo}{'+'};
 $d = $smap->{sym2hindo}{'-'};

 このように、例外的に、'+' と '-' は、接続シンボルではなく、頻度の補正値
 として使われる。

=head1 HISTORY

 Ver.5.00 2002/04/30 UNA-CmnToolsとしてRCS管理に
 Ver.5.01 2002/08/13 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.dist
                     に変更

=head1 METHODS

=cut

$DEBUG = 0;

use utf8;
use open IN => ":utf8";

=head2 new

 $smap = new UNA::SMap($smap_file);

 UNA::SMapのオブジェクトを返す。
 $smap_fileが指定された場合は、シンボルマップリストファイルを読みこむ。

=cut

sub new {
    my($class, $file) = @_;
    my $self =
	{
	 file => $file,		# 品詞グループ定義のファイル名
	 sym2hindo => {},	# 接続シンボル→値の変換表
	 default_sym => '',	# デフォルトの接続シンボル
	};
    bless $self, $class;
    $self->load($file) if $file;
    $self;
}

=head2 load

 $smap->load($smap_file);

 $smap_fileで指定されたシンボルマップを読みこむ。

=cut

sub load {
    my($self, $file) = @_;
    open(SMAP, $file) || die "can't read $file: $!\n";
    while (<SMAP>) {
	chomp;
	s/^\s+//;		# 行頭の空白を削除
	s/\s+$//;		# 行の後ろの空白を削除
	s/(^|\s+)\#.*$//;	# コメントを除去
	next if !$_;		# 空行は無視
	if (!/^(\S+)\s+(\S+)/) {
	    die "ERROR: bad format: $file($.): $_\n";
	}
	my($sym, $hindo) = ($1, $2);
	$self->{sym2hindo}{$sym} = $hindo;
	$self->{default_sym} ||= $sym;	# 最初の$symだけが設定される
    }
    close SMAP;
}

=head2 get_sym_and_hindo

 ($sym, $hindo) = $smap->get_sym_and_hindo($sym);

 接続シンボルから、頻度$hindoを得る。接続シンボルには、"t+"、"y--" と
 いった補正用シンボルがついていてもよい。返り値には接続シンボルも
 そのまま返るが、$symから''の場合には、デフォルトの接続シンボルが返る。

=cut

sub get_sym_and_hindo {
    my($self, $sym) = @_;
    $sym = $self->{default_sym} if !$sym;
    $sym = "x" if !$sym; # default_symも無い場合のデフォルト
    my $hindo = 0;
    for (split //, $sym) {
	my $h = $self->{sym2hindo}{$_} || 0;
	if (/^[\+\-]/) {
	    $hindo *= $h;
	}
	else {
	    $hindo = $h;
	}
    }
    ($sym, $hindo);
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::MkMoConn>

=cut

1;
