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
# MkKuTbl.pm - Make Kakari-uke tables
#

package UNA::MkKuTbl;

=head1 NAME

 UNA::MkKuTbl - かかりうけ属性テーブルとかかりうけ表ソースを作る

=head1 SYNOPSIS

 use UNA::MkKuTbl
 $mkku = new UNA::MkKuTbl;
 $mkku->{input_code} = $icode;	# euc,utf8,sjis,ucs2l,ucs2b
 $mkku->load('kurule', $kurule_file);
 $mkku->load('hinlst', $hinlst_file);
 $mkku->load('hingrp', $hingrp_file);
 $mkku->load('gobitbl', $gobitbl_file);
 $mkku->{output_code} = $ocode;	# euc,utf8,sjis,ucs2l,ucs2b
 $mkku->make_ku_tables($kuattr_file, $kutbl_file);

=head1 DESCRIPTION

 かかりうけ属性テーブルとかかりうけ表ソースを作る。

=head1 HISTORY

 Ver.5.00 2002/04/30 UNA-CmnToolsとしてRCS管理に
 Ver.5.01 2002/08/20 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.dist
                     に変更

=head1 METHODS

=cut

$DEBUG = 0;

use lib "../tools/perl/lib";
use utf8;
use open IN  => ":utf8";
use open OUT => ":utf8";
binmode(STDOUT, ":utf8");
binmode(STDERR, ":utf8");

use UNA::Cmn;
use UNA::Cmn::KuRule;
use UNA::HinLst;
use UNA::Cmn::HinGrp;
use UNA::Cmn::GobiTbl;

# フラグから頻度への変換表
%Flag2hindo =
    (
     't' => 400,		# 近い連体修飾
     'T' => 80,			# 遠い連体修飾
     'y' => 340,		# 近い連用修飾
     'Y' => 68,			# 遠い連用修飾
     'h' => 120,		# 近い並列関係
     'H' => 24,			# 遠い並列関係
     'c' => 120,		# 近い接続関係
     'C' => 24,			# 遠い接続関係
     'f' => 72,			# 複合語関係
     'p' => 68,			# 括弧の関係
     'k' => 47,			# 孤立
     'x' => 0,			# 無関係(default)
    );
$Default_flag = 'x';

=head2 new

 $mkku = new UNA::MkKuTbl;

 UNA::MkKuTblのオブジェクトを返す。

=cut

sub new {
    my($class) = @_;
    my $kurule = new UNA::Cmn::KuRule;
    my $hinlst = new UNA::HinLst;
    my $hingrp = new UNA::Cmn::HinGrp;
    my $gobitbl = new UNA::Cmn::GobiTbl;
    my $self =
	{
	 kurule => $kurule,	# かかりうけルール
	 hinlst => $hinlst,	# 品詞リスト
	 hingrp => $hingrp,	# 品詞グループ
	 gobitbl => $gobitbl,	# 語尾テーブル
	 basehins => [],	# 全ての基本品詞を記録
	 mae_basehin => {},	# 品詞→前基本品詞の変換表
	 ato_basehin => {},	# 品詞→後基本品詞の変換表
	 max_cost => -1,	# 出力したコストの最大値
	 min_cost => 99999,	# 出力したコストの最小値
	};
    bless $self, $class;
    $self;
}

=head2 load

 データファイルを読みこむ。

=cut

sub load {
    my($self, $type, $file) = @_;
    if ($type !~ /^(kurule|hinlst|hingrp|gobitbl)$/) {
	die "unknown file type: $type\n";
    }
    $self->{$type}->load($file);
}

=head2 make_ku_tables

 $mkdic->make_ku_tables($kuattr_file, $kutbl_file);

 かかりうけ属性テーブルとかかりうけ表ソースを作る。

=cut

