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

# Windows版Purifyテスト用実行スクリプト
#   初めからテストする場合にはテスト前に以下のスクリプトを実行
#      ./purify_init.sh

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
purify_sh_mode=1; export purify_sh_mode 

# 最初に確認したいテスト
dotest_c()
{
    bash SydTest.sh -s 0000 -lnp single
}

# 基本 (すべてのテストを実施するように記載します。)
dotest_o()
{
    bash SydTest.sh -f 4300 -t 4310 -lnp single  # 4312, 4314 は最後に記載
    bash SydTest.sh -f 4201 -t 4209 -lep single  # 4311
    bash SydTest.sh -f 20421 -t 20441 -lep single
    bash SydTest.sh -f 9300 -t 9398 -lnp multi
    bash SydTest.sh -f 4300 -t 4306 -lnp recovery
    bash SydTest.sh -f 4201 -t 4231 -lep recovery

# オプションつき
    bash SydTest.sh -f 13000 -t 13004 -lnNp single
    bash SydTest.sh -f 13006 -t 13010 -lnNp single
    bash SydTest.sh -f 13012 -t 13016 -lnNp single
    bash SydTest.sh -f 13020 -t 13026 -lnNp single
    bash SydTest.sh -f 13200 -t 13204 -lnNp single
    bash SydTest.sh -f 13210 -t 13214 -lnNp single
    bash SydTest.sh -f 13220 -t 13224 -lnNp single
    bash SydTest.sh -f 13230 -t 13234 -lnNp single
    bash SydTest.sh -f 13240 -t 13244 -lnNp single
    bash SydTest.sh -f 13250 -t 13254 -lnNp single
    bash SydTest.sh -f 13300 -t 13304 -lnNp single
    bash SydTest.sh -f 13310 -t 13314 -lnNp single
    bash SydTest.sh -f 13320 -t 13332 -lnNp single
    bash SydTest.sh -f 13001 -t 13015 -leNMp single
    bash SydTest.sh -f 13021 -t 13033 -leNMp single

    bash SydTest.sh -f 13501 -t 13801 -leMp single

    bash SydTest.sh -f 22500 -t 22528 -lnup single
    bash SydTest.sh -f 22500 -t 22528 -lnup recovery
    bash SydTest.sh -f 25500 -t 25600 -lnup multi
}

dotest_dist()
{
    bash SydTest.sh -f 200000 -t 200800 -lnp single_dist
    bash SydTest.sh -f 200000 -t 200800 -lep single_dist
    bash SydTest.sh -f 210000 -t 211000 -lnNp single_dist
    bash SydTest.sh -f 211002 -t 212000 -lnp single_dist
    bash SydTest.sh -f 200000 -t 200800 -lnp recovery_dist

    bash SydTest.sh -s 214302 -lnp single_dist

    bash SydTest.sh -f 220000 -t 220044 -lnp single_dist
    bash SydTest.sh -f 220100 -t 220124 -lnNp single_dist
    bash SydTest.sh -f 220130 -t 220174 -lnp single_dist
    bash SydTest.sh -f 220214 -t 220250 -lnp single_dist
    bash SydTest.sh -f 220300 -t 220304 -lnNp single_dist
    bash SydTest.sh -f 220310 -t 220334 -lnp single_dist
    bash SydTest.sh -f 220340 -t 220444 -lnp single_dist
    #bash SydTest.sh -s 220450 -lnp single_dist            # v17.1未対応
    bash SydTest.sh -f 220454 -t 220470 -lnp single_dist
    #bash SydTest.sh -s 220474-lnp single_dist            # v17.1未対応
    bash SydTest.sh -s 220480 -lnp single_dist
    #bash SydTest.sh -f 220484 -t 220490 -lnp single_dist  # v17.1未対応   
    bash SydTest.sh -f 220494 -t 220630 -lnp single_dist

    bash SydTest.sh -f 220700 -t 220840 -lnp single_dist  
}

dotest_single()
{
    bash SydTest.sh -f 0000 -t 1102 -lnp single
    bash SydTest.sh -f 1226 -t 2858 -lnp single #2860を除く
    bash SydTest.sh -f 2862 -t 4998 -lnp single
    bash SydTest.sh -f 10000 -t 14998 -lnp single
    bash SydTest.sh -f 20000 -t 23298 -lnp single #23310, 23312, 23314を除く
    bash SydTest.sh -f 23320 -t 24998 -lnp single

    bash SydTest.sh -f 0001 -t 4999 -lep single
    bash SydTest.sh -f 10001 -t 14999 -lep single
    bash SydTest.sh -f 20001 -t 24999 -lep single
}

dotest_recovery()
{
    bash SydTest.sh -f 0000 -t 9998 -lnp recovery
    bash SydTest.sh -f 0001 -t 9999 -lep recovery
    bash SydTest.sh -f 10000 -t 29998 -lnp recovery
    bash SydTest.sh -f 10001 -t 29999 -lep recovery
}

