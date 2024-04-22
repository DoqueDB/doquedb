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
test_cursor.py -- src.doquedb.driver.cursor モジュールのテスト
"""
import pytest
from unittest.mock import patch

import platform
import subprocess

from src.doquedb.driver.connection import Connection
from src.doquedb.client.constants import ProtocolVersion
from src.doquedb.client.datasource import DataSource
from src.doquedb.client.resultset import ResultSet
from src.doquedb.driver.dbapi import DATETIME
from src.doquedb.driver.cursor import Cursor, CursorPrepared
from src.doquedb.client.prepare_statement import PrepareStatement
from src.doquedb.exception.exceptions import (
    NotSupportedError, ProgrammingError)


@pytest.fixture(scope='module')
def create_db():
    # クラス単位のセットアップ作業
    # 外部コマンドでsqliを実行し、テストに使用するテーブルを作成する
    if platform.system() == 'Linux':
        create_table = "sqli -remote localhost 54321 -database DefaultDB \
            -user root -password doqadmin \
            -sql 'DROP TABLE Test_Cursor IF EXISTS;\
            CREATE TABLE Test_Cursor (\
            test_int INT,\
            test_float FLOAT,\
            test_char NVARCHAR(100),\
            test_datetime DATETIME);'"
        subprocess.run(create_table, shell=True)
    else:
        raise Exception('Windows, Macは非対応')

    yield

    # クラス単位のティアダウン作業
    # 外部コマンドでsqliを実行し、テーブルを削除する
    delete_table = "sqli /remote localhost 54321 /database DefaultDB \
        /user root /password doqadmin \
        /sql DROP TABLE Test_Cursor IF EXISTS;"
    subprocess.run(delete_table, shell=True)


class TestCursor():
    """
    Notes:
        事前準備として、ローカルのDoqueDBサーバを以下の設定で作成する.
        ホスト名: localhost
        ポート番号: 54321
        ユーザー名: root
        パスワード: doqadmin
        データベース名: DefaultDB

        外部コマンドを実行するために、システム環境変数のPATHにC:\\Program Files\\Ricoh\\doquedb
        を登録する。
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
        # Connectionインスタンスを生成する
        self.connection = Connection('localhost',
                                     54321,
                                     ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                                     self.session,
                                     'root',
                                     'doqadmin',
                                     self.datasource.master_id,
                                     'utf-8'
                                     )

    def teardown_method(self, method):
        # 後処理
        # datasourceをcloseする
        self.datasource.close()

        # ティアダウン
        # 外部コマンドでsqliを実行し、テーブルの中身を削除する
        if platform.system() == 'Linux':
            delete_table = "sqli -remote localhost 54321 -database DefaultDB\
                -user root -password doqadmin\
                -sql 'DELETE FROM Test_Cursor'"
            subprocess.run(delete_table, shell=True)
        else:
            raise Exception('Windows, Macは非対応')

    def test_init(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = Cursor(self.connection)

        # 初期値が正しくセットされているかチェック
        assert cursor._connection == self.connection
        assert cursor.arraysize == 1
        assert cursor.description == []
        assert cursor._Cursor__metadata is None
        assert cursor._resultset is None
        assert cursor.is_closed is False

    def test_setter_arraysize(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        cursor.arraysize = 1
        assert cursor.arraysize == 1

    def test_setter_arraysize_error(self, create_db):
        # 異常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        # arraysizeに0を指定
        with pytest.raises(ProgrammingError):
            cursor.arraysize = 0

        # arraysizeに負の値を設定
        with pytest.raises(ProgrammingError):
            cursor.arraysize = -1

    def test_getter_rowcount(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()
        # リザルトセットの取得
        cursor.execute('SELECT * FROM Test_Cursor')

        assert cursor.rowcount == 0

    def test_getter_rowcount_no_resultset(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        assert cursor.rowcount == -1

    def test_close(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        cursor.close()
        assert cursor._resultset is None
        assert cursor.arraysize == 1
        assert cursor.description == []
        assert cursor._Cursor__metadata is None
        assert cursor._Cursor__is_closed is True

    def test_close_resultset_exist(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()
        # リザルトセットの取得
        cursor.execute('SELECT * FROM Test_Cursor')

        cursor.close()
        assert cursor._resultset is None
        assert cursor.arraysize == 1
        assert cursor.description == []
        assert cursor._Cursor__metadata is None
        assert cursor._Cursor__is_closed is True

    def test_close_twice(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        cursor.close()
        # ２回クローズしても問題ない
        cursor.close()
        assert cursor._resultset is None
        assert cursor.arraysize == 1
        assert cursor.description == []
        assert cursor._Cursor__metadata is None
        assert cursor._Cursor__is_closed is True

    def test_execute(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        cursor.execute('SELECT * FROM Test_Cursor')
        row = cursor.fetchone()
        assert row is None
        assert isinstance(cursor._resultset, ResultSet)

    def test_execute_parameter(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        values = (1, 1.1, '1', DATETIME(2021, 1, 1, 1, 1, 1))
        cursor.execute('INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)', values)
        cursor.execute('SELECT * FROM Test_Cursor')
        row = cursor.fetchall()
        assert row == [values]

    def test_execute_error_cursor(self, create_db):
        # 異常系のテスト
        # カーソルがない場合
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()
        # カーソルを閉じる
        cursor.close()

        with pytest.raises(ProgrammingError):
            cursor.execute('SELECT * FROM Test_Cursor')

    def test_execute_error_protocol(self, create_db):
        # 異常系のテスト
        # プロトコルバージョンが古い場合
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()
        # プロトコルバージョンを3以下に設定
        cursor._connection._Connection__master_id = 1

        with pytest.raises(NotSupportedError):
            cursor.execute('SELECT * FROM Test_Cursor')

    def test_execute_error_arg(self, create_db):
        # 異常系のテスト
        # operationが空だった場合
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        # stringの中身がない場合
        with pytest.raises(ProgrammingError):
            cursor.execute('')

        # str以外の型だった場合
        with pytest.raises(ProgrammingError):
            cursor.execute(b'1')

    def test_execute_check_sql(self, create_db):
        # 正常系のテスト
        # 一回目実行時に不正なSQLを実行し、次に有効なSQLを実行した場合
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        # 誤ったSQL文を実行
        cursor.execute('wrong sql')
        # 次のSQL文には前回のクエリは影響しない
        cursor.execute('SELECT * FROM Test_Cursor')
        row = cursor.fetchone()
        assert row is None
        assert isinstance(cursor._resultset, ResultSet)

    def test_executemany(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        values = [(1, 1.1, '１', DATETIME(2021, 1, 1)),
                  (2, 2.2, '２', DATETIME(2022, 2, 2))]

        cursor.executemany(
            'INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)', values)
        cursor.execute('SELECT * FROM Test_Cursor ORDER BY test_int ASC')
        rows = cursor.fetchall()
        assert rows == values

    def test_executemany_error(self, create_db):
        # 異常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        # パラメータのリストが空だった場合
        with pytest.raises(ProgrammingError):
            cursor.executemany(
                'INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)', [])

        # パラメータのリストがNoneだった場合
        with pytest.raises(ProgrammingError):
            cursor.executemany(
                'INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)', None)

        # パラメータをリストではない型を渡した場合
        with pytest.raises(TypeError):
            cursor.executemany(
                'INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)', int(1))

    def test_fetchone(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        values = [(-1, -1.1, 'いち', DATETIME(2021, 1, 1)),
                  (-2, -2.2, 'に', DATETIME(2022, 2, 2))]

        # 読み込むためのデータを登録
        cursor.executemany(
            'INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)', values)
        cursor.execute('SELECT * FROM Test_Cursor ORDER BY test_int DESC')
        # 読込み
        row1 = cursor.fetchone()
        row2 = cursor.fetchone()
        # １行目をチェック
        assert row1 == values[0]
        # ２行目をチェック
        assert row2 == values[1]
        # 読込む行がもうない場合はNone
        row3 = cursor.fetchone()
        assert row3 is None

    def test_fetchone_no_resultset(self, create_db):
        # 異常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        values = [(-1, -1.1, '1', DATETIME(2021, 1, 1)),
                  (-2, -2.2, '2', DATETIME(2022, 2, 2))]

        # 読み込むためのデータを登録
        cursor.executemany(
            'INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)', values)
        # SELECT文を実行せず読み込む
        row = cursor.fetchone()
        # 読込みをチェック
        assert row is not None

    def test_fetchmany(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        values = [(1, 1.1, '一', DATETIME(2021, 1, 1)),
                  (2, 2.2, '二', DATETIME(2022, 2, 2)),
                  (3, 3.3, '三', DATETIME(2023, 3, 3))]

        # 読み込むためのデータを登録
        cursor.executemany(
            'INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)', values)
        cursor.execute('SELECT * FROM Test_Cursor ORDER BY test_int ASC')
        # 読込み
        rows1 = cursor.fetchmany(2)
        row2 = cursor.fetchmany(1)
        # １、２行目をチェック
        assert rows1 == [values[0], values[1]]
        # ３行目をチェック
        assert row2 == [values[2]]
        # 読込む行がもうない場合はNone
        row3 = cursor.fetchmany()
        assert row3 is None

    def test_fetchmany_arraysize(self, create_db):
        # 正常系のテスト
        # arraysizeで読み込む行数を指定した場合
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        values = [(1, 1.1, '一', DATETIME(2021, 1, 1)),
                  (2, 2.2, '二', DATETIME(2022, 2, 2)),
                  (3, 3.3, '三', DATETIME(2023, 3, 3))]

        # 読み込むためのデータを登録
        cursor.executemany(
            'INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)', values)
        cursor.execute('SELECT * FROM Test_Cursor ORDER BY test_int ASC')
        # arraysizeを2に設定し読み込む
        cursor.arraysize = 2
        rows1 = cursor.fetchmany()
        # arraysizeを1に設定し読み込む
        cursor.arraysize = 1
        row2 = cursor.fetchmany()
        # １、２行目をチェック
        assert rows1 == [values[0], values[1]]
        # ３行目をチェック
        assert row2 == [values[2]]
        # 読込む行がもうない場合はNone
        row3 = cursor.fetchmany()
        assert row3 is None

    def test_fetchmany_error_size(self, create_db):
        # 異常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        values = [(1, 1.1, '一', DATETIME(2021, 1, 1)),
                  (2, 2.2, '二', DATETIME(2022, 2, 2)),
                  (3, 3.3, '三', DATETIME(2023, 3, 3))]

        # 読み込むためのデータを登録
        cursor.executemany(
            'INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)', values)
        cursor.execute('SELECT * FROM Test_Cursor ORDER BY test_int ASC')

        # サイズを-1に設定して実行
        with pytest.raises(ProgrammingError,
                           match='size should be greater or equal to 1'):
            cursor.fetchmany(-1)

        # サイズを0に設定して実行
        with pytest.raises(ProgrammingError,
                           match='size should be greater or equal to 1'):
            cursor.fetchmany(0)

        # サイズを0.5に設定して実行
        with pytest.raises(ProgrammingError,
                           match='size should be greater or equal to 1'):
            cursor.fetchmany(0.5)

    def test_fetchmany_error_no_resulset(self, create_db):
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        # リザルトセットを取得せずに実行
        with pytest.raises(ProgrammingError, match='no results to read'):
            cursor.fetchmany()

    def test_fetchall(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        values = [(1, 1.1, '1', DATETIME(2021, 1, 1)),
                  (2, 2.2, '2', DATETIME(2022, 2, 2))]

        # 読み込むためのデータを登録
        cursor.executemany(
            'INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)', values)
        cursor.execute('SELECT * FROM Test_Cursor ORDER BY test_int ASC')
        # 読込み
        rows = cursor.fetchall()
        # 読み込んだデータをチェック
        assert rows == values
        rows = cursor.fetchall()
        assert rows is None

    def test_fetchall_error(self, create_db):
        # 異常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = self.connection.cursor()

        # リザルトセットを読み込んでいないのでエラーとなる
        with pytest.raises(ProgrammingError):
            cursor.fetchall()

    @ pytest.mark.skip(reason='未実装')
    def test_setinputsize(self, create_db):
        # 正常系のテスト
        # TODO: 未実装
        pass

    @ pytest.mark.skip(reason='未実装')
    def test_setoutputsize(self, create_db):
        # 正常系のテスト
        # TODO: 未実装
        pass

    @ pytest.mark.skip(reason='未実装')
    def test_callproc(self, create_db):
        # 正常系のテスト
        # TODO: 未実装
        pass

    @ pytest.mark.skip(reason='未実装')
    def test_cancel(self, create_db):
        # 正常系のテスト
        # TODO: 未実装
        pass


class TestCursorPrepared():
    """
    Notes:
        事前準備として、ローカルのDoqueDBサーバを以下の設定で作成する.
        ホスト名: localhost
        ポート番号: 54321
        ユーザー名: root
        パスワード: doqadmin
        データベース名: DefaultDB

        外部コマンドを実行するために、システム環境変数のPATHにC:\\Program Files\\Ricoh\\doquedb
        を登録する。
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
        # Connectionインスタンスを生成する
        self.connection = Connection('localhost',
                                     54321,
                                     ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                                     self.session,
                                     'root',
                                     'doqadmin',
                                     self.datasource.master_id,
                                     'utf-8'
                                     )

    def teardown_method(self, method):
        # 後処理
        # datasourceをcloseする
        self.datasource.close()

        # ティアダウン
        # 外部コマンドでsqliを実行し、テーブルの中身を削除する
        if platform.system() == 'Linux':
            delete_table = "sqli -remote localhost 54321 -database DefaultDB\
                -user root -password doqadmin\
                -sql 'DELETE FROM Test_Cursor'"
            subprocess.run(delete_table, shell=True)
        else:
            raise Exception('Windows, Macは非対応')

    def test_init(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = CursorPrepared(self.connection)

        # 初期値が正しくセットされているかチェック
        assert cursor._connection == self.connection

    def test_execute(self, create_db):
        # 正常系のテスト
        # 前準備
        # カーソルインスタンスを生成
        cursor = CursorPrepared(self.connection)

        # 実行するSQL文
        sql_select = 'SELECT * FROM Test_Cursor ORDER BY test_int ASC'
        sql_insert = 'INSERT INTO Test_Cursor VALUES (?, ?, ?, ?)'

        cursor.execute(sql_select)
        # プリペアードステートメントが登録されたかチェック
        prepared_select = cursor._connection._session.prepared_map[
            sql_select]
        assert isinstance(prepared_select, PrepareStatement)
        row = cursor.fetchone()
        assert row is None
        assert isinstance(cursor._resultset, ResultSet)

        # 別のプリペアードステートメントを作成し、機能するかチェック
        values = [(1, 1.1, '1', DATETIME(2021, 1, 1)),
                  (2, 2.2, '2', DATETIME(2022, 2, 2))]

        # valuesの0番目の要素をインサートする
        cursor.execute(sql_insert, values[0])
        # プリペアードステートメントが登録されたかチェック
        prepared_insert = cursor._connection._session.prepared_map[
            sql_insert]
        assert isinstance(prepared_insert, PrepareStatement)
        # テーブルに値が挿入されたかチェック
        cursor.execute(sql_select)
        # 読込み
        rows = cursor.fetchall()
        # 読み込んだデータをチェック
        assert rows == [values[0]]

        # create()が実行されていないことを確認するためにモックに差し替える
        with patch('src.doquedb.client.prepare_statement.PrepareStatement.create') as mock:
            # valuesの1番目の要素をインサートする
            cursor.execute(sql_insert, values[1])
            # create()が実行されていないことを確認
            mock.assert_not_called()

        # テーブルに値が挿入されたかチェック
        cursor.execute(sql_select)

        # 読込み
        rows = cursor.fetchall()
        # 読み込んだデータをチェック
        assert rows == values
