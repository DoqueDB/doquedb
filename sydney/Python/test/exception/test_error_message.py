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
test_error_message.py -- src.doquedb.exception.error_message モジュールのテスト
"""
import locale

from src.doquedb.exception.error_message import ErrorMessage
from src.doquedb.exception.errorcode import ErrorCode


class TestErrorMessage:
    def test_make_error_message(self):
        # 正常系のテスト
        # 引数がない場合
        message = ErrorMessage.make_error_message(
            ErrorCode.ConnectionNotExist.value, [])
        # ロケールを調べる
        if locale.getdefaultlocale()[0] == 'ja_JP':
            # 日本語のエラーメッセージを検証する
            assert message == '指定されたセッションが存在しません。'
        else:
            # 日本以外は英語
            assert message == \
                'Connection exception - connection does not exist.'

        # 引数がある場合（１つ）
        message = ErrorMessage.make_error_message(
            ErrorCode.ClientNotExist.value, ['1'])
        # ロケールを調べる
        if locale.getdefaultlocale()[0] == 'ja_JP':
            # 日本語のエラーメッセージを検証する
            assert message == '指定されたクライアント(ID=1)が存在しません。'
        else:
            # 日本以外は英語
            assert message == \
                'Connection exception - client(ID=1) does not exist.'

        # 引数がある場合（２つ）
        message = ErrorMessage.make_error_message(
            ErrorCode.DynamicParameterNotMatch.value, ['1', '2'])
        # ロケールを調べる
        if locale.getdefaultlocale()[0] == 'ja_JP':
            # 日本語のエラーメッセージを検証する
            assert message == 'パラメーター値の数(1)が必要な数(2)と一致しません。'
        else:
            # 日本以外は英語
            assert message == \
                'The number of parameter values(1) does not match parameters(2).'
