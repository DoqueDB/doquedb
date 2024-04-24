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
test_abstract.py -- src.doquedb.common.abstract モジュールのテスト
"""
import pytest

from src.doquedb.exception.exceptions import UnexpectedError
from src.doquedb.common.abstracts import ArrayData
from src.doquedb.common.constants import DataType
from src.doquedb.common.scalardata import Integer64Data


class TestArrayData:
    def setup_method(self, method):
        # 前処理
        self.test_array_data = ArrayData(DataType.INTEGER64.value)

    def teardown_method(self, method):
        # 後処理
        self.test_array_data.clear()

    def test_add(self):
        # _addの正常系テスト
        self.test_array_data._add(Integer64Data(1))
        self.test_array_data._add(Integer64Data(2))
        assert self.test_array_data[0] == Integer64Data(1)
        assert self.test_array_data[1] == Integer64Data(2)

    def test_len(self):
        # __len__の正常系テスト
        assert len(self.test_array_data) == 0
        self.test_array_data._add(Integer64Data(1))
        assert len(self.test_array_data) == 1
        self.test_array_data._add(Integer64Data(2))
        assert len(self.test_array_data) == 2

    def test_eq(self):
        # __eq__の正常系テスト
        # 値がない場合
        test_equal_data = ArrayData(DataType.INTEGER64.value)
        test_unequal_data = ArrayData(DataType.INTEGER.value)
        assert self.test_array_data == test_equal_data
        assert self.test_array_data != test_unequal_data
        # 値がある場合
        self.test_array_data._add(Integer64Data(1))
        test_equal_data._add(Integer64Data(1))
        test_unequal_data = ArrayData(DataType.INTEGER64.value)
        assert self.test_array_data == test_equal_data
        assert self.test_array_data != test_unequal_data

    def test_eq_diff_value(self):
        # __eq__の正常系テスト、同じデータ型で違う値を保持している場合のテスト
        test_diff_array_value = ArrayData(DataType.INTEGER64.value)
        test_diff_array_value._add(Integer64Data(2))
        self.test_array_data._add(Integer64Data(1))
        assert self.test_array_data != test_diff_array_value

    def test_str(self):
        # __str__の正常系テスト
        # 要素がない場合
        assert str(self.test_array_data) == '[]'

        # 要素が１個の場合
        self.test_array_data._add(Integer64Data(1))
        assert str(self.test_array_data) == '[1]'

        # 要素が２個の場合
        self.test_array_data._add(Integer64Data(2))
        assert str(self.test_array_data) == '[1, 2]'

    def test_getitem(self):
        # 要素を追加する前に値を取得する場合
        with pytest.raises(IndexError):
            self.test_array_data[0]

        # __getitem__の正常系テスト
        self.test_array_data._add(Integer64Data(1))
        assert self.test_array_data[0] == Integer64Data(1)
        # __getitem__の異常系テスト
        with pytest.raises(IndexError):
            self.test_array_data[1]

    def test_setitem(self):
        # __setitem__の正常系テスト
        self.test_array_data._add(Integer64Data(1))
        self.test_array_data[0] = Integer64Data(2)
        assert self.test_array_data[0] == Integer64Data(2)

    def test_setitem_index_over(self):
        # indexが配列要素数よりも大きい場合のテスト
        self.test_array_data[1] = Integer64Data(2)
        # indexが配列要素数よりも大きい場合配列は拡張される
        assert self.test_array_data[1] == Integer64Data(2)
        assert self.test_array_data[0] is None
        with pytest.raises(IndexError):
            self.test_array_data[2]
        # TODO setitemはNoneを許容するかどうか。
        # 許容しない場合は例外を出す使用に変更

    def test_setitem_diff_value(self):
        # 異常系のテスト
        self.test_array_data._add(Integer64Data(1))
        # Data型インスタンス以外をセットする場合
        with pytest.raises(UnexpectedError):
            self.test_array_data[0] = 1

    def test_clear(self):
        # clearの正常系テスト
        # 要素がない段階でclearが通るかのテスト
        self.test_array_data.clear()
        assert len(self.test_array_data) == 0

        # 要素を追加した後でclearが機能するかのテスト
        self.test_array_data._add(Integer64Data(1))
        self.test_array_data.clear()
        assert len(self.test_array_data) == 0
        with pytest.raises(IndexError):
            self.test_array_data[0]
