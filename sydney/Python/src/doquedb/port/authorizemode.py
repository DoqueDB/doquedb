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
authorizemode.py -- 認証方式の実装モジュール
"""

from enum import IntEnum


class AuthorizeMode(IntEnum):
    """認証方式
    """
    # なし
    NONE = 0
    # パスワード認証
    PASSWORD = 0x01000000

    # Mask
    # マスターID取得用
    MaskMasterID = 0x0000FFFF
    # 認証方式取得用
    MaskMode = 0x0F000000
