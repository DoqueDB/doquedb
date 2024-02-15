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
#定数の定義
############
. env.sh

#
# Exception::FakeErrorのカバレッジを図る
# `='も`=='も可能
bash setreg.sh "Exception_FakeError" "Os::File::write_DiskFull count=10, Os::File::write_DiskFull count==20"
bash SydTest.sh -s 2890 -lnc single
#rm -f coverage/normal/s_2890-2890.cfy.gz

bash setreg.sh "Exception_FakeError" "Os::File::write_DiskFull count = 30,	Os::File::write_DiskFull count==40"
bash SydTest.sh -s 2890 -lnc single
#rm -f coverage/normal/s_2890-2890.cfy.gz

bash setreg.sh "Exception_FakeError" "Os::File::write_DiskFull count=>50 & Os::File::write_DiskFull count<=60"
bash SydTest.sh -s 2890 -lnc single
#rm -f coverage/normal/s_2890-2890.cfy.gz

bash setreg.sh "Exception_FakeError" "Os::File::write_DiskFull count=>50 , Os::File::write_DiskFull count<=60"
bash SydTest.sh -s 2890 -lnc single
#rm -f coverage/normal/s_2890-2890.cfy.gz

bash setreg.sh "Exception_FakeError" "Os::File::write_DiskFull count>70	&	Os::File::write_DiskFull count<80"
bash SydTest.sh -s 2890 -lnc single
#rm -f coverage/normal/s_2890-2890.cfy.gz

# 文法的に正しくないレジストリ値
bash setreg.sh "Exception_FakeError" "Os::File::write_DiskFull count><kaunto, Os::File::write_DiskFull count=="
bash SydTest.sh -s 0000 -lnc single
bash setreg.sh "Exception_FakeError" "Os::File::random_key count=33 & Os::File::write_DiskFull kaunto=0"
bash SydTest.sh -s 0000 -lnc single

# あとしまつ
bash delreg.sh "Exception_FakeError"

# 必要なのか今の所不明
#pass () {
#fakeerrorのテスト(1)
#bash FakeErrorTest.sh -c -e diskfull 2890   10  10  500
#bash FakeErrorTest.sh -c -e diskfull 4200 1000 200 6000
#bash FakeErrorTest.sh -c -e diskfull 1650  500  10 1100
##fakeerrorのテスト(2)
#bash FakeErrorTest.sh -c -e diskfull 2930  500  25 1250
#bash FakeErrorTest.sh -c -e diskfull 0200  500  50 3000
#bash FakeErrorTest.sh -c -e mapfull  5500 1000 200 3400
#bash FakeErrorTest.sh -c -e diskfull 2720 1000 100 4000
#}

#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
