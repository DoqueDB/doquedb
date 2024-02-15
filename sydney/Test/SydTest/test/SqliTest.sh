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


# sqli.exeの場所を特定させる
sqli="$PROGRAMFILES/ricoh/TRMeister/Sqli.exe"

#
userpasswd_op="/user root /password doqadmin"
dm="C:/Program Files/Ricoh/TRMeister/db"

# sqliを事前にinstrumentする
cov_sqli="$PROGRAMFILES/Rational/PurifyPlus/cache/Sqli\$Coverage_C_Program Files_ricoh_TRMeister.exe"


if [ ! -e "$cov_sqli" ] ; then
  "$coverage" /run=no "$sqli"
fi

exeprefix=$coverage


# c:\sydney\内にdatabaseが存在する場合は削除
#chmod -Rf u+w ${dm}/data
#chmod -Rf u+w ${dm}/system
rm -rf "${dm}/data"
rm -rf "${dm}/system"

# Sqli /install のテスト 
echo "start testing SqliTest"
"$exeprefix" /savedata=${testtop}/coverage/sqli_test.cfy "$sqli" $userpasswd_op /install
echo "end testing SqliTest"

#削除される前のdatabaseに戻す
#chmod -Rf u+w ${dm}/data
#chmod -Rf u+w ${dm}/system
rm -rf "${dm}/data"
rm -rf "${dm}/system"
cp -Rf "${save}/data" "${dm}"
cp -Rf "${save}/system" "${dm}"

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
