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

# 回帰テスト用実行スクリプト
# VC10追加版
# V16.4以降

############
# 変数の設定
############

# isWindows, mytop, hdrive, xdrive, testtype を読み込む
# [Linux] systemconffile, を読み込む
if [ `uname` = Linux ]; then
    . env.sh
fi

# テスト開始時
if [ $isWindows -eq 1 ] ; then
    if [ ! -e ./sydtest_default.reg ] ; then
        echo "defaultのregがありません。"
    fi
else
    regression_mode=1; export regression_mode         # Core出力確認
    if [ ! -e ./sydtest_default.conf ] ; then
        echo "defaultのconfがありません。"
    fi
fi

# 引数より実行するスクリプトを選択する。
debug=1
tera=1
reg=0
cont=4
regi=1
explain=""
timeout=""
while getopts 'idcspgtxT:' opt ; do
    case $opt in
    i) regi=0 ;;                  #regをデフォルト戻す
    d) reg=0
        cont=1 ;;                 #1周目
    c) reg=1
        cont=2 ;;                 #env_checkpoint_period が10
    p) reg=2
        cont=3 ;;                 #Plan_CollectionType に "FILE"
    s) reg=3 ;;                   #SydTest オプション /P
    g) debug=0 ;;
    t) tera=0 ;;
    x) explain="-o /x" ;;         # explain on
    T) timeout="-T $OPTARG" ;;
    *) exit 1 ;;
    esac
done

############
# 環境の確認
############

# install package を判定
# SydneyVersionを調べる ($sydtop/c.$package/ 以下の) 
# レジストリの初期化
# OpenMPを使い始めるデータ数 閾値 
if [ $regi -eq 1 ]; then
    if [ $isWindows -eq 1 ] ; then
        $REGEDITCMD /s env_FullText2_MinimumNumberForEachThread.reg
        $REGEDITCMD /e sydtest_start.reg "$regpath_sydney"
    else
        cat env_FullText2_MinimumNumberForEachThread.conf >> ${systemconffile}
        cp -r ${systemconffile} sydtest_start.conf
    fi
    echo "*** テスト reg ***"
else
    if [ $isWindows -eq 1 ] ; then
        $REGCMD DELETE "$regpath_sydney" /f
        $REGEDITCMD /s sydtest_default.reg
        $REGEDITCMD /s env_FullText2_MinimumNumberForEachThread.reg
        $REGEDITCMD /e sydtest_start.reg "$regpath_sydney"
    else
        cp -r sydtest_default.conf sydtest_start.conf
        cat env_FullText2_MinimumNumberForEachThread.conf >> sydtest_start.conf
    fi
    echo "*** default reg ***"
fi

###############
# ローカル関数
###############
set_registry()
{
case $reg in
    0)
        if [ $isWindows -eq 0 ] ; then
            echo "** cp start.conf **"
            cp -r sydtest_start.conf ${systemconffile}
        fi
        echo "回帰 START"  ;;
    1)
        if [ $isWindows -eq 1 ] ; then
            $REGCMD DELETE "$regpath_sydney" /f
            $REGEDITCMD /s sydtest_start.reg
            $REGEDITCMD /s env_checkpoint_period.reg
        else
            cp -r sydtest_start.conf ${systemconffile}
            cat env_checkpoint_period.conf >> ${systemconffile}
            echo "${systemconffile}"
        fi
        echo "** env_checkpoint_period START **"  ;;
    2)
        if [ $isWindows -eq 1 ] ; then
            $REGCMD DELETE "$regpath_sydney" /f
            $REGEDITCMD /s sydtest_start.reg
            $REGEDITCMD /s env_plan_collectiontype.reg
        else
            cp -r sydtest_start.conf ${systemconffile}
            cat env_plan_collectiontype.conf >> ${systemconffile}
            echo "${systemconffile}"
        fi
        echo "** Plan_CollectionType FILE START **"  ;;
    3)
        if [ $isWindows -eq 1 ] ; then
            $REGCMD DELETE "$regpath_sydney" /f
            $REGEDITCMD /s sydtest_start.reg
        else
            cp -r sydtest_start.conf ${systemconffile}
            echo "${systemconffile}"
        fi
        export reg
        echo "** SydTest /P **"  ;;
    *)
        echo "ご確認ください"
        exit 1;;
    esac
}

