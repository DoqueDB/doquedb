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
# MkMoConn.pm - Make (UNA) Morphological Connection Table
#

package UNA::MkMoConn;

=head1 NAME

 UNA::MkMoConn - 形態素品詞接続表ソースを作る

=head1 SYNOPSIS

 use UNA::MkMoConn;
 $mkconn = new UNA::MkMoConn;
 $mkconn->load('conn', $conn_file);
 $mkconn->load('hingrp', $hingrp_file);
 $mkconn->load('gobitbl', $gobitbl_file);
 $mkconn->load('fealst', $fealst_file);
 $mkconn->load('hinlst', $hinlst_file);
 $mkconn->load('smap', $smap_file);
 $mkconn->make_morphconn($morphconn_file);

=head1 DESCRIPTION

 形態素品詞接続表ソースを作るモジュールである。
 loadメソッドで各種データファイルを読みこんだ後に、
 make_morphconnメソッドで、接続表ソースを作成できる。
 ファイルの文字コードは、$icodeや$ocodeで指定できる。

=head1 HISTORY

 Ver.5.00 2002/04/30 UNA-CmnToolsとしてRCS管理に
 Ver.5.01 2002/08/20 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.distに変更

=head1 METHODS

=cut

$DEBUG = 0;

use lib "../tools/perl/lib";
use utf8;
use open IN  => ":utf8";
use open OUT => ":utf8";

use UNA::Cmn;
use UNA::Cmn::Conn;
use UNA::Cmn::HinGrp;
use UNA::Cmn::GobiTbl;
use UNA::Cmn::FeaLst;
use UNA::HinLst;
use UNA::SMap;

=head2 new

 $mkconn = new UNA::MkMoConn;

 UNA::MkMoConnのオブジェクトを返す。

=cut

sub new {
	my($class) = @_;
	my $conn = new UNA::Cmn::Conn;
	my $hingrp = new UNA::Cmn::HinGrp;
	my $gobitbl = new UNA::Cmn::GobiTbl;
	my $fealst = new UNA::Cmn::FeaLst;
	my $hinlst = new UNA::HinLst;
	my $smap = new UNA::SMap;
	my $self =
	{
	 conn => $conn,		# コモン辞書
	 hingrp => $hingrp,	# 品詞グループ
	 gobitbl => $gobitbl,	# 語尾テーブル
	 fealst => $fealst,	# 素性リスト
	 hinlst => $hinlst,	# 品詞リスト
	 smap => $smap,		# シンボルマップ
	 max_cost => -1,	# 出力したコストの最大値
	 min_cost => 99999,	# 出力したコストの最小値
	 checkfile => '',	# 接続チェックファイル
	};
	bless $self, $class;
	$self;
}

=head2 load

 $mkconn->load('conn', $conn_file);
 $mkconn->load('hingrp', $hingrp_file);
 $mkconn->load('gobitbl', $gobitbl_file);
 $mkconn->load('fealst', $fealst_file);
 $mkconn->load('hinlst', $hinlst_file);
 $mkconn->load('smap', $smap_file);

 データファイルを読みこむ。

=cut

sub load {
	my($self, $type, $file) = @_;
	if ($type !~ /^(conn|hingrp|gobitbl|fealst|hinlst|smap)$/) {
		die "unknown file type: $type\n";
	}
	$self->{$type}->load($file);
}

=head2 make_morphconn

 $mkconn->make_morphconn($morphconn_file);

 形態素品詞間接続表を作成する。

=cut

