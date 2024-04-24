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
abstracts.py -- データ型をあらわすクラスの基底クラスのモジュール

:class: `Data`
:class: `ArrayData`
:class: `ScalarData`
"""

from abc import ABC
from ..exception import exceptions
from typing import List, Any

from .constants import DataType


class Data(ABC):
    """データ型をあらわすクラス共通の基底クラス.

    Args:
        type (int): データタイプ

    Attributes:
        __type (int): データタイプ
    """

    def __init__(self, type) -> None:
        self.__type = type

    @property
    def type(self) -> int:
        """データタイプ :obj: `DataType` のゲッター."""
        return self.__type

    @property
    def value(self) -> Any:
        """データが保持する値のゲッター"""
        return None

    @property
    def element_type(self) -> int:
        """配列要素のデータタイプ(:obj: `DataType`)を得る.

        Notes:
            常にUNDEFINDを返す. :obj: `ArrayData`で上書きする.
        """
        return DataType.UNDEFINED.value


class ArrayData(Data, ABC):
    """配列データ型をあらわすクラス共通の基底クラス.

    Args:
        element_type (int): 配列要素の :obj: `DataType`

    Attributes:
        __element_type (int): 配列要素の :obj: `DataType`
        _array (list): 配列データ
    """

    def __init__(self, element_type: int) -> None:
        super().__init__(DataType.ARRAY.value)
        self.__element_type = element_type
        self._array: List[Any] = []

    @property
    def element_type(self) -> int:
        """配列要素のデータタイプ(:obj: `DataType`)のゲッター."""
        return self.__element_type

    def __len__(self) -> int:
        return len(self._array)

    def __eq__(self, other: Any) -> bool:
        result = False
        if isinstance(other, ArrayData):
            if self._array == other._array and\
                    self.element_type == other.element_type:
                result = True
        return result

    def __str__(self) -> str:
        p = '[' + ', '.join(map(str, self._array)) + ']'
        return p

    def __getitem__(self, index_: int) -> Any:
        """配列内のインデックスの位置にある要素を返す.

        Args:
            index_ (int): 配列内の位置

        Returns:
            Any: 指定されたインデックスにある要素
        """
        return self._array[index_]

    def __setitem__(self, index_: int, element: Any) -> Any:
        """配列内のインデックスの位置にある要素を、``element_``に置き換える.

        Args:
            index_ (int): 配列内の位置
            element_ (Any): 格納する要素

        Returns:
            Any: インデックスの位置に以前あった要素
        """
        if not isinstance(element, Data):
            raise exceptions.UnexpectedError(
                'element must be subclass of Data')

        # ``index_`` が配列要素数よりも大きい場合リストを拡張する
        if index_ >= len(self._array):
            self._array.extend(None for _ in range(index_-len(self._array)+1))

        prev_element = self._array[index_]
        self._array[index_] = element

        return prev_element

    def _add(self, element: Any) -> None:
        """配列の末尾に要素を追加する.

        Args:
            element_ (Any): 挿入される要素
        """
        self._array.append(element)

    def clear(self) -> None:
        """全ての要素を削除.
        """
        self._array.clear()


class ScalarData(Data, ABC):
    """スカラーデータ型をあらわすクラス共通の基底クラス.

    Args:
        type_: データタイプ
    """

    def __init__(self, type_: int) -> None:
        super().__init__(type_)