test_backup()
{
case $reg in
    1)
        result=result_d
        echo "1周目のテストを終了:  `date`" >> Report.txt
        echo "*** 1周目のテスト終了 ***"  ;;
    2)
        result=result_c
        echo "2周目のテストを終了:  `date`" >> Report.txt
        echo "*** env_checkpoint_period テスト終了 ***"  ;;
    3)
        result=result_p
        echo "3周目のテストを終了:  `date`" >> Report.txt
        echo "*** Plan_CollectionType FILE テスト終了 ***"  ;;
    4)
        result=result_s
        echo "4周目のテストを終了:  `date`" >> Report.txt
        echo "*** /P オプション  テスト終了 ***"  ;;
    *)
        echo "ご確認くださいtest_backup"
        exit 1;;
    esac

for i in single recovery single_utf8 ; do
    pushd $i
    if [ -e normal/result ] ; then
        mv normal/result normal/$result
    fi

    if [ -e except/result ] ; then
        mv except/result except/$result
    fi
    popd
done
}


# 通常のテスト項目
dotest_d()
{
    #1週目1122 2週目1124
    bash SydTest.sh $explain -f 0000 -t 1120 -ln single

    if [ $reg -eq 1 ]; then #env_checkpoint_period が10であるか
        bash SydTest.sh $explain -s 1124 -ln single
    else
        bash SydTest.sh $explain -s 1122 -ln single
    fi

    bash SydTest.sh $explain -f 1126 -t 2858 -ln single #2860を除く
    bash SydTest.sh $explain -f 2862 -t 4298 -ln single
    bash SydTest.sh $explain -f 4350 -t 4924 -ln single
    bash SydTest.sh $explain -f 4930 -t 4998 -ln single
    bash SydTest.sh $explain -f 10200 -t 12998 -ln single
    bash SydTest.sh $explain -f 14000 -t 14098 -ln single

    # Execution_LikeNormalizedString 0以外のテスト
    bash SydTest.sh $explain -f 14100 -t 14170 -lnL single

    bash SydTest.sh $explain -f 14180 -t 14998 -ln single

    bash SydTest.sh $explain -f 20000 -t 20040 -ln single
    bash SydTest.sh $explain -f 20100 -t 20498 -ln single

    bash SydTest.sh $explain -f 20640 -t 20998 -ln single
    bash SydTest.sh $explain -f 21000 -t 21100 -ln single

    bash SydTest.sh $explain -f 0001 -t 3699 -le single
    bash SydTest.sh $explain -f 4001 -t 4199 -le single
    bash SydTest.sh $explain -f 4301 -t 4999 -le single
    bash SydTest.sh $explain -f 14901 -t 14909 -le single 
    bash SydTest.sh $explain -f 20001 -t 20009 -le single
    bash SydTest.sh $explain -f 20451 -t 23001 -le single

    bash SydTest.sh $explain -f 0000 -t 4298 -ln recovery
    bash SydTest.sh $explain -f 4350 -t 4998 -ln recovery
    bash SydTest.sh $explain -f 20060 -t 20098 -ln recovery
    bash SydTest.sh $explain -f 20480 -t 20490 -ln recovery
    bash SydTest.sh $explain -f 0001 -t 3699 -le recovery
    bash SydTest.sh $explain -f 4001 -t 4199 -le recovery
    bash SydTest.sh $explain -f 4401 -t 4999 -le recovery

    bash SydTest.sh $explain -f 21100 -t 22300 -ln single
    bash SydTest.sh $explain -f 22530 -t 22560 -ln single
    bash SydTest.sh $explain -f 22600 -t 22998 -ln single
    bash SydTest.sh $explain -f 22500 -t 22528 -lnu single
    bash SydTest.sh $explain -f 22500 -t 22528 -lnu recovery

    #
    bash SydTest.sh $explain -f 13020 -t 13026 -lnN single
    bash SydTest.sh $explain -f 13200 -t 13204 -lnN single
    bash SydTest.sh $explain -f 13210 -t 13214 -lnN single
    bash SydTest.sh $explain -f 13220 -t 13224 -lnN single
    bash SydTest.sh $explain -f 13230 -t 13234 -lnN single
    bash SydTest.sh $explain -f 13240 -t 13244 -lnN single
    bash SydTest.sh $explain -f 13250 -t 13254 -lnN single

    # 
    bash SydTest.sh $explain -f 13300 -t 13304 -lnN single
    bash SydTest.sh $explain -f 13310 -t 13314 -lnN single
    bash SydTest.sh $explain -f 13320 -t 13332 -lnN single

    bash SydTest.sh $explain -f 23010 -t 23098 -ln single
    bash SydTest.sh $explain -f 23300 -t 23406 -ln single
    bash SydTest.sh $explain -f 23011 -t 23029 -le single
    bash SydTest.sh $explain -f 23010 -t 23098 -ln recovery
    bash SydTest.sh $explain -f 23011 -t 23099 -le recovery

    bash SydTest.sh $explain -f 24002 -t 24048 -ln single       #
    bash SydTest.sh $explain -f 24060 -t 24598 -ln single       #
    bash SydTest.sh $explain -f 24602 -t 24988 -ln single       #
    bash SydTest.sh $explain -f 24001 -t 24048 -le single       #
    bash SydTest.sh $explain -f 24000 -t 24998 -ln recovery     #
    bash SydTest.sh $explain -f 24001 -t 24999 -le recovery     #

    # env_checkpoint_period が10のテストをおこなわない
    if [ $reg -ne 1 ]; then
        bash SydTest.sh $explain -f 23100 -t 23298 -ln single
        bash SydTest.sh $explain -f 23408 -t 23428 -ln single
        bash SydTest.sh $explain -f 23101 -t 23299 -le single

        bash SydTest.sh $explain -f 24050 -t 24052 -ln single
    elif [ $reg -ne 4 ]; then
        bash SydTest.sh $explain -s 4926 -ln single
    fi

    #
    bash SydTest.sh $explain -f 13802 -t 13808 -lnN single
    bash SydTest.sh $explain -f 13812 -t 13816 -lnN single

    # Ripway SQL
    bash SydTest.sh $explain -f 210000 -t 211000 -lnN single
    bash SydTest.sh $explain -f 211002 -t 212000 -ln single

    # パラメータ再読み込み
    bash SydTest.sh $explain -s 230000 -ln single
    
    # クリティカルセクション情報出力
    if [ $isLinux -eq 1 ] ; then
        bash SydTest.sh $explain -s 24302 -ln single
        bash SydTest.sh $explain -f 23301 -t 23305 -le single
    fi

    # 多言語
    bash SydTest.sh $explain -f 0000 -t 4778 -lnL single_utf8
    bash SydTest.sh $explain -f 4782 -t 4998 -lnL single_utf8

    if [ $isWindows -eq 1 ] ; then
        bash SydTest.sh $explain -f 20050 -t 20058 -ln single           #v14のDB
        bash SydTest.sh $explain -f 24990 -t 24998 -ln single           #mount
        bash SydTest.sh $explain -f 20011 -t 20019 -le single           #v14のDB
        bash SydTest.sh $explain -s 24000 -ln single                    #una
        bash SydTest.sh $explain -s 24600 -ln single                    #una
    fi
}

