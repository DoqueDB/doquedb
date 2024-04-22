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
test_scenario.py -- doquedbのシナリオテスト
"""


import pytest
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from src.doquedb.common.data import WordData

import platform
import subprocess
import locale
import numpy as np

import src.doquedb as dq
from src.doquedb.exception.database_exceptions import (
    AlreadyBeginTransaction,
    AuthorizationFailed,
    DynamicParameterNotMatch,
    SQLSyntaxError,
    InvalidExplicitTransaction)
from src.doquedb.driver.cursor import Cursor
from src.doquedb.exception.exceptions import DatabaseError


@pytest.fixture(scope='module')
def create_db():
    # クラス単位のセットアップ作業
    # 外部コマンドでsqliを実行し、テストに使用するテーブルを作成する
    if platform.system() == 'Linux':
        create_table = "sqli -remote localhost 54321 -database DefaultDB \
            -user root -password doqadmin -sql \
            'CREATE TABLE Test_Scenario (\
            test_int INT,\
            test_float FLOAT,\
            test_char NVARCHAR(100),\
            test_datetime DATETIME);\
            CREATE TABLE Test_Binary (\
            id INT,\
            test_binary BINARY(400),\
            test_image IMAGE);\
            CREATE TABLE Test_Decimal (\
            id INT,\
            prec5_scale0 DECIMAL(5, 0),\
            prec5_scale2 DECIMAL(5, 2),\
            prec5_scale5 DECIMAL(5, 5));\
            CREATE TABLE Test_Language (\
            id INT,\
            test_language LANGUAGE);\
            CREATE TABLE Test_Word (\
            id INT,\
            data NVARCHAR(no limit),\
            l LANGUAGE);'"
        subprocess.run(create_table, shell=True)
    else:
        raise Exception('Windows, Macは非対応')

    yield

    # クラス単位のティアダウン作業
    # 外部コマンドでsqliを実行し、テーブルを削除する
    delete_table = "sqli -remote localhost 54321 -database DefaultDB \
        -user root -password doqadmin -sql \
        'DROP TABLE Test_Scenario IF EXISTS;\
        DROP TABLE Test_CreateTable IF EXISTS;\
        DROP TABLE Test_Binary IF EXISTS;\
        DROP TABLE Test_Decimal IF EXISTS;\
        DROP TABLE Test_Language IF EXISTS;\
        DROP TABLE Test_Word IF EXISTS;'"
    subprocess.run(delete_table, shell=True)


class TestScenario:
    """
    Notes:
        事前準備として、ローカルのDoqueDBサーバを以下の設定で作成する.
        ホスト名: localhost
        ポート番号: 54321
        ユーザー名: root
        パスワード: doqadmin
        データベース名: DefaultDB
    """

    def teardown_method(self, method):
        # ティアダウン
        dq.close()

        # 外部コマンドでsqliを実行し、テーブルの中身を削除する
        if platform.system() == 'Linux':
            delete_table = "sqli -remote localhost 54321 -database DefaultDB\
                -user root -password doqadmin\
                -sql 'DELETE FROM Test_Scenario'"
            subprocess.run(delete_table, shell=True)
        else:
            raise Exception('Windows, Macは非対応')

    def test_create_table_and_insert_values(self, create_db):
        # 正常系
        # autocommitがオンの場合CREATE文が実行できることを確認
        conn_autocommit = dq.connect(host='localhost',
                                     port=54321,
                                     dbname='DefaultDB',
                                     user='root',
                                     password='doqadmin',
                                     autocommit=True)
        conn_transaction = dq.connect(host='localhost',
                                      port=54321,
                                      dbname='DefaultDB',
                                      user='root',
                                      password='doqadmin',
                                      autocommit=False)

        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur_autocommit = conn_autocommit.cursor()
        cur_transaction = conn_transaction.cursor()

        # autocommitがオンなのでCREATE文が実行できる
        cur_autocommit.execute(
            'CREATE TABLE Test_CreateTable (test_int INT, test_char VARCHAR(100))')
        cur_autocommit.fetchall()

        # トランザクションを開始してインサート文を実行する
        values = [(1, 'autocommit'), (2, 'transaction')]
        conn_transaction.begin_transaction()
        cur_transaction.executemany(
            'INSERT INTO Test_CreateTable VALUES (?, ?)', values)
        # コミットする
        conn_transaction.commit()

        # 結果を取得する
        cur_autocommit.execute('SELECT * FROM Test_CreateTable')
        rows = cur_autocommit.fetchall()
        assert rows == values

        # 後処理
        cur_autocommit.execute('DROP TABLE Test_CreateTable')
        cur_autocommit.close()
        cur_transaction.close()
        conn_autocommit.close()
        conn_transaction.close()

    def test_error_create_table(create_db):
        # 異常系
        # トランザクションモードでCREATE文を実行した場合
        # Connectionオブジェクトをコンストラクト
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin')
        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor()

        # トランザクション機能がオンの時にDDL文は実行できない
        with pytest.raises(InvalidExplicitTransaction):
            cur.execute(
                'CREATE TABLE Test_CreateTable (test_int INT, test_char VARCHAR(100))')
            cur.fetchall()

    def test_error_parameters_not_match(create_db):
        # 異常系
        # エラーが正しく取得されるかチェックする
        # executeのパラメータの数が一致しない場合
        # Connectionオブジェクトをコンストラクト
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin')
        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor()
        values = (1, 1.1, '1')
        cur.execute(
            'INSERT INTO Test_Scenario VALUES (?, ?, ?, ?)', values)

        with pytest.raises(DynamicParameterNotMatch) as dpnm:
            cur.fetchall()

        # ロケールを調べる
        if locale.getdefaultlocale()[0] == 'ja_JP':
            # 日本語のエラーメッセージを検証する
            assert dpnm.value.error_message == \
                'パラメーター値の数(3)が必要な数(4)と一致しません。'
            assert str(dpnm.value) == \
                'パラメーター値の数(3)が必要な数(4)と一致しません。'
        else:
            # 日本以外は英語
            assert dpnm.value.error_message == \
                'The number of parameter values(3) does not match parameters(4).'

        # 後処理
        cur.close()
        conn.close()

    def test_error_auth_failed(create_db):
        # 異常系
        # エラーが正しく取得されるかチェックする
        # 認証に失敗した場合
        with pytest.raises(AuthorizationFailed) as af:
            dq.connect(host='localhost',
                       port=54321,
                       dbname='DefaultDB',
                       user='root',
                       password='wrong_password')

        # ロケールを調べる
        if locale.getdefaultlocale()[0] == 'ja_JP':
            # 日本語のエラーメッセージを検証する
            assert af.value.error_message == \
                '認証に失敗しました。'
        else:
            # 日本以外は英語
            assert af.value.error_message == \
                'Connection exception - authorization failed.'

    def test_error_bad_syntax(self, create_db):
        # 異常系
        # エラーが正しく取得されるかチェックする
        # SQL文のシンタックスエラーが発生した場合
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin')
        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor()
        cur.execute(
            'SELECT * FRM Test_Scenario')

        with pytest.raises(SQLSyntaxError) as sse:
            cur.fetchall()

        # ロケールを調べる
        if locale.getdefaultlocale()[0] == 'ja_JP':
            # 日本語のエラーメッセージを検証する
            assert sse.value.error_message == \
                'SQL文の構文エラー: \' near token IDENTIFIER "FRM" at line 1\''
        else:
            # 日本以外は英語
            assert sse.value.error_message == \
                'SQL syntax error \' near token 50 "FRM" at line 1\'.'

        # 後処理
        cur.close()
        conn.close()

    def test_error_begin_transaction(self, create_db):
        # 異常系
        # トランザクション開始のsqlをexecuteで実行した場合
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin')

        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor(prepared=False)
        # トランザクション開始のsqlを実行
        cur.execute('start transaction read write')
        # execute実行時に既にトランザクションが自動で開始されているため、例外が上がる
        with pytest.raises(AlreadyBeginTransaction):
            cur.fetchall()

    def test_begin_transaction_while_autocommit(self, create_db):
        # 正常系
        # autocommit=Trueでコネクションを作成
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin',
                          autocommit=True)
        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor(prepared=False)

        # トランザクションを明示的に開始する
        cur.execute(
            'start transaction read write, isolation level read committed')
        # 明示的に開始した場合内部のフラグは変わらない
        assert conn.in_transaction is False
        # インサート文を実行
        cur.execute(
            "INSERT INTO Test_Scenario VALUES (1, 1.1, '1', '2021-1-1')")
        cur.fetchall()
        # コミット
        cur.execute('commit')
        cur.fetchall()
        # コミットされた結果を確認
        cur.execute('select * from Test_Scenario')
        row_commit = cur.fetchall()
        assert row_commit == [
            (1, 1.1, '1', dq.DATETIME(2021, 1, 1))]

        # 再度トランザクションを明示的に開始する
        cur.execute(
            'start transaction read write, isolation level read committed')
        # 明示的に開始した場合内部のフラグは変わらない
        assert conn.in_transaction is False
        # インサート文を実行
        cur.execute(
            "INSERT INTO Test_Scenario VALUES (2, 2.2, '2', '2022-2-2')")
        cur.fetchall()
        # ロールバック
        cur.execute('rollback')
        cur.fetchall()
        # ロールバックされた結果を確認
        cur.execute('select * from Test_Scenario')
        row_rollback = cur.fetchall()
        assert row_rollback == [
            (1, 1.1, '1', dq.DATETIME(2021, 1, 1))]

        # 後処理
        cur.close()
        conn.close()

    def test_transaction_setting_change_while_autocommit(self, create_db):
        # 異常系
        # autocommit=Trueでコネクションを作成
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin',
                          autocommit=True)
        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor(prepared=False)

        # トランザクションを明示的に開始する
        cur.execute(
            'start transaction read write, isolation level read committed')
        # 明示的に開始した場合内部のフラグは変わらない
        assert conn.in_transaction is False

        # インサート文を実行
        cur.execute(
            "INSERT INTO Test_Scenario VALUES (1, 1.1, '1', '2021-1-1')")
        cur.fetchall()
        # set_transaction_isolationは問題なく実行できる
        conn.set_transaction_isolation(
            conn.isolevel.TRANSACTION_SERIALIZABLE.value)
        # コミット
        cur.execute('commit')
        cur.fetchall()
        # コミットされた結果を確認
        cur.execute('select * from Test_Scenario')
        rows = cur.fetchall()
        assert rows == [
            (1, 1.1, '1', dq.DATETIME(2021, 1, 1))]

        # autocommitでsqlが正常に実行できるか確認
        cur.execute('select * from Test_Scenario')
        rows = cur.fetchall()
        assert rows == [
            (1, 1.1, '1', dq.DATETIME(2021, 1, 1))]

        # トランザクションを明示的に開始する
        cur.execute('start transaction read write')
        # 明示的に開始した場合内部のフラグは変わらない
        assert conn.in_transaction is False
        # set_readonlyは問題なく実行できる
        conn.set_readonly(True)
        # read_onlyは現在のトランザクションには影響しない（INSERTできる）
        cur.execute(
            "INSERT INTO Test_Scenario VALUES (2, 2.2, '2', '2022-2-2')")
        cur.fetchall()
        # コミット
        cur.execute('commit')
        cur.fetchall()
        # インサート文が実行されたことを確認
        cur.execute('select * from Test_Scenario order by test_int')
        rows = cur.fetchall()
        assert rows == [
            (1, 1.1, '1', dq.DATETIME(2021, 1, 1)),
            (2, 2.2, '2', dq.DATETIME(2022, 2, 2))]
        # autocommitでread onlyとなっていることを確認
        with pytest.raises(DatabaseError):
            cur.execute(
                "INSERT INTO Test_Scenario VALUES (3, 3.3, '3', '2023-3-3')")
            cur.fetchall()

        # 後処理
        cur.close()
        conn.close()

    def test_prepared_cursor(self, create_db):
        # 正常系
        # 通常のカーソルを作成して閉じたのち、プリペアードステートメント対応のカーソルを作成する
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin')

        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor(prepared=False)
        # インサートする値
        values = [(1, 1.1, '１', dq.DATETIME(2021, 1, 1)),
                  (2, 2.2, '２', dq.DATETIME(2022, 2, 2))]
        # インサート文の実行（valuesの第１要素）
        cur.execute('INSERT INTO Test_Scenario VALUES (?, ?, ?, ?)', values[0])
        # カーソルを閉じる
        cur.close()

        # カーソルをプリペアードステートメント対応モードで作成
        cur_prepared = conn.cursor(prepared=True)
        # インサート文の実行（valuesの第１要素）
        cur_prepared.execute(
            'INSERT INTO Test_Scenario VALUES (?, ?, ?, ?)', values[1])
        # セレクト文の実行と結果の取得
        cur_prepared.execute(
            'SELECT * FROM Test_Scenario ORDER BY test_int ASC')
        rows = cur_prepared.fetchall()
        # 正しくインサートできたかチェック
        assert rows == values

        # 一度カーソルを閉じて再度カーソルを作成する
        cur_prepared.close()
        cur_prepared = conn.cursor(prepared=True)
        # デリート文の実行
        cur_prepared.execute('DELETE FROM Test_Scenario')
        # プリペアードステートメントがマップに残っているか確認
        assert conn._session.prepared_map[
            'INSERT INTO Test_Scenario VALUES (?, ?, ?, ?)'
        ] is not None
        assert conn._session.prepared_map[
            'SELECT * FROM Test_Scenario ORDER BY test_int ASC'
        ] is not None
        assert conn._session.prepared_map[
            'DELETE FROM Test_Scenario'
        ] is not None

        # 再度prepared=Falseでカーソルを作成
        cur_prepared.close()
        cur = conn.cursor(prepared=False)
        # Cursorとして作成されたか確認
        assert type(cur) is Cursor
        # セレクト文を実行し結果を読込む
        cur.execute(
            'SELECT * FROM Test_Scenario ORDER BY test_int ASC')
        rows = cur.fetchall()
        # テーブルの内容を削除したので結果は取得されない
        assert rows is None

        # 後処理
        cur.close()
        conn.close()

    def test_prepared_multiple_connection(self, create_db):
        # 同じポートで２つのコネクションを作る
        conn1 = dq.connect(host='localhost',
                           port=54321,
                           dbname='DefaultDB',
                           user='root',
                           password='doqadmin')
        conn2 = dq.connect(host='localhost',
                           port=54321,
                           dbname='DefaultDB',
                           user='root',
                           password='doqadmin')
        # カーソルをそれぞれのコネクションに対して作成
        cur1 = conn1.cursor(prepared=True)
        cur2 = conn2.cursor(prepared=True)

        # それぞれのカーソルに対して別のSQL文を実行してプリペアードステートメントを作成する
        sql_select = 'SELECT * FROM Test_Scenario ORDER BY test_int ASC'
        sql_insert = 'INSERT INTO Test_Scenario VALUES (?, ?, ?, ?)'
        value = (1, 1.1, '１', dq.DATETIME(2021, 1, 1))
        cur1.execute(sql_select)
        cur2.execute(sql_insert, value)

        # プリペアードステートメントが正しく保存されたかチェック
        assert conn1._session.prepared_map[sql_select] is not None
        with pytest.raises(KeyError):
            conn1._session.prepared_map[sql_insert]
        assert conn2._session.prepared_map[sql_insert] is not None
        with pytest.raises(KeyError):
            conn2._session.prepared_map[sql_select]

        # execute結果をフェッチする
        cur1.fetchall()
        cur2.fetchall()

        # 後処理
        cur1.close()
        cur2.close()
        conn1.close()
        conn2.close()

    def test_error_parameters_not_match_prepared(self, create_db):
        # 異常系
        # エラーが正しく取得されるかチェックする
        # executeのパラメータの数が一致しない場合（プリペアードステートメント）
        # Connectionオブジェクトをコンストラクト
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin')
        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor(prepared=True)
        values = (1, 1.1, '1')
        cur.execute(
            'INSERT INTO Test_Scenario VALUES (?, ?, ?, ?)', values)

        with pytest.raises(DynamicParameterNotMatch) as dpnm:
            cur.fetchall()

        # ロケールを調べる
        if locale.getdefaultlocale()[0] == 'ja_JP':
            # 日本語のエラーメッセージを検証する
            assert dpnm.value.error_message == \
                'パラメーター値の数(3)が必要な数(4)と一致しません。'
            assert str(dpnm.value) == \
                'パラメーター値の数(3)が必要な数(4)と一致しません。'
        else:
            # 日本以外は英語
            assert dpnm.value.error_message == \
                'The number of parameter values(3) does not match parameters(4).'

        # 後処理
        cur.close()
        conn.close()

    def test_error_bad_syntax_prepared(self, create_db):
        # 異常系
        # エラーが正しく取得されるかチェックする
        # SQL文のシンタックスエラーが発生した場合（プリペアードステートメント）
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin')
        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor(prepared=True)
        with pytest.raises(SQLSyntaxError) as sse:
            cur.execute(
                'SELECT * FRM Test_Scenario')

        # ロケールを調べる
        if locale.getdefaultlocale()[0] == 'ja_JP':
            # 日本語のエラーメッセージを検証する
            assert sse.value.error_message == \
                'SQL文の構文エラー: \' near token 50 "FRM" at line 1\''
        else:
            # 日本以外は英語
            assert sse.value.error_message == \
                'SQL syntax error \' near token 50 "FRM" at line 1\'.'

        # 後処理
        cur.close()
        conn.close()

    def test_binary_data(self, create_db):
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin',
                          autocommit=True)
        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor(prepared=False)

        # 書込むバイナリーデータの準備
        binary_data_0 = b'\x00'
        binary_data_1 = b'\x99'
        binary_data_400 = np.random.bytes(400)
        # 0バイトデータの登録
        cur.execute('INSERT INTO Test_Binary values (?, ?, ?)',
                    (1, binary_data_0, binary_data_0))
        cur.fetchall()
        # 1バイトデータの登録
        cur.execute('INSERT INTO Test_Binary values (?, ?, ?)',
                    (2, binary_data_1, binary_data_1))
        cur.fetchall()
        # 400バイトデータの登録
        cur.execute('INSERT INTO Test_Binary values (?, ?, ?)',
                    (3, binary_data_400, binary_data_400))
        cur.fetchall()

        # バッファサイズ4096よりも大きなサイズの登録
        # 400バイトよりも大きなサイズもIMAGE型に登録は可能
        binary_data_4097 = np.random.bytes(4097)
        cur.execute('INSERT INTO Test_Binary values (?, ?, ?)',
                    (4, binary_data_4097, binary_data_4097))
        cur.fetchall()

        # 結果の取得
        cur.execute('select * from Test_Binary order by id')
        rows = cur.fetchall()

        # 比較用のデータ作成(400バイトのデータにする)
        check_binary_0 = b'\x00'*400
        check_binary_1 = b'\x99' + b'\x00' * 399
        # 400バイトにサイズを削る
        check_binary_4097 = binary_data_4097[:400]

        # 400バイトのデータをリストでまとめてチェックすると分かりづらいので１要素ずつ検証
        assert rows[0] == (1, check_binary_0, binary_data_0)
        assert rows[1] == (2, check_binary_1, binary_data_1)
        assert rows[2] == (3, binary_data_400, binary_data_400)
        assert rows[3] == (4, check_binary_4097, binary_data_4097)

        # 後処理
        cur.close()
        conn.close()

    def test_decimal_data(self, create_db):
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin',
                          autocommit=True)
        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor(prepared=False)

        # Decimal型を定義
        decimal_val_1 = (1,
                         dq.Decimal('12345'),
                         dq.Decimal('123.45'),
                         dq.Decimal('0.12345'))
        cur.execute(
            'INSERT INTO Test_Decimal values (?, ?, ?, ?)', decimal_val_1)
        cur.fetchall()

        # 結果の取得
        cur.execute('select * from Test_Decimal order by id')
        rows = cur.fetchall()

        assert rows == [(1,
                         decimal_val_1[1],
                         decimal_val_1[2],
                         decimal_val_1[3])]
        # precision, scaleの比較
        assert rows[0][1]._precision == decimal_val_1[1]._precision
        assert rows[0][1]._scale == decimal_val_1[1]._scale
        assert rows[0][2]._precision == decimal_val_1[2]._precision
        assert rows[0][2]._scale == decimal_val_1[2]._scale
        assert rows[0][3]._precision == decimal_val_1[3]._precision
        assert rows[0][3]._scale == decimal_val_1[3]._scale

        # precision が作成したカラムより大きい場合はエラー
        with pytest.raises(DatabaseError):
            cur.execute(
                "INSERT INTO Test_Decimal (id, prec5_scale0) values (2, '123456')")
            cur.fetchall()

        # scale が作成したカラムより大きい場合は切り捨て
        cur.execute(
            "INSERT INTO Test_Decimal (id, prec5_scale2, prec5_scale5) values (?, ?, ?)",
            (3, dq.Decimal('123.456'), None))
        cur.fetchall()
        # 結果の取得
        cur.execute('select * from Test_Decimal where id = 3')
        row = cur.fetchall()
        assert row == [(3, None, dq.Decimal('123.45'), None)]
        assert row[0][2]._precision == 5
        assert row[0][2]._scale == 2

        # 後処理
        cur.close()
        conn.close()

    def test_language_data(self, create_db):
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin',
                          autocommit=True)
        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor(prepared=False)

        # Language型の登録
        cur.execute(
            'INSERT INTO Test_Language values (?, ?)', (1, dq.Language('ja')))
        cur.fetchall()
        cur.execute(
            'INSERT INTO Test_Language values (?, ?)', (2, dq.Language('en-us')))
        cur.fetchall()
        cur.execute(
            'INSERT INTO Test_Language values (?, ?)', (3, dq.Language('ja+en-us+zh-cn')))
        cur.fetchall()

        # 結果の取得
        cur.execute('select * from Test_Language order by id')
        rows = cur.fetchall()

        assert rows == [(1, dq.Language('ja')),
                        (2, dq.Language('en-us')),
                        (3, dq.Language('ja+en-us+zh-cn'))]

        # 後処理
        cur.close()
        conn.close()

    def test_word_data(self, create_db):
        conn = dq.connect(host='localhost',
                          port=54321,
                          dbname='DefaultDB',
                          user='root',
                          password='doqadmin',
                          autocommit=True)
        # データベース操作のためのCursorオブジェクトをコンストラクト
        cur = conn.cursor(prepared=False)

        # 文章の準備
        ricoh_materiarity = "リコーグループは、目指すべき持続可能な社会の姿を、経済(Prosperity)\
            ・社会(People)・地球環境(Planet)の3つのPのバランスが保たれている社会「Three Ps Balance」\
            として表しています。この目指すべき社会の実現に向け、経営理念・中期経営計画・ステークホルダー\
            からの期待を反映したマテリアリティを特定し、事業活動を通じてこれらの解決に取り組みます。\n\
            「事業を通じた社会課題解決」とそれを支える「経営基盤の強化」の2つの領域で7つのマテリアリティ\
            を特定し、各マテリアリティに紐づく17のESG目標を設定しています。"
        ricoh_SDGs_1 = "これは、これまでうまく活用されていなかった間伐材を資源として利活用することにより\
            御殿場地区の森林保全と地域活性化、およびセンターの低炭素化を同時に実現する、地域連携の地産地消\
            モデルです。本取り組みは、御殿場市が推進するエコガーデンシティ化プロジェクトの一つとしても位置\
            づけられています。\n\
            リコーはこのモデルを確立し、木質バイオマスの利活用によるエネルギーの地産地消モデルとしてパッケージ化\
            し、御殿場市内を始めとして、他地域への普及促進に努めます。"
        ricoh_SDGs_2 = "リコーは新世代複合機RICOH IM Cシリーズと独自のクラウドプラットフォームを介して提供する\
            各種クラウドサービスを組み合わせ、企業間取引や社内業務で日々発生する多種多様なドキュメントを、\
            さまざまなクラウドサービスや各社パートナーシステムが活用できるよう最適化し、企業間業務を含めた\
            業務プロセスを変革していくことによって、中小企業の生産性向上に貢献しています。"
        ricoh_SDGs_3 = "リコーは自社のみではなく、お客様を含めたバリューチェーン全体での脱炭素社会づくりへの\
            貢献を目指して自社独自の厳しい製品基準が製品に反映されるもの作りプロセス｢RSPP＊｣を設定しており、\
            2016年以降スリープからの復帰を約0.5秒で実現するカラーMFPのラインナップを充実し、環境にやさしい設定\
            と快適な使用の両立を実現。\n\
            これらの機器の最適な配置と機種・機能選定のみならず、機器の導入後も継続的に稼動状況のモニタリングを行い、\
            省エネも含め、狙いの性能が発揮できているか、機器配置がオフィス環境や利用ニーズの変化に対応できているか\
            を検証し、必要に応じて改善を行うマネージド・プリント・サービス（MPS）を提供しています。"

        # データの登録
        ja = dq.Language('ja')
        cur.executemany(
            'INSERT INTO Test_Word values (?, ?, ?)',
            [(1, ricoh_materiarity, ja),
             (2, ricoh_SDGs_1, ja),
             (3, ricoh_SDGs_2, ja),
             (4, ricoh_SDGs_3, ja)]
        )
        cur.fetchall()

        # 全文索引の生成
        cur.execute("create fulltext index Full_Text on Test_Word(data)\
             language column l hint 'inverted=(indexing=word)'")
        cur.fetchall()
        # 全文検索の実行
        cur.execute("select word(data) from Test_Word where data contains freetext\
            ('リコーの社会課題解決' language 'ja') order by word(data).scale desc limit 4")
        rows = cur.fetchall()

        # Word型で返ってくるのでそれぞれの属性で確認
        row0: WordData = rows[0][0]
        row1: WordData = rows[1][0]
        row2: WordData = rows[2][0]
        row3: WordData = rows[3][0]
        # term=リコーの結果
        assert row0.term == 'リコー'
        assert row0.languagestr == 'ja'
        assert row0.category == dq.WORD.CATEGORY_HELPFUL
        assert row0.scale == 0.6666666666666666
        assert row0.docfrequency == 4
        # term=社会の結果
        assert row1.term == '社会'
        assert row1.languagestr == 'ja'
        assert row1.category == dq.WORD.CATEGORY_HELPFUL
        assert row1.scale == 0.6666666666666666
        assert row1.docfrequency == 2
        # term=課題の結果
        assert row2.term == '課題'
        assert row2.languagestr == 'ja'
        assert row2.category == dq.WORD.CATEGORY_HELPFUL
        assert row2.scale == 0.6666666666666666
        assert row2.docfrequency == 1
        # term=解決の結果
        assert row3.term == '解決'
        assert row3.languagestr == 'ja'
        assert row3.category == dq.WORD.CATEGORY_HELPFUL
        assert row3.scale == 0.6666666666666666
        assert row3.docfrequency == 1

        # 後処理
        cur.close()
        conn.close()
