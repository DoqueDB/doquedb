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
session.py -- クライアントセッション
"""
# TODO:
#     :func: `get_database_product_ver()`の実装
from typing import Optional, Dict, TYPE_CHECKING

if TYPE_CHECKING:
    from .datasource import DataSource

from ..exception import exceptions, database_exceptions
from .object import Object
from .constants import ProtocolVersion
from .resultset import ResultSet
from ..common.arraydata import DataArrayData
from ..common.scalardata import IntegerData, StringData
from ..common.serialdata import Request
from .prepare_statement import PrepareStatement


class Session(Object):
    """セッションクラス.

    コネクションに対してセッションを作成する。

    Args:
        datasource (:obj: `DataSource`): データソース
        dbname (str): データベース名
        username (Optional[str]): ユーザー名
        session_id (int): セッションID

    Attributes:
        __datasource (:obj: `DataSource`): データソース
        __dbname (str): データベース名
        __username (str): ユーザー名
        __session_id (int): セッションID
    """

    def __init__(self,
                 datasource: 'DataSource',
                 dbname: str,
                 session_id: int,
                 username: Optional[str] = None
                 ) -> None:
        super().__init__(Object.SESSION)
        self.__datasource = datasource
        self.__dbname = dbname
        self.__username = username
        self.__session_id = session_id
        self.__prepared_map: Dict[int, PrepareStatement] = {}

    @property
    def datasource(self) -> 'DataSource':
        """データソースのゲッター"""
        return self.__datasource

    @property
    def dbname(self) -> str:
        """データベース名のゲッター"""
        return self.__dbname

    @property
    def username(self) -> Optional[str]:
        """ユーザー名のゲッター"""
        return self.__username

    @property
    def id_(self) -> int:
        """セッションIDのゲッター"""
        return self.__session_id

    @property
    def prepared_map(self) -> Dict[str, PrepareStatement]:
        """プリペアステートメントのマップのゲッター"""
        return self.__prepared_map

    @property
    def is_valid(self) -> bool:
        """セッションが有効かどうか
        """
        return False if self.__session_id == 0 else True

    def execute(self,
                statement: str,
                parameters: Optional[DataArrayData] = None
                ) -> ResultSet:
        """オペレーションを実行する.

        Args:
            operation (str): 実行するSQL文.
            param_sets (list): SQL文に埋め込むパラメータのシーケンスのシーケンス.

        Returns:
            resultset

        Raises:
            Exception: IOErrorなどpythonのシステムエラー
        """
        if not self.is_valid:
            raise exceptions.InterfaceError
        # クライアントコネクションを得る
        client_connection = self.__datasource.get_client_connection()
        if client_connection is None:
            raise exceptions.OperationalError('connection does not exist')

        # ワーカを起動する
        port = client_connection.begin_worker()

        try:
            # [<-] リクエスト
            port.write_object(
                Request(Request.request_map['EXECUTE_STATEMENT']))
            # [<-] セッションID
            port.write_object(IntegerData(self.__session_id))
            # [<-] SQL文
            port.write_object(StringData(statement))
            # [<-] パラメータ
            port.write_object(parameters)
            port.flush()
        except Exception as exp:
            port.close()
            raise exp

        # リザルトセットを返す
        return ResultSet(self.__datasource, port)

    def execute_prepare(self,
                        prepare: PrepareStatement,
                        parameter: Optional[DataArrayData] = None
                        ) -> ResultSet:
        """プリペアしたSQL文を実行する.

        Args:
            prepare (PrepareStatement): プリぺステートメント
            parameter (DataArrayData): パラメータ

        Raises:
            Exception: IOErrorなどpythonのシステムエラー

        Returns:
            ResultSet: リザルトセット
        """
        if self.__session_id == 0:
            raise database_exceptions.SessionNotAvailable()

        # クライアントコネクションを得る
        client_connection = self.__datasource.get_client_connection()
        if client_connection is None:
            raise exceptions.OperationalError('connection does not exist')

        # ワーカを起動する
        port = client_connection.begin_worker()

        try:
            # [<-] リクエスト
            port.write_object(
                Request(Request.request_map['EXECUTE_PREPARE_STATEMENT']))
            # [<-] セッションID
            port.write_object(IntegerData(self.__session_id))
            # [<-] SQL文
            port.write_object(IntegerData(prepare.prepare_id))
            # [<-] パラメータ
            port.write_object(parameter)
            port.flush()
        except Exception as exp:
            port.close()
            raise exp

        # リザルトセットを返す
        return ResultSet(self.__datasource, port)

    def create_prepare_statement(self, statement: str) -> PrepareStatement:
        """新しくプリペアステートメントを作成する.

        Args:
            statement (str): SQL文

        Raises:
            ClassNotFound: 指定したIDを持つクラスがない
            DatabaseError: DoqueDBから返却されたエラー
            Exception: その他IOErrorなどpythonのシステムエラー

        Returns:
            PrepareStatement: 新しく作成したプリペアステートメント
        """
        # プロトコルバージョン２以前はサポート外
        if self.datasource.master_id < ProtocolVersion.PROTOCOL_VERSION3.value:
            raise exceptions.NotSupportedError(
                'protocol version older than 3 is not supported')

        # クライアントコネクションを得る
        client_connection = self.__datasource.get_client_connection()
        if client_connection is None:
            raise exceptions.OperationalError('connection does not exist')

        # ワーカを起動する
        port = client_connection.begin_worker()

        prepare_id: IntegerData = None

        try:
            # [<-] リクエスト
            port.write_object(
                Request(Request.request_map['PREPARE_STATEMENT2']))
            # [<-] セッションID
            port.write_object(IntegerData(self.__session_id))
            # [<-] SQL文
            port.write_object(StringData(statement))
            port.flush()

            # [->] PrepareID
            prepare_id = port.read_integerdata()
            # [->] ステータス
            port.read_status()
        except database_exceptions.ClassNotFound as cnf:
            port.close()
            raise cnf
        except exceptions.DatabaseError as dbe:
            if port.is_reuse:
                self.__datasource.push_port(port)
            else:
                port.close()
            raise dbe
        except Exception as exp:
            port.close()
            raise exp

        # コネクションをプールする
        self.__datasource.push_port(port)

        prepare = PrepareStatement(prepare_id.value)

        self.__prepared_map[statement] = prepare

        return prepare

    def erase_prepare_statement(self, prepare_id: int) -> None:
        """プリペアステートメントを削除する

        Args:
            prepare_id (int): プリペアステートメントID

        Raises:
            ClassNotFound: 指定したIDを持つクラスがない
            DatabaseError: DoqueDBから返却されたエラー
            Exception: その他IOErrorなどpythonのシステムエラー
        """
        # クライアントコネクションを得る
        client_connection = self.__datasource.get_client_connection()
        if client_connection is None:
            raise exceptions.OperationalError('connection does not exist')

        # ワーカを起動する
        port = client_connection.begin_worker()

        try:
            # [<-] リクエスト
            port.write_object(
                Request(Request.request_map['ERASE_PREPARE_STATEMENT2']))
            # [<-] セッションID
            port.write_object(IntegerData(self.__session_id))
            # [<-] SQL文
            port.write_object(IntegerData(prepare_id))
            port.flush()

            # [->] ステータス
            port.read_status()
        except database_exceptions.ClassNotFound as cnf:
            port.close()
            raise cnf
        except exceptions.DatabaseError as dbe:
            if port.is_reuse:
                self.__datasource.push_port(port)
            else:
                port.close()
            raise dbe
        except Exception as exp:
            port.close()
            raise exp

        # コネクションをプールする
        self.__datasource.push_port(port)

    def get_database_product_ver(self) -> str:
        """データベース製品のバージョンを取得する.

        Returns:
            str: データベースのバージョン

        TODO:
            中身の実装
        """
        pass

    def close_internal(self) -> int:
        """クローズする.

        Returns:
            int: セッションID

        Notes:
            このメソッドは例外を送出しない.
        """
        # プリペアードステートメントがある場合クローズする
        if any(self.__prepared_map):
            self.close_prepare()

        id_ = self.__session_id
        if self.is_valid:
            try:
                client_connection = self.__datasource.get_client_connection()

                # ワーカを起動し、セッションを終了する
                if client_connection:
                    # ワーカを起動する
                    port = client_connection.begin_worker()

                    # セッションを終了する
                    try:
                        # [<-] リクエスト
                        port.write_object(
                            Request(Request.request_map['END_SESSION']))
                        # [<-] セッションID
                        port.write_object(IntegerData(self.__session_id))
                        port.flush()

                        # [->] ステータス
                        port.read_status()
                    except Exception as err:
                        if port.is_reuse:
                            self.__datasource.push_port(port)
                        else:
                            port.close()
                        raise err

                    # ポートを返す
                    self.__datasource.push_port(port)
            except Exception:
                # 例外はあげない
                pass

            # ``session_id``をinvalid(=0)にする
            self.__session_id = 0

        return id_

    def close_prepare(self):
        """プリペアステートメントのクローズ
        """
        # プリペアードステートメントをクローズする
        for prepare in self.__prepared_map.values():
            if prepare.prepare_id != 0:
                prepare.close(self)
        self.__prepared_map.clear()

    def close(self):
        """セッションのクローズ.
        """
        if self.__session_id == 0:
            return

        id_ = self.close_internal()
        self.datasource.remove_session(id_)
