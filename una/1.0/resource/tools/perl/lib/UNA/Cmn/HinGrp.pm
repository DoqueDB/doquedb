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
# HinGrp.pm - use Common Hinshi Group Definition
#

package UNA::Cmn::HinGrp;

=head1 NAME

 UNA::Cmn::HinGrp - common形式の品詞グループ定義を扱う

=head1 SYNOPSIS

 use UNA::Cmn::HinGrp;
 $hingrp = new UNA::Cmn::HinGrp($hingrp_file);
 $hingrp->load($hingrp_file);
 $hingrp->set_from_gobitbl($gobitbl);
 $hingrp->{is_group}{$hin1}{$hin2}...
 $hingrp->dump_groups($file);

=head1 DESCRIPTION

 common形式の品詞グループ定義を扱うモジュールである。
 loadメソッドで読みこみ、set_from_gobitblメソッドで、
 活用グループ定義を品詞グループ定義としてとりこむことができる。

 $hingrp->{is_group}{$hin1}{$hin2}

 品詞グループ$hin1が、品詞$hin2を含むとき、この式は真になる。

=head1 HISTORY

 Ver.5.00 2002/04/30 UNA-CmnToolsとしてRCS管理に
 Ver.5.01 2002/07/31 mkmodicレビューにより見つかったバグを修正
 Ver.5.02 2002/08/20 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.dist
                     に変更

=cut

$DEBUG = 0;

=head2 new

 $hingrp = new UNA::Cmn::HinGrp($hingrp_file);

 UNA::Cmn::HinGrpのオブジェクトを返す。
 $hingrp_fileを指定すると、指定された品詞グループ定義ファイルを
 loadメソッドを使って読みこむ。

=cut

use lib "../tools/perl/lib";
use utf8;
use open IN => ":utf8";

sub new {
	my($class, $file) = @_;
	my $self =
	{
	 file => $file,		# 品詞グループ定義のファイル名
	 def => {},		# 品詞グループ定義
	 children => {},	# 展開後の品詞グループ定義
	 is_group => {},	# グループかどうかを記憶
	};
	bless $self, $class;
	$self->load($file) if $file;
	$self;
}

=head2 load

 $hingrp->load($hingrp_file);

 品詞グループ定義ファイルを読みこむ。

=cut

sub load {
	my($self, $file) = @_;
	open(HINGRP, $file) || die "can't read $file: $!\n";
	while (<HINGRP>) {
		chomp;
		s/^\s+//;
		s/\s+$//;
		s/(^|\s+)\#.*$//;	# コメントを除去
		next if !$_;		# 空行は無視
		# 行のパース
		my(@a) = split(/\s+/, $_);
		# 素性がないことをチェック
		for (@a) {
			# \x{ff5e}：半角形/全角形の「～」
			# \x{ff0d}：半角形/全角形の「－」
			if (/(＋|\x{ff0d}|※|＝|\x{ff5e})/) {
				die "ERROR: hinshi with features was found: $file($.): $_\n";
			}
		}
		my $parent = shift @a;
		if (!@a) {
			die "ERROR: no group definition: $file($.): $_\n";
		}
		# 二重定義のチェック(子)
		my @children = ();
		my %mark;
		my $cyclic;
		for (@a) {
			if ($parent eq $_) {
				$cyclic = 1;
			}
			if ($mark{$_}) {
				die "ERROR: redundant definition: $file($.): $parent $_\n";
			}
			$mark{$_}++;
			push(@children, $_)
		}
		if ($cyclic) {
			die "ERROR: group definition is cyclic: $file($.): $_\n";
		}
		# 二重定義のチェック(親)
		if (defined $self->{def}{$parent}) {
			die "ERROR: group redefinition: $file($.): $_\n";
		}
		# 登録
		$self->{def}{$parent} = \@children;
	}
	close HINGRP;
	$self->expand;
}

# 品詞グループ定義を展開する
sub expand {
	my($self) = @_;
	$self->{children} = {};	# 初期化
	$self->{busy} = {};		# 初期化
	for $parent (keys %{$self->{def}}) {
		$self->expand_it($parent);
	}
}

# ひとつのグループ定義を再帰的に展開する
sub expand_it {
	my($self, $parent) = @_;
	if (!defined $self->{children}{$parent}) { # まだ展開していない
		if ($self->{busy}{$parent}) { # 定義中だと循環定義
			die "ERROR: cyclic definition: $self->{file}: $parent\n";
		}
		$self->{busy}{$parent} = 1; # 定義中にする
		# 子を再帰的に
		for (@{$self->{def}{$parent}}) {
			$self->{is_group}{$parent}{$_}++; # 子自身
			if (!defined $self->{def}{$_}) {
				next;		# 定義がなければなにもしない
			}
			my $a = $self->expand_it($_);
			for (@$a) {
				$self->{is_group}{$parent}{$_}++;
			}
		}
		# 展開した結果を登録
		if ($self->{is_group}{$parent}) {
			my @a = keys %{$self->{is_group}{$parent}};
			$self->{children}{$parent} = \@a;
		}
		$self->{busy}{$parent} = 0; # もう定義中ではない。
	}
	$self->{children}{$parent};
}

=head2 set_from_gobitbl

 $hingrp->set_from_gobitbl($gobitbl);

 語尾テーブルの活用グループを、品詞グループとして登録する。
 $gobitblはUNA::Cmn::GobiTblのオブジェクトである。

=cut

use UNA::Cmn::GobiTbl;

sub set_from_gobitbl {
	my($self, $gobitbl) = @_;
	for $parent (keys %{$gobitbl->{is_group}}) {
		if ($self->{is_group}{$parent}) {
			# 既に定義されていたら同一か確認する
			my @gobi_def = keys %{$gobitbl->{is_group}{$parent}};
			my $gobi_def = join(' ', sort @gobi_def);
			my @hingrp_def = keys %{$self->{is_group}{$parent}};
			my $hingrp_def = join(' ', sort @hingrp_def);
			if ($gobi_def ne $hingrp_def) {
				die "ERROR: gobi group was already defined in $self->{file}: $parent\n: $gobi_def <=> $hingrp_def\n";
			}
			next; # 定義済みで同一ならばやることはない
		}
		my @children = keys %{$gobitbl->{is_group}{$parent}};
		for $child (@children) {
			if (defined $self->{is_group}{$child}) {
				die "ERROR: katsuyou hinshi was used as group in $self->{file}: $parent: $child\n";
			}
		}
		# 親子関係を登録
		$self->{def}{$parent} = \@children;
	}
	$self->expand;
}

=head2 dump_groups

 $hingrp->dump_groups($file);

 デバッグ用途のメソッド。全ての is_group の定義をファイル $file に
 出力する。

=cut

sub dump_groups {
	my($self, $file) = @_;
	open(DUMP, ">$file") || die "can't write $file: $!\n";
	binmode(DUMP, ":utf8");
	for $parent (sort keys %{$self->{is_group}}) {
		print DUMP "$parent\t";
		for $child (sort keys %{$self->{is_group}{$parent}}) {
			print DUMP " $child";
		}
		print DUMP "\n\n";
	}
    close DUMP;
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

 UNA::Cmn::Dic(3), UNA::Cmn::GobiTbl(3), UNA::Cmn::FeaLst(3),
 UNA::Cmn::Conn(3)

=cut

1;
