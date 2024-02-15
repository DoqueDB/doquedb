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

# Coverageを最初からテストする場合にのみ使用

############
# 変数の設定
############

# isWindows, mytop, hdrive, xdrive, testtype を読み込む
. env.sh

# カバレージテストのビルドオプションで作られたパッケージ
package=O-coverage

############
# 環境の確認
############

# テストはWindows版のみ
if [ $isWindows -eq 0 ] ; then
  echo "This coverage test is only for Windows version"
  exit 1
fi

###############
# ローカル関数
###############

writereg() #レジストリを退避する
{
  regedit /e sydtest_mod.reg HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\ModCommonLibrary
  regedit /e sydtest_sydney.reg HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\TRMeister
}

######
# 本体
######

if [ "$testtype" != "coverage" ] ; then

  # フォルダがprojのままなのでテスト不可
  echo "The testtype is not CVS-Root/coverage.."

else

  # キャッシュファイルを削除
  rm -f "$PROGRAMFILES/Rational/PurifyPlus/cache/"*

  # 前回テストした時のcfyファイルが残っているかもしれない
  rm -f $sydtop/[Tt]est/[Ss]yd[Tt]est/test/coverage/*.cfy

  # テスト前に現在のSydneyレジを保存
  writereg
  
fi


exit 0

#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
