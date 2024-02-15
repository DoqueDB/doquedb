#!/usr/bin/perl
# 
# Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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
# HanTbl.pm - use HanTbl Definition
#

package UNA::HanTbl;

=head1 NAME

 UNA::HanTbl - 半角エントリ登録テーブルを扱う

=head1 SYNOPSIS

 use UNA::HanTbl;
 $hantbl = new UNA::HanTbl($hantbl_file);
 $hantbl->load($hantbl_file,$tmp_dir);

=head1 DESCRIPTION

 半角エントリ登録テーブルファイル(han.tbl)を扱うモジュールである。
 半角エントリ登録テーブルを読み込む。
 読み込み時にフォーマットのチェックを行い、合わない場合はエラーを出力し
 停止する。問題がない場合は、全角文字コードをキーとしてハッシュに格納する。
 
 $add_hyouki = $hantbl->{code}[$zencode];

=head1 HISTORY

 Ver.5.00 2002/05/17 UNA-CmnToolsとしてRCS管理に
                     最初の動作版
 Ver.5.01 2002/07/02 jj準拠に対応するためhan.tblの文字コードをUNICODE1.1の
                     表記に変換するようにした。
 Ver.5.02 2002/07/25 レビューによる指摘事項を反映、コメントの追加
 Ver.5.03 2002/08/20 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.dist
                     に変更

=head1 METHODS

=cut

use utf8;
use open IN => ":utf8";

$tool_dir = "../tools/bin";

=head2 new

 $hantbl = new UNA::HanTbl($hantbl_file);

 UNA::HanTblのオブジェクトを返す。
 $hantbl_fileが指定された場合は、半角エントリ登録テーブルファイルを読みこむ。

=cut

sub new {
    my($class, $file) = @_;
    my $self =
	{
	 file => $file,		               # 半角エントリ登録テーブルのファイル名
	 code => {}			               # 全角文字コードに対応する半角文字コード
	};
    bless $self, $class;
    $self->load($file) if $file;
    $self;
}

=head2 load

 $hantbl->load($hantbl_file);

 $hantbl_fileで指定された半角記号登録テーブルを読み、記述されている文字コードを、
 対応する全角文字の16進4桁の文字コードおよび文字そのものをキーとしてハッシュに格納する

=cut

sub load {
    my($self, $hantbl, $tmp_dir) = @_;

    open(HANTBL, $hantbl) || die "can't read $hantbl: $!\n";
    while (<HANTBL>) {
	    chomp;
        s/^\s+//;		    # 行頭の空白を削除
        s/\s+$//;		    # 行の後ろの空白を削除
        s/(^|\s+)\#.*$//;	# コメントを除去
        next if !$_;		# 空行は無視
        if(!/^\\u([0-9a-fA-F]{4})\s\\u([0-9a-fA-F]{4})/){
            die "ERROR: bad format: $hantbl($.): $_\n";
        }
        my $zencode = $1;
        my $hancode = $2;
		$zencode =~ tr/A-Z/a-z/;
		$hancode =~ tr/A-Z/a-z/;
		# 16進4桁の文字コードと文字そのものをキーとして登録する
        $self->{code}{$zencode} = $hancode;
        $self->{code}{chr(eval "0x".$zencode)} = $hancode;
    }
    close HANTBL;
	return;

}

=head1 COPYRIGHT

 Copyright 2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::Cmn>, L<UNA::Cmn::Dic>, L<UNA::Cmn::HinGrp>, L<UNA::Cmn::GobiTbl>,
L<UNA::Cmn::FeaLst>, L<UNA::HinMap>, L<UNA::HanTbl>

=cut

1;
