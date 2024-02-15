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
# Cmn.pm - Useful Common Functions Manipulates Common Data
#

package UNA::Cmn;

=head1 NAME

 UNA::Cmn - common形式のデータを扱う関数ライブラリ

=head1 SYNOPSIS

 use UNA::Cmn;

=head1 DESCRIPTION

 common形式のデータを扱う関数ライブラリである。

=head1 BUG

=head1 HISTORY

 Ver.1.02 2000/10/12 最初の動作版
 Ver.5.00 2002/04/30 UNA-CmnToolsとしてRCS管理に
 Ver.5.01 2002/07/31 mkmodicレビューにより見つかったバグ箇所をコメント
 Ver.5.02 2002/08/20 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.dist
                     に変更

=head1 FUNCTIONS

=cut

$DEBUG = 0;

use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw( hyouki_unite hyouki_element hyouki_split
    hyouki2gokangobi is_katsuyou feahin_split
    fuku2maeato hin2basefea hin2maefeahin hin2atofeahin hin2purehin hin2mainhin
    add_features is_fukufea is_maefea is_atofea
    features_unite features_element features_split
    info_unite info_element);

use utf8;

=head2 hyouki_unite

 表記を素にする

=cut

sub hyouki_unite {
	my($hyouki) = @_;
	$hyouki =~ s/｜//g if $hyouki ne '｜';
	$hyouki =~ s/＾//g if $hyouki ne '＾';

	$hyouki;
}

=head2 hyouki_element

 表記のn番目の語構成を返す

=cut

sub hyouki_element {
	my($hyouki, $n) = @_;
	my @a = ($hyouki eq '｜') ? ($hyouki) : split(/｜/, $hyouki);
	$a[$n];
}

=head2 hyouki_split

 表記を語構成毎に分割する。

=cut

sub hyouki_split {
	my($hyouki) = @_;
	if ($hyouki eq '|') {
		($hyouki);
	}
	else {
		split /｜/, $hyouki;
	}
}

=head2  hyouki2gokangobi

 表記を語幹と語尾に分ける

=cut

sub hyouki2gokangobi {
	my($hyouki) = @_;
	if ($hyouki ne '＾' && $hyouki =~ /^(.*)＾(.*)$/) {
		($1, $2);
	}
	else {
		($hyouki, '');
	}
}

=head2 is_katsuyou

 活用している表記か？

=cut

sub is_katsuyou {
	my($hyouki) = @_;
	$hyouki ne '＾' && $hyouki =~ /＾/;
}

=head2 feahin_split

 素性付き品詞を、基本品詞、素性、素性2、…に分割する。

=cut

sub feahin_split {
	my($feahin) = @_;
	if ($feahin) {
		split /\;/, $feahin;
	}
	else {
		();
	}
}

=head2 fuku2maeato

 品詞(old format)や複合語素性を前品詞と後品詞に分ける

=cut

sub fuku2maeato {
	my($hin) = @_;
	if ($hin =~ /^(.*)＊(.*)/) {
		($1, $2);
	}
	else {
		('', $hin);
	}
}

=head2 hin2basefea

 品詞を基本品詞と素性に分ける

=cut

sub hin2basefea {
	my($hin) = @_;
	if ($hin =~ /^(.+?)\;(.+)$/) {
		($1, $2);
	}
	else {
		($hin, '');
	}
}

=head2 hin2maefeahin

 品詞から前接素性付き品詞を得る。前接素性付き品詞は、
 その品詞の前接に関する属性を表す品詞である。 例えば、

    基本品詞;前接素性1;前接素性2;後接素性1;後接素性2
    基本品詞;前品詞＊後品詞;前接素性1;前接素性2;後接素性1;後接素性2

 このような品詞の場合、前接素性付き品詞は、それぞれ、

    基本品詞;前接素性1;前接素性2
    前品詞;前接素性1;前接素性2

 になる。

=cut

