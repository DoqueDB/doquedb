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

############
#定数の定義
############
. env.sh

# port番号はレジのCommunication_SydneyPortNumberから入手 (defaultは54321)
port=54321


# ant testはjunit.jarが必要
if [ ! -e "$ANT_HOME/lib/junit.jar" ] ; then
  cp "$sydtop/java/JDBC/olib/junit.jar" "$ANT_HOME/lib"
fi


# sydserver.exe, sqli.exeの場所を特定させる
sydserver="$PROGRAMFILES/ricoh/TRMeister/SydServer.exe"
sqli="$PROGRAMFILES/ricoh/TRMeister/Sqli.exe"

# instrumentするsydserver.exeの場所を特定させる
cov_sydserver="$PROGRAMFILES/Rational/PurifyPlus/cache/SydServer\$Coverage_C_Program Files_ricoh_TRMeister.exe"
cov_sqli="$PROGRAMFILES/Rational/PurifyPlus/cache/Sqli\$Coverage_C_Program Files_ricoh_TRMeister.exe"


if [ ! -e "$cov_sydserver" ] ; then
  "$coverage" /run=no "$sydserver"
fi

if [ ! -e "$cov_sqli" ] ; then
  "$coverage" /run=no "$sqli"
fi

exeprefix=$coverage


# JDBCテスト内でパスを変えているかも知れないので一応バックアップ
pathbak=$PATH

# antのパスを加える
PATH=$ANT_HOME/bin:$PATH

# SydServerの開始
"$exeprefix" /savedata=$cvr_dst/JDBCTest_SydServer.cfy "$cov_sydserver" /Local &

# SydServerのサービスを起動
net start sydserver

# 起動を待つ
sleep 10

# JDBCから使われるカバレージ計測開始
pushd ${sydtop}/Java/JDBC

  bash ant test

popd

# SydServerのサービスを停止
net stop sydserver

# SydServerの停止
"$exeprefix" /savedata=$cvr_dst/JDBCTest_Sqli.cfy "$cov_sqli" /remote localhost $port /shutdown

# パスを元に戻す
PATH=$pathbak

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
