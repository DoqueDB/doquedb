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
doquedb -- DoqueDB接続用パッケージ
"""
from .driver import Connection, connect_, datasource_map, protocol_ver
from .driver.dbapi import (apilevel, threadsafety, paramstyle,
                           Date, Time, Timestamp, Binary,
                           Language, Decimal,
                           DateFromTicks, TimestampFromTicks,
                           TimeFromTicks, STRING, BINARY,
                           NUMBER, DATETIME, ROWID,
                           DECIMAL, LANGUAGE, WORD)
from .exception.exceptions import (Error, Warning,
                                   InterfaceError, DatabaseError,
                                   NotSupportedError, DataError,
                                   IntegrityError, ProgrammingError,
                                   OperationalError, InternalError)


datasource_map_ = datasource_map
protocol_version = protocol_ver


def connect(host: str = 'localhost',
            port: int = 54321,
            user: str = None,
            password: str = None,
            dbname: str = 'DefaultDB',
            charset: str = 'utf8',
            autocommit: bool = False
            ) -> Connection:
    """DoqueDBと接続し, Connectionオブジェクトを返す.

    Args:
        host (str): ホスト名
        port (int): ポート番号
        user (str): ユーザー名
        password (str): パスワード
        dbname (int): データベース名
        charset (str): 文字セット
        autocommit (bool): オートコミット. デフォルトはFalse

    Returns:
        Connection: Connectionオブジェクト
    """

    connection = connect_(host, port, dbname, charset,
                          autocommit, user, password)

    return connection


def close() -> None:
    """DoqueDBとの接続を完全にクローズする.
    """
    if len(datasource_map) > 0:
        # マップに登録されているデータソースをクローズする
        for datasource in datasource_map.values():
            datasource.close()
        datasource_map.clear()


__all__ = [
    'Connection',

    # プロトコルバージョン
    'protocol_version',

    # エラー処理
    'Error', 'Warning',
    'InterfaceError', 'DatabaseError',
    'NotSupportedError', 'DataError', 'IntegrityError', 'ProgrammingError',
    'OperationalError', 'InternalError',

    # DBAPI PEP 249
    'connect', 'apilevel', 'threadsafety', 'paramstyle',
    # データ型とコンストラクタ
    'Date', 'Time', 'Timestamp', 'Binary',
    'Language', 'Decimal',
    'DateFromTicks', 'TimestampFromTicks', 'TimeFromTicks',
    'STRING', 'BINARY', 'NUMBER',
    'DATETIME', 'ROWID',
    'DECIMAL', 'LANGUAGE', 'WORD'
]
