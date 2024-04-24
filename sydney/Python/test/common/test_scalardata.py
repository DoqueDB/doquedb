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
test_scalardata.py -- src.doquedb.common.scalardata モジュールのテスト
"""
import pytest
from unittest.mock import MagicMock, patch, call

from src.doquedb.exception.exceptions import ProgrammingError, UnexpectedError
from src.doquedb.common.scalardata import (DateData, DateTimeData, DecimalData,
                                           DoubleData, FloatData, Integer64Data,
                                           IntegerData, LanguageData, StringData,
                                           NullData)
from src.doquedb.common.iostream import InputStream, OutputStream
from src.doquedb.driver.dbapi import Date, Timestamp
from src.doquedb.common.constants import ClassID, DataType
from src.doquedb.common.language import LanguageSet


class TestDateData:
    def setup_method(self, method):
        # 前処理
        self.test_date_data = DateData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

        self.year = 2021
        self.month = 4
        self.day = 1

        self.test_date = Date(self.year, self.month, self.day)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_date_data.value == Date(1000, 1, 1)
        assert self.test_date_data.type == DataType.DATE.value
        # 引数がdbapi.Date型の場合
        self.test_date_data = DateData(self.test_date)
        assert self.test_date_data.value == self.test_date
        # 引数がDateDataの場合
        self.test_date_data = DateData(DateData())
        assert self.test_date_data.value == Date(1000, 1, 1)

    def test_init_bad_args(self):
        # 異常系のテスト
        with pytest.raises(UnexpectedError):
            self.test_date_data = DateData(int(1))

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_date_data.class_id == ClassID.DATE_DATA.value

    def test_str(self):
        # 正常系のテスト
        self.test_date_data.value = self.test_date
        assert str(self.test_date_data) == '2021-04-01'

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        assert self.test_date_data == DateData(Date(1000, 1, 1))
        # 型が違う場合
        assert self.test_date_data != Date(1000, 1, 1)
        # 型が同じで``value``が違う場合
        test_wrong_value = DateData(self.test_date)
        assert self.test_date_data != test_wrong_value

    def test_read_object(self):
        # 正常系のテスト
        mock_list = [self.year, self.month, self.day]
        self.input_.read_int = MagicMock(side_effect=mock_list)
        self.test_date_data.read_object(self.input_)
        assert self.test_date_data == DateData(
            self.test_date)

    def test_write_object(self):
        # 正常系のテスト
        self.test_date_data.value = self.test_date
        self.output_.write_int = MagicMock()
        self.test_date_data.write_object(self.output_)
        # 正しい回数``write_int``が呼ばれたか確認
        assert self.output_.write_int.call_count == 3
        # 正しい引数で``write_int``が呼ばれたか確認
        calls = [call(self.year),
                 call(self.month),
                 call(self.day)]
        self.output_.write_int.assert_has_calls(calls)


class TestDateTimeData:
    def setup_method(self, method):
        # 前処理
        self.test_datetime_data = DateTimeData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

        self.test_year = 2021
        self.test_month = 4
        self.test_day = 1
        self.test_hour = 12
        self.test_minute = 30
        self.test_second = 30
        self.test_millisecond = 1

        self.mock_list = [self.test_year, self.test_month, self.test_day,
                          self.test_hour, self.test_minute, self.test_second,
                          self.test_millisecond, 0]
        self.test_timestamp = Timestamp(
            self.test_year, self.test_month, self.test_day,
            self.test_hour, self.test_minute, self.test_second,
            self.test_millisecond*1000)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_datetime_data.value == Timestamp(1000, 1, 1)
        assert self.test_datetime_data.type == DataType.DATE_TIME.value
        # 引数がdbapi.Timestamp型の場合
        self.test_datetime_data = DateTimeData(self.test_timestamp)
        assert self.test_datetime_data.value == self.test_timestamp
        # 引数がDateTimeDataの場合
        self.test_datetime_data = DateTimeData(DateTimeData())
        assert self.test_datetime_data.value == Timestamp(1000, 1, 1)

    def test_init_bad_args(self):
        # 異常系のテスト
        with pytest.raises(UnexpectedError):
            self.test_datetime_data = DateTimeData(int(1))

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_datetime_data.class_id == ClassID.DATE_TIME_DATA.value

    def test_setter_value(self):
        # 正常系のテスト
        self.test_datetime_data.value = self.test_timestamp
        assert self.test_datetime_data.millisecond == self.test_millisecond

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        assert self.test_datetime_data == DateTimeData(Timestamp(1000, 1, 1))
        # 型が違う場合
        assert self.test_datetime_data != Timestamp(1000, 1, 1)
        # 型が同じで``value``が違う場合
        test_wrong_value = DateTimeData(self.test_timestamp)
        assert self.test_datetime_data != test_wrong_value

    def test_str(self):
        # 正常系のテスト
        self.test_datetime_data.value = self.test_timestamp
        assert str(self.test_datetime_data) == '2021-04-01 12:30:30.001'

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_int = MagicMock(side_effect=self.mock_list)
        self.test_datetime_data.read_object(self.input_)
        assert self.test_datetime_data.value == self.test_timestamp

    def test_write_object(self):
        # 正常系のテスト
        self.test_datetime_data.value = self.test_timestamp
        self.output_.write_int = MagicMock()
        self.test_datetime_data.write_object(self.output_)
        # 正しい回数``write_int``が呼ばれたか確認
        assert self.output_.write_int.call_count == 8
        # 正しい引数で``write_int``が呼ばれたか確認
        calls = [call(self.test_year), call(self.test_month),
                 call(self.test_day), call(self.test_hour),
                 call(self.test_minute), call(self.test_second),
                 call(self.test_millisecond), call(3)]
        self.output_.write_int.assert_has_calls(calls)


class TestDecimalData:
    def setup_method(self, method):
        # 前処理
        self.test_decimal_data = DecimalData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        iDigitPerUnit = 9
        assert self.test_decimal_data.value == '0'
        assert self.test_decimal_data._precision == iDigitPerUnit
        assert self.test_decimal_data._scale == 0
        assert self.test_decimal_data.type == DataType.DECIMAL.value
        # 引数がDecimal型の場合
        self.test_decimal_data = DecimalData('1.1')
        assert self.test_decimal_data.value == '1.1'
        assert self.test_decimal_data._precision == 2
        assert self.test_decimal_data._scale == 1
        # 引数がDecimalData型の場合
        self.test_decimal_data = DecimalData(DecimalData('2.22'))
        assert self.test_decimal_data.value == '2.22'
        assert self.test_decimal_data._precision == 3
        assert self.test_decimal_data._scale == 2

    def test_init_bad_args(self):
        with pytest.raises(UnexpectedError):
            self.test_decimal_data = DecimalData(int(1))

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_decimal_data.class_id == ClassID.DECIMAL_DATA.value

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        assert self.test_decimal_data == DecimalData('0')
        # 型が違う場合
        assert self.test_decimal_data != '0'
        # 型が同じで``value``が違う場合
        test_wrong_value = DecimalData('0.1')
        assert self.test_decimal_data != test_wrong_value
        # 文字列としては違うがDecimalとしては同じ場合
        test_decimal_value = DecimalData('+0.0')
        assert self.test_decimal_data == test_decimal_value

    def test_setter_value(self):
        # 正常系のテスト
        # 小数点以下がある場合
        self.test_decimal_data.value = '123.456789'
        assert self.test_decimal_data.value == '123.456789'
        assert self.test_decimal_data._precision == 9
        assert self.test_decimal_data._scale == 6
        # 小数点以下がある場合(+)
        self.test_decimal_data.value = '+12.3456789'
        assert self.test_decimal_data.value == '+12.3456789'
        assert self.test_decimal_data._precision == 9
        assert self.test_decimal_data._scale == 7
        # 小数点以下がある場合(-)
        self.test_decimal_data.value = '-12345.6789'
        assert self.test_decimal_data.value == '-12345.6789'
        assert self.test_decimal_data._precision == 9
        assert self.test_decimal_data._scale == 4
        # 小数点以下がない場合
        self.test_decimal_data.value = '123456789'
        assert self.test_decimal_data.value == '123456789'
        assert self.test_decimal_data._precision == 9
        assert self.test_decimal_data._scale == 0
        # +を含む場合
        self.test_decimal_data.value = '+123456789'
        assert self.test_decimal_data.value == '+123456789'
        assert self.test_decimal_data._precision == 9
        assert self.test_decimal_data._scale == 0
        # -を含む場合
        self.test_decimal_data.value = '-123456789'
        assert self.test_decimal_data.value == '-123456789'
        assert self.test_decimal_data._precision == 9
        assert self.test_decimal_data._scale == 0
        # 小数点以下の値があり、整数値が0の場合
        self.test_decimal_data.value = '0.123456789'
        assert self.test_decimal_data.value == '0.123456789'
        assert self.test_decimal_data._precision == 9
        assert self.test_decimal_data._scale == 9

        # 小数点以下の値末尾に0が複数ある場合
        self.test_decimal_data.value = '1.20000000'
        assert self.test_decimal_data.value == '1.20000000'
        assert self.test_decimal_data._precision == 9
        assert self.test_decimal_data._scale == 8

    def test_setter_value_error(self):
        # 異常系のテスト
        # strで数値以外入ってくる場合
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '123@456'
        # 指数表記の場合
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '123e10'
        # 途中に+, -を含む場合
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '-123+456'
        # 符号がない場合で.123456789のパターンを追加
        # 小数点以下がない場合
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '+123456789.'
        # 小数点以下のみの場合
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '-.123456789'
        # 整数値がなくかつ符号がない場合
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '.123456789'
        # 整数値の先頭に0が複数ある場合
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '000000001'
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '01'
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '+010'
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '-001'
        # 小数点以下の値があり、整数値に0が複数ある場合
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '00.1'
        # 整数部が01などで小数がある場合
        with pytest.raises(ProgrammingError):
            self.test_decimal_data.value = '01.2'

    @ pytest.mark.skip(reason='上位のテストで確認')
    def test_read_object(self):
        # 上位のテストを先に実装し、問題があれば実装する
        pass

    @ pytest.mark.skip(reason='上位のテストで確認')
    def test_write_object(self):
        # 上位のテストを先に実装し、問題があれば実装する
        pass


class TestDoubleData:
    def setup_method(self, method):
        # 前処理
        self.test_double_data = DoubleData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_double_data.value == 0.0
        assert self.test_double_data.type == DataType.DOUBLE.value
        # 引数がfloat型の場合
        self.test_double_data = DoubleData(1.1)
        assert self.test_double_data.value == 1.1
        # 引数がDoubleDataの場合
        self.test_double_data = DoubleData(DoubleData(2.2))
        assert self.test_double_data.value == 2.2

    def test_init_bad_args(self):
        # 異常系のテスト
        with pytest.raises(UnexpectedError):
            self.test_double_data = DoubleData(int(1))

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_double_data.class_id == ClassID.DOUBLE_DATA.value

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        assert self.test_double_data == DoubleData(0.0)
        # 型が違う場合
        assert self.test_double_data != float(0.0)
        # 型が同じで``value``が違う場合
        test_wrong_value = DoubleData(1.1)
        assert self.test_double_data != test_wrong_value

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_double = MagicMock(return_value=1.1)
        self.test_double_data.read_object(self.input_)
        assert self.test_double_data == DoubleData(1.1)

    def test_write_object(self):
        # 正常系のテスト
        self.test_double_data.value = 1.1
        self.output_.write_double = MagicMock()
        self.test_double_data.write_object(self.output_)
        self.output_.write_double.assert_called_once_with(1.1)


class TestFloatData:
    def setup_method(self, method):
        # 前処理
        self.test_float_data = FloatData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_float_data.value == 0.0
        assert self.test_float_data.type == DataType.FLOAT.value
        # 引数がfloat型の場合
        self.test_float_data = FloatData(1.1)
        assert self.test_float_data.value == 1.1
        # 引数がDoubleDataの場合
        self.test_float_data = FloatData(FloatData(2.2))
        assert self.test_float_data.value == 2.2

    def test_init_bad_args(self):
        # 異常系のテスト
        with pytest.raises(UnexpectedError):
            self.test_float_data = FloatData(int(1))

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_float_data.class_id == ClassID.FLOAT_DATA.value

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        assert self.test_float_data == FloatData(0.0)
        # 型が違う場合
        assert self.test_float_data != float(0.0)
        # 型が同じで``value``が違う場合
        test_wrong_value = FloatData(1.1)
        assert self.test_float_data != test_wrong_value

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_float = MagicMock(return_value=1.1)
        self.test_float_data.read_object(self.input_)
        assert self.test_float_data == FloatData(1.1)

    def test_write_object(self):
        # 正常系のテスト
        self.test_float_data.value = 1.1
        self.output_.write_float = MagicMock()
        self.test_float_data.write_object(self.output_)
        self.output_.write_float.assert_called_once_with(1.1)


class TestInteger64Data:
    def setup_method(self, method):
        # 前処理
        self.test_integer64_data = Integer64Data()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_integer64_data.value == 0
        assert self.test_integer64_data.type == DataType.INTEGER64.value
        # 引数がint型の場合
        self.test_integer64_data = Integer64Data(1)
        assert self.test_integer64_data.value == 1
        # 引数がInteger64Dataの場合
        self.test_integer64_data = Integer64Data(Integer64Data(2))
        assert self.test_integer64_data.value == 2

    def test_init_bad_args(self):
        # 異常系のテスト
        with pytest.raises(UnexpectedError):
            self.test_integer64_data = Integer64Data(float(0.0))

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_integer64_data.class_id ==\
            ClassID.INTEGER64_DATA.value

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        assert self.test_integer64_data == Integer64Data(0)
        # 型が違う場合
        assert self.test_integer64_data != int(0)
        # 型が同じで``value``が違う場合
        test_wrong_value = Integer64Data(1)
        assert self.test_integer64_data != test_wrong_value

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_long = MagicMock(return_value=1)
        self.test_integer64_data.read_object(self.input_)
        assert self.test_integer64_data == Integer64Data(1)

    def test_write_object(self):
        # 正常系のテスト
        self.test_integer64_data.value = 1
        self.output_.write_long = MagicMock()
        self.test_integer64_data.write_object(self.output_)
        self.output_.write_long.assert_called_once_with(1)


class TestIntegerData:
    def setup_method(self, method):
        # 前処理
        self.test_integer_data = IntegerData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_integer_data.value == 0
        assert self.test_integer_data.type == DataType.INTEGER.value
        # 引数がint型の場合
        self.test_integer_data = IntegerData(1)
        assert self.test_integer_data.value == 1
        # 引数がIntegerDataの場合
        self.test_integer_data = IntegerData(IntegerData(2))
        assert self.test_integer_data.value == 2

    def test_init_bad_args(self):
        # 異常系のテスト
        with pytest.raises(UnexpectedError):
            self.test_integer_data = IntegerData(float(0.0))

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_integer_data.class_id == ClassID.INTEGER_DATA.value

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        assert self.test_integer_data == IntegerData(0)
        # 型が違う場合
        assert self.test_integer_data != int(0)
        # 型が同じで``value``が違う場合
        test_wrong_value = IntegerData(1)
        assert self.test_integer_data != test_wrong_value

    def test_read_object(self):
        # 正常系のテスト
        self.input_.read_int = MagicMock(return_value=1)
        self.test_integer_data.read_object(self.input_)
        assert self.test_integer_data == IntegerData(1)

    def test_write_object(self):
        # 正常系のテスト
        self.test_integer_data.value = 1
        self.output_.write_int = MagicMock()
        self.test_integer_data.write_object(self.output_)
        self.output_.write_int.assert_called_once_with(1)


class TestStringData:
    def setup_method(self, method):
        # 前処理
        self.test_string_data = StringData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_string_data.value == ''
        assert self.test_string_data.type == DataType.STRING.value
        # 引数がstr型の場合
        self.test_string_data = StringData('test')
        assert self.test_string_data.value == 'test'
        # 引数がStringDataの場合
        self.test_string_data = StringData(StringData('string_data'))
        assert self.test_string_data.value == 'string_data'

    def test_init_bad_args(self):
        # 異常系のテスト
        with pytest.raises(UnexpectedError):
            self.test_string_data = StringData(int(0))

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_string_data.class_id == ClassID.STRING_DATA.value

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        assert self.test_string_data == StringData('')
        # 型が違う場合
        assert self.test_string_data != str('')
        # 型が同じで``value``が違う場合
        test_wrong_value = StringData('wrong_value')
        assert self.test_string_data != test_wrong_value

    def test_read_object(self):
        # 正常系のテスト
        with patch('src.doquedb.common.unicodestr.UnicodeString.read_object',
                   return_value='test'):
            self.test_string_data.read_object(self.input_)
            assert self.test_string_data == StringData('test')

    def test_write_object(self):
        # 正常系のテスト
        self.test_string_data.value = 'test'
        with patch('src.doquedb.common.unicodestr.UnicodeString.write_object')\
                as mock:
            self.test_string_data.write_object(self.output_)
            mock.assert_called_once_with(self.output_, 'test')


class TestLanguageData:
    def setup_method(self, method):
        # 前処理
        self.test_language_data = LanguageData()
        self.input_ = InputStream(None)
        self.output_ = OutputStream(None)

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_language_data.value == LanguageSet()
        assert self.test_language_data.type == DataType.LANGUAGE.value
        # 引数がstr型の場合
        self.test_language_data = LanguageData('ja')
        assert self.test_language_data.value == LanguageSet('ja')
        # 引数がLanguageDataの場合
        self.test_language_data = LanguageData(LanguageData('en'))
        assert self.test_language_data.value == LanguageSet('en')

    def test_init_bad_args(self):
        # 異常系のテスト
        with pytest.raises(UnexpectedError):
            self.test_language_data = LanguageData(int(0))

    def test_eq(self):
        # 正常系のテスト
        # 空のLanguageData同士の比較
        self.test_language_data = LanguageData()
        assert self.test_language_data == LanguageData()
        # 一致する場合
        self.test_language_data = LanguageData('ja')
        assert self.test_language_data == LanguageData('ja')
        # 型が違う場合
        assert self.test_language_data != str('ja')
        # 型が同じで``value``が違う場合
        test_wrong_value = LanguageData('en')
        assert self.test_language_data != test_wrong_value

    def test_str(self):
        # 正常系のテスト
        self.test_language_data = LanguageData('ja-jp+en-us')
        assert str(self.test_language_data) == 'en-us+ja-jp'

    @ pytest.mark.skip(reason='未実装')
    def test_read_object(self):
        # TODO: 未実装
        pass

    @ pytest.mark.skip(reason='未実装')
    def test_write_object(self):
        # TODO: 未実装
        pass


class TestNullData:
    def setup_method(self, method):
        # 前処理
        self.test_null_data = NullData()

    def test_init(self):
        # 正常系のテスト
        assert self.test_null_data.type == DataType.NULL.value

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_null_data.class_id == ClassID.NULL_DATA.value

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        assert self.test_null_data == NullData()
        # 一致しない場合
        assert self.test_null_data != IntegerData()

    def test_str(self):
        # 正常系のテスト
        assert str(self.test_null_data) == '(null)'
