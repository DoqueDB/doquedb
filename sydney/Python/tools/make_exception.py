#!/bin/python3
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
make_exception.py -- 例外ファイルを自動生成するモジュール.

Notes:
    このプログラムは必ずビルド前に実行する。
    実行ディレクトリ:
        py-doquedb
    実行方法：
        以下のコマンドを実行する
        > python tools/make_exception.py
    前提:
        以下のファイルを py-doquedb/tools 下に配置する
        ErrorDefinition.xml: DoqueDBのエラーが定義されたxmlファイル
        ErrorDefinition.xmlの文字コードはutf-8である(S-JISではない)
    出力:
        以下のファイルが py-doquedb/src/doquedb/exception 配下に生成される
        database_exceptions.py: エラークラス定義モジュール
        errorcode.py: エラーコード定義モジュール
        raise_error.py: 例外クラス生成モジュール
        message_format_eg.py: 英語のエラーメッセージ定義モジュール
        message_format_jp.py: 日本語のエラーメッセージ定義モジュール
"""
import re
import xml.etree.ElementTree as ET

from typing import TextIO


def reshape_message(message: str) -> str:
    """与えられた文章から改行、先頭と末尾の空白を削除して返す

    Args:
        message (str): メッセージ

    Returns:
        message_ (str): 整形したメッセージ
    """
    message_ = message
    # 先頭と末尾の空白を削除
    # 文中の改行や２個以上の空白、タブを削除
    message_ = re.sub(r'\s*\r?\n\s*', '', message_)

    return message_


def write_errorcode_file_from_xml(file: TextIO,
                                  description_jp: str,
                                  code_name: str,
                                  code: str):
    """errocode.pyにxmlからの変数を書き込むための関数

    Args:
        file (TextIOWrapper): 書き込むファイルオブジェクト
        description_jp (str): エラークラスの日本語の説明
        code_name (str): エラークラス名
        code (str): エラーコード番号
    """
    # コメントの書き込み
    file.write('\n    # '+description_jp)
    # エラー名とコード番号の書き込み
    file.write('\n    ' + code_name + ' = ' + code + '\n')


def write_message_file_from_xml(file: TextIO,
                                code_name: str,
                                message: str):
    """message_format_eg.py, message_format_jp.pyにxmlからの変数を書き込むための関数

    Args:
        file (TextIO): 書き込むファイルオブジェクト
        code_name (str): エラークラス名
        message (str): [description]
    """
    # メッセージエントリーのインスタンス化の書き込み
    file.write('        MessageEntry(ErrorCode.' + code_name + '.value,\n')
    file.write('                     "' + message + '"),\n')


def write_raise_error_file_from_xml(file: TextIO,
                                    code: str,
                                    name: str):
    """raise_error.pyにxmlからの変数を書き込むための関数

    Args:
        file (TextIO): 書き込むファイルオブジェクト
        code (str): エラーコード番号
        name (str): エラークラス名
    """
    # エラーコード辞書の書き込み
    file.write('        ' + code + ': db_ex.' + name + ',\n')


def write_error_file_from_xml(file: TextIO,
                              child: ET.Element,
                              name: str,
                              description_jp: str,
                              state_code: str
                              ):
    """database_exceptions.pyにxmlからの変数を書き込むための関数

    Args:
        file (TextIO): 書き込むファイルオブジェクト
        child (ET.Element): XMLから取得したElement
        name (str): エラークラス名
        description_jp (str): エラークラスの日本語の説明
        state_code (str): ステータスコード
    """
    # クラスの宣言を書き込む
    file.write('\n\nclass ' + name + '(DatabaseError):\n')
    file.write('    """' + description_jp + '"""\n\n')

    # コンストラクタを書き込む
    file.write(
        '    def __init__(self, e_: Optional[ExceptionData] = None, *args):')
    # ExceptionDataが与えられた場合のコンストラクタの処理
    file.write('''
        # ExceptionDataが与えられた場合
        if e_:
            super().__init__(
                e_.error_message,
''')
    file.write('                "' + state_code + '",\n')
    file.write('                ErrorCode.' + name + '.value)')
    # ExceptionDataが与えられなかった場合のコンストラクタの処理
    file.write('''
        # ExceptionDataが与えられなかった場合
        else:
            super().__init__(
                self.make_error_message(''')
    arguments = child.findall('MessageArgument')
    if arguments:
        i = 0
        for arg in arguments:
            file.write('args[' + str(i) + ']')
            i += 1
            if i != len(arguments):
                file.write(', ')
            else:
                file.write('),\n')
    else:
        file.write('),\n')
    file.write('                "' + state_code +
               '",\n                ErrorCode.' + name + '.value)\n')

    # make_error_messageメソッドの作成
    file.write('''
    def make_error_message(self, *args) -> str:
        """エラーメッセージを作成する

        Returns:
            str: 作成したエラーメッセージ
        """
        arguments = []

        for arg in args:
            arguments.append(arg)

''')
    file.write('        return ErrorMessage.make_error_message(\n')
    file.write('            ErrorCode.' + name + '.value,\n')
    file.write('            arguments)\n')


"""ファイル書き込みのための処理"""
# 自動生成するファイルのパスを設定
exception_path = 'src/doquedb/exception/database_exceptions.py'
errorcode_path = 'src/doquedb/exception/errorcode.py'
message_format_eg_path = 'src/doquedb/exception/message_format_eg.py'
message_format_jp_path = 'src/doquedb/exception/message_format_jp.py'
raise_error_path = 'src/doquedb/exception/raise_error.py'

# xml データの読込み
tree = ET.parse('tools/ErrorDefinition.xml')
# 一番上の階層の要素を取り出す
root = tree.getroot()

"""ファイルの作成"""
# エラーコードファイルの作成
errorcode_file = open(errorcode_path, mode='w', encoding='utf-8')
errorcode_file.write('''"""errorcode.py -- 自動生成されたエラーコードクラス
"""
from enum import IntEnum, unique


@unique
class ErrorCode(IntEnum):
    """DoqueDBのエラーコードクラス"""''')

# メッセージフォーマットファイルの作成
message_eg_file = open(message_format_eg_path, mode='w', encoding='utf-8')
message_eg_file.write('''"""errorcode.py -- 自動生成されたメッセージフォーマットのクラス\n"""
from typing import List

from ..exception.error_message import MessageEntry
from ..exception.errorcode import ErrorCode


class MessageFormatEnglish:
    """エラーメッセージフォーマット
    """
    table: List[MessageEntry] = [
''')
message_jp_file = open(message_format_jp_path, mode='w', encoding='utf-8')
message_jp_file.write('''"""errorcode.py -- 自動生成されたメッセージフォーマットのクラス
"""
from typing import List

from .error_message import MessageEntry
from .errorcode import ErrorCode


class MessageFormatJapanese:
    """エラーメッセージフォーマット
    """
    table: List[MessageEntry] = [
''')

# 例外インスタンスのファイルの作成
raise_error_file = open(raise_error_path, mode='w', encoding='utf-8')
raise_error_file.write('''"""
raise_error.py -- 自動生成された例外をスローするためのモジュール
"""
import src.doquedb.exception.database_exceptions as db_ex
from ..common.serialdata import ExceptionData
from .exceptions import UnexpectedError


class RaiseClassInstance:
    """例外をスローする
    """
    # 例外のマップ
    error_map = {
''')

# 例外ファイルの作成
error_file = open(exception_path, mode='w', encoding='utf-8')
error_file.write('''"""database_execption.py -- 自動生成された例外クラスのモジュール
"""
from typing import Optional

from ..common.serialdata import ExceptionData
from .exceptions import DatabaseError
from .errorcode import ErrorCode
from .error_message import ErrorMessage
''')


""" xml から各種変数を取得し、ファイルに書き込む
"""
for child in root:
    # 各種変数を取得する
    name = child.find('Name').text
    code = child.find('Number').text
    state_code = child.find('StateCode').text
    level = child.find('Level').text
    message = child.find('Message')
    message_eg = message.find('English').text
    message_jp = reshape_message(message.find('Japanese').text)
    description = child.find('Description')
    description_jp = reshape_message(description.find('Japanese').text)

    # errocode.pyの書き込み
    write_errorcode_file_from_xml(
        errorcode_file, description_jp, name, code)

    # message_format_eg.pyの書き込み
    write_message_file_from_xml(message_eg_file, name, message_eg)
    # message_format_jp.pyの書き込み
    write_message_file_from_xml(message_jp_file, name, message_jp)

    # raise_error.pyの書き込み
    write_raise_error_file_from_xml(raise_error_file, code, name)

    # database_exceptions.pyの書き込み
    write_error_file_from_xml(error_file, child, name,
                              description_jp, state_code)


# message_format_eg.py, message_format_jp.pyの書き込み
message_eg_file.write('    ]')
message_jp_file.write('    ]')

# raise_error.pyにraise_exceptionメソッドを書き込む
raise_error_file.write('''    }

    @staticmethod
    def raise_exception(e: ExceptionData) -> None:
        """例外をスローする.

        Raises:
            DatabaseError: DatabaseErrorを継承したエラーコードに該当するDoqueDBのエラークラス
            UnexpectedError: エラーコードのリストに該当するエラーが存在しなかった場合
        """
        num = e.errno

        try:
            raise RaiseClassInstance.error_map[num](e)
        except KeyError:
            raise UnexpectedError(f'invalid error code {num}')
''')


# ファイルをクローズする
error_file.close()
errorcode_file.close()
message_eg_file.close()
message_jp_file.close()
raise_error_file.close()
