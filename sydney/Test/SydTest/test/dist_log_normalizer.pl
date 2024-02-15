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
# log_normalizer.pl の後に分散環境の正規化を行う
# select ... order by の問い合わせ結果を昇順にする
# 20120729

# TODO: 改行を含むデータへの対応
# TODO: order by が一つのSQL分に複数含まれる場合の対応

$flag_select_without_order_by = 0;
$flag_select = 0;
$flag_update = 0;
$flag_delete = 0;

$sequence_flag = 0;
$temp_str="";
@select_result_array = ();

while(<>) {
    if ( $sequence_flag == 1 ) {
	if ( /^SydTest/ || /\[ERR\]/ ) {
	    print $temp_str;
	    $temp_str="";
	    $sequence_flag = 0;
	}else{
	    $temp_str=$temp_str . $_;
	    next;
	}
    }

    # select 文が複数行に渡る場合への対応
    #   1. カラムに改行コードが含まれているなど、行頭にデータが直接くる場合
    #   2. SQLが長文で途中で改行される場合 (20130514 追加)
    #      (例)
    #      SydTest::Executor: [INFO] [[SQL Query]] select ...
    #      SydTest::Executor: [INFO 1] (上のSQLの続き...)
    #      
    #      TODO) "order by" というキーワードの途中で改行される場合に order by の文と認識できない。
    #            本来は前の文と連結してから評価する必要がある。
    if ( !/^SydTest/ && !/\[ERR\]/ || /^SydTest::Executor: \[INFO \d+\]/ ) {
	print;
	if( $flag_select == 1 ) {
	    $flag_select_without_order_by = 0;
            if ( /order by (((?!order by).)+?)$/ ) {
            #if ( /order by (.*?)$/ ) {
                $sql_fragment = $1;
                # 文字リテラルの中に括弧が含まれている可能があるので文字リテラルを除去
                $sql_fragment =~ s/'.*'//g;
                # order by 以降の SQL 文から左括弧と右括弧の個数を取得
                $num_left_rapen = split(/\(/, $sql_fragment) - 1;
                $num_right_rapen = split(/\)/, $sql_fragment) - 1;
                # 右括弧のほうが多い = order by はサブクエリに係る
                if ($num_left_rapen < $num_right_rapen) {
                    $flag_select_without_order_by = 1;
                }
            } else {
                $flag_select_without_order_by = 1;
            }	    
	}
	next;
    }

    $flag_select = 0;

    # order by がない select 文は結果をソートする (副問合せには含まれていてもよい)
    if( $flag_select_without_order_by == 1 ) {

        # select 結果の終端
        if ( /(.*\[INFO\].* (End Of Data\.|Success\.|Canceled\.)$)/ || /\[ERR\]/ ) {
            @select_result_array = sort @select_result_array;
            foreach $result (@select_result_array) {
                print $result;
            }
	    print;
	    @select_result_array = ();
	    $flag_select_without_order_by = 0;
	    next;

        # エラーメッセージ
#        } elsif (/\[ERR\]/) {
#            print;
#            @select_result_array = ();
#            $flag_select_without_order_by = 0;


        # [INFO] [[Label]]
	# [INFO] [[SQL Parameter]]
	# [INFO] [Main] PreparedCommand
        } elsif ( /\[\[.*\]\]/ || /\[.*?\] \[.*?\]/ ) {
		print;

	# select 結果のデータ
	} else {
            # TODO: 出力結果1つが一行で終わるとは限らない
	    if ( /(.*\[INFO\].* [\{<].*[\}>]$)/ ) {
		#s/(.*\[INFO\].* [\{<].*[\}>]\n$)//;
		push(@select_result_array, $_);
		next;
	    } elsif ( /(.*\[INFO\].* \{.*)$/ ) {
		$sequence_flag=1;
		$temp_str=$_;
		next;
	    }
	}
        

    # update は返ってきた結果(Data)を捨てる
    } elsif ( $flag_update == 1 ) {
        if ( /(.*\[INFO\].* Success\.$)/ ) {
            print;
            $flag_update = 0;
            next;
        }

        if ( /(.*\[INFO\].* End Of Data\.$)/ ) {
            $flag_update = 0;
	    next;
        }

	if ( /\[ERR\]/ ) {
	    print;
	    $flag_update = 0;
	    next;
	}

	if ( /\[\[.*\]\]/ ) {
            print;
        } else {
            s/(.*\[INFO\].* \{.*\}\n$)//;
	}

    # delete は返ってきた結果(Data)を捨てる
    } elsif ( $flag_delete == 1 ) {
        if ( /(.*\[INFO\].* Success\.$)/ ) {
            print;
            $flag_delete = 0;
            next;
        }

        if ( /(.*\[INFO\].* End Of Data\.$)/ ) {
            $flag_delete = 0;
        }

	if ( /\[ERR\]/ ) {
	    print;
	    $flag_update = 0;
	    next;
	}

	if ( /\[\[.*\]\]/ ) {
            print;
        } else {
            #s/(.*\[INFO\].* \{.*\}\n$)//;
	}
    } else {
        print;
		# SQL文にはサブクエリ内に order by が含まれるが、主クエリには order by がない場合はソートする
		# [注意] ここでは、括弧で囲まれていない order by を主クエリの order by として検出
		#if ( /([^\(].*order by.*[^\)])/ ) {

        # Select 文
        if ( /\[\[SQL Query\]\] select/i ) {
            $flag_select = 1;

	    # order by が含まれ、かつ、主クエリではなくサブクエリに対する order by の場合は結果をソートして統一する
	    # TODO: 最後の order by 後の文字列を抽出
            if ( /order by (((?!order by).)+?)$/ ) {
                $sql_fragment = $1;
                # 文字リテラルの中に括弧が含まれている可能があるので文字リテラルを除去
                $sql_fragment =~ s/'.*'//g;
                # order by 以降の SQL 文から左括弧と右括弧の個数を取得
                $num_left_rapen = split(/\(/, $sql_fragment) - 1;
                $num_right_rapen = split(/\)/, $sql_fragment) - 1;
                # 右括弧のほうが多い = order by はサブクエリに係る
                if ($num_left_rapen < $num_right_rapen) {
                    $flag_select_without_order_by = 1;
                }
            } else {
                $flag_select_without_order_by = 1;
            }
	} elsif ( /\[\[SQL Query\]\] update/i ) {
            $flag_update = 1;
        } elsif ( /\[\[SQL Query\]\] delete/i ) {
            $flag_delete = 1;
        }
    }
}

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
