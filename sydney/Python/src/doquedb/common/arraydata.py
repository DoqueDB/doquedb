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
arraydata.py -- :class: `ArrayData` を基底とするクラスの実装モジュール.

:class: `DataArrayData`
:class: `IntegerArrayData`
:class: `ResultSetMetaData`
:class: `StringArrayData`
"""
# TODO: エラー処理追記

from typing import Optional, Union, List, TYPE_CHECKING
if TYPE_CHECKING:
    from .iostream import InputStream, OutputStream
    from .abstracts import Data
    from .serialdata import ColumnMetaData

import copy

from ..exception import exceptions
from .serializable import Serializable
from .abstracts import ArrayData
from .scalardata import IntegerData, StringData
from .unicodestr import UnicodeString
from .constants import ClassID, DataType


class DataArrayData(ArrayData, Serializable):
    """Data配列型をあらわすクラス.

    Args:
        value (Optional[DataArrayData]): 値

    Attributes:
        _array (list): 配列
    """

    @property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.DATA_ARRAY_DATA.value

    def __init__(self, value: Optional['DataArrayData'] = None) -> None:
        super().__init__(DataType.DATA.value)
        if value:
            self._array = copy.deepcopy(value._array)

    @property
    def array(self) -> List['Data']:
        """``self._array``のゲッター"""
        return self._array

    def add_element(self, element_: 'Data') -> None:
        """配列の末尾に要素を追加する.

        Args:
            element_ (Data): 挿入される要素
        """
        self._add(element_)

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        size = input_.read_int()
        for i in range(size):
            self[i] = input_.read_object()

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        size = len(self)
        output_.write_int(size)
        for i in range(size):
            output_.write_object(self[i])

    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return DataArrayData(self)

    def assign(self, data: 'DataArrayData') -> None:
        """中身をアサインする

        Args:
            data (DataArrayData): アサインするデータ
        """
        self._array = copy.deepcopy(data._array)


class IntegerArrayData(ArrayData, Serializable):
    """int配列型をあらわすクラス.

    Args:
        value (Optional['IntegerArrayData']): 値

    Attributes:
        _array (list): 配列
    """

    @ property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.INTEGER_ARRAY_DATA.value

    def __init__(self,
                 value: Union[List[IntegerData], 'IntegerArrayData'] = None
                 ) -> None:
        super().__init__(DataType.INTEGER.value)
        if isinstance(value, list):
            self._array: List[IntegerData] = copy.deepcopy(value)
        elif isinstance(value, IntegerArrayData):
            self._array = copy.deepcopy(value._array)
        elif value is None:
            self._array = []
        else:
            raise exceptions.ProgrammingError("value should be following type,\
                [None, list, IntegerArrayData]")

    def add_element(self, element_: IntegerData) -> None:
        """配列の末尾に要素を追加する.

        Args:
            element_ (int): 挿入される要素
        """
        return self._add(element_)

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.clear()
        size = input_.read_int()
        for i in range(size):
            self.add_element(IntegerData(input_.read_int()))

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        size = len(self)
        output_.write_int(size)
        for i in range(size):
            output_.write_int(self[i].value)

    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return IntegerArrayData(self)


class StringArrayData(ArrayData, Serializable):
    """string配列型をあらわすクラス.

    Args:
        value (Optional['StringArrayData']): 値

    Attributes:
        _array (list): 配列
    """

    def __init__(self, value: Optional['StringArrayData'] = None) -> None:
        super().__init__(DataType.STRING.value)
        if value:
            self._array = copy.deepcopy(value._array)

    @ property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.STRING_ARRAY_DATA.value

    def add_element(self, element_: str) -> None:
        """配列の末尾に要素を追加する.

        Args:
            element_ (str): 挿入される要素
        """
        return self._add(element_)

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        size = input_.read_int()
        for i in range(size):
            self[i] = StringData(UnicodeString.read_object(input_))

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        size = len(self)
        output_.write_int(size)
        for i in range(size):
            UnicodeString.write_object(output_, self[i].value)

    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return StringArrayData(self)


class ResultSetMetaData(ArrayData, Serializable):
    """リザルトセットのメタデータをあらわすクラス.

    Args:
        value (Optional['ResultSetMetaData']): 値

    Attributes:
        _array (list): 配列
    """

    def __init__(self, value: Optional['ResultSetMetaData'] = None) -> None:
        super().__init__(DataType.COLUMN_META_DATA.value)
        if value:
            self._array = copy.deepcopy(value._array)

    @ property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.RESULTSET_META_DATA.value

    def __getitem__(self, index_: int) -> 'ColumnMetaData':
        return super().__getitem__(index_)

    def __setitem__(self, index_: int, element) -> None:
        raise exceptions.NotSupportedError('use add_element method instead')

    def add_element(self, element_: 'ColumnMetaData') -> None:
        """配列の末尾に要素を追加する.

        Args:
            element_ (ColumnMetaData): 挿入される要素
        """
        return self._add(element_)

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        from .serialdata import ColumnMetaData

        size = input_.read_int()
        for i in range(size):
            meta = ColumnMetaData()
            meta.read_object(input_)
            self.add_element(meta)

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        size = len(self)
        output_.write_int(size)
        for i in range(size):
            self[i].write_object(output_)

    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return ResultSetMetaData(self)

    def create_tuple_data(self) -> DataArrayData:
        """メタデータから適切なデータ型が格納されたDataArrayDataを得る.

        Returns:
            DataArrayData: データ型が格納されたタプル
        """
        tuple = DataArrayData()
        for i in range(len(self)):
            tuple._add(self[i].get_datainstance())
        return tuple
