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
constants.py -- 定数を管理するモジュール.
"""
from enum import IntEnum


class ProtocolVersion(IntEnum):
    """プロトコルバージョン管理用の列挙型クラス
    """
    # プロトコルバージョン(v14.0互換モード)
    PROTOCOL_VERSION1 = 0
    # プロトコルバージョン(v15.0初期バージョン互換モード)
    PROTOCOL_VERSION2 = 1
    # プロトコルバージョン(v15.0互換モード)
    PROTOCOL_VERSION3 = 2
    # プロトコルバージョン(HasMoreData対応)
    PROTOCOL_VERSION4 = 3
    # プロトコルバージョン(ユーザー管理対応)
    PROTOCOL_VERSION5 = 4
    CURRENT_PROTOCOL_VERSION = PROTOCOL_VERSION5


class StatusSet(IntEnum):
    """実行ステータス管理用の列挙型クラス
    """
    # 不明なステータス
    UNDEFINED = 0
    # データである
    DATA = 1
    # データ終了である
    END_OF_DATA = 2
    # 正常終了
    SUCCESS = 3
    # キャンセルされた
    CANCELED = 4
    # エラーが発生した
    ERROR = 5
    # 結果集合メタデータ
    META_DATA = 6
    # 続きのデータがある
    HAS_MORE_DATA = 7
