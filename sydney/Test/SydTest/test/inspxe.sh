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

# Intel Inspectorテスト用実行スクリプト
#   初めからテストする場合にはテスト前に以下のスクリプトを実行
#      ./inspxe_ini.sh

############
# 変数の設定
############

# mytop, hdrive, xdrive, testtype を読み込む
. env.sh

############
# 環境の確認
############

# 最初に確認したいテスト
dotest_c()
{
    bash SydTest.sh -s 0000 -lnm single
}

# 基本 (すべてのテストを実施するように記載します。)
dotest_o()
{
    bash SydTest.sh -f 4300 -t 4310 -lnm single  # 4312, 4314 は最後に記載
    bash SydTest.sh -f 4201 -t 4209 -lem single  # 4311
    bash SydTest.sh -f 20421 -t 20441 -lem single
    bash SydTest.sh -f 9300 -t 9398 -lnm multi
    bash SydTest.sh -f 4300 -t 4306 -lnm recovery
    bash SydTest.sh -f 4201 -t 4231 -lem recovery

# オプションつき
    bash SydTest.sh -f 13000 -t 13004 -lnNm single
    bash SydTest.sh -f 13006 -t 13010 -lnNm single
    bash SydTest.sh -f 13012 -t 13016 -lnNm single
    bash SydTest.sh -f 13020 -t 13026 -lnNm single
    bash SydTest.sh -f 13200 -t 13204 -lnNm single
    bash SydTest.sh -f 13210 -t 13214 -lnNm single
    bash SydTest.sh -f 13220 -t 13224 -lnNm single
    bash SydTest.sh -f 13230 -t 13234 -lnNm single
    bash SydTest.sh -f 13240 -t 13244 -lnNm single
    bash SydTest.sh -f 13250 -t 13254 -lnNm single
    bash SydTest.sh -f 13300 -t 13304 -lnNm single
    bash SydTest.sh -f 13310 -t 13314 -lnNm single
    bash SydTest.sh -f 13320 -t 13332 -lnNm single
    bash SydTest.sh -f 13001 -t 13015 -leNMm single
    bash SydTest.sh -f 13021 -t 13033 -leNMm single

    bash SydTest.sh -f 13501 -t 13801 -leMm single

    bash SydTest.sh -f 22500 -t 22528 -lnum single
    bash SydTest.sh -f 22500 -t 22528 -lnum recovery
    bash SydTest.sh -f 25500 -t 25600 -lnum multi
}

dotest_dist()
{
    bash SydTest.sh -f 200000 -t 200800 -lnm single_dist
    bash SydTest.sh -f 200000 -t 200800 -lem single_dist
    bash SydTest.sh -f 210000 -t 211000 -lnNp single_dist
    bash SydTest.sh -f 211002 -t 212000 -lnm single_dist
    bash SydTest.sh -f 200000 -t 200800 -lnm recovery_dist

    bash SydTest.sh -s 214302 -lnm single_dist

    bash SydTest.sh -f 220000 -t 220044 -lnm single_dist
    bash SydTest.sh -f 220100 -t 220124 -lnNm single_dist
    bash SydTest.sh -f 220130 -t 220174 -lnm single_dist
    bash SydTest.sh -f 220214 -t 220250 -lnm single_dist
    bash SydTest.sh -f 220300 -t 220304 -lnNm single_dist
    bash SydTest.sh -f 220310 -t 220334 -lnm single_dist
    bash SydTest.sh -f 220340 -t 220444 -lnm single_dist
    #bash SydTest.sh -s 220450 -lnm single_dist            # v17.1未対応
    bash SydTest.sh -f 220454 -t 220470 -lnm single_dist
    #bash SydTest.sh -s 220474-lnm single_dist            # v17.1未対応
    bash SydTest.sh -s 220480 -lnm single_dist
    #bash SydTest.sh -f 220484 -t 220490 -lnm single_dist  # v17.1未対応   
    bash SydTest.sh -f 220494 -t 220630 -lnm single_dist

    bash SydTest.sh -f 220700 -t 220840 -lnm single_dist  
}

dotest_single()
{
    bash SydTest.sh -f 0000 -t 1102 -lnm single
    bash SydTest.sh -f 1226 -t 2858 -lnm single #2860を除く
    bash SydTest.sh -f 2862 -t 4998 -lnm single
    bash SydTest.sh -f 10000 -t 14998 -lnm single
    bash SydTest.sh -f 20000 -t 23298 -lnm single #23310, 23312, 23314を除く
    bash SydTest.sh -f 23320 -t 24998 -lnm single

    bash SydTest.sh -f 0001 -t 4999 -lem single
    bash SydTest.sh -f 10001 -t 14999 -lem single
    bash SydTest.sh -f 20001 -t 24999 -lem single
}

