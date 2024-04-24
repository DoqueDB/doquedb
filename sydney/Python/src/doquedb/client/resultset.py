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
resultset.py -- クライアントリザルトセット
"""
from typing import Optional, TYPE_CHECKING
if TYPE_CHECKING:
    from .datasource import DataSource
    from .port import Port

from ..exception import exceptions
from .object import Object
from .constants import ProtocolVersion, StatusSet
from ..common.constants import ClassID
from ..common.serialdata import Status
from ..common.arraydata import DataArrayData, ResultSetMetaData
from ..common.scalardata import DecimalData, LanguageData
from ..common.data import WordData
from ..exception import database_exceptions


class ResultSet(Object):
    """リザルトセットクラス.

    Args:
        datasource (:obj: `DataSource`): データソース
        port (:obj: `Port`):　通信ポート

    Attributes:
        __datasource (:obj: `DataSource`): データソース
        __port (:obj: `Port`): 通信ポート
        __status: 通信のステータス
        __metadata (:obj: `ResultSetMetaData`): リザルトセットのメタデータ
        __tupledata (:obj: `DataArrayData`): 一行のデータ

        __status (int): 現在のステータス

        __row (:obj: `DataArrayData`): リザルトセットの現在の行データ
        __rowcount (int): 現在の行番号

        __is_closed(bool): クローズフラグ
    """

    def __init__(self, datasource: 'DataSource', port: 'Port') -> None:
        super().__init__(Object.RESULT_SET)
        self.__datasource = datasource
        self.__port: Optional['Port'] = port
        self.__status = StatusSet.DATA.value
        self.__metadata: Optional[ResultSetMetaData] = None
        self.__tupledata: Optional[DataArrayData] = None

        self.__row: Optional[DataArrayData] = None
        self.__rowcount = 0

        self.__is_closed = False

    @property
    def row(self) -> Optional[DataArrayData]:
        """行データのゲッター"""
        return self.__row

    @property
    def rowcount(self) -> int:
        """行データのゲッター"""
        return self.__rowcount

    @property
    def metadata(self) -> Optional[ResultSetMetaData]:
        """メタデータのゲッター"""
        return self.__metadata

    @property
    def last_status(self) -> int:
        """最後に取得した実行ステータスを得る"""
        return self.__status

    @property
    def is_closed(self) -> bool:
        """:obj: `ResultSet`を閉じたかどうか"""
        return self.__is_closed

    def get_status(self, skip_all: Optional[bool] = True) -> int:
        """実行ステータスを得る.

        Args:
            skipAll_ (bool): True   複文の全ての結果を読み捨てる
                             False  複文の1文の結果のみ読み捨てる

        Notes:
            実行ステータスのみを返す。サーバからデータを受け取っても読み捨てる。
        """
        while self.__status == StatusSet.META_DATA.value \
                or self.__status == StatusSet.DATA.value \
                or self.__status == StatusSet.END_OF_DATA.value \
                or (skip_all and
                    self.__status == StatusSet.HAS_MORE_DATA.value):
            self.get_next_tuple()

        return self.__status

    def get_next_tuple(self, tuple_: Optional[DataArrayData] = None) -> int:
        """次のタプルデータを読む.

        Args:
            tuple_ (Optional[[DataArrayData]): 読み込んだタプルデータ

        Returns:
            int: 実行ステータス
        """
        if self.__port is None:
            return self.__status

        status = StatusSet.UNDEFINED.value

        if self.__tupledata:
            # 中身をアサインする
            if tuple_ is not None:
                tuple_.assign(self.__tupledata)
        else:
            # 中身を解放する
            if tuple_ is not None:
                tuple_.clear()

        try:
            try:
                # 通信ポートから１つの要素を読込む
                object_ = self.__port.read_object(tuple_)

            except Exception as err:
                self.__status = StatusSet.ERROR.value
                raise err

            if object_ is None:
                # データ読込終了
                status = StatusSet.END_OF_DATA.value
                self.__metadata = None
                self.__tupledata = None
            elif object_.class_id == ClassID.RESULTSET_META_DATA.value:
                # メタデータの処理
                assert isinstance(object_, ResultSetMetaData)
                status = StatusSet.META_DATA.value
                self.__metadata = object_
                self.__tupledata = self.__metadata.create_tuple_data()
            elif object_.class_id == ClassID.STATUS.value:
                # ステータスの処理
                assert isinstance(object_, Status)
                if object_.status == Status.status_map['SUCCESS']:
                    status = StatusSet.SUCCESS.value
                elif object_.status == Status.status_map['CANCELED']:
                    status = StatusSet.CANCELED.value
                elif object_.status == Status.status_map['HAS_MORE_DATA']:
                    status = StatusSet.HAS_MORE_DATA.value
                else:
                    # ERRORの場合スルー
                    pass
            elif object_.class_id == ClassID.DATA_ARRAY_DATA.value:
                # タプルデータの処理
                status = StatusSet.DATA.value
            else:
                # ERRORの場合スルー
                pass

            if status == StatusSet.UNDEFINED.value:
                self.__status = StatusSet.ERROR.value
                raise exceptions.InterfaceError('Status Undefined')

            # 現在の実行ステータスを登録
            self.__status = status

        finally:
            # ``status``がDATA, END_OF_DATA, META_DATA, HAS_MORE_DATAの場合は何もしない
            if self.__status == StatusSet.SUCCESS.value:
                # 成功したのでポートを返す
                self.__datasource.push_port(self.__port)
                self.__port = None
            elif self.__status == StatusSet.CANCELED.value:
                # バージョン３以降であればポートを返却
                if self.__port.master_id >= ProtocolVersion.PROTOCOL_VERSION3:
                    self.__datasource.push_port(self.__port)
                    self.__port = None
            elif self.__status == StatusSet.ERROR.value \
                    or self.__status == StatusSet.UNDEFINED.value:
                if self.__port.is_reuse:
                    self.__datasource.push_port(self.__port)
                else:
                    self.__port.close()
                self.__port = None
            else:
                # 何もしない
                pass

        return status

    def next(self) -> bool:
        """リザルトセットを１行読込み、カーソルを１行下に移動する.

        Returns:
            bool: True  成功
                  False 失敗

        Raises:
            ClassNotFound: Class定義が見つからなかった場合
            ConnectionRanOut: 通信でエラーが発生した場合

        Notes:
            カーソルは初期状態では最初の行の前に位置する.呼び出しによって、最初の行が現在の行になる.
            2 番目の呼び出しによって 2 行目が現在の行になり、以降同様に続く.
        """
        if self.__rowcount == -1:
            return False

        try:
            if self.__row is None:
                self.__row = DataArrayData()

            # メタデータ以外が読込まれるまでタプルデータを読込む.
            while True:
                status = self.get_next_tuple(self.__row)

                # 終了条件
                if status != StatusSet.META_DATA.value:
                    break

            if status != StatusSet.DATA.value:
                self.__row = None
                self.__rowcount = -1
                # 実行ステータスを得るまで読み飛ばす
                status = self.get_status(False)
                if status == StatusSet.HAS_MORE_DATA.value:
                    self.get_status(True)

                return False

            self.__rowcount += 1
            return True

        except IOError as ioe:
            # ConnectionRanOutエラーに読み替える
            croe = database_exceptions.ConnectionRanOut()
            raise croe from ioe

    def get_row_as_tuple(self) -> Optional[tuple]:
        """行データをリストに変換して取得する"""
        if self.__row:
            row = []
            array = self.__row.array
            for i in range(len(array)):
                # Decimal型とLanguage型は元のデータ型を用いる
                if isinstance(array[i], (DecimalData, LanguageData, WordData)):
                    row.append(array[i])
                # それ以外はpythonのデータ型を用いる
                else:
                    row.append(array[i].value)
            return tuple(row)
        else:
            raise exceptions.ProgrammingError('no row defined yet')

    def cancel(self) -> None:
        """実行をキャンセルする.
        """
        client_connection = self.__datasource.get_client_connection()
        assert client_connection is not None and self.__port is not None
        client_connection.cancel_worker(self.__port.worker_id)

    def close(self) -> None:
        """リザルトセットのクローズ.

        Notes:
            ゴミが残らないよう, 実行ステータスを得るまで結果を取得.
        """
        if self.__port is None:
            self.__status = StatusSet.UNDEFINED.value
            self.__metadata = None
            self.__tupledata = None
            self.__row = None
            self.__rowcount = 0
            self.__is_closed = True
            return

        try:
            if self.__status == StatusSet.DATA.value \
                    or self.__status == StatusSet.END_OF_DATA.value \
                    or self.__status == StatusSet.META_DATA.value\
                    or self.__status == StatusSet.HAS_MORE_DATA.value:
                # 実行ステータスを得ていないので、得るまで読込む
                self.get_status(True)

        except Exception:
            pass

        self.__status = StatusSet.UNDEFINED.value
        self.__metadata = None
        self.__tupledata = None
        self.__row = None
        self.__rowcount = 0
        self.__is_closed = True
