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
# Dic.pm - use Common Word Dictonary
#

package UNA::Cmn::Dic;

=head1 NAME

 UNA::Cmn::Dic - common形式の単語辞書を扱う

=head1 SYNOPSIS

 use UNA::Cmn::Dic;
 $dic = new UNA::Cmn::Dic($dic_file, $mpat);
 $dic->load($dic_file, $mpat);
 @hins = @{$dic->{hins}};
 @wids = @{$dic->{wids}};
 $hin = $dic->{hin}{$wid};
 $hyouki0 = $dic->{hyouki0}{$wid};
 $n = $dic->{ihyoukin}{$wid};
 $info = $dic->get_info($mark, $wid, $ino);

=head1 DESCRIPTION

 common形式の辞書を扱うモジュールである。
 読み込まれる辞書は予めID順にソートされていなければならない。
 辞書の全ての情報を読み込むのではなく、$mpatで指定したマークの行だけ
 を読み込む。(loadメソッド参照。)
 読み込んだ後は、以下の方法によってそれぞれの情報を得ることができる。

 @hins = @{$dic->{hins}};

  すべての品詞の配列。ここで品詞とは、*行に書かれた品詞であり、
  素性が付く前のものである。

 @wids = @{$dic->{wids}};

  すべての単語IDの配列。

 $hin = $dic->{hin}{$wid};

  $widの単語IDを持つ単語の品詞。ここで品詞とは、*行に書かれた品詞であり、
  素性が付く前のものである。

 $hyouki0 = $dic->{hyouki0}{$wid};

  $widの単語IDを持つ単語の代表表記。「｜」や「＾」を含む。

 $n = $dic->{ihyoukin}{$wid};

  $widの単語IDを持つ単語の異表記の数。異表記番号は"01"から始まる
  番号であり、sprintf("%02d", $n)をすると、通常、最後の異表記の番号になる。
  ただし、欠番があることもあり、その場合、異表記番号の最大値を表す。

=head1 HISTORY

 Ver.5.00 2002/04/30 UNA-CmnToolsとしてRCS管理に
 Ver.5.01 2002/07/31 mkmodicレビューにより見つかったバグを修正
 Ver.5.02 2002/08/20 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.dist
                     に変更

=head1 METHODS

=cut

$DEBUG = 0;

use utf8;
use open IN => ":utf8";

$MAX_HYOUKI_LEN = 255;		# 表記の最大文字数
$DEFAULT_MPAT = 'h0f0g0m0s0b0t0c0y0z0a0e0u0'; # デフォルトの読み込みマーク行

=head2 new

 $dic = new UNA::Cmn::Dic($dic_file, $mpat);

 UNA::Cmn::Dicのオブジェクトを作成する。
 $dic_fileは読み込むcommon辞書のファイル名である。
 $mpatでどのマークの行を読み込むかを指定する。
 loadメソッドにより、この辞書ファイルが読み込まれる。
 (詳しくは、loadメソッドを参照)

=cut

sub new {
	my($class, $file, $mpat) = @_;
	$mpat ||= $DEFAULT_MPAT;
	my $self =
	{
	 file => $file,		# 辞書のファイル名
	 mpat => $mpat,		# マーク番号パターン
	 info => {},		# 辞書そのもの(大量のメモリを使用する)
	 wids => [],		# 全ての単語ID
	 hins => [],		# 全ての品詞
	 hin => {},		# 単語毎の品詞
	 hyouki0 => {},		# 単語毎の代表表記
	};
	bless $self, $class;
	$self->load($file, $mpat) if $file;
	$self;
}

=head2 load

 $dic->load($dic_file, $mpat);

 $dic_fileで指定された辞書を読み込む。
 $mpatには読み込みたいマークの行を指定する。
 辞書の行のマークが $mpat に含まれていれば、その行は読み込まれる。
 $mpatが指定されていなければデフォルトとして、
   "f0g0m0s0h0b0t0c0y0z0a0e0u0"
 が使われる。なお例外的に、*0行は(*の指定がなければ)常に読み込まれる。
 *の指定があれば、その指定に従う。(例えば、'*1'などを指定できる。)

=cut

