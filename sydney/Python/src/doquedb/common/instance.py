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
instance.py --  class_idに相当するクラスのインスタンスを得る
"""
from typing import Optional, Any

from ..exception import exceptions
from .constants import ClassID
from .abstracts import Data


class Instance:
    """クラスID(:obj: `ClassID`)に相当するクラスのインスタンスを得る
    """
    # object_map, python_to_trobject_mapをクラス変数で定義する
    object_map = None
    python_to_trobject_map = None

    @staticmethod
    def get(id: int) -> Any:  # 戻り値はSerializableを継承したサブクラス
        """クラスID(:obj: `ClassID`)に相当するSerializableのサブクラスのインスタンスを得る

        Args:
            id (int): クラスID

        Returns:
            Serializable: :obj: `Serializable` のサブクラス

        TODO:
            :dict: ``object_map``をメソッドの外で定義する
        """
        # メソッド外でインポートすると相互参照でエラーになるためメソッド内でインポートする
        from .serialdata import (ColumnMetaData,
                                 ErrorLevel,
                                 ExceptionData,
                                 Request,
                                 Status
                                 )
        from .arraydata import (IntegerArrayData,
                                ResultSetMetaData,
                                StringArrayData,
                                DataArrayData
                                )
        from .scalardata import (FloatData,
                                 NullData,
                                 StringData,
                                 IntegerData,
                                 Integer64Data,
                                 DecimalData,
                                 DoubleData,
                                 DateData,
                                 DateTimeData,
                                 LanguageData,
                                 )
        from .data import BinaryData, WordData

        # object_mapを生成する
        if Instance.object_map is None:
            Instance.object_map = {
                'STATUS': Status,
                'BINARY_DATA': BinaryData,
                'INTEGER_DATA': IntegerData,
                'UNSIGNED_INTEGER_DATA': IntegerData,
                'INTEGER64_DATA': Integer64Data,
                'UNSIGNED_INTEGER64_DATA': Integer64Data,
                'FLOAT_DATA': FloatData,
                'DOUBLE_DATA': DoubleData,
                'DECIMAL_DATA': DecimalData,
                'STRING_DATA': StringData,
                'DATE_DATA': DateData,
                'DATE_TIME_DATA': DateTimeData,
                'INTEGER_ARRAY_DATA': IntegerArrayData,
                'UNSIGNED_INTEGER_ARRAY_DATA': IntegerArrayData,
                'STRING_ARRAY_DATA': StringArrayData,
                'DATA_ARRAY_DATA': DataArrayData,
                'NULL_DATA': NullData,
                'EXCEPTION_DATA': ExceptionData,
                # 'PARAMETER' : Parameter,
                # 'BITSET' : BitSet,
                'COMPRESSED_STRING_DATA': StringData,
                # 'COMPRESSED_BINARY_DATA': CompressedBinaryData,
                # 'OBJECTID_DATA' : ObjectIDData,
                'REQUEST': Request,
                'LANGUAGE_DATA': LanguageData,
                # 'SQL_DATA' : SQLData,
                'COLUMN_META_DATA': ColumnMetaData,
                'RESULTSET_META_DATA': ResultSetMetaData,
                'WORD_DATA': WordData,
                'ERROR_LEVEL': ErrorLevel,
            }

        # ``id``がClassIDに存在するか確認
        object: Optional[Any] = None
        if id == ClassID.NONE.value:
            return object
        elif id in list(map(int, ClassID)):
            # class_idを``id``から逆引き検索
            for item in ClassID:
                if id == item.value:
                    # インスタンス化
                    object = Instance.object_map[item.name]()
        else:
            raise exceptions.UnexpectedError('Class ID is not valid')

        return object

    @staticmethod
    def get_data(type_: type, value: Any) -> Data:
        # メソッド外でインポートすると相互参照でエラーになるためメソッド内でインポートする
        from .scalardata import (NullData,
                                 StringData,
                                 Integer64Data,
                                 DecimalData,
                                 DoubleData,
                                 DateData,
                                 DateTimeData,
                                 LanguageData,
                                 )
        from .data import BinaryData
        from ..driver.dbapi import Date, Timestamp, Language, Decimal, Binary

        # pythonの既定クラスとインスタンスの対応
        # python_to_trobject_mapを生成する
        if Instance.python_to_trobject_map is None:
            Instance.python_to_trobject_map = {
                int: Integer64Data,
                float: DoubleData,
                str: StringData,
                Date: DateData,
                Timestamp: DateTimeData,
                Decimal: DecimalData,
                Language: LanguageData,
                Binary: BinaryData
            }

        if type_ is not type(None):
            if not isinstance(value, type_):
                raise exceptions.UnexpectedError(
                    'expected type is {}, but value has type {}'
                    .format(type_, type(value)))
        else:
            # Noneが与えられた場合はNullDataのインスタンスを返す
            return NullData()

        try:
            object_ = Instance.python_to_trobject_map[type_](value)
        except IndexError as ie:
            raise exceptions.UnexpectedError(f'invalid type: {type_}') from ie

        assert isinstance(object_, Data)
        return object_
