#!/usr/bin/perl
# 
# Copyright (c) 2018, 2023 Ricoh Company, Ltd.
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
# UnaDicSrc.pm - use una dictionary source
#

package UNA::UnaDicSrc;

=head1 NAME

 UNA::UnaDicSrc - UNA用辞書ソースを扱う

=head1 SYNOPSIS

 use UNA::UnaDicSrc;
 $unadicsrc = new UNA::UnaDicSrc($unadicsrc_file);
 $unadicsrc->load($unadicsrc_file);

=head1 DESCRIPTION

 UNA用辞書ソースを扱うモジュールである。
 loadメソッドで、UNA用辞書ソースファイルを読みこんだ後は、
 以下のようにして、UNA用辞書ソースの情報と頻度を得ることができる。

 $maedicinfo = $unadicsrc->{maedicinfo}{$wid};
 $hindo = $unadicsrc->{hindo}{$wid};
 $atodicinfo = $unadicsrc->{atodicinfo}{$wid};

=head1 HISTORY

 Ver.1.00 2018/01/19 初版

=head1 METHODS

=cut

$DEBUG = 0;

use utf8;
use open IN => ":utf8";

=head2 new

 $unadicsrc = new UNA::UnaDicSrc($unadicsrc_file);

 UnaDicSrcのオブジェクトを作成する。
 $unadicsrc_fileを指定した場合、UNA用辞書ソースファイルを
 loadメソッドを使って、読みこむ。

=cut

sub new {
	my($class, $file) = @_;
	my $self =
	{
	 file => $file,				# 品詞グループ定義のファイル名
     unadicsrcinfo => [],		# UNA用辞書ソースの情報
	};
	bless $self, $class;
	$self->load($file) if $file;
	$self;
}

=head2 load

 $unadicsrcinfo->load($unadicsrc_file);

 $unadicsrc_fileで指定されたUNA辞書ソースファイルを読みこむ。

=cut

sub load {
	my($self, $file) = @_;
	open(UNADICSRC, $file) || die "can't read $file: $!\n";
	while (<UNADICSRC>) {
		chomp;
		my $line = $_;
		next if !$_;		# 空行は無視

		# 行のパース
		if (!/^((\d{9}\:*)+\s{1}\S+\s{1})(\S+)\s{1}(\d+\.*\d*)(\s{1}\S+\(.*\))$/) {
		    die "ERROR: bad format: $file($.): $line\n";
		}
		my($maedicinfo, $hin, $hindo, $atodicinfo) = ($1, $3, $4, $5);
		# 配列に登録
		push(@{$self->{unadicsrcinfo}}, [$maedicinfo, $hin, $hindo, $atodicinfo]);
	}
    close UNADICSRC;
}

=head1 COPYRIGHT

 Copyright 2018, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

 updatedic, mksrcdata

=cut

1;
