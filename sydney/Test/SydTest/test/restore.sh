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

# 2860等で使用
# オリジナルはtools/setup/package以下 (040713)

# テストスクリプトがあるディレクトリから使うので
# ./conf.shから変更した
. ../../conf.sh

#if [ `whoami` != root ]; then
#	installpath=/proj/sydney/work/`whoami`$installpath
#	databasepath=/proj/sydney/work/`whoami`$databasepath
#fi

dumppath=$1
if [ -z "$dumppath" ]; then dumppath=save; fi

# data2, system2, などは消去しないために
# rmとmkdirの対象ディレクトリを指定した。
rm -rf $databasepath/data
rm -rf $databasepath/system
mkdir -p -m 0755 $databasepath/data
mkdir -p -m 0755 $databasepath/system
cp -r $installpath/$dumppath/. $databasepath
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
