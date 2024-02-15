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

# 環境変数設定
testtype=proj
. env.sh

port=23456
objdir=$setup/sydney
sydserverexe=${objdir}/sydserver.exe
sydtestexe=${objdir}/sydtest.exe
sqliexe=${objdir}/sqli.exe

# SydServerの開始
$sydserverexe /Local &
# 起動を待つ
sleep 7
# 何か適当に一つ実行してみる
pushd ${testtop}/recovery/normal
SydTest.exe /remote localhost $port /c /p /b 4816.txt
SydTest.exe /remote localhost $port /c /p /b 4816r.txt
popd

# SydServerの停止
$sqliexe /remote localhost $port /shutdown

# SydServerを停止しないとrestoreできない
bash restore_sydtest.sh

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
