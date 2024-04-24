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
cursor.py -- DB-API 2.0 のカーソルクラス
"""

from typing import Optional, List, Tuple, TYPE_CHECKING

if TYPE_CHECKING:
    from ..driver.connection import Connection


from ..exception import exceptions
from ..exception.database_exceptions import (
    ClassNotFound, ConnectionRanOut, Unexpected)
from ..client.resultset import ResultSet
from ..client.constants import ProtocolVersion
from ..common.arraydata import DataArrayData, ResultSetMetaData
from ..common.instance import Instance
from ..client.prepare_statement import PrepareStatement


class Cursor():
    """カーソルクラス.

    接続したデータベースとのセッションに対するカーソル機能を実装する.

    Attributes:
        __connection (:Obj: `Connection`): DB-APIのコネクションオブジェクト

        arraysize (int): fetchmany()で何行フェッチするかの設定変数. Defaults to １
        _rowcount (int): フェッチした結果に対するカーソルの位置. オペレーションがない場合は-1。
        _description (list):
            フェッチした行のカラムごとの特性を表す７つのシーケンスを、カラム数の分だけ保持するリスト.
            オペレーションがない場合はNone.
        __metadata: メタデータ
        __resultset (Optional[ResultSet]): リザルトセット
    """

    def __init__(self, connection: 'Connection') -> None:
        self._connection: Optional['Connection'] = connection
        self.__arraysize = 1
        self._description: List[Tuple[str, int, int, int, int, int, bool]] = []
        self.__metadata: Optional[ResultSetMetaData] = None
        self._resultset: Optional[ResultSet] = None
        self.__is_closed = False

    @property
    def arraysize(self) -> int:
        return self.__arraysize

    @arraysize.setter
    def arraysize(self, arraysize: int) -> None:
        if arraysize <= 0:
            raise exceptions.ProgrammingError(
                'arraysize should be greater than 1')

        self.__arraysize = arraysize

    @property
    def rowcount(self) -> int:
        """行番号のゲッター"""
        if self._resultset:
            return self._resultset.rowcount
        else:
            return -1

    @property
    def description(self) -> list:
        """カラムごとの特性を表す７つのシーケンスのリストのゲッター"""
        return self._description

    @property
    def is_closed(self) -> bool:
        """カーソルを閉じたかどうか"""
        return self.__is_closed

    def close(self) -> None:
        """カーソルオブジェクトをクローズする.
        """
        if self.__is_closed:
            return

        # リザルトセットが残っていた場合クローズする.
        if self._resultset and not self._resultset.is_closed:
            self._resultset.close()
            self._resultset = None

        self.arraysize = 1
        self._description.clear()
        self.__metadata = None

        self.__is_closed = True

    def execute(self, operation: str,
                parameters: Optional[Tuple] = None
                ) -> None:
        """オペレーションの実行メソッド.

        Args:
            operation (str): 実行するSQL文.
            parameters (Optional[Tuple[Any]]): SQL文に埋め込むパラメータのシーケンス.

        Raises:
            ProgrammingError: paramstyleがqmark以外だった場合に発生

        """
        if self.__is_closed:
            raise exceptions.ProgrammingError('cursor closed')

        # 前回のリザルトセットが残っていた場合クローズする
        if self._resultset:
            self._resultset.close()

        # プロトコルバージョン3以下はサポート外
        if self._connection is not None \
                and self._connection.master_id\
                < ProtocolVersion.PROTOCOL_VERSION4.value:
            raise exceptions.NotSupportedError(
                f'Protocol version{self._connection.master_id}\
                     is not supported')

        # オペレーションが空だった場合のエラー処理
        if not isinstance(operation, str) or len(operation) == 0:
            raise exceptions.ProgrammingError('bad argument')

        # ``parameters``を:obj: `DataArrayData`に変換
        parameters_ = None
        if parameters:
            parameters_ = DataArrayData()
            for param in parameters:
                param_ = Instance.get_data(type(param), param)
                parameters_.add_element(param_)

        try:
            assert self._connection is not None
            # トランザクション中でない場合、トランザクションを開始する
            if not self._connection.is_autocommit \
                    and not self._connection.in_transaction:
                self._connection.begin_transaction()

            # オペレーションを実行し, リザルトセットを生成する
            self._resultset = self._connection._session.execute(
                operation, parameters_)

        except IOError as ioe:
            raise ConnectionRanOut() from ioe
        except ClassNotFound as cnf:
            raise Unexpected() from cnf

    def executemany(self,
                    operation: str,
                    param_sets: list
                    ) -> None:
        """複数のオペレーションの実行メソッド.

        受け取ったシーケンスの数だけオペレーションを実行する。

        Args:
            operation (str): 実行するSQL文.
            param_sets (list): SQL文に埋め込むパラメータのシーケンスのシーケンス.

        Examples:
            テーブル(test_table)に対してインサートを２回行う場合の例

            >>> query = "INSERT INTO test_table VALUES (?, ?, ?)"
            >>> param_sets = [(1, 'MONA', 3000),　(2, 'XP', 1000),]
            >>> executemany(query, param_sets)

        """
        # パラメータのリストが空だった場合のエラー処理
        if param_sets is None or len(param_sets) == 0:
            raise exceptions.ProgrammingError('bad argument')

        for params in param_sets:
            self.execute(operation, params)

    def fetchone(self) -> Optional[tuple]:
        """リザルトセットの次の行を取得する.

        Returns:
            Optional[tuple]: 取得した行. 取得する行がなくなった場合 None.

        """
        if self._resultset is None:
            raise exceptions.ProgrammingError('no results to read')

        # 1行読込む
        has_more_data = self._resultset.next()

        # 読込に失敗した場合の処理
        if not has_more_data:
            return None

        # 読込に成功した場合の処理
        if self.__metadata is None:
            # ``self._description``にメタデータを登録する
            self.__metadata = self._resultset.metadata
            assert self.__metadata is not None
            self._description = []
            for i in range(len(self.__metadata)):
                element = self.__metadata[i]
                column_description = (
                    element.colname,
                    element.type,
                    element.displaysize,
                    0,  # 内部データサイズはメタデータにない
                    element.precision,
                    element.scale,
                    element.isnot_nullable
                )

                self._description.append(column_description)

        return self._resultset.get_row_as_tuple()

    def fetchmany(self, size: int = None) -> Optional[list]:
        """リザルトセットを複数行取得する.

        size に指定された数だけ結果をフェッチする. size が指定されなかった場合 .arraysize を参照する.

        Args:
            size (int, optional): 取得する行数. Defaults to None.

        Returns:
            rows (list): 取得した行のリスト.

        """
        if self._resultset is None:
            raise exceptions.ProgrammingError('no results to read')

        # 引数チェック
        if size is not None and size < 1:
            raise exceptions.ProgrammingError(
                'size should be greater or equal to 1')

        rows = None
        rows_ = []
        cnt = (size or self.__arraysize)
        while True:
            row = self.fetchone()
            cnt -= 1
            rows_.append(row)

            # 終了条件
            if cnt <= 0 or row is None:
                break

        if rows_ != [None]:
            rows = rows_

        return rows

    def fetchall(self) -> Optional[list]:
        """残っているリザルトセットを全て取得する.

        Returns:
            rows (list): 取得した行のリスト. Defaults to None.
        """
        if self._resultset is None:
            raise exceptions.ProgrammingError('no results to read')

        rows = None
        rows_ = []
        while True:
            row = self.fetchone()

            # 終了条件
            if row is None:
                break

            # リストに行を追加
            rows_.append(row)

        if rows_ != []:
            rows = rows_

        return rows

    def setinputsize(self, sizes: list) -> None:
        """.execute に渡すパラメータのためのメモリ領域を準備する.

        Args:
            sizes (list): パラメータごとの型を保持するシーケンス.

        Raises:
            NotSupportedError: サポート外のメソッド呼び出し時に発生

        Notes:
            未実装

        """
        raise exceptions.NotSupportedError("not supported")

    def setoutputsize(self, size: int, column=None) -> None:
        """サイズが大きいカラムのためにバッファーを定義する.

        Args:
            size (int): バッファーサイズ
            column: バッファーを設定するカラム. Defaults to None.

        Raises:
            NotSupportedError: サポート外のメソッド呼び出し時に発生

        Notes:
            未実装

        """
        raise exceptions.NotSupportedError("not supported")

    def callproc(self, procname: str, parameters: list) -> None:
        """ストアドプロシージャを呼び出す.

        Args:
            procname (str): ストアドプロシージャ名
            parameters (list): 埋め込むパラメータのシーケンス.

        Raises:
            NotSupportedError: サポート外のメソッド呼び出し時に発生

        Notes:
            DoqueDBはストアドプロシージャをサポートしていない.
        """
        raise exceptions.NotSupportedError("does not support stored procedure")

    def cancel(self) -> None:
        """実行をキャンセルする.
        """
        raise exceptions.NotSupportedError("not supported")


class CursorPrepared(Cursor):
    """プリペアステートメントに対応したカーソルクラス.
    """

    def __init__(self, connection: 'Connection') -> None:
        super().__init__(connection)

    def execute(self, operation: str,
                parameters: Optional[Tuple] = None
                ) -> None:
        """オペレーションの実行メソッド.

        Args:
            operation (str): 実行するSQL文.
            parameters (Optional[Tuple[Any]]): SQL文に埋め込むパラメータのシーケンス.

        Raises:
            ProgrammingError: paramstyleがqmark以外だった場合に発生

        """
        if self.is_closed:
            raise exceptions.ProgrammingError('cursor closed')

        # 前回のリザルトセットが残っていた場合クローズする
        if self._resultset:
            self._resultset.close()

        # プロトコルバージョン3以下はサポート外
        if self._connection is not None \
                and self._connection.master_id\
                < ProtocolVersion.PROTOCOL_VERSION4.value:
            raise exceptions.NotSupportedError(
                f'Protocol version{self._connection.master_id}\
                     is not supported')

        # オペレーションが空だった場合のエラー処理
        if not isinstance(operation, str) or len(operation) == 0:
            raise exceptions.ProgrammingError('bad argument')

        # ``parameters``を:obj: `DataArrayData`に変換
        parameters_ = DataArrayData()
        if parameters:
            for param in parameters:
                param_ = Instance.get_data(type(param), param)
                parameters_.add_element(param_)

        # プリペアードステートメントが存在しない場合、新しく作成する
        prepared_map = self._connection._session.prepared_map

        try:
            prepared = prepared_map[operation]
        except KeyError:
            # プリペアードステートメントを新しく作成する
            prepared = PrepareStatement.create(
                self._connection._session, operation)
            # プリペアードステートメントをマップに登録する
            prepared_map[operation] = prepared

        # プリペアードステートメントを実行する
        try:
            assert self._connection is not None
            # トランザクション中でない場合、トランザクションを開始する
            if not self._connection.is_autocommit \
                    and not self._connection.in_transaction:
                self._connection.begin_transaction()

            # オペレーションを実行し, リザルトセットを生成する
            self._resultset = prepared.execute(
                self._connection._session, parameters_)

        except IOError as ioe:
            raise ConnectionRanOut() from ioe

        except ClassNotFound as cnf:
            raise Unexpected() from cnf