dotest_recovery()
{
    bash SydTest.sh -f 0000 -t 9998 -lnm recovery
    bash SydTest.sh -f 0001 -t 9999 -lem recovery
    bash SydTest.sh -f 10000 -t 29998 -lnm recovery
    bash SydTest.sh -f 10001 -t 29999 -lem recovery
}

dotest_multi()
{
    bash SydTest.sh -f 5000 -t 6498 -lnm multi
    bash SydTest.sh -f 6608 -t 9398 -lnm multi
    bash SydTest.sh -f 9500 -t 9998 -lnm multi
    bash SydTest.sh -f 5001 -t 9999 -lem multi
}

dotest_languages()
{
    bash SydTest.sh -f 0000 -t 4998 -lnm single_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnJm single_ja
    bash SydTest.sh -f 0000 -t 4998 -lnJm single_ja_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnEm single_eu
    bash SydTest.sh -f 0000 -t 4998 -lnEm single_eu_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnZm single_zh_utf8
}

dotest_swf()
{
    bash SydTest.sh -f 30000 -t 34298 -lnm single
    bash SydTest.sh -f 34400 -t 34998 -lnm single
    bash SydTest.sh -f 40000 -t 44998 -lnm single
    bash SydTest.sh -f 60000 -t 64298 -lnm single
    bash SydTest.sh -f 64400 -t 64998 -lnm single
    bash SydTest.sh -f 70000 -t 74998 -lnm single
    bash SydTest.sh -f 30001 -t 33699 -lem single
    bash SydTest.sh -f 34001 -t 34199 -lem single
    bash SydTest.sh -f 34401 -t 34999 -lem single
    bash SydTest.sh -f 50001 -t 54999 -lem single
    bash SydTest.sh -f 60001 -t 63699 -lem single
    bash SydTest.sh -f 64001 -t 64199 -lem single
    bash SydTest.sh -f 64401 -t 64999 -lem single
    bash SydTest.sh -f 70001 -t 74999 -lem single
    bash SydTest.sh -f 35000 -t 39298 -lnm multi
    bash SydTest.sh -f 65000 -t 69298 -lnm multi
    bash SydTest.sh -f 35001 -t 39999 -lem multi
    bash SydTest.sh -f 65001 -t 69999 -lem multi
    bash SydTest.sh -f 30000 -t 33698 -lnm recovery
    bash SydTest.sh -f 34000 -t 34198 -lnm recovery
    bash SydTest.sh -f 34400 -t 39998 -lnm recovery
    bash SydTest.sh -f 60000 -t 63698 -lnm recovery
    bash SydTest.sh -f 64000 -t 64198 -lnm recovery
    bash SydTest.sh -f 64400 -t 69998 -lnm recovery
    bash SydTest.sh -f 30001 -t 33699 -lem recovery
    bash SydTest.sh -f 60001 -t 63699 -lem recovery
    bash SydTest.sh -f 33905 -t 39999 -lem recovery
    bash SydTest.sh -f 63905 -t 69999 -lem recovery

    bash SydTest.sh -f 30000 -t 34998 -lnLm single_utf8
    bash SydTest.sh -f 60000 -t 64998 -lnLm single_utf8

    if [ $diskfull -eq 0 ] ; then #diskfulltest
        if [ $isWindows -eq 0 ] ; then
            (sleep 10; kill_sydtest) &
        fi
        bash SydTest.sh -f 33701 -t 33901 -lem single
        bash SydTest.sh -f 63701 -t 63901 -lem single
        bash SydTest.sh -f 33701 -t 33903 -lem recovery
        bash SydTest.sh -f 63701 -t 63903 -lem recovery
    fi

    bash SydTest.sh -f 34300 -t 34308 -lnm single
    bash SydTest.sh -f 64300 -t 64308 -lnm single
    bash SydTest.sh -f 34201 -t 34221 -lem single
    bash SydTest.sh -f 64201 -t 64221 -lem single
    bash SydTest.sh -f 50421 -t 50441 -lem single
    bash SydTest.sh -f 80421 -t 80441 -lem single
    bash SydTest.sh -f 94201 -t 94399 -lem single
    bash SydTest.sh -f 39300 -t 39398 -lnm multi
    bash SydTest.sh -f 69300 -t 69398 -lnm multi
    bash SydTest.sh -f 34300 -t 34306 -lnm recovery
    bash SydTest.sh -f 64300 -t 64306 -lnm recovery
    bash SydTest.sh -f 34201 -t 34231 -lem recovery
    bash SydTest.sh -f 64201 -t 64231 -lem recovery
}

######################
# SydTest.shの呼び出し
######################
dotest_c
dotest_o
dotest_dist
dotest_single
dotest_recovery
dotest_multi
dotest_languages
dotest_swf

exit 0
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