sub make_ku_tables {
    my($self, $attr_file, $tbl_file) = @_;
    $self->{hingrp}->set_from_gobitbl($self->{gobitbl});
    $self->set_total_basehins;
    my(%kset, %ku_flag, @kwork, $last_kno);
    my(%uset, %uk_flag, @uwork, $last_uno);
    for $defs (@{$self->{kurule}{defs}}) {
	my($hin1, $hin2, $hin3, $hin4, $flag) = @$defs;
	# かかり側(hin1,hin2)について処理
	my $new_kno = ++$last_kno;
	$kset{$new_kno} = $self->extend_pairs($hin1, $hin2);
	for $kno (@kwork) {
	    next if !%{$kset{$kno}}; # 既に空
	    my $a = $kset{$kno};
	    my $b = $kset{$new_kno};
	    my $a_and_b = {};
	    grep($a->{$_} && ($a_and_b->{$_} = 1,
		delete $a->{$_}, delete $b->{$_}), keys %$b);
	    if (%$a_and_b) {
		my $pro_kno = ++$last_kno;
		$kset{$pro_kno} = $a_and_b;
		# リンクをコピーする
		my($uno, $flag);
		while (($uno, $flag) = each %{$ku_flag{$kno}}) {
		    $ku_flag{$pro_kno}{$uno}
		        = $uk_flag{$uno}{$pro_kno} = $flag;
		}
	    }
	    last if !%$b;
	}
	# うけ側(hin3,hin4)について処理
	my $new_uno = ++$last_uno;
	$uset{$new_uno} = $self->extend_pairs($hin3, $hin4);
	for $uno (@uwork) {
	    next if !%{$uset{$uno}}; # 既に空
	    my $a = $uset{$uno};
	    my $b = $uset{$new_uno};
	    my $a_and_b = {};
	    grep($a->{$_} && ($a_and_b->{$_} = 1,
		delete $a->{$_}, delete $b->{$_}), keys %$b);
	    if (%$a_and_b) {
		my $pro_uno = ++$last_uno;
		$uset{$pro_uno} = $a_and_b;
		# リンクをコピーする
		my($kno, $flag);
		while (($kno, $flag) = each %{$uk_flag{$uno}}) {
		    $uk_flag{$pro_uno}{$kno}
		        = $ku_flag{$kno}{$pro_uno} = $flag;
		}
	    }
	    last if !%$b;
	}
	# かかり側←→うけ側について処理
	for $kno ($new_kno .. $last_kno) {
	    for $uno ($new_uno .. $last_uno) {
		$ku_flag{$kno}{$uno} = $flag;
		$uk_flag{$uno}{$kno} = $flag;
	    }
	}
	# 作業領域に追加
	@kwork = grep(%{$kset{$_}}, (@kwork, $new_kno .. $last_kno));
	@uwork = grep(%{$uset{$_}}, (@uwork, $new_uno .. $last_uno));
    }
    # 番号の逆引テーブルとラベルへの変換表を作る
    my(%kno_tbl, %uno_tbl);
    my(%klabel, %ulabel);
    my $klabel = "K0001";
    my $ulabel = "U0001";
    for $kno (@kwork) {
	$klabel{$kno} = $klabel++;
	for $hin12 (keys %{$kset{$kno}}) {
	    $kno_tbl{$hin12} = $kno;
	}
    }
    for $uno (@uwork) {
	$ulabel{$uno} = $ulabel++;
	for $hin34 (keys %{$uset{$uno}}) {
	    $uno_tbl{$hin34} = $uno;
	}
    }
    # かかり属性テーブルに出力
    if ($attr_file) {
	open(OUT, ">$attr_file") || die "can't write $attr_file: $!\n";
	select OUT;
    }
    for $hin1 ('-', @{$self->{hinlst}{hins}}) {
	my $maehin1 = $self->{mae_basehin}{$hin1};
	my $atohin1 = $self->{ato_basehin}{$hin1};
	for $hin2 ('-', @{$self->{hinlst}{hins}}) {
	    my $maehin2 = $self->{mae_basehin}{$hin2};
	    my $atohin2 = $self->{ato_basehin}{$hin2};
	    my $kno = $kno_tbl{"$atohin1 $atohin2"};
	    my $uno = $uno_tbl{"$maehin1 $maehin2"};
	    my $klabel = $kno ? $klabel{$kno} : '----';
	    my $ulabel = $uno ? $ulabel{$uno} : '----';
	    my $line = "$hin1 $hin2 $klabel $ulabel\n";
	    print $line;
	}
    }
    if ($attr_file) {
	close OUT;
	select STDOUT;
    }
    # かかりうけ表ソースの出力
    if ($tbl_file) {
	open(OUT, ">$tbl_file") || die "can't write $tbl_file: $!\n";
	select OUT;
    }
    for $kno (@kwork) {
	# コストの計算
	my $all_hindo = 0;
	for $uno (@uwork) {
	    my $flag = $ku_flag{$kno}{$uno} || $uk_flag{$uno}{$kno};
	    $flag ||= $Default_flag;
	    my $hindo = $Flag2hindo{$flag} || 0;
	    $all_hindo += $hindo;
	}
	# 出力
	for $uno (@uwork) {
	    my $flag = $ku_flag{$kno}{$uno} || $uk_flag{$uno}{$kno};
	    $flag ||= $Default_flag;
	    my $hindo = $Flag2hindo{$flag} || 0;
	    my $cost = $hindo;
	    $cost /= $all_hindo if $all_hindo > 0;
	    $cost = ($cost == 0) ? -1 : -log($cost);
	    $self->{max_cost} = $cost if $cost > $self->{max_cost};
	    $self->{min_cost} = $cost if $cost < $self->{min_cost};
	    $cost = sprintf("%f", $cost);
	    $cost =~ s/^\-(0\.0+)$/$1/; # -0.000...を防ぐ
	    my $klabel = $klabel{$kno} || '----';
	    my $ulabel = $ulabel{$uno} || '----';
	    my $line = "$klabel $ulabel $flag $cost\n";
	    print $line;
	}
    }
    if ($tbl_file) {
	close OUT;
	select STDOUT;
    }
}

