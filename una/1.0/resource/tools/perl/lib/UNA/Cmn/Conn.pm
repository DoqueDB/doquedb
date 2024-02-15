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
# Conn.pm - use Common Connection Source
#

package UNA::Cmn::Conn;

=head1 NAME

 UNA::Cmn::Conn - common形式の接続表を扱う

=head1 SYNOPSIS

 use UNA::Cmn::Conn;
 $conn = new UNA::Cmn::Conn($conn_file);
 $conn->load($conn_file);
 $conn->set_from_hingrp($hingrp);
 $conn_flag = $conn->{conn_flag}{$hin1}{$hin2};
 $conn->{logf} = "conn.log";

=head1 DESCRIPTION

 common形式の接続表ソースを扱うモジュールである。
 接続表を読み込んだ後、以下の方法によって、２つの品詞($hin1と$hin2)
 間の接続フラグを求めることができる。

 $conn_flag = $conn->{conn_flag}{$hin1}{$hin2};

 接続表はnewのときに読み込んでも、loadメソッドを用いて読み込んでも
 どちらでもよい。また複数のファイルに別れていてもよい。

 品詞グループ定義があるならばそのオブジェクトを使って、
 個々の品詞を展開することができる。(set_from_hingrpメソッド参照)

 $conn->{logf} = "conn.log";

 これにより、接続表の展開のログファイル(conn.log)ができる。
 ログファイルは各行に次の情報が書かれたファイルである。

    展開後前品詞-展開後後品詞 タイムスタンプ 展開前前品詞-展開前後品詞 フラグ

 タイムスタンプは8桁の数字で、定義されるたびに1づつ増えていく。
 このログファイルをソートすると、展開後の品詞間の接続がどう定義されているか、
 わかる。

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

=head2 new

 $conn = new UNA::Cmn::Conn($conn_file);

 UNA::Cmn::Connのオブジェクトを作成する。
 $conn_fileで、common形式の接続表ソースをloadメソッドを使って
 読み込む。
 $conn_fileは省略してもよく、その場合は読み込まない。    

=cut

sub new {
	my($class, $file) = @_;
	my $self =
	{
	 file => $file,		# 接続素性定義のファイル名
	 defs => [],		# 接続定義をそのまま格納
	 conn_flag => {},	# 接続フラグを格納
	 logf => '',		# デバッグ用ログファイル
	 time_stamp => 0,	# ログファイルに書くタイムスタンプ
	};
	bless $self, $class;
	$self->load($file) if $file;
	$self;
}

=head2 load

 $conn->load($conn_file);

 $conn_fileで指定した接続表ソースを読み込む。

=cut

sub load {
	my($self, $file) = @_;
	open(CONN, $file) || die "can't read $file: $!\n";
	while (<CONN>) {
		chomp;
		s/^\s+//;
		s/\s+$//;
		s/(^|\s+)\#.*$//;	# コメントを除去
		next if !$_;		# 空行は無視
		# 行のパース
		if (!/^(\S+)\s+(\S+)\s+([A-Za-z][\+\-]*)$/) {
			die "ERROR: bad format: $file($.): $_\n";
		}
		# 前接品詞と後接品詞と接続フラグを登録
		my($hin1, $hin2, $flag) = ($1, $2, $3);
		$self->{conn_flag}{$hin1}{$hin2} = $flag;
		push(@{$self->{defs}}, [$hin1, $hin2, $flag]);
	}
	close CONN;
}

=head2 set_from_hingrp

 $conn->set_from_hingrp($hingrp);

 品詞グループ定義を使用する。
 $hingrpは、UNA::Cmn::HinGrpのオブジェクトである。
 これにより、接続表の個々の品詞が展開された形の品詞間の
 接続フラグを得ることができる。

=cut

use UNA::Cmn::HinGrp;

sub set_from_hingrp {
	my($self, $hingrp) = @_;
	return if !$hingrp;
	$self->{conn_flag} = {};	# 一旦削除
	if ($self->{logf}) {
		$self->{time_stamp} = 0;
		open(LOG, ">$self->{logf}") || die "can't write $self->{logf}: $!\n";
	}
	for $def (@{$self->{defs}}) {
		my($hin1, $hin2, $flag) = @$def;
		# 前接品詞を全て展開する
		my @hin1 = ($hin1);
		if ($hingrp->{is_group}{$hin1}) {
			push(@hin1, keys %{$hingrp->{is_group}{$hin1}});
		}
		# 後接品詞を全て展開する
		my @hin2 = ($hin2);
		if ($hingrp->{is_group}{$hin2}) {
			push(@hin2, keys %{$hingrp->{is_group}{$hin2}});
		}
		# すべての組み合わせをもとめる
		for $hinx1 (@hin1) {
			for $hinx2 (@hin2) {
				if ($self->{logf}) {
					$self->{time_stamp}++;
					printf LOG "$hinx1-$hinx2 %08d $hin1-$hin2 $flag\n",
					$self->{time_stamp};
				}
				$self->{conn_flag}{$hinx1}{$hinx2} = $flag; # 後定義優先
			}
		}
	}
	if ($self->{logf}) {
		close LOG;
	}
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

 UNA::Cmn::Dic(3), UNA::Cmn::HinGrp(3), UNA::Cmn::GobiTbl(3),
 UNA::Cmn::FeaLst(3)

=cut

1;
