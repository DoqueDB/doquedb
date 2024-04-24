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
driver -- DoqueDBアクセス用のドライバーパッケージ
"""
from typing import Optional, Dict, Tuple

from ..driver.connection import Connection
from ..client.datasource import DataSource
from ..client.session import Session
from ..client.constants import ProtocolVersion
from ..port.authorizemode import AuthorizeMode

# データソース記録用の辞書
datasource_map: Dict[Tuple[str, int, int], DataSource] = {}
# プロトコルバージョンの設定.
# 現在のプロトコルバージョンについては :obj: `ProtocolVersion`を参照
protocol_ver = ProtocolVersion.CURRENT_PROTOCOL_VERSION.value


def connect_(hostname: str,
             portnum: int,
             dbname: str,
             charset: str,
             autocommit: bool,
             user: Optional[str] = None,
             password: Optional[str] = None,
             ) -> Connection:
    """DoqueDBと接続する.

    Args:
        hostname (str): ホスト名
        portnum (int): ポート番号
        dbname (int): データベース名
        charset (str): 文字セット
        autocommit (bool): オートコミット. デフォルトはFalse
        user (str): ユーザー名
        password (str): パスワード

    Returns:
        Connection: Connectionオブジェクト
    """
    connection: Optional[Connection] = None
    datasource: Optional[DataSource] = None
    session: Optional[Session] = None

    try:
        # データソースを開く
        try:
            # データソースを検索
            datasource = datasource_map[(hostname, portnum, protocol_ver)]
        except KeyError:
            # データソースがないので作成
            datasource = DataSource(hostname, portnum)
            try:
                datasource.open(protocol_ver)
                # マップに挿入
                datasource_map[(hostname, portnum, protocol_ver)] = datasource
            except Exception as err:
                datasource.close()
                raise err

        # セッションを得る
        if datasource.authorization == AuthorizeMode.NONE.value:
            session = datasource.create_session(dbname)
        else:
            session = datasource.create_session(dbname, user, password)

        # コネクションを作成
        connection = Connection(hostname, portnum, protocol_ver,
                                session, user, password,
                                datasource.master_id, charset, autocommit)

    except Exception as err:
        if session is not None:
            # セッションのクローズ
            session.close()
        if connection is not None:
            # コネクションのクローズ
            connection.close()
        raise err

    return connection
