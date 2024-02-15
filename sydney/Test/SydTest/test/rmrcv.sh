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
# 削除対象:
# ・一時表関係
# ・トランザクション種別だけ違うもの(1つだけ残す)
pushd recovery
    pushd normal
        rm -f 10??.txt 1280.txt
        rm -f 309?.txt 3[12]??.txt 33[0-2]?.txt
        rm -f 5060.txt
        rm -f 5[5-9]?[0246].txt
        rm -f 6[0-3]?[0246].txt 6[56]??.txt
    cd ../except
        rm -f 120?.txt
        rm -f 3099.txt 3[12]??.txt 3301.txt
    popd
popd

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
