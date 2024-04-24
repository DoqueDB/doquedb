# Copyright (c) 2024 Ricoh Company, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
constants.py -- IDやタイプ値など定数定義のモジュール
"""

from enum import IntEnum, unique


@unique
class ConnectionType(IntEnum):
    """コネクションのタイプ
    """
    # ローカルコネクション(メモリー)
    LOCAL = 1
    # リモートコネクション(ソケット)
    REMOTE = 2


@unique
class ConnectionSlaveID(IntEnum):
    """スレーブIDを管理するクラス
    """
    # スレーブIDの最小値
    MINIMUM = 0
    # スレーブIDの最大値
    MAXIMUM = 0x7fffffff
    # 任意のスレーブIDをあらわす数
    ANY = -0x80000000  # = 0x80000000
    # 未定義をあらわす数
    UNDEFINED = -0x00000001  # = 0xffffffff

    @classmethod
    def is_normal(cls, slave_id: int) -> bool:
        return slave_id >= int(cls.MINIMUM) and slave_id < int(cls.MAXIMUM)