# Plan_CollectionType に "FILE"用のテスト項目
dotest_p()
{
    bash SydTest.sh $explain -s 0000 -ln single

    # 自分自身にinsert
    # order by付きでインデックスが付いていないSQLがあるスクリプト
    bash SydTest.sh $explain -f 0200 -t 0394 -ln single
    bash SydTest.sh $explain -f 30200 -t 30394 -ln single
    bash SydTest.sh $explain -f 60200 -t 60394 -ln single

    nameno=(1000 1010 1020 1030 1232 1600 1640 1642 4574 4642 4664 4722 4852 4864 4870 \
    11108 11118 11128 11138 11148 11158 11190 11798 11802 11804 11900 11902 11904 11954 \
    14204 14710 14712 14714 14716 14748 14820 14822 14824 14826 14858 \
    20642 24000 24004 24008 24120 24126 24972 24982 24984 24986 24952 24974 \
    34642 44204 64642 74204)

    for i in "${nameno[@]}" ; do
        bash SydTest.sh $explain -s $i -ln single
    done

    bash SydTest.sh $explain -f 0200 -t 0394 -ln recovery
    bash SydTest.sh $explain -f 30200 -t 30394 -ln recovery
    bash SydTest.sh $explain -f 60200 -t 60394 -ln recovery

    # 更新前の列を使ってUPDATE (LOBを除く)
    bash SydTest.sh $explain -s 1102 -ln single
    bash SydTest.sh $explain -s 1104 -ln single
}

