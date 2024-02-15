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

#
userpasswd_op="/user root /password doqadmin"
dm="C:/Program Files/Ricoh/TRMeister/db"

# sydserver.exe, sqli.exeの場所を特定させる
sydserver="$PROGRAMFILES/ricoh/TRMeister/SydServer.exe"
sqli="$PROGRAMFILES/ricoh/TRMeister/Sqli.exe"

# instrumentするsydserver.exeの場所を特定させる
cov_sydserver="$PROGRAMFILES/Rational/PurifyPlus/cache/SydServer\$Coverage_C_Program Files_ricoh_TRMeister.exe"
cov_sqli="$PROGRAMFILES/Rational/PurifyPlus/cache/Sqli\$Coverage_C_Program Files_ricoh_TRMeister.exe"

if [ ! -e "$cov_sydtest" ] ; then
  "$coverage" /run=no "$sydtest"
fi

if [ ! -e "$cov_sydserver" ] ; then
  "$coverage" /run=no "$sydserver"
fi

if [ ! -e "$cov_sqli" ] ; then
  "$coverage" /run=no "$sqli"
fi

exeprefix=$coverage

# SydServerの開始
"$exeprefix" /savedata=${testtop}/coverage/sydserver.cfy "$sydserver" /Local &

# 起動を待つ
sleep 30

# 何か適当に一つ実行してみる
pushd ${sydtop}/test/sydtest/test/single/normal

  "$exeprefix" /savedata=${testtop}/coverage/sydtest_sydserver.cfy "$sydtest" $userpasswd_op /remote localhost $port /c /p /b 4350.txt

popd

# SydServerの停止
"$exeprefix" /savedata=${testtop}/coverage/sqli_sydserver.cfy "$sqli" $userpasswd_op /remote localhost $port /shutdown


# DB領域のrestore
#chmod -Rf u+w "${dm}/data"{,2}
#chmod -Rf u+w "${dm}/system"{,2}
#chmod -Rf u+w "${dm}/"*area*
#chmod -Rf u+w "${dm}/"*alter*
rm -rf "${dm}/data"{,2}
rm -rf "${dm}/system"{,2}
rm -rf "${dm}/"*area* "${dm}/"*alter*

cp -Rf "${save}/data" "${dm}"
cp -Rf "${save}/system" "${dm}"

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
