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
unicodestr.py -- :class: `UnicodeString`の実装モジュール
"""

from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from .iostream import InputStream, OutputStream


class UnicodeString:
    """ModUnicodeStringに対応するクラス
    """

    @staticmethod
    def read_object(input_: 'InputStream') -> str:
        """ストリームから読込む.

        Args:
            input_ (InputStream): 入力用のストリーム

        Returns:
            str: ストリームから読込んだstrクラス
        """
        len = input_.read_int()
        buffer = ''
        for i in range(len):
            buffer += input_.read_char()
        return buffer

    @staticmethod
    def write_object(output_: 'OutputStream', data: str) -> None:
        """ストリームに書き出す.

        Args:
            input_ (InputStream): 出力用のストリーム
        """
        len_ = len(data)
        output_.write_int(len_)
        for i in range(len_):
            output_.write_char(ord(data[i]))
