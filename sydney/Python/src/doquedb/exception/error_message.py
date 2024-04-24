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
error_message.py -- エラーメッセージのモジュール
"""

from typing import Dict, List
import locale


class MessageEntry:
    """メッセージフォーマットのエントリクラス

    Args:
        code_(int): エラーコード
        format_(str): メッセージフォーマット
    """

    def __init__(self, code_: int, format_: str):
        # エラー番号
        self.code = code_
        # エラーメッセージフォーマット
        self.format = format_


class ErrorMessage:
    """エラーメッセージ作成クラス
    """
    # 循環参照を回避するためにクラス内でインポートしている
    from ..exception.message_format_eg import MessageFormatEnglish
    from ..exception.message_format_jp import MessageFormatJapanese

    # エラーメッセージのフォーマットのマップ
    __format_map: Dict[int, str] = {}

    # 初期化
    if len(__format_map) == 0:
        # デフォルトは英語
        table: List[MessageEntry] = MessageFormatEnglish.table

        # ロケールを調べる
        if locale.getdefaultlocale()[0] == 'ja_JP':
            # 日本語に置き換える
            table = MessageFormatJapanese.table

        # マップに格納する
        for element in table:
            __format_map[element.code] = element.format

    @staticmethod
    def make_error_message(errno_: int, arguments_: list) -> str:
        """エラーメッセージを作成する

        Args:
            errno_ (int): エラー番号
            arguments_ (list): その他引数

        Returns:
            str: エラーメッセージ
        """
        format = ErrorMessage.__format_map[errno_]
        if format is None:
            return ''

        buf = ''
        format_len = len(format)
        i = 0
        while i < format_len:
            if format[i] == '%':
                # %の次は数字
                i += 1
                num = ''
                while i < format_len and\
                        format[i] >= '0' and\
                        format[i] <= '9':
                    num += format[i]
                    i += 1

                buf += str(arguments_[int(num)-1])
            else:
                buf += format[i]
                i += 1

        return buf
