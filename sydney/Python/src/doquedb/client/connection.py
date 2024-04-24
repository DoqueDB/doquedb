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
connection.py -- クライアントコネクション
"""
# TODO:is_database_availableの実装
from typing import Optional, List, TYPE_CHECKING
if TYPE_CHECKING:
    from .datasource import DataSource

from .object import Object
from .port import Port
from ..exception import exceptions
from ..common.serialdata import Request
from ..port.constants import ConnectionSlaveID
from ..common.arraydata import IntegerArrayData
from ..common.scalardata import IntegerData


class Connection(Object):
    """クライアントコネクションクラス.

    サーバとのコネクションを管理する. サーバ側にはこのクライアントコネクション１つごとにスレッドが存在している.
    """

    def __init__(self, datasource: 'DataSource', port: Port) -> None:
        super().__init__(Object.CONNECTION)
        self.__datasource = datasource
        self.__port: Optional[Port] = port

    def begin_connection(self) -> 'Connection':
        """新しいコネクションを得る.

        Returns:
            Connection: 新しいコネクション
        """
        assert self.__port is not None

        # [<-] リクエスト
        self.__port.write_object(
            Request(Request.request_map['BEGIN_CONNECTION']))
        self.__port.flush()
        # [->] スレーブIDを受け取る
        slave_data = self.__port.read_integerdata()

        # 新しい通信ポートを得る
        port = self.__datasource.new_port(slave_data.value)
        port.open()

        # [->] ステータス
        self.__port.read_status()
        # [->] 新しい通信ポートからステータスを得る
        port.read_status()

        return Connection(self.__datasource, port)

    def begin_worker(self) -> Port:
        """ワーカを起動する.

        Returns:
            Port: 接続ポート

        Raises:
            UnexpectedError: データソースから持ってきたポートが開放されていなかった場合

        Notes:
            ポートプールにポートがない場合、slaveID=ANYとしてDoqueDBから返ってくるポートを使用する.

        """
        assert self.__port is not None
        # ポートプールからポートを得る
        port = self.__datasource.pop_port()
        slave_id = ConnectionSlaveID.ANY.value

        try:
            # datasoureにportが登録されていればそれを用いる
            if port:
                slave_id = port.slave_id
                # slave_idがANYだった場合例外をあげる
                if slave_id == ConnectionSlaveID.ANY.value:
                    raise exceptions.UnexpectedError(
                        'tried to use invalid port from datasource')

            # [<-] リクエスト
            self.__port.write_object(
                Request(Request.request_map['BEGIN_WORKER']))
            # [<-] スレーブID
            self.__port.write_object(IntegerData(slave_id))
            self.__port.flush()

            # [->] スレーブID
            slave_data = self.__port.read_integerdata()
            # [->] ワーカID
            worker_data = self.__port.read_integerdata()
            # [->] ステータス
            self.__port.read_status()
        except Exception as err:
            if port:
                if err is exceptions.UnexpectedError:
                    # ConnectionSlaveIDがANYだった場合portは返さない
                    port.close()
                else:
                    # popしたportを戻す
                    self.__datasource.push_port(port)
            raise err

        if slave_id == ConnectionSlaveID.ANY.value:
            # 新しい通信ポート
            port = self.__datasource.new_port(slave_data.value)
            port.open()
        assert port is not None
        # ワーカIDを設定
        port.worker_id = worker_data.value

        return port

    def cancel_worker(self, worker_id: int) -> None:
        """ワーカをキャンセルする.

        Args:
            worker_id (int): キャンセルするワーカのID
        """
        assert self.__port is not None
        # [<-] リクエスト
        self.__port.write_object(Request(Request.request_map['CANCEL_WORKER']))
        # [<-] ワーカID
        self.__port.write_object(IntegerData(worker_id))
        self.__port.flush()

        # [->] ステータス
        self.__port.read_status()

    def disconnect_port(self, slave_id: List[IntegerData]) -> None:
        """使用しない通信ポートを切断する.

        Args:
            slave_id (List[int]): 切断するポートのスレーブIDのリスト
        """
        assert self.__port is not None
        # [<-] リクエスト
        self.__port.write_object(
            Request(Request.request_map['NO_REUSE_CONNECTION']))
        # [<-] スレーブIDの配列
        self.__port.write_object(IntegerArrayData(slave_id))
        self.__port.flush()

        # [->] ステータス
        self.__port.read_status()

    def is_serever_available(self) -> bool:
        """サーバの利用可能性を得る.

        Returns:
            bool: サーバが利用可能な場合はTrue, 利用不可能な場合はFalse
        """
        assert self.__port is not None
        # [<-] リクエスト
        self.__port.write_object(
            Request(Request.request_map['CHECK_AVAILABILITY']))
        # [<-] チェック対象
        self.__port.write_object(IntegerData(
            Request.request_map['AVAILABILITY_TARGET_SERVER']))

        # [->] チェック結果
        result = self.__port.read_integerdata()
        # [->] ステータス
        self.__port.read_status()

        return result.value == 1

    def is_database_available(self) -> bool:
        """任意のデータベースの利用可能性を得る.

        Returns:
            bool: データベースが利用可能な場合はTrue , 利用不可能な場合はFalse
        """
        # TODO: 実装
        pass

    def close(self) -> None:
        """クローズする.

        Notes:
            このメソッドは例外を送出しない.
        """
        if self.__port is None:
            return

        try:
            # [<-] リクエスト
            self.__port.write_object(
                Request(Request.request_map['END_CONNECTION']))
            self.__port.flush()
            # [->] ステータス
            self.__port.read_status()
        except Exception:
            # 例外は無視する
            pass

        try:
            self.__port.close()
        except Exception:
            # 例外は無視する
            pass
        self.__port = None
