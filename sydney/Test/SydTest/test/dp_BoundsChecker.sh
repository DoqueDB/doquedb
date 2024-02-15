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

# BoundsCheckerテスト

############
# 変数の設定
############

# isWindows, mytop, hdrive, xdrive, testtype を読み込む
. env.sh

############
# 環境の確認
############

if [ ! -e $xdrive/ ]; then
    echo "${xdrive} is not mounted or don't exist, so you can't examine any test(ex: 1350.txt)."
fi

diskfull=1
if [ ! -e ${hdrive} ]; then
    echo "${hdrive} is not mounted, so you can't examine diskfulltest(ex: 3701.txt)."
    diskfull=0
fi

# ディスクフル用マウントサイズチェック
if [ $isWindows -eq 1 ] && [ $diskfull -eq 1 ] ; then
    if [ `uname | grep '6' | wc -w` -ge 1 ]; then
        if ! `df --block-size=1 $hdrive|grep 11476992 >/dev/null` ; then
            echo "Initial $hdrive size is incorrect. Stop"
            #exit 1
        fi
    else
        if ! `df --block-size=1 $hdrive|grep 11423232 >/dev/null` ; then
            echo "Initial $hdrive size is incorrect. Stop"
            #exit 1
        fi
    fi
fi

# 実施したテストをスキップ
bc_sh_mode=1; export bc_sh_mode

# 最初に確認したいテスト
dotest_c()
{
    bash SydTest.sh -s 0000 -lnb single
}

# 基本 (すべてのテストを実施するように記載します。)
dotest_o()
{
    bash SydTest.sh -f 4300 -t 4310 -lnb single  # 4312, 4314 は最後に記載
    bash SydTest.sh -f 4201 -t 4209 -leb single  # 4311
    bash SydTest.sh -f 20421 -t 20441 -leb single
    bash SydTest.sh -f 9300 -t 9398 -lnb multi
    bash SydTest.sh -f 4300 -t 4306 -lnb recovery
    bash SydTest.sh -f 4201 -t 4231 -leb recovery

# オプションつき   Terminateしないで終了するとエラー出力がでます。
    bash SydTest.sh -f 13000 -t 13004 -lnNb single
    bash SydTest.sh -f 13006 -t 13010 -lnNb single
    bash SydTest.sh -f 13012 -t 13016 -lnNb single
    bash SydTest.sh -f 13020 -t 13026 -lnNb single
    bash SydTest.sh -f 13200 -t 13204 -lnNb single
    bash SydTest.sh -f 13210 -t 13214 -lnNb single
    bash SydTest.sh -f 13220 -t 13224 -lnNb single
    bash SydTest.sh -f 13230 -t 13234 -lnNb single
    bash SydTest.sh -f 13240 -t 13244 -lnNb single
    bash SydTest.sh -f 13250 -t 13254 -lnNb single
    bash SydTest.sh -f 13300 -t 13304 -lnNb single
    bash SydTest.sh -f 13310 -t 13314 -lnNb single
    bash SydTest.sh -f 13320 -t 13332 -lnNb single
    bash SydTest.sh -f 13001 -t 13015 -leNMb single
    bash SydTest.sh -f 13021 -t 13033 -leNMb single

    bash SydTest.sh -f 13501 -t 13801 -leMb single

    bash SydTest.sh -f 22500 -t 22528 -lnub single
    bash SydTest.sh -f 22500 -t 22528 -lnub recovery
    bash SydTest.sh -f 25500 -t 25600 -lnub multi
}

dotest_dist()
{
    bash SydTest.sh -f 200000 -t 200800 -lnb single_dist
    bash SydTest.sh -f 200000 -t 200800 -leb single_dist
    bash SydTest.sh -f 210000 -t 211000 -lnNb single_dist
    bash SydTest.sh -f 211002 -t 212000 -lnb single_dist
    bash SydTest.sh -f 200000 -t 200800 -lnb recovery_dist

    bash SydTest.sh -f 220000 -t 220044 -lnb single_dist
    bash SydTest.sh -f 220100 -t 220124 -lnNb single_dist
    bash SydTest.sh -f 220130 -t 220174 -lnb single_dist
    bash SydTest.sh -f 220214 -t 220250 -lnb single_dist
    bash SydTest.sh -f 220300 -t 220304 -lnNb single_dist
    bash SydTest.sh -f 220310 -t 220334 -lnb single_dist
    bash SydTest.sh -f 220340 -t 220444 -lnb single_dist
    #bash SydTest.sh -s 220450 -lnb single_dist            # v17.1未対応
    bash SydTest.sh -f 220454 -t 220470 -lnb single_dist
    #bash SydTest.sh -s 220474 -lnb single_dist            # v17.1未対応
    bash SydTest.sh -s 220480 -lnb single_dist
    #bash SydTest.sh -f 220484 -t 220490 -lnb single_dist  # v17.1未対応   
    bash SydTest.sh -f 220494 -t 220630 -lnb single_dist   
}

dotest_single()
{
    bash SydTest.sh -f 0000 -t 1102 -lnb single
    bash SydTest.sh -f 1226 -t 2858 -lnb single #2860を除く
    bash SydTest.sh -f 2862 -t 4998 -lnb single
    bash SydTest.sh -f 10000 -t 14998 -lnb single
    bash SydTest.sh -f 20000 -t 24998 -lnb single

    bash SydTest.sh -f 0001 -t 4999 -leb single
    bash SydTest.sh -f 10001 -t 14999 -leb single
    bash SydTest.sh -f 20001 -t 24999 -leb single
}