dotest_swf()
{
    bash SydTest.sh $explain -f 30000 -t 34298 -ln single
    bash SydTest.sh $explain -f 34400 -t 34998 -ln single
    bash SydTest.sh $explain -f 40000 -t 44998 -ln single
    bash SydTest.sh $explain -f 60000 -t 64298 -ln single
    bash SydTest.sh $explain -f 64400 -t 64998 -ln single
    bash SydTest.sh $explain -f 70000 -t 74998 -ln single
    bash SydTest.sh $explain -f 30001 -t 33699 -le single
    bash SydTest.sh $explain -f 34001 -t 34199 -le single
    bash SydTest.sh $explain -f 34401 -t 34999 -le single
    bash SydTest.sh $explain -f 50001 -t 54999 -le single
    bash SydTest.sh $explain -f 60001 -t 63699 -le single
    bash SydTest.sh $explain -f 64001 -t 64199 -le single
    bash SydTest.sh $explain -f 64401 -t 64999 -le single
    bash SydTest.sh $explain -f 70001 -t 74999 -le single
    bash SydTest.sh $explain -f 30000 -t 33698 -ln recovery
    bash SydTest.sh $explain -f 34000 -t 34198 -ln recovery
    bash SydTest.sh $explain -f 34400 -t 34998 -ln recovery
    bash SydTest.sh $explain -f 60000 -t 63698 -ln recovery
    bash SydTest.sh $explain -f 64000 -t 64198 -ln recovery
    bash SydTest.sh $explain -f 64400 -t 64998 -ln recovery
    bash SydTest.sh $explain -f 30001 -t 33699 -le recovery
    bash SydTest.sh $explain -f 60001 -t 63699 -le recovery
    bash SydTest.sh $explain -f 33905 -t 34999 -le recovery
    bash SydTest.sh $explain -f 63905 -t 64999 -le recovery

    bash SydTest.sh $explain -f 30000 -t 34998 -lnL single_utf8
    bash SydTest.sh $explain -f 60000 -t 64998 -lnL single_utf8
}

dotest()
{
    case $reg in
        0)
            dotest_d
            dotest_swf ;;
        1)
            dotest_d
            dotest_swf ;;
        2)
            dotest_p ;;
        3)
            dotest_d
            dotest_swf ;;
        *)
            echo "ご確認くださいdotest"
            exit 1;;
        esac
}

######################
# SydTest.shの呼び出し
######################
# 開始時刻 終了時刻を出力する
mainstart=`date`
echo "START              :  $mainstart" >> Report.txt
if [ "$testtype" != "coverage" ] ; then #通常の回帰テスト
    while [ $reg -lt $cont ] ; do
        set_registry
        dotest
        reg=$(expr $reg + 1)
        test_backup
    done
else # coverageテスト
    echo "The test root is the CVS-Root/coverage."
    echo "Try again under the CVS-Root/proj."
fi
echo "END                :  `date`" >> Report.txt

exit 0
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
