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
# GobiTbl.pm - use Common Gobi Table
#

package UNA::Cmn::GobiTbl;

=head1 NAME

 UNA::Cmn::GobiTbl - common形式の語尾テーブルを扱う

=head1 SYNOPSIS

 use UNA::Cmn::GobiTbl;
 $gobitbl = new UNA::Cmn::GobiTbl($gobitbl_file);
 $gobitbl->load($gobitbl_file);
 @hin_and_gobi = $gobitbl->{brothers}{$hin}{$gobi};
 ($hin_i, $gobi_i) = @{$hin_and_gobi[$i]};
 $gobitbl->{is_group}{$hin1}{$hin2}...

=head1 DESCRIPTION

 common形式の語尾テーブルを扱うモジュールである。
 loadメソッドで読みこんだ後、以下の方法で、値を得ることができる。

 @hin_and_gobi = $gobitbl->{brothers}{$hin}{$gobi};

 品詞$hinで語尾が$gobiである語のすべての活用を返す。
 この配列の要素は次のような、品詞と語尾からなる。

 ($hin_i, $gobi_i) = @{$hin_and_gobi[$i]};

 2つの品詞が活用グループの関係にあるならば、以下の値は真になる。  

 $gobitbl->{is_group}{$hin1}{$hin2}...

 この場合、$hin1は活用グループ名であり、$hin2はそのグループ内の品詞である。

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

 $gobitbl = new UNA::Cmn::GobiTbl($gobitbl_file);

 UNA::Cmn::GobiTblのオブジェクトを返す。
 $gobitbl_fileが指定されていれば、語尾テーブルを読みこむ。

=cut

sub new {
	my($class, $file) = @_;
	my $self =
	{
	 file => $file,		# 語尾テーブルのファイル名
	 brothers => {},	# 同じ活用グループの語尾と品詞の配列
	 is_group => {},        # 活用グループを格納
	};
	bless $self, $class;
	$self->load($file) if $file;
	$self;
}

=head2 load

 $gobitbl->load($gobitbl_file);

 語尾テーブルを読みこむ。

=cut

sub load {
	my($self, $file) = @_;
	open(GOBITBL, $file) || die "can't read $file: $!\n";
	local($/) = undef;
	my $text = <GOBITBL>;	# 一度に読み込む
	close GOBITBL;
	for $group (split/\n[ \t\r]*\n/, $text) { # 活用グループ定義毎に分割
		$self->parse_group($group);
	}
}

# ひとつの活用グループ定義をパース
sub parse_group {
	my($self, $group) = @_;
	my @a = split(/[ \t]*\n/, $group);
	my @children;
	for (@a) {
		s/^\s+//;
		s/\s+$//;
		s/(^|\s+)\#.*$//;	# コメントを除去
		next if !$_;
		push(@children, $_);
	}
	return if !@children;
	my $name = shift @children;
	if ($name =~ /\s/) {	# 2つ以上のトークンがあった場合
		die "ERROR: gobi group name consists of two more words: $name\n";
	}
	if (!@children) {
		die "ERROR: $name has no definition: $self->{file}: $name\n";
	}
	my $cnt = 0;		# 何番目の語尾か
	my($hin0, $gobi0);		# 代表の品詞と語尾
	for (@children) {
		my @a = split(/\s+/, $_);
		my($hin, $gobi);
		if (scalar(@a) != 2) {
			die "ERROR: bad format: $self->{file}: $name: $_\n";
		}
		($hin, $gobi) = @a;
		$gobi = '' if $gobi eq '＊';
		# 循環定義かどうかのチェック
		if ($name eq $hin) {
			die "ERROR: katsuyou hinshi is used as katsuyou group: $name: $hin\n";
		}
		if ($cnt == 0) {	# 代表なら
			# 格納するための配列をセットする
			if (defined $self->{brothers}{$hin}{$gobi}) {
				die "ERROR: redefinition: $self->{file}: $name, $hin, $gobi\n";
			}
			($hin0, $gobi0) = ($hin, $gobi);
		}
		# 登録する
		push(@{$self->{brothers}{$hin0}{$gobi0}}, [$hin, $gobi]);
		$self->{is_group}{$name}{$hin}++;
		$cnt++;
	}
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

 UNA::Cmn::Dic(3), UNA::Cmn::HinGrp(3), UNA::Cmn::FeaLst(3),
 UNA::Cmn::Conn(3)

=cut

1;
