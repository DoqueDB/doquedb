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

# カバレージ計測による結果ファイル(.cfy)のマージ用スクリプト
#   ただし、cfyファイル数が多い(約15以上)と仮想メモリ不足によりスラッシングが発生し失敗する

############
# 変数の設定
############

# isWindows, mytop, hdrive, xdrive, testtype を読み込む
. env.sh

############
# 環境の確認
############

pushd ./coverage >/dev/null 2>&1

  for arg in *.cfy ; do
    list="$list$arg;"
  done

  echo $list

  # マージ開始
  coverage /MergeFileList=$list /SaveMergeData=$cvr_dst/all_merged_result.cfy

popd >/dev/null 2>&1

exit 0

#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
