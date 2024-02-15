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

# Windows版カバレージテスト用実行スクリプト
#   初めからテストする場合にはテスト前に以下のスクリプトを実行
#      tools/setup/setupForCoverage.sh
#      ./coverage_init.sh

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

# mount size check
if [ $isWindows -eq 1 ] ; then
   if ! `df --block-size=1 h:|grep 11423232 >/dev/null` ; then
      echo "Initial H drive size is incorrect. Stop"
      exit 1
   fi
fi 

# モジュールテストのビルド忘れをチェックする
if [ ! -e $sydtop/ModuleTest/OsTest/c.O-[Cc]overage/[Oo]s[Tt]est.exe ] ; then
    echo "You forgot to build module tests. Stop"
    exit 1
fi

###############
# ローカル関数
###############

# coverage_init.sh で保存したレジストリに戻す
restore_registry()
{
    REG DELETE "$regpath_sydney" /f > nul 2>&1
    regedit /s sydtest_sydney.reg
    regedit /s sydtest_mod.reg
}

######################
# SydTest.shの呼び出し
######################

if [ "$testtype" != "coverage" ] ; then #通常の回帰テスト

    echo "The test root is the CVS-Root/proj."
    echo "Try again under the CVS-Root/coverage."

else # coverageテスト

    #SydTestの本スクリプト
    bash SydTest.sh -f 0000 -t 4998 -lnc single
    bash SydTest.sh -f 10200 -t 14998 -lnc single
    bash SydTest.sh -f 20000 -t 20100 -lnc single
    bash SydTest.sh -f 20400 -t 20420 -lnc single
    bash SydTest.sh -f 0001 -t 3699 -lec single
    bash SydTest.sh -f 3903 -t 4999 -lec single
    bash SydTest.sh -f 20001 -t 20101 -lnc single

    bash SydTest.sh -f 5000 -t 9998 -lnc multi
    bash SydTest.sh -f 5001 -t 9999 -lec multi

    bash SydTest.sh -f 0000 -t 9998 -lnc recovery
    bash SydTest.sh -f 20480 -t 20490 -lnc recovery
    bash SydTest.sh -f 0001 -t 3699 -lec recovery
    bash SydTest.sh -f 3905 -t 9999 -lec recovery

    if [ $diskfull -eq 1 ] ; then #diskfulltest
        bash SydTest.sh -f 3701 -t 3901 -lec single
        bash SydTest.sh -f 3701 -t 3903 -lec recovery
    fi

    bash SydTest.sh -f 0000 -t 4998 -lnc single_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnJc single_ja
    bash SydTest.sh -f 0000 -t 4998 -lnJc single_ja_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnEc single_eu
    bash SydTest.sh -f 0000 -t 4998 -lnEc single_eu_utf8
    bash SydTest.sh -f 0000 -t 4998 -lnZc single_zh_utf8

    #レジストリを直す
    restore_registry
    
    #サーバテスト
    bash SydServerTest.sh

    #Sqliテスト
    bash SqliTest.sh

    #JDBCテスト
    bash JDBCTest.sh
    
    #fakeerror周りのcoverage上昇のため
    bash FakeCov.sh

    #レジストリを直す
    restore_registry

    #Newフォルダ内のテスト
    bash NewScriptCov.sh
    
    #レジストリを直す
    restore_registry

    #モジュールテスト
    bash moduletests.sh

    #レジストリを直す
    restore_registry

fi

exit 0

#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
