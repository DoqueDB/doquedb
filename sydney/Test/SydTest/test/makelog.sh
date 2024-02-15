#!/bin/sh
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
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

result=result

lang=
btr=
utf8=
dist=
rec=0
inspect_raw_output=0
# 引数より実行するディレクトリと内容を得る
while getopts '12jezurdi' opt ; do
    case $opt in
    j) lang=_ja ;;
    e) lang=_eu ;;
    z) lang=_zh ;;
    1) btr=_b1 ;;
    2) btr=_b2 ;;
    u) utf8=_utf8 ;;
    r) rec=1 ;;
    d) dist=_dist ;;
    i) inspect_raw_output=1 ;;      # 外部コマンドの出力をそのまま結果としてログに残す (multi のときのみ有効)
    *) exit 1 ;;
    esac
done
shift `expr $OPTIND - 1`
single=single$dist$lang$btr$utf8
multi=multi$dist$btr
recovery=recovery$dist$btr

filter1()
{
    if [ ! -d "$1/$2/expect" ] ; then
        mkdir $1/$2/expect
    fi

    if [ "$4" = "multi" ] ; then
        if [ $3 -eq 9050 -o $3 -eq 9512 -o $3 -eq 9514 -o $3 -eq 225024 ] ; then
            x=50
        else
            x=9
        fi

        if [ "$dist" = "_dist" ] ; then
            for s in `seq 0 $x` 100 ; do
                grep "<<""$s>>" $1/$2/$result/$3.out
            done \
            | perl preedit_resultout_for_windows.pl \
            | perl log_normalizer.pl | perl dist_log_normalizer.pl > $1/$2/expect/$3.log
        else
            for s in `seq 0 $x` 100 ; do
                grep "<<""$s>>" $1/$2/$result/$3.out
            done \
            | perl preedit_resultout_for_windows.pl \
            | perl log_normalizer.pl > $1/$2/expect/$3.log
        fi

        if [ -e $1/$2/recovery_$3.out ] ; then
            perl preedit_resultout_for_windows.pl  $1/$2/$result/recovery_$3.out \
            | perl log_normalizer.pl > $1/$2/expect/$3.log
        fi

        if [ $rec -eq 1 ] ; then
            cat recovery.log >> $1/$2/expect/$3.log
        fi

        if [ $inspect_raw_output -eq 1 ]; then
            # 外部コマンドにより出力された行をそのまま結果として残す
            # このオプションをつけて正解ログを生成したテストは SydTest.sh のオプションにも -i をつけて実行すること

            grep -v "([0-9]\+) SydTest::" $1/$2/$result/$3.out \
             | perl preedit_resultout_for_windows.pl \
             | perl log_normalizer.pl >> $1/$2/expect/$3.log
        fi
    else
        if [ "$dist" = "_dist" ]; then
            perl preedit_resultout_for_windows.pl  $1/$2/$result/$3.out \
            | perl log_normalizer.pl | perl dist_log_normalizer.pl > $1/$2/expect/$3.log
        else
            perl preedit_resultout_for_windows.pl  $1/$2/$result/$3.out \
            | perl log_normalizer.pl > $1/$2/expect/$3.log
        fi
    fi
}

filter0()
{
    # $1はスクリプト番号
    case $1 in
    *[0-4]???)
        sorm=$single
        thread=single;;       # 下四桁が0000-4999なら
    *[5-9]???)
        sorm=$multi
        thread=multi;;
    *)
        echo "$1 is unexpected."
        exit 1;;
    esac

    if [ $rec -eq 1 ] ; then
        sorm=$recovery
    fi

    case $1 in
    *[02468])
        nore=normal;;        # 偶数なら
    *[13579])
        nore=except;;
    *)
        echo "$1 is unexpected."
        exit 1;;
    esac

    filter1 $sorm $nore $1 $thread
}

for i in $@; do filter0 $i; done

#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
