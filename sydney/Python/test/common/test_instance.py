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
test_instance.py -- src.doquedb.common.instance モジュールのテスト
"""
import pytest

from src.doquedb.exception.exceptions import UnexpectedError
from src.doquedb.common.instance import Instance
from src.doquedb.common.constants import ClassID
from src.doquedb.common.serialdata import (ColumnMetaData, ErrorLevel,
                                           ExceptionData,
                                           Request, Status)
from src.doquedb.common.scalardata import (FloatData, Integer64Data, DoubleData,
                                           DateData, DateTimeData, IntegerData,
                                           StringData, DecimalData,
                                           LanguageData, NullData)
from src.doquedb.common.arraydata import (DataArrayData, IntegerArrayData,
                                          ResultSetMetaData, StringArrayData)
from src.doquedb.common.language import LanguageSet
from src.doquedb.common.data import BinaryData, WordData
from src.doquedb.driver.dbapi import Date, Timestamp, Language, Decimal


class TestInstance:
    def test_get(self):
        # getの正常系テスト
        # None
        instance = Instance.get(ClassID.NONE.value)
        assert instance is None

        # Status
        instance = Instance.get(ClassID.STATUS.value)
        assert isinstance(instance, Status)

        # BinaryData
        instance = Instance.get(ClassID.BINARY_DATA.value)
        assert isinstance(instance, BinaryData)

        # IntegerData
        instance = Instance.get(ClassID.INTEGER_DATA.value)
        assert isinstance(instance, IntegerData)

        # UnsignedIntegerData
        instance = Instance.get(ClassID.UNSIGNED_INTEGER_DATA.value)
        assert isinstance(instance, IntegerData)

        # Integer64Data
        instance = Instance.get(ClassID.INTEGER64_DATA.value)
        assert isinstance(instance, Integer64Data)

        # UnsignedIntegerData
        instance = Instance.get(ClassID.UNSIGNED_INTEGER64_DATA.value)
        assert isinstance(instance, Integer64Data)

        # FloatData
        instance = Instance.get(ClassID.FLOAT_DATA.value)
        assert isinstance(instance, FloatData)

        # DoubleData
        instance = Instance.get(ClassID.DOUBLE_DATA.value)
        assert isinstance(instance, DoubleData)

        # DecimalData
        instance = Instance.get(ClassID.DECIMAL_DATA.value)
        assert isinstance(instance, DecimalData)

        # StringData
        instance = Instance.get(ClassID.STRING_DATA.value)
        assert isinstance(instance, StringData)

        # DateData
        instance = Instance.get(ClassID.DATE_DATA.value)
        assert isinstance(instance, DateData)

        # DateTimeData
        instance = Instance.get(ClassID.DATE_TIME_DATA.value)
        assert isinstance(instance, DateTimeData)

        # IntegerArrayData
        instance = Instance.get(ClassID.INTEGER_ARRAY_DATA.value)
        assert isinstance(instance, IntegerArrayData)

        # UnsignedIntegerArrayData
        instance = Instance.get(ClassID.UNSIGNED_INTEGER_ARRAY_DATA.value)
        assert isinstance(instance, IntegerArrayData)

        # StringArrayData
        instance = Instance.get(ClassID.STRING_ARRAY_DATA.value)
        assert isinstance(instance, StringArrayData)

        # DataArrayData
        instance = Instance.get(ClassID.DATA_ARRAY_DATA.value)
        assert isinstance(instance, DataArrayData)

        # NullData
        instance = Instance.get(ClassID.NULL_DATA.value)
        assert isinstance(instance, NullData)

        # ExceptionData
        instance = Instance.get(ClassID.EXCEPTION_DATA.value)
        assert isinstance(instance, ExceptionData)

        # CompressedStringData
        instance = Instance.get(ClassID.COMPRESSED_STRING_DATA.value)
        assert isinstance(instance, StringData)

        # Request
        instance = Instance.get(ClassID.REQUEST.value)
        assert isinstance(instance, Request)

        # LangaugeData
        instance = Instance.get(ClassID.LANGUAGE_DATA.value)
        assert isinstance(instance, LanguageData)

        # ColumnMetaData
        instance = Instance.get(ClassID.COLUMN_META_DATA.value)
        assert isinstance(instance, ColumnMetaData)

        # ResultSetMetaData
        instance = Instance.get(ClassID.RESULTSET_META_DATA.value)
        assert isinstance(instance, ResultSetMetaData)

        # WordData
        instance = Instance.get(ClassID.WORD_DATA.value)
        assert isinstance(instance, WordData)

        # ErrorLevel
        instance = Instance.get(ClassID.ERROR_LEVEL.value)
        assert isinstance(instance, ErrorLevel)

    def test_get_bad_id(self):
        # getの異常系テスト
        with pytest.raises(UnexpectedError):
            Instance.get(-1)

    def test_get_data(self):
        # get_dataの正常系テスト
        # int to Integer64Data
        object_ = Instance.get_data(int, 1)
        assert object_ == Integer64Data(1)

        # float to DoubleData
        object_ = Instance.get_data(float, 1.1)
        assert object_ == DoubleData(1.1)

        # str to StringData
        object_ = Instance.get_data(str, 'string')
        assert object_ == StringData('string')

        # dbapi.Date to DateData
        object_ = Instance.get_data(Date, Date.today())
        assert object_ == DateData(Date.today())

        # dbapi.TimeStamp to DateTimeData
        # TODO: fix
        # object_ = Instance.get_data(Timestamp, Timestamp.today())
        # assert object_ == DateTimeData(Timestamp.today())

        # Decimal to DecimalData
        object_ = Instance.get_data(Decimal, Decimal('3.14'))
        assert object_ == DecimalData(Decimal('3.14'))

        # LanguageData to LanguageData
        object_ = Instance.get_data(Language, LanguageData('ja'))
        assert object_ == LanguageData('ja')

        # None to NullData
        object_ = Instance.get_data(type(None), None)
        assert object_ == NullData()

    def test_get_data_wrong(self):
        # 異常系のテスト
        # typeが異なる場合
        with pytest.raises(KeyError):
            Instance.get_data(list, [1, 2])
        # valueが異なる場合
        with pytest.raises(UnexpectedError):
            Instance.get_data(int, 0.11)
