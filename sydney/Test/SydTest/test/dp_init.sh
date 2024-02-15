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

# DevPartnerを１から初める場合にのみ使用
# BoundsChecker, Performance Analysis

############
# 変数の設定
############

# isWindows, mytop, hdrive, xdrive, testtype を読み込む
. env.sh

######
# 本体
######
# 前回テストした時のDPbcl又はdpprfファイルが残っているかもしれないのでフォルダを削除して改めて作成する
pushd $dp_dst >/dev/null 2>&1
for dir1 in single multi recovery single_dist multi_dist recovery_dist single_eu single_ja single single_utf8 single_eu_utf8 single_ja_utf8 single_zh_utf8 
do
    for dir2 in normal except 
    do
        if `find ./../$dir1/$dir2 >/dev/null 2>&1` ; then
            rm -Rf $dir1/$dir2
            mkdir -p $dir1/$dir2
        fi
    done
done
popd >/dev/null 2>&1

exit 0

#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
