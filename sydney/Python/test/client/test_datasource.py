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
test_datasource.py -- src.doquedb.client.datasource モジュールのテスト
"""
from unittest.mock import MagicMock
import pytest

from src.doquedb.client.object import Object
from src.doquedb.client.datasource import DataSource
from src.doquedb.port.authorizemode import AuthorizeMode
from src.doquedb.client.constants import ProtocolVersion
from src.doquedb.client.connection import Connection
from src.doquedb.client.port import Port
from src.doquedb.port.constants import ConnectionSlaveID
from src.doquedb.client.session import Session
from src.doquedb.exception.exceptions import DatabaseError, UnexpectedError


class TestDataSource:
    """
    Notes:
        事前準備として、ローカルのDoqueDBサーバを以下の設定で作成する.
        ホスト名: localhost
        ポート番号: 54321
        ユーザー名: root
        パスワード: doqadmin
        データベース名: DefaultDB
    """

    def setup_method(self, method):
        # 前処理
        self.test_port = Port('localhost', 54321,
                              ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                              ConnectionSlaveID.ANY.value)
        self.datasource = None

    def teardown_method(self, method):
        # 後処理
        if self.datasource is not None:
            self.datasource.close()

    def test_init(self):
        # 正常系のテスト
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)

        # 各メンバ変数が正しくセットされたかチェック
        assert self.datasource.type == Object.DATA_SOURCE
        assert self.datasource._DataSource__connection_list == []
        assert self.datasource._DataSource__connection_element == 0
        assert self.datasource._DataSource__portmap == {}
        assert self.datasource._DataSource__expunge_port == []
        assert self.datasource._DataSource__hostname == 'localhost'
        assert self.datasource._DataSource__portnum == 54321
        assert self.datasource._DataSource__protocol_ver == 0
        assert self.datasource._DataSource__session_map == {}
        assert self.datasource._DataSource__is_closed is False
        assert self.datasource._DataSource__master_id == 0
        assert self.datasource._DataSource__authorization == 0

    def test_open(self):
        # 正常系のテスト
        protocol_ver = ProtocolVersion.CURRENT_PROTOCOL_VERSION.value
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)

        self.datasource.open(protocol_ver)
        # コネクションインスタンスが正しく生成されたかチェック
        assert self.datasource._DataSource__connection_list[0].\
            _Connection__datasource == self.datasource
        assert self.datasource._DataSource__connection_list[0].\
            _Connection__port._Port__connection.\
            _RemoteClientConnection__portnum == 54321
        # 正しいマスターIDが保存されたかチェック
        password_mode = protocol_ver | AuthorizeMode.PASSWORD.value
        assert self.datasource.master_id == password_mode
        # 正しい認証方式が保存されたかチェック
        assert self.datasource.authorization == AuthorizeMode.PASSWORD.value
        # closedがFalseに設定されたかチェック
        assert self.datasource._DataSource__is_closed is False

    def test_create_session(self):
        # 正常系のテスト
        # 前準備
        username = 'root'
        password = 'doqadmin'
        dbname = 'DefaultDB'
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # データソースを開く
        self.datasource.open(ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)

        session = self.datasource.create_session(dbname, username, password)
        # 返り値のチェック
        assert session.dbname == dbname
        assert session.username == username
        # ポートマップにポートが登録されたかチェック
        assert len(self.datasource._DataSource__portmap) == 1
        # セッションマップにセッションが登録されたかチェック
        assert self.datasource._DataSource__session_map[session.id_] == session

    def test_create_session_session_exist(self):
        # 異常系のテスト
        # 利用中のセッションがある場合
        # 前準備
        username = 'root'
        password = 'doqadmin'
        dbname = 'DefaultDB'
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # データソースを開く
        self.datasource.open(ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        # closeをモックに差し替える
        self.datasource.close = MagicMock()

        # 一度セッションを作成する
        session = self.datasource.create_session(dbname, username, password)
        # get_client_connectionの返り値をNoneにする
        self.datasource.get_client_connection = MagicMock(return_value=None)
        with pytest.raises(UnexpectedError,
                           match=r'.*(1).*'):
            self.datasource.create_session(dbname, username, password)
        # セッションが閉じず、self.close()が呼ばれていないことをチェックする
        assert self.datasource._DataSource__session_map[session.id_] == session
        self.datasource.close.assert_not_called()

    def test_create_session_get_connection_fail_once(self):
        # 正常系のテスト
        # client_connectionが２回目で取得できた場合
        # 前準備
        username = 'root'
        password = 'doqadmin'
        dbname = 'DefaultDB'
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # データソースを開く
        self.datasource.open(ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        # データソースをクローズする
        self.datasource.close()

        # client_connectionがない場合でも、再度datasourceを開くので問題なく実行できる
        session = self.datasource.create_session(dbname, username, password)
        # 返り値のチェック
        assert session.dbname == dbname
        assert session.username == username
        # ポートマップにポートが登録されたかチェック
        assert len(self.datasource._DataSource__portmap) == 1
        # セッションマップにセッションが登録されたかチェック
        assert self.datasource._DataSource__session_map[session.id_] == session

    def test_create_session_get_connection_fail_twice(self):
        # 正常系のテスト
        # client_connectionが３回目で取得できた場合
        username = 'root'
        password = 'doqadmin'
        dbname = 'DefaultDB'
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # データソースを開く
        self.datasource.open(ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        # １回目のconnectionではワーカーを起動できないよう設定する
        bad_connection = self.datasource.get_client_connection()
        bad_connection.begin_worker = MagicMock(side_effect=UnexpectedError)

        # client_connectionがない場合でも、再度datasourceを開くので問題なく実行できる
        session = self.datasource.create_session(dbname, username, password)
        # 返り値のチェック
        assert session.dbname == dbname
        assert session.username == username
        # ポートマップにポートが登録されたかチェック
        assert len(self.datasource._DataSource__portmap) == 1
        # セッションマップにセッションが登録されたかチェック
        assert self.datasource._DataSource__session_map[session.id_] == session
        # 別のコネクションが作成されたかチェック
        new_connection = self.datasource.get_client_connection()
        assert new_connection != bad_connection

    def test_create_session_get_connection_fail(self):
        # 異常系のテスト
        # client_connectionが３回目でも取得できなかった場合
        # 前準備
        username = 'root'
        password = 'doqadmin'
        dbname = 'DefaultDB'
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # データソースを開く
        self.datasource.open(ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        # get_client_connectionの返り値を常にNoneにする
        self.datasource.get_client_connection = MagicMock(return_value=None)

        # 再度datasourceを開いてもconnectionが得られないのでエラーとなる
        with pytest.raises(UnexpectedError,
                           match=r'.*(2).*'):
            self.datasource.create_session(dbname, username, password)

    def test_create_session_begin_session_fail(self):
        # 異常系のテスト
        # セッションの開始に失敗した場合（ユーザー管理あり）
        bad_username = 'wrong_name'
        bad_password = 'wrong_password'
        bad_dbname = 'BadDB'
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # データソースを開く
        self.datasource.open(ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        # portをデータソースに登録しておく
        connection = self.datasource.get_client_connection()
        port = connection.begin_worker()
        self.datasource.push_port(port)

        # ユーザー認所に失敗し、DataBaseErrorが上がる
        with pytest.raises(DatabaseError):
            # 誤ったユーザー名、パスワードでセッションを開始する
            self.datasource.create_session(
                bad_dbname, bad_username, bad_password)
        # datasourceにportが返却されたかチェックする
        self.datasource._DataSource__portmap[port.slave_id] == port
        # portがcloseされていないかチェックする
        assert port._Port__connection.is_opened is True

    def test_get_client_connection(self):
        # 正常系のテスト
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # connection_listにコネクションを１つ登録
        connection = Connection(self.datasource, self.test_port)
        self.datasource._DataSource__connection_list.append(connection)

        new_connection = self.datasource.get_client_connection()
        assert new_connection == connection

    def test_get_client_connection_round(self):
        # 正常系のテスト
        # ラウンドロビンが機能しているかのテスト
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # connectionを２つ登録する
        connection1 = Connection(self.datasource, self.test_port)
        connection2 = Connection(self.datasource, self.test_port)
        self.datasource._DataSource__connection_list.append(connection1)
        self.datasource._DataSource__connection_list.append(connection2)

        new_connection1 = self.datasource.get_client_connection()
        assert new_connection1 == connection1
        new_connection2 = self.datasource.get_client_connection()
        assert new_connection2 == connection2
        # ３回目で１つ目のコネクションが返ってくるかチェックする
        new_connection3 = self.datasource.get_client_connection()
        assert new_connection3 == connection1

    def test_get_client_connection_list_empty(self):
        # 正常系のテスト
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)

        new_connection = self.datasource.get_client_connection()
        assert new_connection is None

    def test_new_client_connection(self):
        # 正常系のテスト
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # connection_listにコネクションを登録
        connection = Connection(self.datasource, self.test_port)
        self.datasource._DataSource__connection_list.append(connection)

        self.datasource.new_client_connection()
        # セッション数がThresholdを超えていないので、connectionは増えない
        assert len(self.datasource._DataSource__connection_list) == 1

    def test_new_client_connection_threshold(self):
        # 正常系のテスト
        # Thresholdを超えた場合
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # connection_listにコネクションを登録
        connection = Connection(self.datasource, self.test_port)
        self.datasource._DataSource__connection_list.append(connection)
        # begin_connectionをモックに差し替える
        new_connection = Connection(self.datasource, self.test_port)
        connection.begin_connection = MagicMock(return_value=new_connection)
        # session_mapにThresholdまでのセッションを登録
        for i in range(DataSource.CONNECTION_THRESHOLD):
            self.datasource._DataSource__session_map[i] = Session(
                'DefaultDB', i, 'username')

        self.datasource.new_client_connection()
        assert self.datasource._DataSource__connection_list[1]\
            == new_connection

    def test_pop_port(self):
        # 正常系のテスト
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # portを登録
        slave_id = 1
        self.datasource._DataSource__portmap[slave_id] = self.test_port

        pop_port = self.datasource.pop_port()
        assert pop_port == self.test_port
        assert len(self.datasource._DataSource__portmap) == 0

    def test_push_port(self):
        # 正常系のテスト
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        slave_id = ConnectionSlaveID.ANY.value

        # portを登録
        self.datasource.push_port(self.test_port)
        assert self.datasource._DataSource__portmap[slave_id] == self.test_port

    def test_new_port(self):
        # 正常系のテスト
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        slave_id = 1

        port = self.datasource.new_port(slave_id)
        assert port._Port__connection._RemoteClientConnection__hostname\
            == self.datasource._DataSource__hostname
        assert port._Port__connection._RemoteClientConnection__portnum\
            == self.datasource._DataSource__portnum
        assert port._Port__connection.master_id\
            == self.datasource._DataSource__protocol_ver
        assert port._Port__connection.slave_id == slave_id

    @ pytest.mark.skip(reason='未使用メソッド')
    def test_expunge_port(self):
        # 正常系のテスト
        pass

    def test_session_exist(self):
        # 正常系のテスト
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)

        # sessionがない場合のチェック
        assert self.datasource.session_exist() is False
        # sessionがある場合のチェック
        self.datasource._DataSource__session_map[1] = Session(
            'TestDB', 1, 'username')
        assert self.datasource.session_exist() is True

    def test_remove_session(self):
        # 正常系のテスト
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # sessionを登録
        session_id = 1
        self.datasource._DataSource__session_map[session_id] = Session(
            'TestDB', 1, 'username')

        self.datasource.remove_session(session_id)
        assert len(self.datasource._DataSource__session_map) == 0
        with pytest.raises(KeyError):
            self.datasource._DataSource__session_map[session_id]

    @ pytest.mark.skip(reason='未使用メソッド')
    def test_is_server_available(self):
        # 正常系のテスト
        pass

    @ pytest.mark.skip(reason='未実装メソッド')
    def test_is_database_available(self):
        # 正常系のテスト
        pass

    @ pytest.mark.skip(reason='未使用メソッド')
    def test_shutdown(self):
        # 正常系のテスト
        pass

    @ pytest.mark.skip(reason='未使用メソッド')
    def test_shutdown_old_server(self):
        # 正常系のテスト
        pass

    @ pytest.mark.skip(reason='未使用メソッド')
    def test_shutdown_error(self):
        # 異常系のテスト
        pass

    def test_close(self):
        # 正常系のテスト
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # データソースを開く
        self.datasource.open(ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        # セッションを作成
        self.datasource.create_session('TestDB', 'root', 'doqadmin')

        self.datasource.close()
        # connection_listがクリアされたかチェック
        assert self.datasource._DataSource__session_map == {}
        # connection_listがクリアされたかチェック
        assert self.datasource._DataSource__connection_list == []
        # connection_listがクリアされたかチェック
        assert self.datasource._DataSource__portmap == {}
        # __is_closedのフラグがFalseに設定されたかチェック
        assert self.datasource._DataSource__is_closed is True

    def test_close_twice(self):
        # 正常系のテスト
        # 前準備
        # DataSourceインスタンスを生成
        self.datasource = DataSource('localhost', 54321)
        # データソースを開く
        self.datasource.open(ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        # セッションを作成
        self.datasource.create_session('TestDB', 'root', 'doqadmin')

        self.datasource.close()
        # ２回閉じても問題ない
        self.datasource.close()
        # connection_listがクリアされたかチェック
        assert self.datasource._DataSource__session_map == {}
        # connection_listがクリアされたかチェック
        assert self.datasource._DataSource__connection_list == []
        # connection_listがクリアされたかチェック
        assert self.datasource._DataSource__portmap == {}
        # __is_closedのフラグがFalseに設定されたかチェック
        assert self.datasource._DataSource__is_closed is True