# 全ての基本品詞の配列をセットする
sub set_total_basehins {
    my($self) = @_;
    my %basehins;
    # 単語無し(-)は特別に処理する
    $self->{mae_basehin}{'-'} = '-';
    $self->{ato_basehin}{'-'} = '-';
    # 全ての品詞について基本品詞、前品詞、後品詞を設定する。
    for $hin (@{$self->{hinlst}{hins}}) {
	my $mae_basehin = (&hin2basefea(&hin2maefeahin($hin)))[0];
	my $ato_basehin = (&hin2basefea(&hin2atofeahin($hin)))[0];
	$self->{mae_basehin}{$hin} = $mae_basehin;
	$self->{ato_basehin}{$hin} = $ato_basehin;
	$basehins{$mae_basehin}++;
	$basehins{$ato_basehin}++;
    }
    @{$self->{basehins}} = keys %basehins;
}

# 品詞対の展開
sub extend_pairs {
    my($self, $hin1, $hin2) = @_;
    my %r;
    my(@hin1, @hin2);
    if ($hin1 eq '*') {
	@hin1 = @{$self->{basehins}};
	push(@hin1, '-');
    }
    else {
	my %mark;
	if ($self->{hingrp}->{is_group}{$hin1}) {
	    %mark = %{$self->{hingrp}->{is_group}{$hin1}}; # copy
	}
	elsif ($hin1 ne '-' && !grep($hin1 eq $_, @{$self->{basehins}})) {
	    die "ERROR: unknown base-hinshi or hinshi-group: $hin1\n";
	}
	$mark{$hin1} = 1;	# 定義自身も候補
	@hin1 = grep {$mark{$_}} @{$self->{basehins}}; # 基本品詞のみ
    }
    if ($hin2 eq '*') {
	@hin2 = @{$self->{basehins}};
	push(@hin2, '-');
    }
    else {
	my %mark;
	if ($self->{hingrp}->{is_group}{$hin2}) {
	    %mark = %{$self->{hingrp}->{is_group}{$hin2}}; # copy
	}
	elsif ($hin2 ne '-' && !grep($hin2 eq $_, @{$self->{basehins}})) {
	    die "ERROR: unknown base-hinshi or hinshi-group: $hin2\n";
	}
	$mark{$hin2} = 1;	# 定義自身も候補
	@hin2 = grep {$mark{$_}} @{$self->{basehins}}; # 基本品詞のみ
    }
    for $hinx1 (@hin1) {
	for $hinx2 (@hin2) {
	    $r{"$hinx1 $hinx2"} = 1; # スペースでつなげて使う
	}
    }
    \%r;
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::Cmn>, L<UNA::Cmn::KuRule>, L<UNA::HinLst>, L<UNA::Cmn::HinGrp>,
L<UNA::Cmn::GobiTbl>

=cut

1;