sub load {
	my($self, $file, $mpat) = @_;
	$mpat ||= $self->{mpat};
	my $prev_wid = -1;		# ひとつ前の単語ID
	my %all_wids = map {$_ => 1} @{$self->{wids}}; # 全ての単語IDを記録
	open(DIC, $file) || die "can't read $file: $!\n";
	while (<DIC>) {
		chomp;
		s/\s+$//;		# 行末の空白を除去
		s/(^|\s+)\#.*$//;	# コメントを除去
		next if !$_;		# 空行は無視
		# 行のパース
		if (!/^([\*fgmshbtcyzaeu])([0-9]) ([0-9]{8})([0-9]{2})(.*)/) {
			# 全く形式に合わない
			die "ERROR: not common dic format: $file($.): $_\n";
		}
		# マーク、マーク番号、単語ID、異表記番号、実情報を設定
		my($mark, $mno, $wid, $ino, $info) = ($1, $2, $3, $4, $5);
		#$info=~s/\\x\{ff5e\}/〜/g;	# expand-*.dicの品詞素性の記号を変換
		$info=~s/\\x\{(\S{4})\}/\\u$1/g;

		# 実情報の前に空白が無いかチェック
		if ($info) {
			if (!($info =~ s/^\s+//)) {
				die "ERROR: bad info field (no space): $file($.): $_\n";
			}
		}
		# 単語ID順かチェック
		if ($wid < $prev_wid) {
			die "ERROR: word ids are not sorted: $file($.): $_\n";
		}
		# 指定されたマーク番号ではなければ読み飛ばす。
		my $no = '';
		if (index($mpat, "$mark$mno") >= 0) {
			$no = $mno;
		}
		elsif ($mark eq '*') {	# *は必ず読み込む
			$no = '0';
		}
		if ($no eq '' || $mno ne $no) {
			next;		# 読み飛ばす
		}
		# マーク毎の異表記番号などのチェック
		if ($mark eq '*') {
			if ($ino ne '00') {	# 代表表記は00のみ
				die "ERROR: bad ihyouki number: $file($.): $_\n";
			}
			my @a = split(/\s+/, $info);
			if (@a != 2) {
				die "ERROR: bad daihyou entry information: $file($.): $_\n";
			}
			if (length($a[0]) > $MAX_HYOUKI_LEN) {
				die "ERROR: too long ihyouki: $file($.): $_\n";
			}
			# 異表記番号の設定
		    $self->{ihyoukin}{$wid} ||= 0; # とりあえず0を記録
		}
		elsif ($mark eq 'h') {
			if ($ino eq '00') {	# 異表記は01から
				die "ERROR: bad ihyouki number: $file($.): $_\n";
			}
			if (!$info) {
				die "ERROR: empty ihyouki: $file($.): $_\n";
			}
			if ($info =~ /\s/) {
				die "ERROR: bad ihyouki: $file($.): $_\n";
			}
			if (length($info) > $MAX_HYOUKI_LEN) {
				die "ERROR: too long ihyouki: $file($.): $_\n";
			}
			# 異表記の最大値をカウント
			my $i = $ino + 0;	# 数値化
			$self->{ihyoukin}{$wid} = $i if $self->{ihyoukin}{$wid} < $i;
		}
		elsif ($mark eq 'm') {
			if (!$info) {
				die "ERROR: empty features: $file($.): $_\n";
			}
		}
		elsif ($mark eq 't') {
			if (!$info) {
				die "ERROR: empty hindo: $file($.): $_\n";
			}
			elsif (!($info =~ /^[0-9\s]+$/ || $info =~ /^\-+$/)) {
				die "ERROR: bad hindo: $file($.): $_\n";
			}
		}
		# 二重定義のチェック
		if (defined $self->{info}{"$mark $wid $ino"}) {
			die "ERROR: redifinition: $file($.): $_\n";
		}
		# 登録
		$all_wids{$wid} = 1;
		$self->{info}{"$mark $wid $ino"} = $info;
		# ひとつ前の単語IDの記憶
		$prev_wid = $wid;
	}
	close DIC;
	# 読み込んだ辞書を解析する。
	my %mark;
	@{$self->{hins}} = ();	# 一旦クリア
	my $ino_max = '';		# 異表記番号の最大
	# 全ての単語IDをソートして記憶
	@{$self->{wids}} = sort keys %all_wids;
	for $wid (@{$self->{wids}}) {
		# 代表エントリがあるかどうかのチェック
		if (!$self->{info}{"\* $wid 00"}) {
			die "ERROR: daihyou entry not found: $self->{file}: wid=$wid\n";
		}
		# 品詞と表記を登録
		my($hyouki, $hin) = split(/\s+/, $self->{info}{"\* $wid 00"});
		if (!$mark{$hin}) {
			push(@{$self->{hins}}, $hin); # 出てきた順に登録
			$mark{$hin}++;
		}
		$self->{hin}{$wid} = $hin;
		$self->{hyouki0}{$wid} = $hyouki;
    }
}

=head2 get_info

 $info = $dic->get_info($mark, $wid, $ino);

  $widで指定された単語の、マークが$markで異表記番号が$inoである行の
  実情報に書かれた情報。読み込まれてなかったり、定義されてなければ
  undef になる。

=cut

sub get_info {
	my($self, $mark, $wid, $ino) = @_;
	$self->{info}{"$mark $wid $ino"};
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

 UNA::Cmn::HinGrp(3), UNA::Cmn::GobiTbl(3), UNA::Cmn::FeaLst(3),
 UNA::Cmn::Conn(3)

=cut

1;