dotest_multi()
{
    bash SydTest.sh -f 5000 -t 6498 -lnp multi
    bash SydTest.sh -f 6608 -t 9398 -lnp multi
    bash SydTest.sh -f 9500 -t 9998 -lnp multi
    bash SydTest.sh -f 5001 -t 9999 -lep multi
}

dotest_languages()
{
    bash SydTest.sh -f 0000 -t 4998 -lnp single_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnJp single_ja
    bash SydTest.sh -f 0000 -t 4998 -lnJp single_ja_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnEp single_eu
    bash SydTest.sh -f 0000 -t 4998 -lnEp single_eu_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnZp single_zh_utf8
}

dotest_swf()
{
    bash SydTest.sh -f 30000 -t 34298 -lnp single
    bash SydTest.sh -f 34400 -t 34998 -lnp single
    bash SydTest.sh -f 40000 -t 44998 -lnp single
    bash SydTest.sh -f 60000 -t 64298 -lnp single
    bash SydTest.sh -f 64400 -t 64998 -lnp single
    bash SydTest.sh -f 70000 -t 74998 -lnp single
    bash SydTest.sh -f 30001 -t 33699 -lep single
    bash SydTest.sh -f 34001 -t 34199 -lep single
    bash SydTest.sh -f 34401 -t 34999 -lep single
    bash SydTest.sh -f 50001 -t 54999 -lep single
    bash SydTest.sh -f 60001 -t 63699 -lep single
    bash SydTest.sh -f 64001 -t 64199 -lep single
    bash SydTest.sh -f 64401 -t 64999 -lep single
    bash SydTest.sh -f 70001 -t 74999 -lep single
    bash SydTest.sh -f 35000 -t 39298 -lnp multi
    bash SydTest.sh -f 65000 -t 69298 -lnp multi
    bash SydTest.sh -f 35001 -t 39999 -lep multi
    bash SydTest.sh -f 65001 -t 69999 -lep multi
    bash SydTest.sh -f 30000 -t 33698 -lnp recovery
    bash SydTest.sh -f 34000 -t 34198 -lnp recovery
    bash SydTest.sh -f 34400 -t 39998 -lpn recovery
    bash SydTest.sh -f 60000 -t 63698 -lnp recovery
    bash SydTest.sh -f 64000 -t 64198 -lnp recovery
    bash SydTest.sh -f 64400 -t 69998 -lnp recovery
    bash SydTest.sh -f 30001 -t 33699 -lep recovery
    bash SydTest.sh -f 60001 -t 63699 -lep recovery
    bash SydTest.sh -f 33905 -t 39999 -lep recovery
    bash SydTest.sh -f 63905 -t 69999 -lep recovery

    bash SydTest.sh -f 30000 -t 34998 -lnLp single_utf8
    bash SydTest.sh -f 60000 -t 64998 -lnLp single_utf8

    if [ $diskfull -eq 0 ] ; then #diskfulltest
        if [ $isWindows -eq 0 ] ; then
            (sleep 10; kill_sydtest) &
        fi
        bash SydTest.sh -f 33701 -t 33901 -lep single
        bash SydTest.sh -f 63701 -t 63901 -lep single
        bash SydTest.sh -f 33701 -t 33903 -lep recovery
        bash SydTest.sh -f 63701 -t 63903 -lep recovery
    fi

    bash SydTest.sh -f 34300 -t 34308 -lnp single
    bash SydTest.sh -f 64300 -t 64308 -lnp single
    bash SydTest.sh -f 34201 -t 34221 -lep single
    bash SydTest.sh -f 64201 -t 64221 -lep single
    bash SydTest.sh -f 50421 -t 50441 -lep single
    bash SydTest.sh -f 80421 -t 80441 -lep single
    bash SydTest.sh -f 94201 -t 94399 -lep single
    bash SydTest.sh -f 39300 -t 39398 -lnp multi
    bash SydTest.sh -f 69300 -t 69398 -lnp multi
    bash SydTest.sh -f 34300 -t 34306 -lnp recovery
    bash SydTest.sh -f 64300 -t 64306 -lnp recovery
    bash SydTest.sh -f 34201 -t 34231 -lep recovery
    bash SydTest.sh -f 64201 -t 64231 -lep recovery
}

dotest_skiplist()
{
    if [ -e $pur_dst/Skiplist.log ];then
        perl -p -e "s!Skip (\w+)/normal/(\w+)!\1/lnp/\2!; s!Skip (\w+)/except/(\w+)!\1/lep/\2!; s!(\w+)/(\w+)/(\w+)!bash SydTest.sh -s \3 -\2 \1!" $pur_dst/Skiplist.log > SkiplistTest.sh
        purify_sh_mode=0; export purify_sh_mode
        bash SkiplistTest.sh
    fi
}

######################
# SydTest.shの呼び出し
######################

if [ "$testtype" != "coverage" ] ; then #通常の回帰テスト

    echo "The test root is the CVS-Root/proj."
    echo "Try again under the CVS-Root/coverage."

else # purifyテスト

    dotest_c
    dotest_o
    dotest_dist
    dotest_single
    dotest_recovery
    dotest_multi
    dotest_languages
    dotest_swf
    dotest_skiplist

fi

exit 0
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
