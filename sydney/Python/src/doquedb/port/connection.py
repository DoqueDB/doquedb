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
connection.py -- DoqueDBとのコネクション機能を実装するモジュール
"""

from abc import ABC, abstractmethod
from ..port import authorizemode
from typing import Optional, TYPE_CHECKING
if TYPE_CHECKING:
    from ..common.serializable import Serializable

import socket

from ..common.iostream import InputStream, OutputStream
from .constants import ConnectionType, ConnectionSlaveID
from ..exception import exceptions


class Connection(ABC):
    """DoqueDBとのコネクションの基底クラス.

    すべてのコネクションクラスはこのクラスを継承する必要がある.

    Args:
        connection_type (int): コネクションのタイプ
        master_id (int): マスターID(=プロトコルバージョン)
        slave_id(int): スレーブID(=ポート番号)

    Attributes:
        __connection_type (int): コネクションのタイプ
        __master_id (int): マスターID(=プロトコルバージョン)
        __slave_id(int): スレーブID(=ポート番号)
        _inputstream (InputStream): 入力ストリーム
        _outputstream (OutputStream): 出力ストリーム
    """

    def __init__(self,
                 connection_type: int,
                 master_id: int,
                 slave_id: int
                 ) -> None:
        self.__connection_type = connection_type
        self.__master_id = master_id
        self.__slave_id = slave_id
        self._inputstream: Optional[InputStream] = None
        self._outputstream: Optional[OutputStream] = None

    @property
    def type(self) -> int:
        """コネクションのタイプのゲッター"""
        return self.__connection_type

    @property
    def master_id(self) -> int:
        """マスターIDのゲッター."""
        return self.__master_id

    @master_id.setter
    def master_id(self, master_id: int) -> None:
        self.__master_id = master_id

    @property
    def slave_id(self) -> int:
        """スレーブIDのゲッター."""
        return self.__slave_id

    @slave_id.setter
    def slave_id(self, slave_id: int) -> None:
        """スレーブIDのセッター"""
        self.__slave_id = slave_id

    @property
    def authorization(self) -> int:
        """認証方式のゲッター"""
        return self.__master_id & authorizemode.AuthorizeMode.MaskMode.value

    @abstractmethod
    def open(self) -> None:
        """コネクションをオープンする.

        Notes:
            継承先で実装
        """
        pass

    @abstractmethod
    def close(self) -> None:
        """コネクションをクローズする.

        Notes:
            継承先で実装
        """
        pass

    def read_object(self, data: Optional['Serializable'] = None
                    ) -> Optional['Serializable']:
        """オブジェクトを読み込む.
        """
        assert self._inputstream is not None
        if data:
            return self._inputstream.read_object(data)
        else:
            return self._inputstream.read_object()

    def write_object(self, object_: Optional['Serializable'] = None) -> None:
        """オブジェクトを読み込む.
        """
        assert self._outputstream is not None
        self._outputstream.write_object(object_)

    def flush(self) -> None:
        """出力をフラッシュする.
        """
        assert self._outputstream is not None
        self._outputstream.flush()


class RemoteClientConnection(Connection):
    """DoqueDBとのリモートクライアントコネクション.

    Args:
        hostname (str): ホスト名
        portnum (int): ポート番号
        master_id (int): マスターID
        slave_id (int): スレーブID

    Attributes:
        __hostname (str): ホスト名
        __portnum (str): ポート番号
        __socket (socket.socket): ソケット
        __ipmap (dict): 接続に成功したIPアドレスのマップ
    """

    def __init__(self,
                 hostname: str,
                 portnum: int,
                 master_id: int,
                 slave_id: int
                 ) -> None:
        super().__init__(ConnectionType.REMOTE.value, master_id, slave_id)
        self.__hostname = hostname
        self.__portnum = portnum
        self.__socket: Optional[socket.socket] = None
        self.__is_opened = False

    @property
    def is_opened(self) -> bool:
        return self.__is_opened

    def open(self) -> None:
        """コネクションをオープンする.
        """
        if self.__is_opened:
            raise exceptions.UnexpectedError('connection is already opened')

        try:
            # ソケットの作成
            self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.__socket.connect((self.__hostname, self.__portnum))

            # ストリームの作成
            istream = self.__socket.makefile(mode='rb', buffering=4096)
            ostream = self.__socket.makefile(mode='wb', buffering=4096)

            self._inputstream = InputStream(istream)
            self._outputstream = OutputStream(ostream)

            # [<-] マスターID
            self._outputstream.write_int(self.master_id)
            # [<-] スレーブID
            self._outputstream.write_int(self.slave_id)
            self._outputstream.flush()

            # [->] マスターID
            master_id = self._inputstream.read_int()
            # [->] スレーブID
            slave_id = self._inputstream.read_int()
            # チェック
            if not ConnectionSlaveID.is_normal(slave_id):
                raise exceptions.InterfaceError('connect doquedb failed.')

            self.__is_opened = True

        except Exception as err:
            self.close()
            raise err

        # マスターID, スレーブIDの設定
        self.master_id = master_id
        self.slave_id = slave_id

    def close(self):
        """コネクションをクローズする.
        """
        if self.__is_opened is False:
            return

        if self._inputstream:
            self._inputstream.close()
            self._inputstream = None
        if self._outputstream:
            self._outputstream.close()
            self._outputstream = None

        if self.__socket:
            try:
                self.__socket.close()
            except OSError:
                pass
            self.__socket = None

        self.__is_opened = False
