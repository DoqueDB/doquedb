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
datasource.py -- クライアントのデータソース
"""
# TODO:
#    port_map, session_mapなどは外部で管理できるような実装に変える
#    :func: `new_client_connection`の実装変更
from typing import Optional, List, Dict

import socket

from ..exception import exceptions
from .object import Object
from .constants import ProtocolVersion
from .connection import Connection
from .session import Session
from .port import Port
from ..common.serialdata import Request
from ..common.scalardata import IntegerData, StringData
from ..port.constants import ConnectionSlaveID
from ..port.authorizemode import AuthorizeMode


class DataSource(Object):
    """データソースクラス.

    クライアント側のデータソースをあらわすクラス. 接続に必要な情報を保持する.

    Args:
        hostname (str): 接続先のホスト名
        portnum (int): ポート番号

    Attributes:
        __connection_list (list): コネクションのリスト
        __connection_element (int): 最近返したコネクションの要素番号
        __portmap (dict): ポートのハッシュマップ(辞書)
        __expunge_port (list): クライアントが廃棄したポート

        __hostname (str): 接続先のホスト名
        __portnumber (int): ポート番号
        __protocol_ver (int): プロトコルバージョン

        __sessionmap (dict): セッションのハッシュマップ(辞書)
        __is_closed (bool): クローズしたかどうか
        __masterID (int): マスターID(=プロトコルバージョン)
        __authorization (int): 認証方式
    """
    # 全データベースをあらわす ID
    DATABASE_ALL = 0xffffffff

    # １つのコネクションが管理する最大セッション数 (JDBCからの値)
    CONNECTION_THRESHOLD = 20
    # ポートプールチェック間隔の最小単位(ms) (JDBCからの値)
    TIME_UNIT = 500
    # ポートプールの最大ポート数 (JDBCからの値)
    MAXIMUM_CONNECTION_POOL_COUNT = 100
    # ポートプール数をチェックする間隔(ms) (JDBCからの値)
    CHECK_CONNECTION_POOL_PERIOD = 60000

    def __init__(self, hostname: str, portnum: int) -> None:
        super().__init__(Object.DATA_SOURCE)
        # メンバー変数の初期化
        self.__connection_list: List[Connection] = []
        self.__connection_element = 0
        self.__portmap: Dict[int, Port] = {}
        self.__expunge_port: List[int] = []
        self.__hostname = hostname
        self.__portnum = portnum
        self.__protocol_ver = 0
        self.__session_map: Dict[int, Session] = {}
        self.__is_closed = False
        self.__master_id = 0
        self.__authorization = 0

    @property
    def master_id(self) -> int:
        """マスターIDのゲッター"""
        return self.__master_id

    @property
    def authorization(self) -> int:
        """認証方式のゲッター."""
        return self.__authorization

    def open(self, protocol_ver: int) -> None:
        """データソースをオープンする.

        Args:
            protocol_ver(int): プロトコルバージョン
        """
        self.__protocol_ver = protocol_ver

        # 認証方式が指定されていない場合はパスワード認証を指定する
        if (self.__protocol_ver & AuthorizeMode.MaskMode.value) \
                == AuthorizeMode.NONE.value:
            self.__protocol_ver |= AuthorizeMode.PASSWORD.value

        # 新たにポートを得る
        port = self.new_port(ConnectionSlaveID.ANY.value)
        port.open()

        # [<-] リクエスト
        port.write_object(Request(Request.request_map['BEGIN_CONNECTION']))
        # [<-] ホスト名
        localhost = socket.gethostbyaddr(
            socket.gethostbyname(socket.gethostname()))[0]
        port.write_object(StringData(localhost))
        port.flush()
        # [->] ステータス
        port.read_status()
        # クライアントコネクションを作成する
        client_connection = Connection(self, port)
        # 配列に加える
        self.__connection_list.append(client_connection)
        # マスターIDを保存する
        self.__master_id = port.master_id
        # 認証方式を保存する
        self.__authorization = port.authorization

        self.__is_closed = False

    def create_session(self,
                       dbname: str,
                       username: Optional[str] = None,
                       password: Optional[str] = None
                       ) -> Session:
        """新しくセッションを作成する.

        Args:
            dbname(str): データベース名
            username(Optional[str]): ユーザー名
            password(Optional[str]): パスワード

        Raises:
            UnexpectedError: クライアントコネクションが取得できなかった場合

        TODO:
            セッション開始の例外処理の詳細実装
        """
        # プロトコルバージョン4以下はユーザー管理非対応
        if self.__master_id < ProtocolVersion.PROTOCOL_VERSION5.value:
            username = None
            password = None

        port: Optional[Port] = None

        # クライアントコネクションを得る.
        client_connection = self.get_client_connection()

        # ワーカの起動
        try:
            if client_connection is None:
                # 一旦クローズされているので再度 open する
                self.open(self.__protocol_ver)
                client_connection = self.get_client_connection()
            # ワーカを起動する
            if client_connection:
                port = client_connection.begin_worker()
            else:
                raise exceptions.UnexpectedError(
                    'failed to get client connection (1)')
        except Exception as err:
            if (self.session_exist()):
                # まだ利用中のセッションがあるので再接続しない
                raise err

            # サーバが再起動したかもしれないので、データソースを初期化する
            try:
                self.close()
                self.open(self.__protocol_ver)
                client_connection = self.get_client_connection()
                if client_connection:
                    port = client_connection.begin_worker()
                else:
                    raise exceptions.UnexpectedError(
                        'failed to get client connection (2)')
            except Exception as err:
                if port is None:
                    # 再初期化してもダメ
                    raise err

        assert port is not None
        session_id: Optional[IntegerData] = None

        # 引数にusernameとpasswordがあった場合
        if username and password:
            # セッションの開始
            try:
                # [<-] BeginSession
                port.write_object(
                    Request(Request.request_map['BEGIN_SESSION2']))
                # [<-] データベース名
                port.write_object(StringData(dbname))
                # [<-] ユーザー名
                port.write_object(StringData(username))
                # [<-] パスワード
                port.write_object(StringData(password))
                port.flush()

                # [->] セッション番号
                session_id = port.read_integerdata()
                # [->] ステータス
                port.read_status()
            except exceptions.DatabaseError as dbe:
                if port.is_reuse:
                    self.push_port(port)
                else:
                    port.close()
                raise dbe
            except exceptions.UnexpectedError as ue:
                port.close()
                raise ue
            except Exception as err:
                port.close()
                raise err

        # 引数がdbnameのみの場合
        else:
            assert username is None and password is None
            try:
                # [<-] BeginSession
                port.write_object(
                    Request(Request.request_map['BEGIN_SESSION']))
                # [<-] データベース名
                port.write_object(StringData(dbname))
                port.flush()

                # [->] セッション番号
                session_id = port.read_integerdata()
                # [->] ステータス
                port.read_status()
            except exceptions.DatabaseError as dbe:
                if port.is_reuse:
                    self.push_port(port)
                else:
                    port.close()
                raise dbe
            except exceptions.UnexpectedError as ue:
                port.close()
                raise ue
            except Exception as err:
                port.close()
                raise err

        # コネクションをプールする
        self.push_port(port)

        # セッションを新しく作成して登録する.
        session = Session(self, dbname, session_id.value, username)
        self.__session_map[session.id_] = session

        # 必要なら新しいクライアントコネクションを得る
        self.new_client_connection()

        return session

    def get_client_connection(self) -> Optional[Connection]:
        """コネクション管理クラスを得る.

        ラウンドロビン方式でコネクション管理クラスを入手する.

        Returns:
            Connection: コネクション管理クラス
        """
        if len(self.__connection_list) == 0:
            return None
        if self.__connection_element >= len(self.__connection_list):
            self.__connection_element = 0

        connection = self.__connection_list[self.__connection_element]
        self.__connection_element += 1
        return connection

    def new_client_connection(self) -> None:
        """セッション数がCONNECTION_THRESHOLDを越えた場合に、
        新しいクライアントコネクションを作成する.

        TODO:
            connection_listは今の実装では増え続けてしまうので、
            定期的に監視するように実装を変える必要がある。
        """
        size = len(self.__session_map)
        # CONNECTION_THRESHOLDをセッション数が越えたので、
        # 新しいクライアントコネクションを作る.
        if size >= self.CONNECTION_THRESHOLD * len(self.__connection_list):
            client_connection = self.get_client_connection()
            # 新しいクライアントコネクションを得る
            assert client_connection is not None
            new_client_connection = client_connection.begin_connection()
            # 配列に加える
            self.__connection_list.append(new_client_connection)

    def pop_port(self) -> Optional[Port]:
        """ポートプールからポートを取り出す.

        Returns:
            Port: ポートプールから受け取ったポート
        """
        port = None
        if len(self.__portmap) > 0:
            k, v = self.__portmap.popitem()
            port = v
        return port

    def push_port(self, port: Port) -> None:
        """ポートプールにポートを挿入する.

        Args:
            port_(Port): ポート
        """
        port.reset()
        self.__portmap[port.slave_id] = port

    def new_port(self, slave_id: int) -> Port:
        """新しいポートのインスタンスを得る

        Args:
            slave_id(int): スレーブID

        Returns:
            Port: 新しいポートインスタンス
        """
        return Port(
            self.__hostname, self.__portnum, self.__protocol_ver, slave_id)

    def expunge_port(self, port: Port) -> None:
        """廃棄したポートを登録する.

        Args:
            port (Port): 廃棄するポート
        """
        self.__expunge_port.append(port.slave_id)
        port.close()

    def session_exist(self) -> bool:
        """セッションが存在しているかどうか

        Returns:
            bool: 存在している場合True, それ以外はFalse
        """
        if len(self.__session_map) == 0:
            return False
        return True

    def remove_session(self, id: int) -> None:
        """セッションを削除する.
        """
        del self.__session_map[id]

    def is_server_available(self) -> bool:
        """サーバの利用可能性を得る.

        Returns:
            bool: サーバが利用可能な場合はTrue, 利用不可能な場合はFalse
        """
        # クライアントコネクションを得る
        client_connection = self.get_client_connection()
        assert client_connection is not None
        # 利用可能性を問い合わせる
        return client_connection.is_serever_available()

    def is_database_available(self) -> bool:
        """任意のデータベースの利用可能性を得る.

        Returns:
            bool: データベースが利用可能な場合はTrue , 利用不可能な場合はFalse
        """
        pass

    def shutdown(self,
                 username: Optional[str] = None,
                 password: Optional[str] = None
                 ) -> None:
        """サーバを停止する.
        """
        # 新たにポートを得る
        port = self.new_port(ConnectionSlaveID.ANY.value)
        port.open()

        old = False
        # ユーザ認証のあるサーバに対しての処理
        if username and password:
            try:
                # [<-] リクエスト
                port.write_object(Request(Request.request_map['SHUTDOWN2']))
                port.write_object(StringData(username))
                port.write_object(StringData(password))
                port.flush()
                # [->] ステータス
                port.read_status()
            except exceptions.DatabaseError:
                old = True
            finally:
                port.close()
        # 通常の処理
        else:
            # [<-] リクエスト
            port.write_object(Request(Request.request_map['SHUTDOWN']))
            port.flush()
            # [->] ステータス
            port.read_status()

            port.close()

        # 古いサーバと接続しているので、古い shutdown 処理を実行
        if old is True:
            self.shutdown()

    def close(self) -> None:
        """データソースをクローズする.
        """
        if self.__is_closed:
            return

        # 全てのセッションをクローズする
        for session in self.__session_map.values():
            session.close_internal()
        self.__session_map.clear()

        # 全てのコネクションをクローズする
        for i in range(len(self.__connection_list)):
            c = self.__connection_list[i]
            c.close()
        self.__connection_list.clear()

        # 全てのポートをクローズする
        for port in self.__portmap.values():
            port.close()
        self.__portmap.clear()
        self.__expunge_port.clear()

        self.__is_closed = True
