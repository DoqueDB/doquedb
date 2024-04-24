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
serialdata.py -- :class: `Serializable` を基底とするクラスの実装モジュール.

:class: `ColumnMetaData`
:class: `Request`
:class: `Status`
:class: `ErrorLevel`
:class: `ExceptionData`
"""
from typing import Union, Optional, List, TYPE_CHECKING

if TYPE_CHECKING:
    from .iostream import InputStream, OutputStream

from .serializable import Serializable
from .constants import SQLType, ClassID, DataType
from .arraydata import DataArrayData
from .scalardata import (StringData,
                         IntegerData,
                         Integer64Data,
                         DecimalData,
                         DoubleData,
                         DateData,
                         DateTimeData,
                         LanguageData,
                         )
from .data import BinaryData, WordData
from .unicodestr import UnicodeString
from ..exception import exceptions
from ..exception.error_message import ErrorMessage

AnyData = Union[DataArrayData,
                StringData,
                BinaryData,
                IntegerData,
                Integer64Data,
                DecimalData,
                DoubleData,
                DateData,
                DateTimeData,
                LanguageData,
                WordData,
                None
                ]


class ColumnMetaData(Serializable):
    """カラムのメタデータを表すクラス

    Args:
        metadata (Optional[ColumnMetaData]): 元となるカラムのメタデータ

    Attributes:
        __type (int): SQLデータ型

        __typename (str): データ型名
        __colname (str): カラム名
        __tablename (str): テーブル名
        __dbname (str): データベース名
        __column_aliasname (str): カラム別名
        __table_aliasname (str): テーブル別名

        __displaysize (int): 最大表示サイズ
        __precision (int): 10進桁数
        __scale (int): 小数点以下の桁数
        __cardinality (int): 配列要素数
        __flag (int): 属性
    """

    # 自動採番
    __AUTO_INCREMENT = (1 << 0)
    # 大文字小文字が区別されない
    __CASE_INSENSITIVE = (1 << 1)
    # 符号なし
    __UNSIGNED = (1 << 2)
    # 検索不可
    __NOT_SEARCHABLE = (1 << 3)
    # 読み出し専用
    __READ_ONLY = (1 << 4)
    # NULL不許可
    __NOT_NULL = (1 << 5)
    # ユニーク
    __UNIQUE = (1 << 6)

    # データに対するクラスのマップ
    data_map = {
        'STRING': StringData,
        'BINARY': BinaryData,
        'INTEGER': IntegerData,
        'INTEGER64': Integer64Data,
        'DECIMAL': DecimalData,
        'DOUBLE': DoubleData,
        'DATE': DateData,
        'DATE_TIME': DateTimeData,
        'LANGUAGE': LanguageData,
        'WORD': WordData,
    }

    def __init__(self, metadata: Optional['ColumnMetaData'] = None) -> None:
        if metadata:
            self.__type = metadata.type
            self.__typename = metadata.typename
            self.__colname = metadata.colname
            self.__tablename = metadata.tablename
            self.__dbname = metadata.dbname
            self.__column_aliasname = metadata.column_aliasname
            self.__table_aliasname = metadata.table_aliasname
            self.__displaysize = metadata.displaysize
            self.__precision = metadata.precision
            self.__scale = metadata.scale
            self.__cardinality = metadata.cardinality
            self.__flag = metadata.flag
        else:
            self.__type = SQLType.UNKNOWN.value
            self.__typename = ''
            self.__colname = ''
            self.__tablename = ''
            self.__dbname = ''
            self.__column_aliasname = ''
            self.__table_aliasname = ''
            self.__displaysize = 0
            self.__precision = 0
            self.__scale = 0
            self.__cardinality = 0
            self.__flag = 0

    @property
    def type(self) -> int:
        """SQLデータ型のゲッター"""
        return self.__type

    @type.setter
    def type(self, type: int) -> None:
        """データ型のセッター"""
        self.__type = type

    @property
    def typename(self) -> str:
        """データ型名のゲッター"""
        return self.__typename

    @typename.setter
    def typename(self, typename: str) -> None:
        """データ型名のセッター"""
        self.__typename = typename

    @property
    def colname(self) -> str:
        """カラム名のゲッター"""
        return self.__colname

    @colname.setter
    def colname(self, colname: str) -> None:
        """カラム名のセッター"""
        self.__colname = colname

    @property
    def tablename(self) -> str:
        """テーブル名のゲッター"""
        return self.__tablename

    @tablename.setter
    def tablename(self, tablename: str) -> None:
        """テーブル名のセッター"""
        self.__tablename = tablename

    @property
    def dbname(self) -> str:
        """データベース名のゲッター"""
        return self.__dbname

    @dbname.setter
    def dbname(self, dbname: str) -> None:
        """データベース名のセッター"""
        self.__dbname = dbname

    @property
    def column_aliasname(self) -> str:
        """カラム別名のゲッター"""
        return self.__column_aliasname

    @column_aliasname.setter
    def column_aliasname(self, column_aliasname: str) -> None:
        """カラム別名のセッター"""
        self.__column_aliasname = column_aliasname

    @property
    def table_aliasname(self) -> str:
        """テーブル別名のゲッター"""
        return self.__table_aliasname

    @table_aliasname.setter
    def table_aliasname(self, table_aliasname: str) -> None:
        """データベース名のセッター"""
        self.__table_aliasname = table_aliasname

    @property
    def displaysize(self) -> int:
        """最大表示サイズのゲッター"""
        return self.__displaysize

    @displaysize.setter
    def displaysize(self, displaysize: int) -> None:
        """最大表示サイズのセッター"""
        self.__displaysize = displaysize

    @property
    def precision(self) -> int:
        """10進桁数のゲッター"""
        return self.__precision

    @precision.setter
    def precision(self, precision: int) -> None:
        """10進桁数のセッター"""
        self.__precision = precision

    @property
    def scale(self) -> int:
        """小数点以下の桁数のゲッター"""
        return self.__scale

    @scale.setter
    def scale(self, scale: int) -> None:
        """小数点以下の桁数のセッター"""
        self.__scale = scale

    @property
    def cardinality(self) -> int:
        """配列要素数のゲッター"""
        return self.__cardinality

    @cardinality.setter
    def cardinality(self, cardinality: int) -> None:
        """データ型のセッター"""
        self.__cardinality = cardinality

    @property
    def flag(self) -> int:
        """属性のゲッター"""
        return self.__flag

    @property
    def is_autoincrement(self) -> bool:
        """自動採番かどうか"""
        return (self.__flag & self.__AUTO_INCREMENT) != 0

    @is_autoincrement.setter
    def is_autoincrement(self, v: bool) -> None:
        """自動採番かどうかを設定する"""
        if v:
            self.__flag &= self.__AUTO_INCREMENT
        else:
            self.__flag &= ~self.__AUTO_INCREMENT

    @property
    def is_case_insensitive(self) -> bool:
        """大文字小文字が区別されないかどうか"""
        return (self.__flag & self.__CASE_INSENSITIVE) != 0

    @is_case_insensitive.setter
    def is_case_insensitive(self, v: bool) -> None:
        """大文字小文字が区別されないかどうかを設定する"""
        if v:
            self.__flag &= self.__CASE_INSENSITIVE
        else:
            self.__flag &= ~self.__CASE_INSENSITIVE

    @property
    def is_unsigned(self) -> bool:
        """符号なしかどうか"""
        return (self.__flag & self.__UNSIGNED) != 0

    @is_unsigned.setter
    def is_unsigned(self, v: bool) -> None:
        """符号なしかどうかを設定する"""
        if v:
            self.__flag &= self.__UNSIGNED
        else:
            self.__flag &= ~self.__UNSIGNED

    @property
    def isnot_searchable(self) -> bool:
        """検索不可かどうか"""
        return (self.__flag & self.__NOT_SEARCHABLE) != 0

    @isnot_searchable.setter
    def isnot_searchable(self, v: bool) -> None:
        """検索不可かどうかを設定する"""
        if v:
            self.__flag &= self.__NOT_SEARCHABLE
        else:
            self.__flag &= ~self.__NOT_SEARCHABLE

    @property
    def is_readonly(self) -> bool:
        """読み出し専用かどうか"""
        return (self.__flag & self.__READ_ONLY) != 0

    @is_readonly.setter
    def is_readonly(self, v: bool) -> None:
        """読み出し専用かどうかを設定する"""
        if v:
            self.__flag &= self.__READ_ONLY
        else:
            self.__flag &= ~self.__READ_ONLY

    @property
    def isnot_nullable(self) -> bool:
        """NULL をセットできないかどうか"""
        return (self.__flag & self.__NOT_NULL) != 0

    @isnot_nullable.setter
    def isnot_nullable(self, v: bool) -> None:
        """NULL をセットできないかどうかを設定する"""
        if v:
            self.__flag &= self.__NOT_NULL
        else:
            self.__flag &= ~self.__NOT_NULL

    @property
    def is_unique(self) -> bool:
        """ユニークかどうか"""
        return (self.__flag & self.__UNIQUE) != 0

    @is_unique.setter
    def is_unique(self, v: bool) -> None:
        """ユニークかどうかを設定する"""
        if v:
            self.__flag &= self.__UNIQUE
        else:
            self.__flag &= ~self.__UNIQUE

    @property
    def is_array(self) -> bool:
        """配列かどうか"""
        return self.__cardinality != 0

    @property
    def class_id(self) -> int:
        """class_id を得る."""
        return ClassID.COLUMN_META_DATA.value

    def __str__(self) -> str:
        """文字列に変換する.

        Returns:
            str: 文字列
        """
        return self.column_aliasname

    def get_datatype(self, type: int) -> int:
        """datatype を得る.

        Returns:
            int: データタイプ値
        """
        datatype_map = {
            'CHARACTER': DataType.STRING.value,
            'CHARACTER_VARYING': DataType.STRING.value,
            'NATIONAL_CHARACTER': DataType.STRING.value,
            'NATIONAL_CHARACTER_VARYING': DataType.STRING.value,

            'BINARY': DataType.BINARY.value,
            'BINARY_VARYING': DataType.BINARY.value,

            'INTEGER': DataType.INTEGER.value,
            'BIG_INT': DataType.INTEGER64.value,

            'DECIMAL': DataType.DECIMAL.value,
            'NUMERIC': DataType.DECIMAL.value,

            'DOUBLE_PRECISION': DataType.DOUBLE.value,

            'DATE': DataType.DATE.value,
            'TIMESTAMP': DataType.DATE_TIME.value,

            'LANGUAGE': DataType.LANGUAGE.value,

            'WORD': DataType.WORD.value,
        }

        datatype = DataType.UNDEFINED.value

        if type == SQLType.UNKNOWN.value:
            return datatype

        # datatypesをsqltype値から逆引き検索
        for item in SQLType:
            if type == item.value:
                datatype = datatype_map[item.name]
                break

        return datatype

    def get_datainstance(self) -> AnyData:
        """適切なデータインスタンスを得る.

        Returns:
            AnyData: データインスタンス
        """

        data: Optional[AnyData] = None
        if self.is_array:
            data = DataArrayData()
        else:
            # datatypesをsqltype値から逆引き検索
            datatype = self.get_datatype(self.type)
            # UNDEFINEDが返ってきた場合Noneを返す
            if datatype == DataType.UNDEFINED.value:
                return data

            for item in DataType:
                if datatype == item.value:
                    data = self.data_map[item.name]()
                    break

        return data

    def read_object(self, input: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        # データタイプの読込
        self.__type = input.read_int()

        # ModUnicodeString
        c = input.read_int()
        if c != 0:
            self.__typename = UnicodeString.read_object(input)
            c -= 1
        if c != 0:
            self.__colname = UnicodeString.read_object(input)
            c -= 1
        if c != 0:
            self.__tablename = UnicodeString.read_object(input)
            c -= 1
        if c != 0:
            self.__dbname = UnicodeString.read_object(input)
            c -= 1
        if c != 0:
            self.__column_aliasname = UnicodeString.read_object(input)
            c -= 1
        if c != 0:
            self.__table_aliasname = UnicodeString.read_object(input)
            c -= 1

        # int
        c = input.read_int()
        if c != 0:
            self.__displaysize = input.read_int()
            c -= 1
        if c != 0:
            self.__precision = input.read_int()
            c -= 1
        if c != 0:
            self.__scale = input.read_int()
            c -= 1
        if c != 0:
            self.__cardinality = input.read_int()
            c -= 1

        # flag
        self.__flag = input.read_int()

    def write_object(self, output: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        # データタイプ
        output.write_int(self.__type)

        # ModUnicodeString
        output.write_int(6)
        UnicodeString.write_object(output, self.__typename)
        UnicodeString.write_object(output, self.__colname)
        UnicodeString.write_object(output, self.__tablename)
        UnicodeString.write_object(output, self.__dbname)
        UnicodeString.write_object(output, self.__column_aliasname)
        UnicodeString.write_object(output, self.__table_aliasname)

        # int
        output.write_int(4)
        output.write_int(self.__displaysize)
        output.write_int(self.__precision)
        output.write_int(self.__scale)
        output.write_int(self.__cardinality)

        # flag
        output.write_int(self.__flag)


class Request(Serializable):
    """リクエストをあらわすクラス

    Args:
        request_ (int): リクエスト値

    Attributes:
        __request (int): リクエスト値
    """
    request_map = {
        # コネクション開始
        'BEGIN_CONNECTION': 1,
        # コネクション終了
        'END_CONNECTION': 2,
        # セッション開始
        'BEGIN_SESSION': 3,
        # セッション終了
        'END_SESSION': 4,
        # ワーカ開始
        'BEGIN_WORKER': 5,
        # ワーカ中断
        'CANCEL_WORKER': 6,

        # サーバ終了
        'SHUTDOWN': 7,

        # SQL文の実行
        'EXECUTE_STATEMENT': 8,
        # SQL文のコンパイル
        'PREPARE_STATEMENT': 9,
        # コンパイル結果の実行
        'EXECUTE_PREPARE_STATEMENT': 10,
        # コンパイル結果の削除
        'ERASE_PREPARE_STATEMENT': 11,

        # コネクションを再利用する
        'REUSE_CONNECTION': 12,
        # コネクションを再利用しない
        'NO_REUSE_CONNECTION': 13,

        # 利用可能性チェック
        'CHECK_AVAILABILITY': 14,

        # SQL文のコンパイル
        'PREPARE_STATEMENT2': 15,
        # コンパイル結果の削除
        'ERASE_PREPARE_STATEMENT2': 16,

        # セッション開始(ユーザーつき)
        'BEGIN_SESSION2': 17,
        # セッション終了(ユーザーつき;未使用)
        'END_SESSION2': 18,

        # ユーザーの追加
        'CREATE_USER': 19,
        # ユーザーの削除
        'DROP_USER': 20,
        # パスワードの変更(自分自身)
        'CHANGE_OWN_PASSWORD': 21,
        # パスワードの変更(他人)
        'CHANGE_PASSWORD': 22,
        # サーバの終了
        'SHUTDOWN2': 23,

        # 同期を取る
        'SYNC': 101,

        # ProductVersionを問い合わせる
        'QUERY_PRODUCT_VERSION': 201,

        # 不明なリクエスト
        'UNDEFINED': -1,

        # 利用可能性のチェック対象：サーバー
        'AVAILABILITY_TARGET_SERVER': 0,

        # 利用可能性のチェック対象：データベース
        'AVAILABILITY_TARGET_DATABASE': 1,
    }

    def __init__(self, request_: int = request_map['UNDEFINED']) -> None:
        self.__request = request_

    @ property
    def request(self) -> int:
        """リクエスト値のゲッター"""
        return self.__request

    @ request.setter
    def request(self, request_: int) -> None:
        """リクエスト値のセッター"""
        self.__request = request_

    @ property
    def class_id(self) -> int:
        return ClassID.REQUEST.value

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.__request = input_.read_int()

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        output_.write_int(self.__request)


class Status(Serializable):
    """ステータスをあらわすクラス

    Args:
        status_ (int): ステータス値

    Attributes:
        __status (int): ステータス値
    """
    status_map = {
        'SUCCESS': 0,
        'ERROR': 1,
        'CANCELED': 2,
        'HAS_MORE_DATA': 3,
        'UNDEFINED': -1,
    }

    def __init__(self, status_: int = status_map['UNDEFINED']) -> None:
        self.__status = status_

    @ property
    def status(self) -> int:
        return self.__status

    @ status.setter
    def status(self, status_: int) -> None:
        self.__status = status_

    @ property
    def class_id(self) -> int:
        return ClassID.STATUS.value

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.__status = input_.read_int()

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        output_.write_int(self.__status)


class ErrorLevel(Serializable):
    """エラーレベルをあらわすクラス

    Args:
        level_ (int): エラーレベル値

    Attributes:
        __level (int): エラーレベル値
    """
    USER = 1
    SYSTEM = 2
    UNDEFINED = -1

    def __init__(self, level_: int = UNDEFINED) -> None:
        self.__level = level_

    @ property
    def level(self) -> int:
        return self.__level

    @ level.setter
    def level(self, level_: int) -> None:
        self.__level = level_

    @ property
    def class_id(self) -> int:
        return ClassID.ERROR_LEVEL.value

    def is_userlevel(self) -> bool:
        """ユーザレベルかどうか
        """
        return self.__level == self.USER

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.__level = input_.read_int()

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        output_.write_int(self.__level)


class ExceptionData(Serializable):
    """例外データをあらわすクラス

    Args:
        errno (int): エラー番号

    Attributes:
        __errno (int): エラー番号
        __args (list): 引数
        __modulename (str): モジュール名
        __linenumber (int): 行番号
    """

    def __init__(self, errno_: int = 0) -> None:
        self.__args: List[str] = []
        self.__errno = errno_
        self.__modulename = ''
        self.__filename = ''
        self.__linenum = 0

    @ property
    def errno(self) -> int:
        """エラー番号のゲッター"""
        return self.__errno

    @ property
    def error_message(self) -> str:
        """エラーメッセージのゲッター
        """
        return ErrorMessage.make_error_message(self.__errno, self.__args)

    @ property
    def class_id(self) -> int:
        return ClassID.EXCEPTION_DATA.value

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.__args.clear()
        # エラー番号
        self.__errno = input_.read_int()
        # 引数の数
        len = input_.read_int()
        # 引数
        for i in range(len):
            size = input_.read_int()
            buffer = ''
            for j in range(size):
                buffer += input_.read_char()
            self.__args.append(buffer)
        # モジュール名
        size = input_.read_int()
        buffer = ''
        for j in range(size):
            buffer += input_.read_char()
        self.__modulename = buffer
        # ファイル名
        size = input_.read_int()
        buffer = ''
        for j in range(size):
            buffer += input_.read_char()
        self.__filename = buffer
        # 行番号
        self.__linenum = input_.read_int()

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        raise exceptions.OperationalError(
            'trying to write ExceptionData object')
