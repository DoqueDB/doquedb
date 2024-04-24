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
test_serialdata.py -- src.doquedb.common.serialdata モジュールのテスト
"""
import pytest
from unittest.mock import MagicMock, patch, call

from src.doquedb.common.serialdata import (
    ColumnMetaData, ErrorLevel, ExceptionData, Request, Status)
from src.doquedb.common.constants import SQLType, DataType, ClassID
from src.doquedb.common.scalardata import (IntegerData, Integer64Data,
                                           DoubleData, StringData,
                                           DecimalData, DateData,
                                           DateTimeData, LanguageData)
from src.doquedb.common.data import BinaryData, WordData
from src.doquedb.common.iostream import InputStream, OutputStream
from src.doquedb.exception.exceptions import OperationalError


class TestColumnMetaData:
    def setup_method(self, method):
        # 前処理
        self.test_metadata = ColumnMetaData()

    def test_init(self):
        # 正常系のテスト
        # 属性の初期値が正しくセットされているかチェック
        assert self.test_metadata.type == SQLType.UNKNOWN.value
        assert self.test_metadata.typename == ''
        assert self.test_metadata.colname == ''
        assert self.test_metadata.tablename == ''
        assert self.test_metadata.dbname == ''
        assert self.test_metadata.column_aliasname == ''
        assert self.test_metadata.table_aliasname == ''
        assert self.test_metadata.displaysize == 0
        assert self.test_metadata.precision == 0
        assert self.test_metadata.scale == 0
        assert self.test_metadata.cardinality == 0
        assert self.test_metadata._ColumnMetaData__flag == 0

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_metadata.class_id == ClassID.COLUMN_META_DATA.value

    def test_str(self):
        assert str(self.test_metadata) == ''
        self.test_metadata.column_aliasname = 'test'
        assert str(self.test_metadata) == 'test'

    def test_flag_getter(self):
        # 正常系のテスト
        # flagがFalseになるパターン
        self.test_metadata._ColumnMetaData__flag = 0b0000000
        assert not self.test_metadata.is_autoincrement
        assert not self.test_metadata.is_case_insensitive
        assert not self.test_metadata.is_unsigned
        assert not self.test_metadata.isnot_searchable
        assert not self.test_metadata.is_readonly
        assert not self.test_metadata.isnot_nullable
        assert not self.test_metadata.is_unique
        # flagがTrueになるパターン
        self.test_metadata._ColumnMetaData__flag = (1 << 0)
        assert self.test_metadata.is_autoincrement
        self.test_metadata._ColumnMetaData__flag = (1 << 1)
        assert self.test_metadata.is_case_insensitive
        self.test_metadata._ColumnMetaData__flag = (1 << 2)
        assert self.test_metadata.is_unsigned
        self.test_metadata._ColumnMetaData__flag = (1 << 3)
        assert self.test_metadata.isnot_searchable
        self.test_metadata._ColumnMetaData__flag = (1 << 4)
        assert self.test_metadata.is_readonly
        self.test_metadata._ColumnMetaData__flag = (1 << 5)
        assert self.test_metadata.isnot_nullable
        self.test_metadata._ColumnMetaData__flag = (1 << 6)
        assert self.test_metadata.is_unique

    def test_get_datatype(self):
        # 正常系のテスト
        # CHARACTER to STRING
        test_datatype = self.test_metadata.get_datatype(
            SQLType.CHARACTER.value)
        assert test_datatype == DataType.STRING.value
        # CHARACTER_VARYING to STRING
        test_datatype = self.test_metadata.get_datatype(
            SQLType.CHARACTER_VARYING.value)
        assert test_datatype == DataType.STRING.value
        # NATIONAL_CHARACTER to STRING
        test_datatype = self.test_metadata.get_datatype(
            SQLType.NATIONAL_CHARACTER.value)
        assert test_datatype == DataType.STRING.value
        # NATIONAL_CHARACTER_VARYING to STRING
        test_datatype = self.test_metadata.get_datatype(
            SQLType.NATIONAL_CHARACTER_VARYING.value)
        assert test_datatype == DataType.STRING.value
        # BINARY to BINARY
        test_datatype = self.test_metadata.get_datatype(SQLType.BINARY.value)
        assert test_datatype == DataType.BINARY.value
        # BINARY_VARYING to BINARY
        test_datatype = self.test_metadata.get_datatype(
            SQLType.BINARY_VARYING.value)
        assert test_datatype == DataType.BINARY.value
        # INTEGER to INTEGER
        test_datatype = self.test_metadata.get_datatype(SQLType.INTEGER.value)
        assert test_datatype == DataType.INTEGER.value
        # BIG_INT to INTEGER64
        test_datatype = self.test_metadata.get_datatype(SQLType.BIG_INT.value)
        assert test_datatype == DataType.INTEGER64.value
        # DECIMAL to DECIMAL
        test_datatype = self.test_metadata.get_datatype(SQLType.DECIMAL.value)
        assert test_datatype == DataType.DECIMAL.value
        # NUMERIC to DECIMAL
        test_datatype = self.test_metadata.get_datatype(SQLType.NUMERIC.value)
        assert test_datatype == DataType.DECIMAL.value
        # DOUBLE_PRECISION to DOUBLE
        test_datatype = self.test_metadata.get_datatype(
            SQLType.DOUBLE_PRECISION.value)
        assert test_datatype == DataType.DOUBLE.value
        # DATE to DATE
        test_datatype = self.test_metadata.get_datatype(SQLType.DATE.value)
        assert test_datatype == DataType.DATE.value
        # TIMESTAMP to DATE_TIME
        test_datatype = self.test_metadata.get_datatype(
            SQLType.TIMESTAMP.value)
        assert test_datatype == DataType.DATE_TIME.value
        # LANGUAGE to LANGUAGE
        test_datatype = self.test_metadata.get_datatype(SQLType.LANGUAGE.value)
        assert test_datatype == DataType.LANGUAGE.value
        # WORD to WORD
        test_datatype = self.test_metadata.get_datatype(SQLType.WORD.value)
        assert test_datatype == DataType.WORD.value
        # 存在しないSQLTypeを指定した場合
        test_datatype = self.test_metadata.get_datatype(-1)
        assert test_datatype == DataType.UNDEFINED.value

    def test_get_datainstance(self):
        # 正常系のテスト
        # CHARACTER to STRING
        self.test_metadata.type = SQLType.CHARACTER.value
        test_data = self.test_metadata.get_datainstance()
        assert test_data == StringData()
        # BINARY to BINARY
        self.test_metadata.type = SQLType.BINARY.value
        test_data = self.test_metadata.get_datainstance()
        assert test_data == BinaryData()
        # INTEGER to INTEGER
        self.test_metadata.type = SQLType.INTEGER.value
        test_data = self.test_metadata.get_datainstance()
        assert test_data == IntegerData()
        # BIG_INT to INTEGER64
        self.test_metadata.type = SQLType.BIG_INT.value
        test_data = self.test_metadata.get_datainstance()
        assert test_data == Integer64Data()
        # DECIMAL to DECIMAL
        self.test_metadata.type = SQLType.DECIMAL.value
        test_data = self.test_metadata.get_datainstance()
        assert test_data == DecimalData()
        # DOUBLE_PRECISION to DOUBLE
        self.test_metadata.type = SQLType.DOUBLE_PRECISION.value
        test_data = self.test_metadata.get_datainstance()
        assert test_data == DoubleData()
        # DATE to DATE
        self.test_metadata.type = SQLType.DATE.value
        test_data = self.test_metadata.get_datainstance()
        assert test_data == DateData()
        # TIMESTAMP to DATE_TIME
        self.test_metadata.type = SQLType.TIMESTAMP.value
        test_data = self.test_metadata.get_datainstance()
        assert test_data == DateTimeData()
        # LANGUAGE to LANGUAGE
        self.test_metadata.type = SQLType.LANGUAGE.value
        test_data = self.test_metadata.get_datainstance()
        assert test_data == LanguageData()
        # WORD to WORD
        self.test_metadata.type = SQLType.WORD.value
        test_data = self.test_metadata.get_datainstance()
        assert test_data == WordData()
        # 存在しないSQLTypeを指定した場合
        self.test_metadata.type = -1
        test_data = self.test_metadata.get_datainstance()
        assert test_data is None

    def test_read_object(self):
        # 正常系のテスト
        # モックを作成
        self.input_ = InputStream(None)
        self.input_.read_int = MagicMock(
            side_effect=[SQLType.INTEGER.value, 6, 4, 0, 1, 2, 3, 4])
        with patch('src.doquedb.common.unicodestr.UnicodeString.read_object',
                   side_effect=[
                       'type name', 'colname', 'tablename', 'dbname',
                       'column alias', 'table alias']):
            self.test_metadata.read_object(self.input_)
            # 正しい値を読み込んだかチェック
            assert self.test_metadata.typename == 'type name'
            assert self.test_metadata.colname == 'colname'
            assert self.test_metadata.tablename == 'tablename'
            assert self.test_metadata.dbname == 'dbname'
            assert self.test_metadata.column_aliasname == 'column alias'
            assert self.test_metadata.table_aliasname == 'table alias'
            assert self.test_metadata.displaysize == 0
            assert self.test_metadata.precision == 1
            assert self.test_metadata.scale == 2
            assert self.test_metadata.cardinality == 3
            assert self.test_metadata._ColumnMetaData__flag == 4

    def test_write_object(self):
        # 正常系のテスト
        # パラメータの設定
        self.test_metadata.typename = 'type name'
        self.test_metadata.colname = 'column name'
        self.test_metadata.tablename = 'table name'
        self.test_metadata.dbname = 'db name'
        self.test_metadata.column_aliasname = 'column alias'
        self.test_metadata.table_aliasname = 'table alias'
        self.test_metadata.displaysize = 0
        self.test_metadata.precision = 1
        self.test_metadata.scale = 2
        self.test_metadata.cardinality = 3
        self.test_metadata._ColumnMetaData__flag = 4
        self.output_ = OutputStream(None)
        # モックを作成
        self.output_.write_int = MagicMock()
        with patch('src.doquedb.common.unicodestr.UnicodeString.write_object')\
                as mock:
            # 引数をチェックする
            calls_write_int = [call(0), call(6), call(4), call(
                0), call(1), call(2), call(3), call(4)]
            calls_write_object = [call(self.output_, 'type name'),
                                  call(self.output_, 'column name'),
                                  call(self.output_, 'table name'),
                                  call(self.output_, 'db name'),
                                  call(self.output_, 'column alias'),
                                  call(self.output_, 'table alias')]
            self.test_metadata.write_object(self.output_)
            self.output_.write_int.assert_has_calls(calls_write_int)
            mock.assert_has_calls(calls_write_object)


class TestRequest:
    def setup_method(self, method):
        # 前処理
        self.test_request = Request(Request.request_map['BEGIN_CONNECTION'])
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がある場合
        assert self.test_request.request == self.test_request.request_map[
            'BEGIN_CONNECTION']
        # 引数がない場合
        self.test_request = Request()
        assert self.test_request.request == self.test_request.request_map[
            'UNDEFINED']

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_request.class_id == ClassID.REQUEST.value

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_int = MagicMock(
            return_value=Request.request_map['BEGIN_CONNECTION'])
        self.test_request.read_object(self.input_)
        assert self.test_request.request == Request.request_map[
            'BEGIN_CONNECTION']

    def test_write_object(self):
        # 正常系のテスト
        self.test_request = Request(Request.request_map['BEGIN_CONNECTION'])
        self.output_.write_int = MagicMock()
        self.test_request.write_object(self.output_)
        self.output_.write_int.assert_called_once_with(
            Request.request_map['BEGIN_CONNECTION'])


class TestStatus:
    def setup_method(self, method):
        # 前処理
        self.test_status = Status()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_status.status == self.test_status.status_map[
            'UNDEFINED']
        # 引数がある場合
        self.test_status = Status(Status.status_map['SUCCESS'])
        assert self.test_status.status == self.test_status.status_map[
            'SUCCESS']

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_status.class_id == ClassID.STATUS.value

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_int = MagicMock(
            return_value=Status.status_map['SUCCESS'])
        self.test_status.read_object(self.input_)
        assert self.test_status.status == Status.status_map['SUCCESS']

    def test_write_object(self):
        # 正常系のテスト
        self.test_status = Status(Status.status_map['SUCCESS'])
        self.output_.write_int = MagicMock()
        self.test_status.write_object(self.output_)
        self.output_.write_int.assert_called_once_with(
            Status.status_map['SUCCESS'])


class TestErrorLevel:
    def setup_method(self, method):
        # 前処理
        self.test_error_lv = ErrorLevel()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数を指定しない場合
        assert self.test_error_lv.level == ErrorLevel.UNDEFINED
        # 引数を指定した場合
        self.test_error_lv = ErrorLevel(ErrorLevel.USER)
        assert self.test_error_lv.level == ErrorLevel.USER

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_error_lv.class_id == ClassID.ERROR_LEVEL.value

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_int = MagicMock(
            return_value=ErrorLevel.USER)
        self.test_error_lv.read_object(self.input_)
        assert self.test_error_lv.level == ErrorLevel.USER
        # is_userlevelのテスト
        assert self.test_error_lv.is_userlevel()

    def test_write_object(self):
        # 正常系のテスト
        self.test_error_lv.level = ErrorLevel.SYSTEM
        self.output_.write_int = MagicMock()
        self.test_error_lv.write_object(self.output_)
        self.output_.write_int.assert_called_once_with(
            self.test_error_lv._ErrorLevel__level)
        # is_userlevelのテスト
        assert not self.test_error_lv.is_userlevel()


class TestExceptionData:
    def setup_method(self, method):
        # 前処理
        self.test_exception_data = ExceptionData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数を指定しない場合
        assert self.test_exception_data.errno == 0
        assert self.test_exception_data._ExceptionData__args == []
        assert self.test_exception_data._ExceptionData__modulename == ''
        assert self.test_exception_data._ExceptionData__filename == ''
        assert self.test_exception_data._ExceptionData__linenum == 0
        # 引数を指定した場合
        self.test_exception_data = ExceptionData(1)
        assert self.test_exception_data.errno == 1
        assert self.test_exception_data._ExceptionData__modulename == ''
        assert self.test_exception_data._ExceptionData__filename == ''
        assert self.test_exception_data._ExceptionData__linenum == 0

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_exception_data.class_id ==\
            ClassID.EXCEPTION_DATA.value

    @pytest.mark.skip(reason='未実装')
    def test_error_message(self):
        # TODO: 未実装
        pass

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_int = MagicMock(
            side_effect=[0, 1, 1, 1, 1, 1])
        self.input_.read_char = MagicMock(
            side_effect=['a', 'b', 'c'])

        self.test_exception_data.read_object(self.input_)
        assert self.test_exception_data.errno == 0
        # :func: `error_message` 実装後error_messageでチェック
        assert self.test_exception_data._ExceptionData__args == ['a']
        assert self.test_exception_data._ExceptionData__modulename == 'b'
        assert self.test_exception_data._ExceptionData__filename == 'c'
        assert self.test_exception_data._ExceptionData__linenum == 1

    def test_write_object(self):
        # 正常系のテスト
        with pytest.raises(OperationalError):
            self.test_exception_data.write_object(self.output_)
