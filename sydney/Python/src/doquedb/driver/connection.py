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
connection.py -- DB-API 2.0 のコネクションクラス
"""
from enum import IntEnum
from typing import Optional

from ..driver.cursor import Cursor, CursorPrepared
from ..client.constants import StatusSet
from ..client.session import Session
from ..exception import exceptions


class TransactionMode(IntEnum):
    """トランザクションモードの列挙型クラス
    """
    # READ WRITE トランザクションモード。
    TRANSACTION_MODE_READ_WRITE = 1
    # READ ONLY トランザクションモード。
    TRANSACTION_MODE_READ_ONLY = 2
    # READ ONLY USING SNAPSHOT トランザクションモード。
    TRANSACTION_MODE_READ_ONLY_USING_SNAPSHOT = 3


class TransactionIsolationLevel(IntEnum):
    """トランザクション遮断レベルの列挙型クラス
    """
    # ダーティリード, ノンリピータブルリード, ファントムリードを許可する
    TRANSACTION_READ_UNCOMMITTED = 0x1
    # ダーティリードを不許可
    TRANSACTION_READ_COMMITTED = 0x2
    # ダーティリード, ノンリピータブルリードを不許可
    TRANSACTION_REPEATABLE_READ = 0x4
    # ダーティリード, ノンリピータブルリード, ファントムリードを不許可
    TRANSACTION_SERIALIZABLE = 0x8

    # READ ONLY USING SNAPSHOT トランザクション遮断レベル
    TRANSACTION_USING_SNAPSHOT = 0x100


class Connection():
    """DB-API 2.0 のコネクションクラス.
    特定のデータベースとの接続 (セッション) を表現します。

    Args:
        hostname (str): ホスト名
        portnum (int): ポート番号
        protocol_ver (int): プロトコルバージョン
        session (Session): セッション
        user (str): ユーザー名
        password (str): パスワード
        master_id (int): マスターID
        charset (str): 文字セット
        autocommit (bool): オートコミットのオンオフ. デフォルトはFalse

    Attributes:
        info (list): ホスト名、ポート番号、プロトコルバージョンのリスト
        _session (Session): セッション
        username (str): ユーザー名
        password (str): パスワード
        master_id (int): マスターID
        charset (str): 文字セット
        in_autocommit (bool): オートコミットのオンオフ
        is_closed (bool): クローズしたかどうか
        readonly (bool): 読込み専用モードかどうか
        in_transaction (bool): トランザクション中かどうか
        isolation_level (bool): トランザクション遮断レベル
    """
    # トランザクションモードの列挙型クラス
    mode = TransactionMode

    # トランザクション遮断レベルの列挙型クラス
    isolevel = TransactionIsolationLevel

    def __init__(self,
                 hostname: str,
                 portnum: int,
                 protocol_ver: int,
                 session: Session,
                 user: Optional[str],
                 password: Optional[str],
                 master_id: int,
                 charset: str,
                 autocommit: bool = False
                 ) -> None:
        self.__hostname = hostname
        self.__portnum = portnum
        self.__protocol_ver = protocol_ver
        self._session = session
        self.__user = user
        self.__password = password
        self.__master_id = master_id
        self.__charset = charset
        self.__autocommit = autocommit

        self.__is_closed = False
        self.__readonly = False
        self.__set_readmode = False
        self.__in_transaction = False
        self.__isolation_level = self.isolevel.TRANSACTION_READ_COMMITTED.value

        self.__cursor: Cursor = None

    @property
    def info(self) -> tuple:
        """接続情報（ホスト名, ポート番号, プロトコルバージョン）のタプル"""
        return (self.__hostname, self.__portnum, self.__protocol_ver)

    @property
    def username(self) -> Optional[str]:
        """ユーザー名のゲッター"""
        return self.__user

    @property
    def password(self) -> Optional[str]:
        """パスワードのゲッター"""
        return self.__password

    @property
    def master_id(self) -> int:
        """マスターIDのゲッター"""
        return self.__master_id

    @property
    def charset(self) -> str:
        """文字コードのゲッター."""
        return self.__charset

    @property
    def is_autocommit(self) -> bool:
        """オートコミットのオンオフのゲッター"""
        return self.__autocommit

    @property
    def in_transaction(self) -> bool:
        """トランザクションのオンオフのゲッター"""
        return self.__in_transaction

    @property
    def readonly(self) -> bool:
        """"読込み専用モードかどうか"""
        return self.__readonly

    @property
    def isolation_level(self) -> int:
        """"トランザクション遮断レベルのゲッター"""
        return self.__isolation_level

    def close(self) -> None:
        """セッションのクローズ

        自動解除を待たずにDoqueDBとのセッションのクローズ処理を直ちに行う。
        """
        if not self.__is_closed:
            # カーソルのクローズ
            if self.__cursor and not self.__cursor.is_closed:
                self.__cursor.close()
            # セッションの解放
            self._session.close()
            self.__is_closed = True

    def commit(self) -> None:
        """現在のトランザクションをコミットする.

        Raises:
            ProgrammingError: プログラミングの誤りによって発生するエラー
            InterfaceError: 通信プロトコル周りのエラー時に発生
        """
        # オートコミットがオンまたはコネクションがクローズされていた場合のエラー
        if self.__autocommit:
            raise exceptions.ProgrammingError('autocommit is on. create new\
                 connection with autocommit False to use this method')
        if self.__is_closed:
            raise exceptions.ProgrammingError(
                'connection is closed. create new connection')

        if self.__in_transaction:
            if self.__cursor and self.__cursor._resultset:
                # リザルトセットがカーソルに残っているのでクローズする
                self.__cursor._resultset.close()
            try:
                # コミットしてステータス確認のためにリザルトセットを得る
                resultset = self._session.execute('commit')
                if resultset.get_status() == StatusSet.ERROR.value:
                    raise exceptions.InterfaceError(
                        'status error returned from doquedb')

                self.__in_transaction = False
            except Exception as err:
                raise exceptions.InterfaceError('failed to commit') from err

    def rollback(self) -> None:
        """現在のトランザクションをロールバックする.

        Raises:
            exceptions.InterfaceError: 通信プロトコル周りのエラー時に発生

        """
        # オートコミットがオンまたはコネクションがクローズされていた場合のエラー
        if self.__autocommit:
            raise exceptions.ProgrammingError('autocommit is on')
        if self.__is_closed:
            raise exceptions.ProgrammingError('connection is closed')

        if self.__in_transaction:
            if self.__cursor and self.__cursor._resultset:
                # リザルトセットがカーソルに残っているのでクローズする
                self.__cursor._resultset.close()
            try:
                # ロールバックしてステータス確認のためにリザルトセットを得る
                resultset = self._session.execute('rollback')
                if resultset.get_status() == StatusSet.ERROR.value:
                    raise exceptions.InterfaceError(
                        'status error returned from doquedb')

                self.__in_transaction = False
            except Exception as err:
                raise exceptions.InterfaceError('failed to rollback') from err

    def cursor(self, prepared: Optional[bool] = False
               ) -> Cursor:
        """カーソルの生成

        データベース操作のためのカーソルオブジェクトを生成する

        Args:
            prepared (Optional[bool]): プリペアードステートメント利用フラグ. デフォルトはFalse

        Returns:
            Cursor: カーソルオブジェクト

        Raises:
            ProgrammingError: 複数のカーソルを作成しようとした場合に発生

        Notes:
            カーソルはコネクションに対して１つしか作成できない。
            複数のカーソルを同時に作成したい場合はコネクションを新しく生成する必要がある。
        """
        # Connectionに対してCursorは常に一つ
        if self.__cursor is not None and not self.__cursor.is_closed:
            raise exceptions.ProgrammingError(
                'cursor already exists. make new connection or \
                    close cursor before creating new cursor')

        # プリペアードステートメントが指定された場合
        if prepared is True:
            self.__cursor = CursorPrepared(self)
        # 指定がない場合デフォルトのCursorクラスを作成
        else:
            self.__cursor = Cursor(self)

        return self.__cursor

    def begin_transaction(self,
                          transaction_mode: Optional[int]
                          = None
                          ) -> None:
        """トランザクションを開始する.

        Args:
            transaction_mode: :obj: `TransactionMode`内のいずれかのトランザクションモード

        Notes:
            このメソッドは自動コミットモードが無効の時にのみ使用する.
        """
        if self.__autocommit:
            raise exceptions.ProgrammingError('autocommit is on')

        if transaction_mode is None:
            transaction_mode = \
                TransactionMode.TRANSACTION_MODE_READ_WRITE.value
            if self.__readonly:
                if self.__isolation_level ==\
                        self.isolevel.TRANSACTION_USING_SNAPSHOT.value:
                    transaction_mode = self.mode.\
                        TRANSACTION_MODE_READ_ONLY_USING_SNAPSHOT.value
                else:
                    transaction_mode = \
                        self.mode.TRANSACTION_MODE_READ_ONLY.value

        operation = 'start transaction'

        # ``operation``末尾にトランザクションモードを追加
        if transaction_mode == \
                self.mode.TRANSACTION_MODE_READ_WRITE.value:
            operation = operation + ' read write'
        elif transaction_mode == \
                self.mode.TRANSACTION_MODE_READ_ONLY.value:
            operation = operation + ' read only'
        elif transaction_mode == \
                self.mode.TRANSACTION_MODE_READ_ONLY_USING_SNAPSHOT.value:
            operation = operation + ' read only, using snapshot'
        else:
            raise exceptions.ProgrammingError(
                f'argument {transaction_mode} not valid. Available transactions modes are listed in Connection.mode')

        # ``operation``末尾にトランザクション遮断レベルを追加
        if self.__isolation_level == \
                self.isolevel.TRANSACTION_READ_COMMITTED.value:
            operation = operation + ', isolation level read committed'
        elif self.__isolation_level == \
                self.isolevel.TRANSACTION_READ_UNCOMMITTED.value:
            operation = operation + ', isolation level read uncommitted'
        elif self.__isolation_level == \
                self.isolevel.TRANSACTION_REPEATABLE_READ.value:
            operation = operation + ', isolation level repeatable read'
        elif self.__isolation_level == \
                self.isolevel.TRANSACTION_SERIALIZABLE.value:
            operation = operation + ', isolation level serializable'
        elif self.__isolation_level == \
                self.isolevel.TRANSACTION_USING_SNAPSHOT.value:
            # Snapshotが設定されていた場合、isolevelは指定しない
            pass
        else:
            assert False

        # トランザクション開始のためのクエリを送信
        r = self._session.execute(operation)

        if r.get_status() == StatusSet.ERROR.value:
            raise exceptions.UnexpectedError(
                'status error returned from doquedb')

        self.__in_transaction = True

    def set_readonly(self, readonly_: bool) -> None:
        """読込み専用モードをセットする.

        Args:
            readonly_ (bool): 読込み専用モードの設定値

        Raises:
            ProgrammingError
            UnexpectedError
        """
        if self.__is_closed:
            raise exceptions.ProgrammingError('connection closed')
        if self.__in_transaction:
            raise exceptions.ProgrammingError('already in transaction')
        if self.__isolation_level == self.isolevel.TRANSACTION_USING_SNAPSHOT.value:
            if readonly_:
                return
            else:
                raise exceptions.ProgrammingError('incompatible transaction')

        # まだモードを設定していない場合、または、モードが違う場合のみ実行する
        if not self.__set_readmode or self.__readonly != readonly_:
            if readonly_:
                sql = 'set transaction read only'
            else:
                sql = 'set transaction read write'

            r = self._session.execute(sql)

            if r.get_status() == StatusSet.ERROR.value:
                raise exceptions.UnexpectedError(
                    'status error returned from doquedb')

            self.__set_readmode = True

        self.__readonly = readonly_

    def set_transaction_isolation(self, isolation_level: int) -> None:
        """トランザクション遮断レベルを指定されたものに変更する.

        Args:
            isolation_level (int): :obj: `TransactionIsolationLevel`内のいずれかの定数値

        Raises:
            ProgrammingError
            UnexpectedError
        """
        if self.__is_closed:
            raise exceptions.ProgrammingError('Connection already closed')

        if self.__in_transaction:
            raise exceptions.ProgrammingError('already in transaction')

        operation = 'set transaction isolation level '

        if isolation_level == \
                self.isolevel.TRANSACTION_READ_COMMITTED.value:
            operation = operation + 'read committed'
        elif isolation_level == \
                self.isolevel.TRANSACTION_READ_UNCOMMITTED.value:
            operation = operation + 'read uncommitted'
        elif isolation_level == \
                self.isolevel.TRANSACTION_REPEATABLE_READ.value:
            operation = operation + 'repeatable read'
        elif isolation_level == \
                self.isolevel.TRANSACTION_SERIALIZABLE.value:
            operation = operation + 'serializable'
        elif isolation_level == \
                self.isolevel.TRANSACTION_USING_SNAPSHOT:
            operation = 'set transaction read only, using snapshot'
            # SNAPSHOTはRead OnlyがTrueの場合のみ
            self.set_readonly(True)
        else:
            raise exceptions.ProgrammingError(
                f'argument {isolation_level} not valid. Available isolation levels are listed in Connection.isolevel')

        # isolevelの設定のためのクエリを送信
        r = self._session.execute(operation)

        if r.get_status() == StatusSet.ERROR.value:
            # UnexpectedErrorに変える
            raise exceptions.UnexpectedError(
                'status error returned from doquedb')

        self.__isolation_level = isolation_level
