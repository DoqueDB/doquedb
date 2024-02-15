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

############
# 変数の設定
############

# isWindows, mytop, hdrive, xdrive, testtype を読み込む
. env.sh

module_ini=$sydtop/tools/setup
syd_dir="$PROGRAMFILES/ricoh/sydney"


############
# 環境の確認
############

# もし以下のファイルが存在しなかったら, SetupForCoverage.shのし忘れ
if [ ! -e $module_ini/sydney/SydTest_pure.ini ] ; then
  bash $module_ini/setupForCoverage.sh
fi


###############
# ローカル関数
###############

writereg() #レジストリを退避する
{
  regedit /e sydtest_mod.reg HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\ModCommonLibrary
  regedit /e sydtest_sydney.reg HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\Sydney
}

readreg() #レジストリを復元する
{
  regedit /s sydtest_mod.reg
  regedit /s sydtest_sydney.reg
}

moduletest()
{
#  if [ "$1" == "Inv" ] ; then
#    pushd $sydtop/test/invtest
#    # できあいのDBのコピー
#    cp -Rf x:/work/InvTestV10New/input/save ./input
#    startupInvTest.bat /d x:/benchmark/patent/src.V2 /t coverage >startupInvTest.log 2>&1
#  fi  
  writereg
  pushd $sydtop/ModuleTest/${1}Test >/dev/null 2>&1
    ./TestExec.bat
    if [ -e ${1}Test.cfy ] ; then
      cp -f ${1}Test.cfy $cvr_dst
    #else
      #cp coverage/${1}Test.cfy $cvr_dst # BtreeTest.cfyのため
    fi
  popd >/dev/null 2>&1
  readreg
}

#cp coverage/merged_result.cfy coverage/SydTest_main.cfy


# 先にやる
# moduletest Btree # テスト不要
moduletest Inv
#cp $testroot/InvTest/Output/log/InvTest_AutoMerge.cfy coverage/InvTest.cfy # V15ではこんなファイルは存在していない

moduletest Common
moduletest Os
moduletest PhysicalFile
moduletest Lock
moduletest Exception
moduletest LogicalFile
moduletest Statement
moduletest Vector
moduletest Trans
#moduletest FullTextFile # V15のモジュールテスト内にフォルダが存在しない


# スキーマテストはSydTest.exeをいじるのでバックアップする (もしdllもいじるなら, 再インストの方がいいかも)
cp -f "$sydtest" "$sydtest".bak

# スキーマテストのみiniファイルを使用
cp -f $module_ini/sydney/SydTest_pure.ini "$syd_dir"

moduletest Schema

mv -f ./coverage/merged_result.cfy coverage/SchemaTest.cfy
mv -f "$sydtest".bak "$sydtest"
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
