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
serializable.py -- DoqueDBと互換性を持ったシリアライズインターフェース
"""

from abc import ABC, abstractmethod
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from .iostream import InputStream, OutputStream


class Serializable(ABC):
    """シリアル化インターフェース

    DoqueDBと互換性を保持しつつシリアル化を行うためのインターフェース.
    odbcのデータ型クラスは、 本インターフェースを継承する必要がある.
    """
    @property
    @abstractmethod
    def class_id(self) -> int:
        pass

    @abstractmethod
    def read_object(self, input_: 'InputStream') -> None:
        """ストリームから読込む.
        """
        pass

    @abstractmethod
    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームに書込む.
        """
        pass