sub make_morphconn {
	my($self, $outfile) = @_;
	$self->{hingrp}->set_from_gobitbl($self->{gobitbl});
	$self->{fealst}->set_from_hingrp($self->{hingrp});
	$self->{conn}->set_from_hingrp($self->{hingrp});
	if ($outfile) {
		open(OUT, ">$outfile") || die "can't write $outfile: $!\n";
		select OUT;
	}
	my $hno1 = 0;
	for $hin1 (@{$self->{hinlst}{hins}}) { # 前接品詞
		$hno1++;
		my $all_hindo = 0;
		my(%flag, %hindo);
		my %added;
		# 後接に関する属性を得る
		my($atofeahin) = &hin2atofeahin($hin1);
		my($basehin1, @feas1) = &feahin_split($atofeahin);
		for $hin2 (@{$self->{hinlst}{hins}}) { # 後接品詞
			# 前接に関する属性を得る
			my($maefeahin) = &hin2maefeahin($hin2);
			my($basehin2, @feas2) = &feahin_split($maefeahin);
			# 基本の接続を得る
			my $flag = $self->{conn}{conn_flag}{$basehin1}{$basehin2};
			$flag ||= '';
			# 新しい接続を調べる
			my $prev_f;		# 前回に使われた素性
			for $f (@feas2) {	# まず前接素性の処理
				if ($self->{fealst}{conn_flag}{$f}{$basehin2}{$basehin1}) {
					if ($prev_f) {
						warn "ERROR: ambiguous features: $prev_f<->$f: $hin1 -> $hin2\n";
					}
					$prev_f = $f;
					$flag = $self->{fealst}{conn_flag}{$f}{$basehin2}{$basehin1};
				}
		    }
		    for $f (@feas1) {	# 次に後接素性処理
				if ($self->{fealst}{conn_flag}{$f}{$basehin1}{$basehin2}) {
					 if ($prev_f) {
						warn "ERROR: ambiguous features: $prev_f<->$f: $hin1 -> $hin2\n";
					}
					$prev_f = $f;
					$flag = $self->{fealst}{conn_flag}{$f}{$basehin1}{$basehin2};
				}
			}
			my $hindo;
			($flag, $hindo) = $self->{smap}->get_sym_and_hindo($flag);
			$flag{$hin2} = $flag;
			if ($self->{checkfile}) { # 接続チェックファイルを作る
				$FlagStr1{$hin1} .= $flag;
				$FlagStr2{$hin2} .= $flag;
			}
			# 品詞内単語頻度累計で正規化する。
			my $hin_hindo = $self->{hinlst}{hin_hindo}{$hin2};
			$hin_hindo = 1 if $hin_hindo == 0;
			$hindo *= $hin_hindo;
			$hindo{$hin2} = $hindo;
			if (!$added{$maefeahin}) {
				# その前接素性付き品詞で足してないときだけ足す。
				# 値は同じなので、先に素性付き品詞の分を処理しても問題無い。
				$all_hindo += $hindo;
				$added{$maefeahin}++;
			}
		}
		# 一行分出力
		my $hno2 = 0;
		for $hin2 (@{$self->{hinlst}{hins}}) { # 後接品詞
			$hno2++;
			my $flag = $flag{$hin2};
			my $hindo = $hindo{$hin2};
			my $cost = $hindo;
			$cost /= $all_hindo if $all_hindo > 0;
			$cost = ($cost == 0) ? -1 : -log($cost);
			$self->{max_cost} = $cost if $cost > $self->{max_cost};
			$self->{min_cost} = $cost if $cost < $self->{min_cost};
			$cost = sprintf("%f", $cost);
			$cost =~ s/^\-(0\.0+)$/$1/; # -0.000...を防ぐ
			my $line = "$hin1 $hin2 $flag $cost";
			if ($DEBUG) {
				$line .= " # cost(${hno1}x${hno2})=-log($hindo/$all_hindo)";
			}
			$line .= "\n";
			print $line;
		}
	}
	if ($outfile) {
		close OUT;
		select STDOUT;
	}
	if ($self->{checkfile}) { # 接続チェックファイルの作成
		my $file = $self->{checkfile};
		open(OUT, ">$file") || die "can't write $file: $!\n";
		for $hin (@{$self->{hinlst}{hins}}) {
			print OUT "1 $FlagStr1{$hin} $hin\n";
		}
		for $hin (@{$self->{hinlst}{hins}}) {
			print OUT "2 $FlagStr2{$hin} $hin\n";
		}
		close OUT;
	}
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::Cmn>, L<UNA::Cmn::HinGrp>, L<UNA::Cmn::GobiTbl>, L<UNA::Cmn::FeaLst>,
L<UNA::Cmn::Conn>

=cut

1;
