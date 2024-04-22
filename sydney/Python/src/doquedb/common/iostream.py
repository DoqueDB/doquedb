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
iostream.py -- DoqueDBと互換性のある入出力ストリームモジュール
"""

# TODO: 例外処理の追記
from typing import BinaryIO, Optional, TYPE_CHECKING
if TYPE_CHECKING:
    from .serializable import Serializable
import struct
from .instance import Instance
from ..exception import exceptions
from .constants import ClassID


class InputStream():
    """DoqueDBとの互換性のある入力ストリームクラス.

    Args:
        socket_ (socket.socket): ソケット

    Attributes:
        __socket (socket.socket): ソケット
    """

    def __init__(self, istream: BinaryIO) -> None:
        self.__istream: Optional[BinaryIO] = istream

    def read(self, bufsize: int) -> bytes:
        """ストリームから``bufsize``だけ読込む.

        Args:
            bufsize (int): バッファーサイズ

        Returns:
            bytes: 読込んだ値
        """
        if self.__istream:
            return self.__istream.read(bufsize)
        # closeしていた場合エラーとなる
        else:
            raise exceptions.UnexpectedError('InputStream is closed')

    def read_int(self) -> int:
        """4バイトのデータを読込んでint型に変換する.

        Returns:
            int: 読込んだ値
        """
        return struct.unpack('>i', self.read(4))[0]

    def read_short(self) -> int:
        """2バイトのデータを読込んでint型に変換する.

        Returns:
            int: 読込んだ値
        """
        return struct.unpack('>h', self.read(2))[0]

    def read_long(self) -> int:
        """8バイトのデータを読込んでint型に変換する.

        Returns:
            int: 読込んだ値
        """
        return struct.unpack('>q', self.read(8))[0]

    def read_double(self) -> float:
        """8バイトのデータを読込んでdouble(=float)型に変換する.

        Returns:
            float: 読込んだ値
        """
        return struct.unpack('>d', self.read(8))[0]

    def read_float(self) -> float:
        """4バイトのデータを読込んでfloat型に変換する.

        Returns:
            float: 読込んだ値
        """
        return struct.unpack('>f', self.read(4))[0]

    def read_char(self) -> str:
        """1文字(2バイト)のデータを読込む.

        Returns:
            bytes: 読込んだ文字のバイト列
        """
        char = struct.unpack('>H', self.read(2))[0]
        return chr(char)

    def read_object(self,
                    data: Optional['Serializable'] = None
                    ) -> Optional['Serializable']:
        """DoqueDBとの互換性を維持し、Serializableのサブクラスを読み込む.

        Args:
            data (Optional[Serializable]): データを格納するSerializableオブジェクト
        """
        # クラスIDの読込み
        id = self.read_int()

        object = None
        if data and data.class_id == id:
            # 引数の``data``からオブジェクトのインスタンスを得る
            object = data
        else:
            # クラスIDに対応したオブジェクトのインスタンスを得る
            object = Instance.get(id)

        if object is not None:
            # オブジェクトごとに実装されている`read_object()`を実行
            object.read_object(self)

        return object

    def close(self) -> None:
        """ストリームをクローズする.

        Notes:
            仮の実装
        """
        if self.__istream:
            self.__istream.close()
            self.__istream = None


class OutputStream():
    """DoqueDBとの互換性のある出力ストリームクラス.

    Args:
        socket_ (socket.socket): ソケット

    Attributes:
        __socket (socket.socket): ソケット
        __buffer (List[bytes]): 出力バッファー
    """

    def __init__(self, ostream: BinaryIO) -> None:
        self.__ostream: Optional[BinaryIO] = ostream

    def write(self, b: bytes) -> None:
        """ストリームに書き込む.

        Args:
            b (bytes): 書き込む値
        """
        assert b is not None

        if self.__ostream:
            self.__ostream.write(b)
        else:
            raise exceptions.UnexpectedError('OutputStream is closed')

    def flush(self) -> None:
        """出力をフラッシュする.
        """
        if self.__ostream:
            self.__ostream.flush()
        else:
            raise exceptions.UnexpectedError('OutputStream is closed')

    def write_int(self, v: int) -> None:
        """int型を書き込む.

        Args:
            v (int): 書き込む値
        """
        # ``v`` を4バイトに変換して書き込む
        self.write(struct.pack('>i', v))

    def write_short(self, v: int) -> None:
        """int型を書き込む.

        Args:
            v (int): 書き込む値
        """
        # ``v`` を2バイトに変換して書き込む
        self.write(struct.pack('>h', v))

    def write_long(self, v: int) -> None:
        """long型を書き込む.

        Args:
            v (int): 書き込む値
        """
        # ``v`` を8バイトに変換して書き込む
        self.write(struct.pack('>q', v))

    def write_double(self, v: float) -> None:
        """double型を書き込む.

        Args:
            v (float): 書き込む値
        """
        self.write(struct.pack('>d', v))

    def write_float(self, v: float) -> None:
        """float型を書き込む.

        Args:
            v (float): 書き込む値
        """
        self.write(struct.pack('>f', v))

    def write_char(self, v: int) -> None:
        """1文字書き込む.

        Args:
            v (int): 書き込む値
        """
        self.write(v.to_bytes(2, byteorder='big'))

    def write_object(self, object_: Optional['Serializable'] = None) -> None:
        """DoqueDBとの互換性を維持し、Serializableのサブクラス書き込む.

        Args:
            object_ (Serializable): 書き込むSerializableのサブクラス
        """
        if object_:
            # クラスIDを書き込む
            self.write_int(object_.class_id)
            # 中身を書き込む
            object_.write_object(self)
        else:
            self.write_int(ClassID.NONE.value)

    def close(self) -> None:
        """ストリームをフラッシュしてクローズする.
        """
        if self.__ostream:
            # 残っているバッファをフラッシュする
            self.flush()
            self.__ostream.close()
            self.__ostream = None
