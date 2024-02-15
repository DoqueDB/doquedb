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
# KuRule.pm - use Common Kakari-uke Rules
#

package UNA::Cmn::KuRule;

=head1 NAME

 UNA::Cmn::KuRule - common形式のかかりうけルールソースを扱う

=head1 SYNOPSIS

 use UNA::Cmn::KuRule;
 $kurule = new UNA::Cmn::KuRule($kurule_file);
 $kurule->load($kurule_file);
 @defs = @{$kurule->{defs}};
 ($hin1, $hin2, $hin3, $hin4, $flag) = @{$defs[i]};

=head1 DESCRIPTION

 common形式のかかりうけルールソースを扱うモジュールである。
 loadメソッドにより、かかりうけルールソースファイルを読みこむことができる。    
 読みこんだルールは、$kurule->{defs}に以下のように、ほぼそのままの形で
 登録されている。

 @defs = @{$kurule->{defs}};
 ($hin1, $hin2, $hin3, $hin4, $flag) = @{$defs[i]};

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

 $kurule = new UNA::Cmn::KuRule($kurule_file);

 UNA::Cmn::KuRuleのオブジェクトを返す。
 $kurule_file が指定されていた場合、かかりうけルールソースを
 loadメソッドを使って読みこむ。

=cut

sub new {
	my($class, $file) = @_;
	my $self =
	{
	 file => $file,		# 接続素性定義のファイル名
	 defs => [],		# 接続定義をそのまま格納
	};
	bless $self, $class;
	$self->load($file) if $file;
	$self;
}

=head2 load

 $kurule->load($kurule_file);

 $kurule_fileで指定されたかかりうけルールソースを読みこむ。

=cut

sub load {
	my($self, $file) = @_;
	open(CONN, $file) || die "can't read $file: $!\n";
	while (<CONN>) {
		chomp;
		# 行のパース
		s/^\s+//;
		s/\s+$//;
		s/(^|\s+)\#.*$//;	# コメントを除去
		next if !$_;		# 空行は無視
		if (!/^(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+([A-Za-z])$/) {
			die "ERROR: bad format: $file($.): $_\n";
		}
		# 登録
		my($hin1, $hin2, $hin3, $hin4, $flag) = ($1, $2, $3, $4, $5);
		push(@{$self->{defs}}, [$hin1, $hin2, $hin3, $hin4, $flag]);
	}
	close CONN;
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

 UNA::Cmn::Dic(3), UNA::Cmn::HinGrp(3), UNA::Cmn::GobiTbl(3),
 UNA::Cmn::FeaLst(3)

=cut

1;
