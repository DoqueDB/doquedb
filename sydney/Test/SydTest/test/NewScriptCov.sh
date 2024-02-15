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

# New, New2以下のテストスクリプトをカバレージ計測するためのシェルスクリプト

# New以下
#   全ファイルを番号名にコピーしてからテスト、終了したらそれらのファイルは削除
#
# New2以下
#   前バージョンもテストしていない, 尚且つ現時点でのSydTest.shが番号以外に対応していないので
#   new2以下のスクリプトをテストすることが難しい。Newと同じ方法もあるが、
#   テスト自体はスクリプト名(a,b,c)と関係しているので問題がある。よって今の所は保留しておく。


############
# 変数の設定
############

# isWindows, mytop, hdrive, xdrive, testtype を読み込む
. env.sh


############
# 環境の確認
############

# テストはWindows版のみ
if [ $isWindows -eq 0 ] ; then
  echo "This coverage test is for only Windows version.  Stop"
  exit 1
fi

# port番号はレジのCommunication_SydneyPortNumberから入手 (defaultは54321)
port=54321

# sydserver.exeの場所を特定させる
sydserver="$PROGRAMFILES/ricoh/TRMeister/SydServer.exe"

# instrumentするsydserver.exeの場所を特定させる
cov_sydserver="$PROGRAMFILES/Rational/PurifyPlus/cache/SydServer\$Coverage_C_Program Files_ricoh_TRMeister.exe"

# sydserverを事前にinstrumentする
if [ ! -e "$cov_sydserver" ] ; then
  "$coverage" /run=no "$sydserver"
fi

# 注 sydtestはSydTest.sh内でするので必要ない


###############
# ローカル関数
###############

# 全ファイルを番号名にコピーする
copy_file()
{
  if [ $2 = "normal" ] ; then
    i=0100
  else
    i=0101
  fi
  
  pushd ./$1/$2 >/dev/null 2>&1
    for file in *.txt ; do
      cp -f $file $i.txt
      i=0`expr $i + 2`
    done
  popd >/dev/null 2>&1
}

# copy_fileで作成したファイルを削除
del_file() 
{
  pushd ./$1/$2 >/dev/null 2>&1
    for file in [0][0-9][0-9][0-9].txt ; do
      rm -f $file 
    done
  popd >/dev/null 2>&1
}


######
# 本体
######

  # スクリプト内でパスを変えているかも知れないので一応バックアップ
  pathbak=$PATH

  copy_file new normal
  copy_file new except

  bash SydTest.sh -f 0100 -t 0999 -rnc new
  bash SydTest.sh -f 0101 -t 0999 -rec new

  del_file new normal
  del_file new except
  
  # パスを元に戻す
  PATH=$pathbak

exit 0

#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
