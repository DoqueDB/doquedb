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
# FeaLst.pm - use Common Feature List
#

package UNA::Cmn::FeaLst;

=head1 NAME

 UNA::Cmn::FeaLst - common形式の接続素性定義を扱う

=head1 SYNOPSIS

 use UNA::Cmn::FeaLst;
 $fealst = new UNA::Cmn::FeaLst($fealst_file);
 $fealst->load($fealst_file);
 $fealst->set_from_hingrp($hingrp);
 $conn_flag = $fealst->{conn_flag}{$feature}{$hin}{$hin2}

=head1 DESCRIPTION

 common形式の接続素性定義を扱うモジュールである。
 loadメソッドで、ファイルを読みこみ、set_from_hingrpメソッドで、
 品詞グループ定義によって品詞グループを展開することができる。
 
 $conn_flag = $fealst->{conn_flag}{$feature}{$hin}{$hin2}

 素性$featureがついた品詞$hinと$hin2との接続を得ることができる。
 なお、素性が前接素性ならば、$hin2が前に来る場合の接続を表す。
 素性が後接素性ならば、$hin2が後ろに来る場合の接続を表す。

=head1 HISTORY

 Ver.5.00 2002/04/30 UNA-CmnToolsとしてRCS管理に
 Ver.5.01 2002/08/13 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.dist
                     に変更

=head1 METHODS

=cut

$DEBUG = 0;

use lib "../tools/perl/lib";
use utf8;
use open IN => ":utf8";

=head2 new

 $fealst = new UNA::Cmn::FeaLst($fealst_file);

 UNA::Cmn::FeaLstのオブジェクトを返す。
 $fealst_fileで指定された素性定義ファイルをloadメソッドにより読みこむ。
 $fealst_fileが省略された場合には、ファイルは読みこまない。

=cut

sub new {
	my($class, $file) = @_;
	my $self =
	{
	 file => $file,		# 接続素性定義のファイル名
	 defs => [],		# 定義そのものを記録
	 conn_flag => {},	# 接続フラグを記録
	};
	bless $self, $class;
	$self->load($file) if $file;
	$self;
}

=head2 load

 $fealst->load($fealst_file);

 素性定義ファイルを読みこむ。

=cut

sub load {
	my($self, $file) = @_;
	open(FEALST, $file) || die "can't read $file: $!\n";
	my($feature, @hins);
	my %features; # 一度使った素性を記録
	while (<FEALST>) {
		chomp;
		# 行のパース
		s/^\s+//;
		s/\s+$//;
		s/(^|\s+)\#.*$//;	# コメントを除去
		next if !$_;		# 空行は無視
		if (!/\s/) {
			# 素性名
			# \x{ff5e}：半角形/全角形の「～」
			# \x{ff0d}：半角形/全角形の「－」
			if (!/^(\x{ff5e}|！|＋|\x{ff0d}|＝)/ && !/(\x{ff5e}|！|＋|\x{ff0d}|＝)$/) {
				die "ERROR: bad feature name: $file($.): $_\n";
			}
			$feature = $_;
			@hins = ();
			if ($features{$feature}) {
				die "ERROR: feature redefinition: $file($.): $feature: $_\n";
			}
		}
		else {
			if (!$feature) {
				die "ERROR: no feauture name specified: $file($.): $_\n";
			}
			if (/^\*\s+(.+)/) {
				# 該当品詞の定義
				@hins = split(/\s+/, $1);
				# 二重定義のチェック
				my %mark;
				for (@hins) {
					if ($mark{$_}) {
						die "ERROR: redefinition: $file($.): $feature: $_\n";
					}
					$mark{$_}++;
				}
			}
			else {
				# 素性の定義
				if (!@hins) {
					die "ERROR: no hinshi has feature($feature): $file($.): $_\n";
				}
				my($flag, @hin2) = split(/\s+/, $_);
				if ($flag !~ /^[A-Za-z][\+\-]*$/) {
					die "ERROR: bad flag: $file($.): $feature: $_\n";
				}
				$features{$feature} = 1;
				for $hin (@hins) {
					for $hin2 (@hin2) {
						$self->{conn_flag}{$feature}{$hin}{$hin2} = $flag;
						push(@{$self->{defs}}, [$feature, $hin, $hin2, $flag]);
					}
				}
	    	}
		}
    }
    close FEALST;
}

=head2 set_from_hingrp

 $fealst->set_from_hingrp($hingrp);

 品詞グループ定義によって、接続定義を展開する。
 $hingrpは、UNA::Cmn::HinGrpのオブジェクト。

=cut

use UNA::Cmn::HinGrp;

sub set_from_hingrp {
	my($self, $hingrp) = @_;
	return if !$hingrp;
	$self->{conn_flag} = {};	#  一旦クリア
	for $def (@{$self->{defs}}) {
		my($feature, $hin1, $hin2, $flag) = @$def;
		# $hin1を展開する。
		my @hin1 = ($hin1);
		if ($hingrp->{is_group}{$hin1}) {
			push(@hin1, keys %{$hingrp->{is_group}{$hin1}});
		}
		# $hin2を展開する
		my @hin2 = ($hin2);
		if ($hingrp->{is_group}{$hin2}) {
			push(@hin2, keys %{$hingrp->{is_group}{$hin2}});
		}
		# 単純に展開しただけ再定義する
		for $hinx1 (@hin1) {
			for $hinx2 (@hin2) {
				$self->{conn_flag}{$feature}{$hinx1}{$hinx2} = $flag; # 後優先
			}
		}
    }
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

 UNA::Cmn::Dic(3), UNA::Cmn::HinGrp(3), UNA::Cmn::GobiTbl(3),
 UNA::Cmn::Conn(3)

=cut

1;
