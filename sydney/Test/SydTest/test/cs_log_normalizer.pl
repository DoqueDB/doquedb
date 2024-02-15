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
# CriticalSection ログ専用の正規化処理
# 出力される値は正解を確かめる術がないので、値はすべて正規化し、出力すべき項目が出力されていることのみを確認する

while(<>) {
    s|\d\d\d\d/\d\d/\d\d-\d\d:\d\d:\d\d \+\d+\(ms\)|yyyy/mm/dd-hh:MM:ss +SSS(ms)|g;
    s|TRMeister \d+\.\d+\.\d+\.\d+|TRMeister {version}|g;

    s|\(empty\)|...|g;
    s|0x[\da-f]+\s+[1-9]\d*|...|g;                              # スレッド番号は1以上
    BEGIN{undef $/;} s|---.(\.\.\..)+|---\n(nomalized)\n|sg;    # 複数行の出力をすべて (normalized) にまとめる
 
    s|locked: \d+|locked: x|g;
    s|total: \d+|total: y|g;

    print;
}

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
