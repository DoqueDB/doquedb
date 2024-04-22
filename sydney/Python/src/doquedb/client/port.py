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
port.py -- クライアントポート
"""
from typing import Optional, TYPE_CHECKING
if TYPE_CHECKING:
    from ..common.serializable import Serializable

from ..exception import exceptions, raise_error
from .object import Object
from ..port.connection import RemoteClientConnection
from ..common.serialdata import Status, ErrorLevel, ExceptionData
from ..common.scalardata import IntegerData, StringData


class Port(Object):
    """クライアントポートクラス.

    Args:
        hostname (str): ホスト名
        portnum (int): ポート番号
        protocol_ver (int): プロトコルバージョン（=マスターID）
        slave_id (int): スレーブID

    Attributes:
        __connection (:obj: `port.Connection`): コネクション
        __worker_id (int): ワーカのID
        __reuse (bool): エラー時に再利用可能かどうか
    """

    def __init__(self,
                 hostname: str,
                 portnum: int,
                 protocol_ver: int,
                 slave_id: int
                 ) -> None:
        super().__init__(Object.PORT)

        # マスターIDは3
        self.__connection = RemoteClientConnection(
            hostname, portnum, protocol_ver, slave_id)

        self.__worker_id = -1
        self.__reuse = False

    @property
    def master_id(self) -> int:
        """マスターIDのゲッター."""
        return self.__connection.master_id

    @property
    def slave_id(self) -> int:
        """スレーブIDのゲッター."""
        return self.__connection.slave_id

    @property
    def worker_id(self) -> int:
        """ワーカIDのゲッター"""
        return self.__worker_id

    @worker_id.setter
    def worker_id(self, worker_id: int) -> None:
        """ワーカIDのセッター"""
        self.__worker_id = worker_id

    @property
    def authorization(self) -> int:
        """認証方式のゲッター."""
        return self.__connection.authorization

    @property
    def is_reuse(self) -> bool:
        return self.__reuse

    def open(self) -> None:
        """ポートのオープン.

        port.Connectionのオープン.
        """
        self.__connection.open()

    def close(self) -> None:
        """ポートのクローズ.

        port.Connectionのクローズ.
        """
        self.__connection.close()

    def read_object(self, data: Optional['Serializable'] = None
                    ) -> Optional['Serializable']:
        """オブジェクトを読み込む.

        Args:
            data (Serializable): データを格納するSerializableクラス

        Returns:
            Serializable: 読み込んだオブジェクト

        Raises:
            DataBaseError: データベース側が例外を返してきた場合
        """
        object_ = self.__connection.read_object(data)

        if isinstance(object_, ErrorLevel):
            level = object_
            self.__reuse = level.is_userlevel()

            # ErrorLevelの次は必ず例外
            object_ = self.__connection.read_object(data)
            assert isinstance(object_, ExceptionData)
            e = object_
            raise_error.RaiseClassInstance.raise_exception(e)
        elif isinstance(object_, ExceptionData):
            e = object_
            raise_error.RaiseClassInstance.raise_exception(e)

        return object_

    def read_integerdata(self) -> IntegerData:
        """IntegerDataを読込む.

        Returns:
            IntegerData: 読込んだIntegerData

        Raises:
            OperationalError: 読込んだObjectがIntegerDataではなかった場合
        """
        object_ = self.read_object()
        if not isinstance(object_, IntegerData):
            raise exceptions.OperationalError(
                f'excpected to read IntegerData but object is\
                     {object_.__class__.__name__}')
        return object_

    def read_stringdata(self) -> StringData:
        """StringDataを読込む.

        Returns:
            StringData: 読込んだStringData

        Raises:
            OperationalError: 読込んだObjectがStringDataではなかった場合
        """
        object_ = self.read_object()
        if not isinstance(object_, StringData):
            raise exceptions.OperationalError(
                f'excpected to read StringData but object is\
                     {object_.__class__.__name__}')
        return object_

    def read_status(self) -> int:
        """Statusを読込む.

        Returns:
            int: 読込んだStatusのタイプ

        Raises:
            OperationalError: 読込んだObjectがStatusではなかった場合
        """
        object_ = self.read_object()
        if not isinstance(object_, Status):
            raise exceptions.OperationalError(
                f'excpected to read Status but object is\
                     {object_.__class__.__name__}'
            )
        return object_.status

    def write_object(self, object_: Optional['Serializable'] = None) -> None:
        """オブジェクトを書き出す.

        Args:
            object_ (Serializable): 書き出すオブジェクト
        """
        self.__connection.write_object(object_)

    def flush(self) -> None:
        """出力をflushする.
        """
        self.__connection.flush()

    def reset(self):
        """再利用するためにリセットする.
        """
        self.__reuse = False