sub hin2maefeahin {
	my($hin) = @_;
	my $newhin;
	for (split /\;/, $hin) {
		if (/^(.+)＊/) { # 複合語素性
			$newhin = $1; # 基本品詞を上書き
		}
		elsif (!$newhin) { # 基本品詞が設定されてないときだけ
			$newhin = $_;
		}
		elsif (&is_maefea($_)) {
			$newhin .= ";$_";
		}
	}
	$newhin;
}

=head2 hin2atofeahin

 品詞から後接素性付き品詞を得る。後接素性付き品詞は、
 その品詞の後接に関する属性を表す品詞である。 例えば、

    基本品詞;前接素性1;前接素性2;後接素性1;後接素性2
    基本品詞;前品詞＊後品詞;前接素性1;前接素性2;後接素性1;後接素性2

 このような品詞の場合、後接素性付き品詞は、それぞれ、

    基本品詞;後接素性1;後接素性2
    後品詞;後接素性1;後接素性2

 になる。

=cut

sub hin2atofeahin {
	my($hin) = @_;
	my $newhin;
	for (split /\;/, $hin) {
		if (/＊(.+)/) { # 複合語素性
		    $newhin = $1; # 基本品詞を上書き
		}
		elsif (!$newhin) { # 基本品詞が設定されてないときだけ
		    $newhin = $_;
		}
		elsif (&is_atofea($_)) {
		    $newhin .= ";$_";
		}
	}
	$newhin;
}

=head2 hin2purehin

 純品詞名を返す。
 実際には","で区切られる部分の前を返す。

=cut

sub hin2purehin {
	my($hin) = @_;
	$hin =~ s/\;.+//;		# 素性以下を削除
	$hin =~ s/\,.+//;		# 細分類素性以下を削除
	$hin;
}

=head2 hin2mainhin

 品詞大分類を返す。
 実際には"."で区切られる部分の一番前を返す。

=cut

sub hin2mainhin {
	my($hin) = @_;
	# 副詞など下位ない品詞で正しく機能していない可能性がある。
	if ($hin =~ /^(.+?)\./) {
		$1;
	}
	else {
		$hin;
	}
}

=head2 add_features

 基本品詞に素性の配列を追加した品詞を返す。
 その品詞に追加していい素性かどうかのチェックは行なわない。

=cut

sub add_features {
	my($hin, @features) = @_;
	if (@features && $features[0]) {
		join(';', $hin, @features);
	}
	else {
		$hin;
	}
}

=head2 is_fukufea

 複合語素性なら真

=cut

sub is_fukufea {
	my($feature) = @_;
	$feature =~ /＊/;
}

=head2 is_maefea

 前接素性なら真

=cut

sub is_maefea {
	my($feature) = @_;
	!&is_fukufea($feature) && !&is_atofea($feature);
}

=head2 is_atofea

 後接素性なら真

=cut

sub is_atofea {
	my($feature) = @_;
	#$feature =~ /^(＋|－|～|！|＝)/;
	# \x{ff5e}：半角形/全角形の「～」
	# \x{301c}：一般句読点の「〜」
	# \x{ff0d}：半角形/全角形の「－」
	# \x{2212}：半角形/全角形の「−」
	$feature =~ /^(＋|\x{ff0d}|\x{ff5e}|！|＝)/;
}

=head2 features_unite

 素性を連結する

=cut

sub features_unite {
	my($features) = @_;
	$features =~ s/\;//g;
	$features;    # 値が空の場合を考慮していない
}

=head2 features_element

 n番目の素性を返す。

=cut

sub features_element {
	my($features, $n) = @_;
	my @a = split /\;/, $features;
	$a[$n];
}

=head2 features_split

 素性を分割する。

=cut

sub features_split {
	my($features) = @_;
	if ($features) {
		split /\;/, $features;
	}
	else {
		();
	}
}

=head2 info_unite

 情報から空白を取り除いて連結

=cut

sub info_unite {
	my($info) = @_;
	$info =~ s/\s//g;
	$info;
}

=head2 info_element

 空白で区切られたn番目の情報を返す

=cut

sub info_element {
	my($info, $n) = @_;
	my @a = split /\s+/, $info;
	$a[$n];
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::MkMoDic>, L<UNA::MkMoConn>, L<UNA::MkKuTbl>

=cut

1;
