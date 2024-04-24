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
test_unicodestr.py -- src.doquedb.common.unicodestr モジュールのテスト
"""
from unittest.mock import MagicMock, call

from src.doquedb.common.iostream import InputStream, OutputStream
from src.doquedb.common.unicodestr import UnicodeString


class TestUnicodeString:
    def setup_method(self, method):
        # 前処理
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_read_object(self):
        # 正常系のテスト
        # 英語
        self.input_.read_int = MagicMock(return_value=3)
        self.input_.read_char = MagicMock(
            side_effect=['a', 'b', 'c'])
        test_unicode_str = UnicodeString.read_object(self.input_)
        assert test_unicode_str == 'abc'

    def test_read_object_jp(self):
        # 正常系のテスト
        # 日本語
        self.input_.read_int = MagicMock(return_value=1)
        self.input_.read_char = MagicMock(side_effect=['あ'])
        test_unicode_str = UnicodeString.read_object(self.input_)
        assert test_unicode_str == 'あ'

    def test_read_object_full_num(self):
        # 正常系のテスト
        # 全角数字
        self.input_.read_int = MagicMock(return_value=1)
        self.input_.read_char = MagicMock(side_effect=['１'])
        test_unicode_str = UnicodeString.read_object(self.input_)
        assert test_unicode_str == '１'

    def test_read_object_half_num(self):
        # 正常系のテスト
        # 半角数字
        self.input_.read_int = MagicMock(return_value=1)
        self.input_.read_char = MagicMock(side_effect=['1'])
        test_unicode_str = UnicodeString.read_object(self.input_)
        assert test_unicode_str == '1'

    def test_write_object(self):
        # 正常系のテスト
        # 英語
        self.output_.write_int = MagicMock()
        self.output_.write_char = MagicMock()
        UnicodeString.write_object(self.output_, 'test')
        self.output_.write_int.assert_called_once_with(4)
        assert self.output_.write_char.call_count == 4
        calls = [call(ord('t')), call(ord('e')),
                 call(ord('s')), call(ord('t'))]
        self.output_.write_char.assert_has_calls(calls)

    def test_write_object_jp(self):
        # 正常系のテスト
        # 日本語
        self.output_.write_int = MagicMock()
        self.output_.write_char = MagicMock()
        UnicodeString.write_object(self.output_, 'テスト')
        self.output_.write_int.assert_called_once_with(3)
        assert self.output_.write_char.call_count == 3
        calls = [call(ord('テ')), call(ord('ス')),
                 call(ord('ト'))]
        self.output_.write_char.assert_has_calls(calls)

    def test_write_object_full_num(self):
        # 正常系のテスト
        # 全角数字
        self.output_.write_int = MagicMock()
        self.output_.write_char = MagicMock()
        UnicodeString.write_object(self.output_, '１')
        self.output_.write_int.assert_called_once_with(1)
        assert self.output_.write_char.call_count == 1
        self.output_.write_char.assert_called_once_with(ord('１'))

    def test_write_object_half_num(self):
        # 正常系のテスト
        # 半角数字
        self.output_.write_int = MagicMock()
        self.output_.write_char = MagicMock()
        UnicodeString.write_object(self.output_, '1')
        self.output_.write_int.assert_called_once_with(1)
        assert self.output_.write_char.call_count == 1
        self.output_.write_char.assert_called_once_with(ord('1'))
