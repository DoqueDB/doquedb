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
test_arraydata.py -- src.doquedb.common.arraydata モジュールのテスト
"""
from unittest.mock import MagicMock, patch, call

from src.doquedb.common.arraydata import (
    DataArrayData, IntegerArrayData, ResultSetMetaData, StringArrayData)
from src.doquedb.common.iostream import InputStream, OutputStream
from src.doquedb.common.scalardata import (
    Integer64Data, IntegerData, StringData)
from src.doquedb.common.constants import ClassID, DataType, SQLType
from src.doquedb.common.serialdata import ColumnMetaData
from src.doquedb.common.unicodestr import UnicodeString


class TestDataArrayData:
    def setup_method(self, method):
        # 前処理
        self.test_data_array_data = DataArrayData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_data_array_data.type == DataType.ARRAY.value
        assert self.test_data_array_data.element_type == DataType.DATA.value
        # 引数がある場合
        self.test_data_array_data.add_element(Integer64Data())
        test_data_array_data_arg = DataArrayData(self.test_data_array_data)
        assert test_data_array_data_arg._array == [Integer64Data()]

    def test_class_id(self):
        # 正常系のテスト
        self.test_data_array_data.class_id == ClassID.DATA_ARRAY_DATA.value

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_int = MagicMock(return_value=2)
        self.input_.read_object = MagicMock(
            side_effect=[Integer64Data(1), StringData('2')])
        self.test_data_array_data.read_object(self.input_)
        assert self.input_.read_object.call_count == 2
        assert self.test_data_array_data[0] == Integer64Data(1)
        assert self.test_data_array_data[1] == StringData('2')

    def test_write_object(self):
        # 正常系のテスト
        self.test_data_array_data.add_element(Integer64Data())
        self.test_data_array_data.add_element(StringData())
        self.output_.write_int = MagicMock()
        self.output_.write_object = MagicMock()
        self.test_data_array_data.write_object(self.output_)
        # 正しい回数``write_int``が呼ばれたか確認
        assert self.output_.write_int.call_count == 1
        # 正しい引数で``write_int``が呼ばれたか確認
        self.output_.write_int.assert_called_once_with(2)
        # 正しい回数``write_object``が呼ばれたか確認
        assert self.output_.write_object.call_count == 2
        # 正しい引数で``write_object``が呼ばれたか確認
        self.output_.write_object.assert_has_calls(
            [call(Integer64Data()), call(StringData())])


class TestIntegerArrayData:
    def setup_method(self, method):
        # 前処理
        self.test_int_array_data = IntegerArrayData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_int_array_data.type == DataType.ARRAY.value
        assert self.test_int_array_data.element_type == DataType.INTEGER.value
        # 引数がある場合
        self.test_int_array_data.add_element(IntegerData(1))
        test_int_array_data_arg = IntegerArrayData(self.test_int_array_data)
        assert test_int_array_data_arg._array == [IntegerData(1)]

    def test_class_id(self):
        # 正常系のテスト
        self.test_int_array_data.class_id == ClassID.INTEGER_ARRAY_DATA.value

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_int = MagicMock(side_effect=[2, 0, 1])
        self.test_int_array_data.read_object(self.input_)
        assert self.test_int_array_data[0] == IntegerData(0)
        assert self.test_int_array_data[1] == IntegerData(1)

    def test_write_object(self):
        # 正常系のテスト
        self.test_int_array_data.add_element(IntegerData(0))
        self.test_int_array_data.add_element(IntegerData(1))
        self.output_.write_int = MagicMock()
        self.test_int_array_data.write_object(self.output_)
        # 正しい回数``write_int``が呼ばれたか確認
        assert self.output_.write_int.call_count == 3
        # 正しい引数で``write_int``が呼ばれたか確認
        self.output_.write_int.assert_has_calls(
            [call(2), call(0), call(1)])


class TestStringArrayData:
    def setup_method(self, method):
        # 前処理
        self.test_str_array_data = StringArrayData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_str_array_data.type == DataType.ARRAY.value
        assert self.test_str_array_data.element_type == DataType.STRING.value
        # 引数がある場合
        self.test_str_array_data.add_element(StringData('a'))
        test_str_array_data_arg = StringArrayData(self.test_str_array_data)
        assert test_str_array_data_arg._array == [StringData('a')]

    def test_class_id(self):
        # 正常系のテスト
        self.test_str_array_data.class_id == ClassID.STRING_ARRAY_DATA.value

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_int = MagicMock(return_value=2)
        with patch('src.doquedb.common.unicodestr.UnicodeString.read_object',
                   side_effect=[StringData('test1'), StringData('test2')]):
            self.test_str_array_data.read_object(self.input_)
            assert self.test_str_array_data[0] == StringData('test1')
            assert self.test_str_array_data[1] == StringData('test2')

    def test_write_object(self):
        # 正常系のテスト
        self.test_str_array_data.add_element(StringData('a'))
        self.test_str_array_data.add_element(StringData('b'))
        self.output_.write_int = MagicMock()
        with patch('src.doquedb.common.unicodestr.UnicodeString.write_object'):
            self.test_str_array_data.write_object(self.output_)
            # 正しい回数``write_int``が呼ばれたか確認
            assert self.output_.write_int.call_count == 1
            # 正しい引数で``write_int``が呼ばれたか確認
            self.output_.write_int.assert_called_once_with(2)
            # 正しい回数``write_object``が呼ばれたか確認
            assert UnicodeString.write_object.call_count == 2
            # 正しい引数で``write_object``が呼ばれたか確認
            UnicodeString.write_object.assert_has_calls(
                [call(self.output_, 'a'), call(self.output_, 'b')])


class TestResultSetMetaData:
    def setup_method(self, method):
        # 前処理
        self.test_resultset = ResultSetMetaData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_resultset.type == DataType.ARRAY.value
        assert self.test_resultset.element_type ==\
            DataType.COLUMN_META_DATA.value
        # 引数がある場合
        self.test_resultset.add_element(ColumnMetaData())
        test_resultset_arg = ResultSetMetaData(self.test_resultset)
        assert isinstance(test_resultset_arg[0], ColumnMetaData)

    def test_class_id(self):
        # 正常系のテスト
        self.test_resultset.class_id == ClassID.RESULTSET_META_DATA.value

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_int = MagicMock(return_value=2)
        # read_objectの返却値を作成したサンプルに変える
        with patch('src.doquedb.common.serialdata.ColumnMetaData.read_object'):
            self.test_resultset.read_object(self.input_)
            assert ColumnMetaData.read_object.call_count == 2

    def test_write_object(self):
        # 正常系のテスト
        self.test_resultset.add_element(ColumnMetaData())
        self.test_resultset.add_element(ColumnMetaData())
        self.output_.write_int = MagicMock()
        with patch('src.doquedb.common.serialdata.ColumnMetaData.write_object'):
            self.test_resultset.write_object(self.output_)
            # 正しい回数``write_int``が呼ばれたか確認
            assert self.output_.write_int.call_count == 1
            # 正しい引数で``write_int``が呼ばれたか確認
            self.output_.write_int.assert_called_once_with(2)
            # 正しい回数``write_object``が呼ばれたか確認
            assert ColumnMetaData.write_object.call_count == 2
            # 正しい引数で``write_object``が呼ばれたか確認
            ColumnMetaData.write_object.assert_has_calls(
                [call(self.output_), call(self.output_)])

    def test_create_tuple_data(self):
        # 正常系のテスト
        test_meta_integer = ColumnMetaData()
        test_meta_string = ColumnMetaData()
        test_meta_integer.type = SQLType.INTEGER.value
        test_meta_string.type = SQLType.CHARACTER.value
        self.test_resultset.add_element(test_meta_integer)
        self.test_resultset.add_element(test_meta_string)

        test_tuple = self.test_resultset.create_tuple_data()
        assert isinstance(test_tuple, DataArrayData)
        assert test_tuple[0] == IntegerData()
        assert test_tuple[1] == StringData()
