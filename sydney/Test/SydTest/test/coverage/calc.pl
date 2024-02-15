#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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
# Sydney のモジュール別 coverage 結果集計
#

# 使い方
#
# SydTest.cfy を開く
# FileView にする
# 全部選択する (スクロールさせて末尾まで表示させておくとよい)
# コピー
# メモ帳を開いてペースト
# ファイルをセーブ result.txt
# perl calc.pl result.txt

# 備考
# モジュール内にFTSInvertedは存在するが、集計項目から外す。
# モジュール内にBtreeは存在するが、参考として集計部分と別にする。


@names_ = qw( Btree Btree2 FileCommon FullText Inverted Lob Record Vector Admin Analysis
              Buffer CheckPoint Client Common Communication Exception Execution Lock LogicalFile
              LogicalLog Message Opt Os PhysicalFile Plan Schema Server Statement Trans Version 
              LemonSrc
			);



foreach $name ( @names_ ){
	( $small = $name ) =~ tr/A-Z/a-z/;
	$names{ $small } = $name;
}


open( SRC, "< $ARGV[ 0 ]" ) or die;

#データの集計
while( <SRC> ){

	@data = split;
	
	next unless scalar( @data ) == 8;

	$name = $data[ 0 ];
	$miss = $data[ 5 ];
	$hit = $data[ 6 ];

	next if $name eq 'Run';
	next if $name eq 'Merge';

	next unless $name =~ /\\?([^\\]+)$/;
	( $name = $1 ) =~ tr/A-Z/a-z/;

	next unless $names{ $name };

	$result{ $names{ $name } }{ total } += $miss + $hit;
	$result{ $names{ $name } }{ hit } += $hit;

#	print "$name -- $miss, $hit\n";
#	push( @all, "$name -- $miss, $hit\n" );
}

#foreach ( sort @all ){
#	print $_;
#}

# 現時点では FTSInvertedはInvertedとStatementはLemonSrcと一緒にさせる。
&merge( 'Inverted', 'FTSInverted' );
&merge( 'Statement', 'LemonSrc' );



# 集計するモジュール
printf( "%15s -- %8s, %8s,  %s\n", "name", "total", "hit", "ratio %" );
print "-------------------------------------------------\n";
foreach $name ( sort keys %result ){

  if ($name ne Btree)
  {
    &print_result;
  }	
}

print "---\n";
printf( "%15s -- %8d, %8d,  %-.2f\n", "total",
	   $result{ all }{ total },
	   $result{ all }{ hit },
	   $result{ all }{ hit } / $result{ all }{ total } * 100 );


# 集計から外して、参考にするもの
print "\n\nReference\n";
print "-------------------------------------------------\n";
foreach $name ( sort keys %result ){

  if ($name eq Btree)
  {
    &print_result;
  }	
}


################
# sub routine  #
################

# モジュール名, total, hit, ratio をプリント
sub print_result
{
	$total = $result{ $name }{ total };
	$hit = $result{ $name }{ hit };
	$ratio = $hit / $total * 100;

	$result{ all }{ total } += $total;
	$result{ all }{ hit } += $hit;

#	print "$name -- $total, $hit,  $ratio\%\n";
	printf( "%15s -- %8d, %8d,  %-.2f\n", $name, $total, $hit, $ratio );
}


sub merge{
	my $to = shift;

	foreach $from ( @_ ){
		$result{ $to }{ total } += $result{ $from }{ total };
		$result{ $to }{ hit } += $result{ $from }{ hit };

		delete $result{ $from };
	}
}

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
