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
test_connection.py -- src.doquedb.driver.connection モジュールのテスト
"""
import pytest
from unittest.mock import patch

import subprocess
import platform

from src.doquedb.driver.connection import Connection, TransactionMode
from src.doquedb.client.datasource import DataSource
from src.doquedb.client.constants import ProtocolVersion
from src.doquedb.driver.cursor import Cursor, CursorPrepared
from src.doquedb.client.constants import StatusSet
from src.doquedb.exception.exceptions import (
    ProgrammingError, InterfaceError, UnexpectedError)
from src.doquedb.exception.database_exceptions import ReadOnlyTransaction


@pytest.fixture(scope='module')
def create_table():
    # モジュール単位のセットアップ作業
    # 外部コマンドでsqliを実行し、テストに使用するテーブルを作成する
    if platform.system() == 'Linux':
        create_table = "/var/lib/DoqueDB/bin/sqli -remote localhost 54321 -database DefaultDB \
            -user root -password doqadmin -sql\
            'CREATE TABLE Test_ReadOnly (\
            id INT,\
            name NVARCHAR(100));\
            CREATE TABLE Test_Commit (\
            id INT,\
            name NVARCHAR(100));\
            CREATE TABLE Test_Rollback (\
            id INT,\
            name NVARCHAR(100));\
            CREATE TABLE Test_Transaction (\
            id INT,\
            name NVARCHAR(100));'"
        subprocess.run(create_table, shell=True)
    else:
        raise Exception('Windows, Macは非対応')

    yield

    # モジュール単位のティアダウン作業
    # 外部コマンドでsqliを実行し、テーブルを削除する
    drop_table = "/var/lib/DoqueDB/bin/sqli -remote localhost 54321 -database DefaultDB \
        -user root -password doqadmin -sql \
        'DROP TABLE Test_ReadOnly IF EXISTS; \
        DROP TABLE Test_Commit IF EXISTS; \
        DROP TABLE Test_Rollback IF EXISTS;\
        DROP TABLE Test_Transaction IF EXISTS;'"
    subprocess.run(drop_table, shell=True)


class TestConnection:
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
        # データソースを作成
        self.datasource = DataSource('localhost', 54321)
        # データソースをオープン
        self.datasource.open(ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        # セッションの作成
        self.session = self.datasource.create_session(
            'DefaultDB', 'root', 'doqadmin')

    def teardown_method(self, method):
        # 後処理
        # datasourをcloseする
        self.datasource.close()

    def create_connection(self) -> Connection:
        # Connectionインスタンスを生成する
        connection = Connection('localhost',
                                54321,
                                ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                                self.session,
                                'root',
                                'doqadmin',
                                self.datasource.master_id,
                                'utf-8'
                                )
        return connection

    def test_init(self):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()

        # 初期値のチェック
        assert connection._Connection__hostname == 'localhost'
        assert connection._Connection__portnum == 54321
        assert connection._Connection__protocol_ver ==\
            ProtocolVersion.CURRENT_PROTOCOL_VERSION.value
        assert connection._session == self.session
        assert connection.username == 'root'
        assert connection.password == 'doqadmin'
        assert connection.master_id == self.datasource.master_id
        assert connection.charset == 'utf-8'
        assert connection.is_autocommit is False

        assert connection._Connection__is_closed is False
        assert connection.readonly is False
        assert connection._Connection__set_readmode is False
        assert connection.in_transaction is False
        assert connection._Connection__isolation_level ==\
            connection.isolevel.TRANSACTION_READ_COMMITTED.value

        assert connection._Connection__cursor is None

    def test_info(self):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()

        # 接続情報が正しく返されたかチェック
        info_ = ('localhost', 54321,
                 ProtocolVersion.CURRENT_PROTOCOL_VERSION.value)
        assert connection.info == info_

    def test_readonly_setter(self):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()

        connection.set_readonly(True)
        assert connection._Connection__set_readmode is True
        assert connection.readonly is True

    def test_readonly_mode_already_set(self):
        # 正常系のテスト
        # リードモードが既に１回、設定されている場合
        # Connectionインスタンスを生成
        connection = self.create_connection()

        connection.set_readonly(True)
        connection.set_readonly(False)
        assert connection.readonly is False

    def test_readonly_same_mode(self):
        # 正常系のテスト
        # 同じフラグを設定した場合
        # Connectionインスタンスを生成
        connection = self.create_connection()

        # readonlyはデフォルトでFalse
        connection.set_readonly(False)
        assert connection._Connection__set_readmode is True
        assert connection.readonly is False

    def test_readonly_autocommit(self, create_table):
        # 正常系のテスト
        # autocommitがTrueの場合
        connection = Connection('localhost',
                                54321,
                                ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                                self.session,
                                'root',
                                'doqadmin',
                                self.datasource.master_id,
                                'utf-8',
                                autocommit=True
                                )

        # autocommitがオンでもreadonlyに設定できる
        connection.set_readonly(True)
        cursor = connection.cursor()
        # readonlyなので書込みはできない
        with pytest.raises(ReadOnlyTransaction):
            cursor.execute(
                "INSERT INTO Test_ReadOnly VALUES (1, 'readonly test')")
            cursor.fetchall()
        # 読込は可能
        cursor.execute('SELECT * FROM Test_ReadOnly')
        rows = cursor.fetchall()
        assert rows is None

    def test_readonly_connection_error(self):
        # 異常系のテスト
        # コネクションが既に閉じている場合
        # Connectionインスタンスを生成
        connection = self.create_connection()
        connection.close()

        with pytest.raises(ProgrammingError, match='connection closed'):
            connection.set_readonly(True)

    def test_readonly_transaction_error(self):
        # 異常系のテスト
        # コネクションが既に閉じている場合
        # Connectionインスタンスを生成
        connection = self.create_connection()
        connection.begin_transaction()

        with pytest.raises(ProgrammingError, match='already in transaction'):
            connection.set_readonly(True)

    def test_readonly_status_error(self):
        # 異常系のテスト
        # エラーのステータスが返ってきた場合
        # Connectionインスタンスを生成
        connection = self.create_connection()

        with patch('src.doquedb.client.resultset.ResultSet.get_status',
                   return_value=StatusSet.ERROR.value):
            with pytest.raises(UnexpectedError):
                connection.set_readonly(True)

    def test_close(self):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()
        # カーソルの作成
        connection.cursor()

        connection.close()
        # cursor.is_closedのチェック+session.session_idのチェック
        assert connection._Connection__cursor.is_closed is True
        assert connection._session._Session__session_id == 0
        assert connection._Connection__is_closed is True

    def test_close_no_cursor(self):
        # 正常系のテスト
        # cursorがない場合
        # Connectionインスタンスを生成
        connection = self.create_connection()
        # カーソルを作成しない
        # connection.cursor()

        connection.close()
        # cursor.is_closedのチェック+session.session_idのチェック
        assert connection._Connection__cursor is None
        assert connection._session._Session__session_id == 0
        assert connection._Connection__is_closed is True

    def test_close_twice(self):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()
        # カーソルの作成
        connection.cursor()

        connection.close()
        # ２回closeを読んでも問題ない
        connection.close()
        # cursor.is_closedのチェック+session.session_idのチェック
        assert connection._Connection__cursor.is_closed is True
        assert connection._session._Session__session_id == 0
        assert connection._Connection__is_closed is True

    def test_commit(self, create_table):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()

        # カーソルを作成しDBにテーブルを作成する
        cursor = connection.cursor()
        # execute実行時に自動でトランザクションは開始する
        cursor.execute("INSERT INTO Test_Commit VALUES (1, 'commit test')")
        cursor.fetchall()
        # トランザクション処理が開始したか確認
        assert connection.in_transaction is True

        connection.commit()
        # commitしたのでトランザクションフラグはFalse
        assert connection.in_transaction is False
        # commitした結果を確認
        cursor.execute('SELECT * FROM Test_Commit')
        row = cursor.fetchall()
        assert row == [(1, 'commit test')]

        # 後処理
        # 外部コマンドでsqliを実行し、テーブルを削除する
        delete_table = "/var/lib/DoqueDB/bin/sqli /remote localhost 54321 /database DefaultDB \
            /user root /password doqadmin \
            /sql 'DELETE FROM Test_Commit;'"
        subprocess.run(delete_table, shell=True)

    def test_commit_autocommit_error(self):
        # 異常系のテスト
        # Connectionインスタンスをオートコミットをオンで生成
        connection = Connection('localhost',
                                54321,
                                ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                                self.session,
                                'root',
                                'doqadmin',
                                self.datasource.master_id,
                                'utf-8',
                                autocommit=True
                                )

        with pytest.raises(ProgrammingError):
            connection.commit()

    def test_commit_closed_error(self):
        # 異常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()
        # コネクションを閉じる
        connection.close()

        with pytest.raises(ProgrammingError):
            connection.commit()

    def test_commit_status_error(self):
        # 異常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()
        # トランザクションを開始する
        connection.begin_transaction()

        # get_statusでERRORが返却されるようモックを作成
        with patch('src.doquedb.client.resultset.ResultSet.get_status',
                   return_value=StatusSet.ERROR.value):
            with pytest.raises(InterfaceError):
                connection.commit()

    def test_rollback(self, create_table):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()

        # カーソルを作成しDBにテーブルを作成する
        cursor = connection.cursor()
        # execute実行時に自動でトランザクションは開始する
        cursor.execute(
            "INSERT INTO Test_Rollback VALUES (2, 'rollback test')")
        cursor.fetchall()
        # トランザクション処理が開始したか確認
        assert connection.in_transaction is True

        connection.rollback()
        # rollbackしたのでトランザクションフラグはFalse
        assert connection.in_transaction is False
        cursor.execute('SELECT * FROM Test_Rollback')
        row = cursor.fetchall()
        # インサートした結果が取り消されているか確認
        assert row is None

    def test_rollback_autocommit_error(self):
        # 異常系のテスト
        # Connectionインスタンスをオートコミットをオンで生成
        connection = Connection('localhost',
                                54321,
                                ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                                self.session,
                                'root',
                                'doqadmin',
                                self.datasource.master_id,
                                'utf-8',
                                autocommit=True
                                )

        with pytest.raises(ProgrammingError):
            connection.rollback()

    def test_rollback_closed_error(self):
        # 異常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()
        # コネクションを閉じる
        connection.close()

        with pytest.raises(ProgrammingError):
            connection.rollback()

    def test_rollback_status_error(self):
        # 異常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()
        # トランザクションを開始する
        connection.begin_transaction()

        # get_statusでERRORが返却されるようモックを作成
        with patch('src.doquedb.client.resultset.ResultSet.get_status',
                   return_value=StatusSet.ERROR.value):
            with pytest.raises(InterfaceError):
                connection.rollback()

    def test_cursor(self):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()

        cursor = connection.cursor()
        assert isinstance(cursor, Cursor)
        assert cursor._connection == connection

    def test_cursor_prepared(self):
        # 正常系のテスト
        # プリペアードステートメントを利用する場合
        connection = self.create_connection()

        cursor = connection.cursor(prepared=True)
        assert isinstance(cursor, CursorPrepared)
        assert cursor._connection == connection

    def test_cursor_already_exist(self):
        # 異常系のテスト
        # 既にカーソルを作成していた場合
        # Connectionインスタンスを生成
        connection = self.create_connection()

        cursor = connection.cursor()
        # 元のカーソルをクローズせずに新しいカーソルは作れない
        with pytest.raises(ProgrammingError):
            connection.cursor()

        cursor.close()

    def test_cursor_closed(self):
        # 正常系のテスト
        # 既にカーソルを閉じている場合
        # Connectionインスタンスを生成
        connection = self.create_connection()

        cursor_closed = connection.cursor()
        cursor_closed.close()
        cursor_new = connection.cursor()
        # 新しくインスタンスが生成されたかチェックする
        assert cursor_closed != cursor_new
        assert cursor_new._connection == connection

    def test_begin_transaction_default(self):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()

        with patch('src.doquedb.client.session.Session.execute') as mock:
            # モード指定なしでトランザクションを開始
            connection.begin_transaction()
            # read write モードでisolation levelは read commitedで実行される
            mock.assert_called_once_with(
                'start transaction read write, isolation level read committed')
            assert connection.in_transaction is True

    def test_begin_transaction_isolevel(self):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()

        with patch('src.doquedb.client.session.Session.execute') as mock:
            # isolevelをread commitedに設定する
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_READ_COMMITTED.value)
            connection.begin_transaction()
            # read write モードでisolation levelは read commitedで実行される
            mock.assert_called_with(
                'start transaction read write, isolation level read committed')
            connection.rollback()

            # isolevelをread uncommitedに設定する
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_READ_UNCOMMITTED.value)
            connection.begin_transaction()
            # read write モードでisolation levelは read uncommitedで実行される
            mock.assert_called_with(
                'start transaction read write, isolation level read uncommitted')
            connection.rollback()

            # isolevelをrepeatable readに設定する
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_REPEATABLE_READ.value)
            connection.begin_transaction()
            # read write モードでisolation levelはrepeatable readで実行される
            mock.assert_called_with(
                'start transaction read write, isolation level repeatable read')
            connection.rollback()

            # isolevelをserializableに設定する
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_SERIALIZABLE.value)
            connection.begin_transaction()
            # read write モードでisolation levelはserializableで実行される
            mock.assert_called_with(
                'start transaction read write, isolation level serializable')
            connection.rollback()

            # isolevelをusing snapshotに設定する
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_USING_SNAPSHOT.value)
            connection.begin_transaction()
            # isolation levelはusing snapshotで実行される
            mock.assert_called_with(
                'start transaction read only, using snapshot')
            # snapshot は readonlyで実行される
            assert connection.readonly is True
            connection.rollback()

            # readonlyがTrueとなった状態で isolevelをread commitedに設定する
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_READ_COMMITTED.value)
            connection.begin_transaction()
            # transaction modeはread only, isolation levelは read commitedで実行される
            mock.assert_called_with(
                'start transaction read only, isolation level read committed')
            # isolation_level が変更されたか確認
            connection.isolation_level == \
                Connection.isolevel.TRANSACTION_READ_COMMITTED.value
            # snapshot で readonlyに設定されたのでTrueのまま
            assert connection.readonly is True

    def test_begin_transaction_readonly(self, create_table):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()
        connection.set_readonly(True)
        cursor = connection.cursor()

        # モード指定なしでトランザクションを開始
        connection.begin_transaction()
        assert connection.in_transaction is True
        # 書き込みができないことを確認
        cursor.execute(
            "INSERT INTO Test_Transaction VALUES (1, 'transaction readonly test')")

        with pytest.raises(ReadOnlyTransaction):
            cursor.fetchall()

    def test_begin_transaction_readonly_readwrite(self, create_table):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()
        # readonlyをTrueに設定
        connection.set_readonly(True)
        cursor = connection.cursor()

        # ReadWriteモードを指定してトランザクションを開始
        connection.begin_transaction(
            transaction_mode=TransactionMode.TRANSACTION_MODE_READ_WRITE.value)
        assert connection.in_transaction is True
        # readonlyフラグは変わらずTrueになる
        assert connection.readonly is True
        # 書き込みができることを確認
        cursor.execute(
            "INSERT INTO Test_Transaction VALUES (2, 'transaction read write test')")
        cursor.fetchall()
        cursor.execute('SELECT * FROM Test_Transaction')
        rows = cursor.fetchall()
        assert rows == [(2, 'transaction read write test')]

    def test_beign_transaction_error_wrong_arg(self):
        # 異常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()

        # 存在しないモードを指定する
        with pytest.raises(ProgrammingError):
            connection.begin_transaction(-1)

    def test_beign_transaction_error_autocommit(self):
        # 異常系のテスト
        # Connectionインスタンスを生成
        connection = Connection('localhost',
                                54321,
                                ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                                self.session,
                                'root',
                                'doqadmin',
                                self.datasource.master_id,
                                'utf-8',
                                autocommit=True
                                )

        # autocommitがonで実行する
        with pytest.raises(ProgrammingError):
            connection.begin_transaction()

    def test_set_transaction_isolation(self):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()

        with patch('src.doquedb.client.session.Session.execute') as mock:
            # isolevelをread commitedに設定する
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_READ_COMMITTED.value)
            # isolation levelは read commitedで実行される
            mock.assert_called_with(
                'set transaction isolation level read committed')
            # isolation_level が変更されたか確認
            connection.isolation_level == \
                Connection.isolevel.TRANSACTION_READ_COMMITTED.value

            # isolevelをread uncommitedに設定する
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_READ_UNCOMMITTED.value)
            # isolation levelは read uncommitedで実行される
            mock.assert_called_with(
                'set transaction isolation level read uncommitted')
            # isolation_level が変更されたか確認
            connection.isolation_level == \
                Connection.isolevel.TRANSACTION_READ_UNCOMMITTED.value

            # isolevelをrepeatable readに設定する
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_REPEATABLE_READ.value)
            # isolation levelはrepeatable readで実行される
            mock.assert_called_with(
                'set transaction isolation level repeatable read')
            # isolation_level が変更されたか確認
            connection.isolation_level == \
                Connection.isolevel.TRANSACTION_REPEATABLE_READ.value

            # isolevelをserializableに設定する
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_SERIALIZABLE.value)
            # isolation levelはserializableで実行される
            mock.assert_called_with(
                'set transaction isolation level serializable')
            # isolation_level が変更されたか確認
            connection.isolation_level == \
                Connection.isolevel.TRANSACTION_SERIALIZABLE.value

            # isolevelをusing snapshotに設定する
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_USING_SNAPSHOT.value)
            # isolation levelはusing snapshotで実行される
            mock.assert_called_with(
                'set transaction read only, using snapshot')
            # snapshot は readonlyで実行される
            assert connection.readonly is True
            # isolation_level が変更されたか確認
            connection.isolation_level == \
                Connection.isolevel.TRANSACTION_USING_SNAPSHOT.value

    def test_set_transaction_isolation_error_closed(self):
        # 異常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()
        # コネクションを閉じる
        connection.close()

        # コネクションは既にクローズしているのでエラーとなる
        with pytest.raises(ProgrammingError):
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_READ_COMMITTED)

    def test_set_transaction_isolation_error_in_transaction(self):
        # 異常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()
        connection.begin_transaction()

        # トランザクション中は遮断レベルを変更できない
        with pytest.raises(ProgrammingError):
            connection.set_transaction_isolation(
                connection.isolevel.TRANSACTION_READ_COMMITTED)

    def test_set_transaction_isolation_error_wrong_arg(self):
        # 異常系のテスト
        # Connectionインスタンスを生成
        connection = self.create_connection()

        # 存在しない遮断レベルを指定する
        with pytest.raises(ProgrammingError):
            connection.set_transaction_isolation(-1)