dotest_recovery()
{
    bash SydTest.sh -f 0000 -t 9998 -lnb recovery
    bash SydTest.sh -f 0001 -t 9999 -leb recovery
    bash SydTest.sh -f 10000 -t 29998 -lnb recovery
    bash SydTest.sh -f 10001 -t 29999 -leb recovery
}

dotest_multi()
{
    bash SydTest.sh -f 5000 -t 6498 -lnb multi
    bash SydTest.sh -f 6608 -t 9398 -lnb multi
    bash SydTest.sh -f 9500 -t 9998 -lnb multi
    bash SydTest.sh -f 5001 -t 9999 -leb multi
}

dotest_languages()
{
    bash SydTest.sh -f 0000 -t 4998 -lnb single_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnJb single_ja
    bash SydTest.sh -f 0000 -t 4998 -lnJb single_ja_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnEb single_eu
    bash SydTest.sh -f 0000 -t 4998 -lnEb single_eu_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnZb single_zh_utf8
}

dotest_swf()
{
    bash SydTest.sh -f 30000 -t 34298 -lnb single
    bash SydTest.sh -f 34400 -t 34998 -lnb single
    bash SydTest.sh -f 40000 -t 44998 -lnb single
    bash SydTest.sh -f 60000 -t 64298 -lnb single
    bash SydTest.sh -f 64400 -t 64998 -lnb single
    bash SydTest.sh -f 70000 -t 74998 -lnb single
    bash SydTest.sh -f 30001 -t 33699 -leb single
    bash SydTest.sh -f 34001 -t 34199 -leb single
    bash SydTest.sh -f 34401 -t 34999 -leb single
    bash SydTest.sh -f 50001 -t 54999 -leb single
    bash SydTest.sh -f 60001 -t 63699 -leb single
    bash SydTest.sh -f 64001 -t 64199 -leb single
    bash SydTest.sh -f 64401 -t 64999 -leb single
    bash SydTest.sh -f 70001 -t 74999 -leb single
    bash SydTest.sh -f 35000 -t 39298 -lnb multi
    bash SydTest.sh -f 65000 -t 69298 -lnb multi
    bash SydTest.sh -f 35001 -t 39999 -leb multi
    bash SydTest.sh -f 65001 -t 69999 -leb multi
    bash SydTest.sh -f 30000 -t 33698 -lnb recovery
    bash SydTest.sh -f 34000 -t 34198 -lnb recovery
    bash SydTest.sh -f 34400 -t 39998 -lnb recovery
    bash SydTest.sh -f 60000 -t 63698 -lnb recovery
    bash SydTest.sh -f 64000 -t 64198 -lnb recovery
    bash SydTest.sh -f 64400 -t 69998 -lnb recovery
    bash SydTest.sh -f 30001 -t 33699 -leb recovery
    bash SydTest.sh -f 60001 -t 63699 -leb recovery
    bash SydTest.sh -f 33905 -t 39999 -leb recovery
    bash SydTest.sh -f 63905 -t 69999 -leb recovery

    bash SydTest.sh -f 30000 -t 34998 -lnLb single_utf8
    bash SydTest.sh -f 60000 -t 64998 -lnLb single_utf8

    if [ $diskfull -eq 0 ] ; then #diskfulltest
        if [ $isWindows -eq 0 ] ; then
            (sleep 10; kill_sydtest) &
        fi
        bash SydTest.sh -f 33701 -t 33901 -leb single
        bash SydTest.sh -f 63701 -t 63901 -leb single
        bash SydTest.sh -f 33701 -t 33903 -leb recovery
        bash SydTest.sh -f 63701 -t 63903 -leb recovery
    fi

    bash SydTest.sh -f 34300 -t 34308 -lnb single
    bash SydTest.sh -f 64300 -t 64308 -lnb single
    bash SydTest.sh -f 34201 -t 34221 -leb single
    bash SydTest.sh -f 64201 -t 64221 -leb single
    bash SydTest.sh -f 50421 -t 50441 -leb single
    bash SydTest.sh -f 80421 -t 80441 -leb single
    bash SydTest.sh -f 94201 -t 94399 -leb single
    bash SydTest.sh -f 39300 -t 39398 -lnb multi
    bash SydTest.sh -f 69300 -t 69398 -lnb multi
    bash SydTest.sh -f 34300 -t 34306 -lnb recovery
    bash SydTest.sh -f 64300 -t 64306 -lnb recovery
    bash SydTest.sh -f 34201 -t 34231 -leb recovery
    bash SydTest.sh -f 64201 -t 64231 -leb recovery
}

dotest_skiplist()
{
    if [ -e $dp_dst/Skiplist.log ];then
        perl -p -e "s!Skip (\w+)/normal/(\w+)!\1/lnb/\2!; s!Skip (\w+)/except/(\w+)!\1/leb/\2!; s!(\w+)/(\w+)/(\w+)!bash SydTest.sh -s \3 -\2 \1!" $dp_dst/Skiplist.log > SkiplistTest.sh
        bc_sh_mode=0; export bc_sh_mode
        bash SkiplistTest.sh
    fi
}

######################
# SydTest.shの呼び出し
######################

dotest_c
dotest_dist
dotest_single
dotest_recovery
dotest_multi
dotest_languages
dotest_swf
dotest_o
dotest_skiplist

exit 0

#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
