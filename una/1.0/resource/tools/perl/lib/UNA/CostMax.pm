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
# CostMax.pm - Uses Maximum Cost File
#

package UNA::CostMax;

=head1 NAME

 UNA::CostMax - costmaxファイルを扱う

=head1 SYNOPSIS

 use UNA::CostMax;

=head1 DESCRIPTION

 costmaxファイルを扱うためのモジュールである。

=head1 BUG

=head1 HISTORY

 Ver.1.01 2000/10/24 最初の動作版
 Ver.5.00 2002/04/30 UNA-CmnToolsとしてRCS管理に
 Ver.5.01 2002/08/20 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.dist
                     に変更

=head1 FUNCTIONS

=cut

$DEBUG = 0;

use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(get_costmax create_costmax update_costmax);

use utf8;

=head2 get_costmax

 $value = get_costmax($costmax_file);

 現在のコストの最大値を得る

=cut

sub get_costmax {
    my($file) = @_;
    open(IN, $file) || die "can't read $file: $!\n";
    my $value = <IN>;
    chomp $value;
    close IN;
    $value + 0;
}

=head2 create_costmax

 &create_costmax($costmax_file, $value)

 現在のコストの最大値を$valueにする。

=cut

sub create_costmax {
    my($file, $value) = @_;
    open(OUT, ">$file") || die "can't write $file: $!\n";
    printf OUT "%f\n", $value;
    close OUT;
}

=head2 update_costmax

 &update_costmax($costmax_file, $value)

 $valueが現在のコストの最大値より大きければ、$valueにする。

=cut

sub update_costmax {
    my($file, $value) = @_;
    my $prev = &get_costmax($file);
    if ($value > $prev) {
	&create_costmax($file, $value);
    }
}

=head1 COPYRIGHT

 Copyright 2000,2002, 2023 RICOH Co, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::MkMoDic>, L<UNA::MkMoConn>, L<UNA::MkKuTbl>

=cut

1;
