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
test_src.doquedb.py -- doquedbのテスト
"""
import pytest

import src.doquedb as dq
from src.doquedb.client.datasource import DataSource
from src.doquedb.driver.connection import Connection
from src.doquedb.client.session import Session
from src.doquedb.client.constants import ProtocolVersion
from src.doquedb.port.authorizemode import AuthorizeMode
from src.doquedb.exception.exceptions import DatabaseError


class TestDoqueDB:
    """
    Notes:
        事前準備として、ローカルのDoqueDBサーバを以下の設定で2つ作成する.
        ホスト名: localhost
        データベース1
            ポート番号: 54321
            ユーザー名: root
            パスワード: doqadmin
            データベース名: DefaultDB
        データベース2
            ポート番号: 54322
            ユーザー名: root
            パスワード: doqadmin
            データベース名: DefaultDB
    """

    def test_connect(self):
        # 正常系のテスト
        # DoqueDBに接続する
        connection1 = dq.connect(host='localhost',
                                 port=54321,
                                 dbname='DefaultDB',
                                 user='root',
                                 password='doqadmin')
        datasource_key1 = ('localhost',
                           54321,
                           ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        # TODO: fix
        # datasource_key3 = ('localhost',
        #                    54322,
        #                    ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)

        assert isinstance(
            dq.datasource_map[datasource_key1], DataSource)
        assert isinstance(connection1, Connection)
        assert connection1._Connection__is_closed is False
        assert isinstance(connection1._session, Session)
        # カーソルを作成し、sqlを実行する
        cursor1 = connection1.cursor()
        cursor1.execute('CREATE TABLE Test')

        # connectionをもう一つ作成し、別のコネクションとなっているかどうかのチェック
        connection2 = dq.connect(host='localhost',
                                 port=54321,
                                 dbname='DefaultDB',
                                 user='root',
                                 password='doqadmin')
        # SessionIDをチェックする
        assert connection1._session.id_ != connection2._session.id_
        # カーソルを作成し、sqlを実行する
        cursor2 = connection2.cursor()
        cursor2.execute('SELECT * FROM Test')

        # 別の接続先に接続した場合にコネクションが作れるかのチェック
        # TODO: fix
        # connection3 = dq.connect(host='localhost',
        #                                     port=54322,
        #                                     dbname='DefaultDB',
        #                                     user='root',
        #                                     password='doqadmin')

        # assert isinstance(
        #     dq.datasource_map[datasource_key3], DataSource)
        # assert isinstance(connection3, Connection)
        # assert connection3._Connection__is_closed is False
        # assert isinstance(connection3._session, Session)
        # # カーソルを作成し、sqlを実行する
        # cursor3 = connection3.cursor()
        # cursor3.execute('CREATE TABLE Test')

        # 後処理
        cursor1.execute('DROP TABLE Test')
        # TODO: cursor3.execute('DROP TABLE Test')

        connection1.close()
        connection2.close()
        # TODO: connection3.close()
        dq.close()

    def test_connect_connection_error_hostname(self):
        # 異常系のテスト
        # ホスト名が誤っている場合
        # DoqueDBに接続する
        with pytest.raises(IOError):
            dq.connect(host='badhost',
                       port=54321,
                       dbname='DefaultDB',
                       user='root',
                       password='doqadmin')

        # 後処理
        dq.close()

    def test_connect_connection_error_port(self):
        # 異常系のテスト
        # ポート番号が誤っている場合
        # DoqueDBに接続する
        with pytest.raises(IOError):
            dq.connect(host='localhost',
                       port=12345,
                       dbname='DefaultDB',
                       user='root',
                       password='doqadmin')

        # 後処理
        dq.close()

    def test_connect_connection_error_user(self):
        # 異常系のテスト
        # ユーザー名が誤っている場合
        # DoqueDBに接続する
        with pytest.raises(DatabaseError):
            dq.connect(host='localhost',
                       port=54321,
                       dbname='DefaultDB',
                       user='bad_user',
                       password='doqadmin')

        # 後処理
        dq.close()

    def test_connect_connection_error_password(self):
        # 異常系のテスト
        # ユーザー名が誤っている場合
        # DoqueDBに接続する
        with pytest.raises(DatabaseError):
            dq.connect(host='localhost',
                       port=54321,
                       dbname='DefaultDB',
                       user='root',
                       password='bad_password')

        # 後処理
        dq.close()

    def test_connect_connection_error_no_user(self):
        # 異常系のテスト
        # ユーザー認証ありの時にユーザー名とパスワードが省略された場合
        # DoqueDBに接続する
        with pytest.raises(DatabaseError):
            dq.connect(host='localhost',
                       port=54321,
                       dbname='DefaultDB')

        # 後処理
        dq.close()

    def test_close(self):
        # 正常系のテスト
        # 前準備
        # DoqueDBに接続するセッションを２つ作成
        dq.connect(host='localhost',
                   port=54321,
                   dbname='DefaultDB',
                   user='root',
                   password='doqadmin')
        datasource_key = ('localhost',
                          54321,
                          ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        datasource = dq.datasource_map[datasource_key]

        dq.close()
        assert datasource._DataSource__is_closed is True
        assert dq.datasource_map == {}
        assert datasource._DataSource__session_map == {}
        assert datasource._DataSource__connection_list == []
        assert datasource._DataSource__portmap == {}

    def test_close_twice(self):
        # 正常系のテスト
        # 前準備
        # DoqueDBに接続するセッションを２つ作成
        dq.connect(host='localhost',
                   port=54321,
                   dbname='DefaultDB',
                   user='root',
                   password='doqadmin')
        datasource_key = ('localhost',
                          54321,
                          ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        datasource = dq.datasource_map[datasource_key]

        dq.close()
        # ２回閉じても問題ない
        dq.close()
        assert datasource._DataSource__is_closed is True
        assert dq.datasource_map == {}
        assert datasource._DataSource__session_map == {}
        assert datasource._DataSource__connection_list == []
        assert datasource._DataSource__portmap == {}
