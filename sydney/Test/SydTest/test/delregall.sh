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
#
#Sydneyのレジストリを丸ごと削除する。

cat >delreg.ini <<_EOF_
[Version]
Signature="\$Chicago\$"

[DefaultInstall]
DelReg=DelRegKey

[DelRegKey]
HKLM,SOFTWARE\\RICOH\\Sydney
_EOF_

$WINDIR/system32/rundll32.exe setupapi,InstallHinfSection DefaultInstall 132 `echo $PWD|perl -pe 's|^/cygdrive/(.)|\1:|; s|/|\\\\|g;'`\\delreg.ini
rm -f delreg.ini

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
