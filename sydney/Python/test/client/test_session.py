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
test_session.py -- src.doquedb.client.session モジュールのテスト
"""
import pytest
from unittest.mock import MagicMock

from src.doquedb.client.object import Object
from src.doquedb.client.session import Session
from src.doquedb.client.datasource import DataSource
from src.doquedb.client.connection import Connection
from src.doquedb.client.port import Port
from src.doquedb.client.constants import ProtocolVersion
from src.doquedb.exception.exceptions import (DatabaseError, InterfaceError,
                                              NotSupportedError,
                                              OperationalError,
                                              UnexpectedError)
from src.doquedb.exception.database_exceptions import ClassNotFound
from src.doquedb.exception.errorcode import ErrorCode
from src.doquedb.common.arraydata import DataArrayData
from src.doquedb.common.scalardata import IntegerData
from src.doquedb.client.prepare_statement import PrepareStatement
from src.doquedb.common.serialdata import ExceptionData


class TestSession:
    def setup_method(self, method):
        # 前処理
        self.data_source = DataSource('localhost', 54321)
        self.test_session_user = Session(self.data_source,
                                         'dbname',
                                         1,
                                         'user')
        self.test_session_no_user = Session(self.data_source,
                                            'dbname',
                                            1)
        self.test_session_invalid = Session(self.data_source,
                                            'dbname',
                                            0)

    def test_init(self):
        # 正常系のテスト
        assert self.test_session_user.type == Object.SESSION
        assert self.test_session_user._Session__datasource == self.data_source
        assert self.test_session_user._Session__dbname == 'dbname'
        assert self.test_session_user._Session__username == 'user'
        assert self.test_session_user._Session__session_id == 1
        assert self.test_session_user._Session__prepared_map == {}
        # ユーザー管理非対応の場合
        assert self.test_session_no_user._Session__username is None

    def test_is_valid(self):
        # 正常系のテスト
        assert self.test_session_user.is_valid is True
        # session_id が0だとFalse
        assert self.test_session_invalid.is_valid is False

    def test_execute(self):
        # 正常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         1)
        # test_portの通信周りのメソッドをモックに差し替える
        test_port.write_object = MagicMock()
        test_port.flush = MagicMock()
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        statement = 'statement'
        parameter = DataArrayData()

        test_resultset = self.test_session_user.execute(
            statement, parameter)
        # 返り値resultsetが正しく返却されたかチェック
        assert test_resultset._ResultSet__datasource ==\
            self.test_session_user._Session__datasource
        assert test_resultset._ResultSet__port == test_port
        # begin_workerが正しく呼ばれたかチェック
        test_connection.begin_worker.assert_called_once()
        # port.write_objectが正しく呼ばれたかチェック
        assert test_port.write_object.call_count == 4
        # port.flushが正しく呼ばれたかチェック
        test_port.flush.assert_called_once()

    def test_execute_invalid_session(self):
        # 異常系のテスト
        with pytest.raises(InterfaceError):
            self.test_session_invalid.execute('statement')

    def test_execute_connection_failed(self):
        # 異常系のテスト
        # 前準備
        # get_client_connectionの返り値をNoneに設定
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=None)

        with pytest.raises(OperationalError):
            self.test_session_user.execute('statement')

    def test_execute_prepare(self):
        # 正常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         1)
        # test_portの通信周りのメソッドをモックに差し替える
        test_port.write_object = MagicMock()
        test_port.flush = MagicMock()
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        prepare = PrepareStatement(prepare_id=1)
        parameter = DataArrayData()

        test_resultset = self.test_session_user.execute_prepare(
            prepare, parameter)
        # 返り値resultsetが正しく返却されたかチェック
        assert test_resultset._ResultSet__datasource ==\
            self.test_session_user._Session__datasource
        assert test_resultset._ResultSet__port == test_port
        # begin_workerが正しく呼ばれたかチェック
        test_connection.begin_worker.assert_called_once()
        # port.write_objectが正しく呼ばれたかチェック
        assert test_port.write_object.call_count == 4
        # port.flushが正しく呼ばれたかチェック
        test_port.flush.assert_called_once()

    def test_create_prepare_statement(self):
        # 正常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         1)
        # datasourceのマスターidをセットする
        self.data_source._DataSource__master_id\
            = ProtocolVersion.CURRENT_PROTOCOL_VERSION.value
        # test_portの通信周りのメソッドをモックに差し替える
        test_port.write_object = MagicMock()
        test_port.read_status = MagicMock()
        test_port.flush = MagicMock()
        # port.read_integerdataでprepare_id=1が返るよう設定
        prepare_id = 1
        test_port.read_integerdata = MagicMock(
            return_value=IntegerData(prepare_id))
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        statement = 'test'

        test_prepare = self.test_session_user.create_prepare_statement(
            statement)
        # プリペアステートメントが正しく生成されたかチェック
        assert isinstance(test_prepare, PrepareStatement)
        assert test_prepare.prepare_id == prepare_id
        # プリペアステートメントがマップに登録されたかチェック
        assert self.test_session_user.prepared_map[statement] == test_prepare
        # begin_workerが正しく呼ばれたかチェック
        test_connection.begin_worker.assert_called_once()
        # port.write_objectが正しく呼ばれたかチェック
        assert test_port.write_object.call_count == 3
        # read_statusが正しく呼ばれたかチェック
        test_port.read_status.assert_called_once()
        # port.flushが正しく呼ばれたかチェック
        test_port.flush.assert_called_once()

    def test_create_prepare_statement_invalid_ver(self):
        # 異常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.PROTOCOL_VERSION2.value,
                         1)
        # test_portの通信周りのメソッドをモックに差し替える
        test_port.write_object = MagicMock()
        test_port.read_status = MagicMock()
        test_port.flush = MagicMock()
        # port.read_integerdataでprepare_id=1が返るよう設定
        prepare_id = 1
        test_port.read_integerdata = MagicMock(return_value=prepare_id)
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        statement = 'test'

        # バージョン2以前では例外が上がる
        with pytest.raises(NotSupportedError):
            self.test_session_user.create_prepare_statement(
                statement)

    def test_create_prepare_statement_valid_ver(self):
        # 正常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.PROTOCOL_VERSION3.value,
                         1)
        # datasourceのマスターidをセットする
        self.data_source._DataSource__master_id\
            = ProtocolVersion.PROTOCOL_VERSION3.value
        # test_portの通信周りのメソッドをモックに差し替える
        test_port.write_object = MagicMock()
        test_port.read_status = MagicMock()
        test_port.flush = MagicMock()
        # port.read_integerdataでprepare_id=1が返るよう設定
        prepare_id = 1
        test_port.read_integerdata = MagicMock(
            return_value=IntegerData(prepare_id))
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        statement = 'test'

        # プロトコルバージョン３はサポート内なので正常に動作する
        test_prepare = self.test_session_user.create_prepare_statement(
            statement)
        # プリペアステートメントがマップに登録されたかチェック
        assert self.test_session_user.prepared_map[statement] == test_prepare

    def test_create_prepare_statement_class_error(self):
        # 異常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         1)
        # datasourceのマスターidをセットする
        self.data_source._DataSource__master_id\
            = ProtocolVersion.CURRENT_PROTOCOL_VERSION.value
        # test_portの通信周りのメソッドをモックに差し替える
        test_port.write_object = MagicMock()
        test_port.read_status = MagicMock()
        test_port.flush = MagicMock()
        test_port.close = MagicMock()
        # port.read_integerdataでClassNotFoundの例外が上がるように設定
        cnf_exception = ExceptionData(ErrorCode.ClassNotFound.value)
        cnf_exception._ExceptionData__args = [1]
        test_port.read_integerdata = MagicMock(
            side_effect=ClassNotFound(cnf_exception))
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        statement = 'class not found'

        with pytest.raises(ClassNotFound):
            self.test_session_user.create_prepare_statement(statement)
        test_port.close.assert_called_once()

    def test_create_prepare_statement_db_error(self):
        # 異常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         1)
        # datasourceのマスターidをセットする
        self.data_source._DataSource__master_id\
            = ProtocolVersion.CURRENT_PROTOCOL_VERSION.value
        # test_portの通信周りのメソッドをモックに差し替える
        test_port.write_object = MagicMock()
        # read_statusで例外が上がるよう設定
        test_port.read_status = MagicMock(side_effect=DatabaseError('test'))
        test_port.flush = MagicMock()
        test_port.close = MagicMock()
        # port.read_integerdataでprepare_id=1が返るよう設定
        prepare_id = 1
        test_port.read_integerdata = MagicMock(return_value=prepare_id)
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        statement = 'db error'

        # reuseの場合
        test_port._Port__reuse = True
        with pytest.raises(DatabaseError):
            self.test_session_user.create_prepare_statement(statement)
        # datasourceにポートが返却されたか確認
        assert self.data_source._DataSource__portmap[test_port.slave_id]\
            == test_port
        test_port.close.assert_not_called()

        # reuseでない場合
        test_port._Port__reuse = False
        with pytest.raises(DatabaseError):
            self.test_session_user.create_prepare_statement(statement)
        test_port.close.assert_called_once()

    def test_create_prepare_statement_exception(self):
        # 異常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         1)
        # datasourceのマスターidをセットする
        self.data_source._DataSource__master_id\
            = ProtocolVersion.CURRENT_PROTOCOL_VERSION.value
        # test_portの通信周りのメソッドをモックに差し替える
        # write_objectで例外が上がるよう設定
        test_port.write_object = MagicMock(side_effect=IOError)
        test_port.read_status = MagicMock()
        test_port.flush = MagicMock()
        test_port.close = MagicMock()
        # port.read_integerdataでprepare_id=1が返るよう設定
        prepare_id = 1
        test_port.read_integerdata = MagicMock(return_value=prepare_id)
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        statement = 'io error'

        with pytest.raises(IOError):
            self.test_session_user.create_prepare_statement(statement)
        test_port.close.assert_called_once()

    def test_erase_prepare_statement(self):
        # 正常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         1)
        # test_portの通信周りのメソッドをモックに差し替える
        test_port.write_object = MagicMock()
        test_port.read_status = MagicMock()
        test_port.flush = MagicMock()
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        # プリペアステートメントを登録する
        prepare_id = 1
        prepare1 = PrepareStatement(prepare_id=prepare_id)
        self.test_session_user._Session__prepared_map['test1']\
            = prepare1

        self.test_session_user.erase_prepare_statement(
            prepare_id)
        # begin_workerが正しく呼ばれたかチェック
        test_connection.begin_worker.assert_called_once()
        # port.write_objectが正しく呼ばれたかチェック
        assert test_port.write_object.call_count == 3
        # read_statusが正しく呼ばれたかチェック
        test_port.read_status.assert_called_once()
        # port.flushが正しく呼ばれたかチェック
        test_port.flush.assert_called_once()

    def test_erase_prepare_statement_class_error(self):
        # 異常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         1)
        # datasourceのマスターidをセットする
        self.data_source._DataSource__master_id\
            = ProtocolVersion.CURRENT_PROTOCOL_VERSION.value
        # test_portの通信周りのメソッドをモックに差し替える
        test_port.write_object = MagicMock()
        # read_statusでClassNotFoundの例外が上がるよう設定
        cnf_exception = ExceptionData(ErrorCode.ClassNotFound.value)
        cnf_exception._ExceptionData__args = [1]
        test_port.read_status = MagicMock(
            side_effect=ClassNotFound(cnf_exception))
        test_port.flush = MagicMock()
        test_port.close = MagicMock()
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        # プリペアステートメントを登録する
        prepare_id = 1
        prepare1 = PrepareStatement(prepare_id=prepare_id)
        self.test_session_user._Session__prepared_map['test1']\
            = prepare1

        with pytest.raises(ClassNotFound):
            self.test_session_user.erase_prepare_statement(prepare_id)
        test_port.close.assert_called_once()

    def test_erase_prepare_statement_db_error(self):
        # 異常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         1)
        # datasourceのマスターidをセットする
        self.data_source._DataSource__master_id\
            = ProtocolVersion.CURRENT_PROTOCOL_VERSION.value
        # test_portの通信周りのメソッドをモックに差し替える
        test_port.write_object = MagicMock()
        # read_statusで例外が上がるよう設定
        test_port.read_status = MagicMock(side_effect=DatabaseError('test'))
        test_port.flush = MagicMock()
        test_port.close = MagicMock()
        # port.read_integerdataでprepare_id=1が返るよう設定
        prepare_id = 1
        test_port.read_integerdata = MagicMock(return_value=prepare_id)
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        # プリペアステートメントを登録する
        prepare_id = 1
        prepare1 = PrepareStatement(prepare_id=prepare_id)
        self.test_session_user._Session__prepared_map['test1']\
            = prepare1

        # reuseの場合
        test_port._Port__reuse = True
        with pytest.raises(DatabaseError):
            self.test_session_user.erase_prepare_statement(prepare_id)
        # datasourceにポートが返却されたか確認
        assert self.data_source._DataSource__portmap[test_port.slave_id]\
            == test_port
        test_port.close.assert_not_called()

        # reuseでない場合
        test_port._Port__reuse = False
        with pytest.raises(DatabaseError):
            self.test_session_user.erase_prepare_statement(prepare_id)
        test_port.close.assert_called_once()

    def test_erase_prepare_statement_exception(self):
        # 異常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         1)
        # datasourceのマスターidをセットする
        self.data_source._DataSource__master_id\
            = ProtocolVersion.CURRENT_PROTOCOL_VERSION.value
        # test_portの通信周りのメソッドをモックに差し替える
        test_port.write_object = MagicMock(side_effect=IOError)
        # read_statusで例外が上がるよう設定
        test_port.read_status = MagicMock()
        test_port.flush = MagicMock()
        test_port.close = MagicMock()
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        # datasourceのget_client_connectionをモックに置き換える
        self.test_session_user._Session__datasource.get_client_connection\
            = MagicMock(return_value=test_connection)
        # プリペアステートメントを登録する
        prepare_id = 1
        prepare1 = PrepareStatement(prepare_id=prepare_id)
        self.test_session_user._Session__prepared_map['test1']\
            = prepare1

        with pytest.raises(IOError):
            self.test_session_user.create_prepare_statement(prepare_id)
        test_port.close.assert_called_once()

    @pytest.mark.skip(reason='未実装')
    def test_get_database_product_ver(self):
        # 正常系のテスト
        pass

    def test_close_internal(self):
        # 正常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        slave_id = 1
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         slave_id)
        test_port.write_object = MagicMock()
        test_port.flush = MagicMock()
        test_port.read_status = MagicMock()
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        self.test_session_user._Session__datasource.\
            get_client_connection = MagicMock(return_value=test_connection)
        self.test_session_user._Session__datasource.push_port = MagicMock()

        session_id = self.test_session_user.close_internal()
        # 正しく返り値session_idが返ったかチェック
        assert session_id == 1
        # session_idがinvalid=0にセットされたかチェック
        assert self.test_session_user._Session__session_id == 0
        # port.write_objectが正しく呼ばれたかチェック
        assert test_port.write_object.call_count == 2
        # datasource.push_portが正しく呼ばれたかチェック
        self.test_session_user._Session__datasource.\
            push_port.call_count == 1

    def test_close_internal_prepared(self):
        # 正常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        slave_id = 1
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         slave_id)
        test_port.write_object = MagicMock()
        test_port.flush = MagicMock()
        test_port.read_status = MagicMock()
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        self.test_session_user._Session__datasource.\
            get_client_connection = MagicMock(return_value=test_connection)
        self.test_session_user._Session__datasource.push_port = MagicMock()
        # プリペアステートメントを登録する
        prepare1 = PrepareStatement(prepare_id=1)
        prepare2 = PrepareStatement(prepare_id=2)
        self.test_session_user._Session__prepared_map['test1']\
            = prepare1
        self.test_session_user._Session__prepared_map['test2']\
            = prepare2

        session_id = self.test_session_user.close_internal()
        # 正しく返り値session_idが返ったかチェック
        assert session_id == 1
        # session_idがinvalid=0にセットされたかチェック
        assert self.test_session_user._Session__session_id == 0
        # プリペアードステートメントをクローズしたかチェック
        assert prepare1.prepare_id == 0
        assert prepare2.prepare_id == 0
        assert self.test_session_user.prepared_map == {}

    def test_close_internal_connection_failed(self):
        # 異常系のテスト
        # コネクションの取得に失敗した場合のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        slave_id = 1
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         slave_id)
        test_port.write_object = MagicMock()
        test_port.flush = MagicMock()
        test_port.read_status = MagicMock()
        test_connection = Connection(self.data_source, 54321)
        # begin_workerで例外が上がるように設定
        test_connection.begin_worker = MagicMock(side_effect=UnexpectedError)
        self.test_session_user._Session__datasource.\
            get_client_connection = MagicMock(return_value=test_connection)
        self.test_session_user._Session__datasource.push_port = MagicMock()

        # 例外は無視されるので実行される
        session_id = self.test_session_user.close_internal()
        # session_idのチェックと返り値のチェック
        assert self.test_session_user._Session__session_id == 0
        assert session_id == 1

    def test_close_internal_error_reuse(self):
        # 異常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        slave_id = 1
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         slave_id)
        test_port.write_object = MagicMock()
        test_port.flush = MagicMock()
        test_port.read_status = MagicMock(side_effect=OSError)
        # reuseフラグをTrueにする
        test_port._Port__reuse = True
        # connection.__is_openedのフラグをTrueにする
        test_port._Port__connection._RemoteClientConnection__is_opened = True
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        self.test_session_user._Session__datasource.\
            get_client_connection = MagicMock(return_value=test_connection)

        # 例外は無視されるので実行される
        self.test_session_user.close_internal()
        # portが閉じていないかチェック
        assert test_port._Port__connection.is_opened is True
        # datasourceのportmapにportが登録されたかチェック
        assert self.data_source._DataSource__portmap[test_port.slave_id]\
            == test_port
        # self.__session_id のチェック
        assert self.test_session_user._Session__session_id == 0

    def test_close_internal_error_not_reuse(self):
        # 異常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        slave_id = 1
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         slave_id)
        test_port.write_object = MagicMock()
        test_port.flush = MagicMock()
        test_port.read_status = MagicMock(side_effect=OSError)
        # reuseフラグはデフォルトでFalse
        # connection.__is_openedのフラグをTrueにする
        test_port._Port__connection._RemoteClientConnection__is_opened = True
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        self.test_session_user._Session__datasource.\
            get_client_connection = MagicMock(return_value=test_connection)

        # 例外は無視されるので実行される
        self.test_session_user.close_internal()
        # connection.__is_openedがFalseかチェック
        test_port._Port__connection._RemoteClientConnection__is_opened is False
        # portが返却されていないことをチェックする
        with pytest.raises(KeyError):
            assert self.data_source._DataSource__portmap[test_port.slave_id]
        # self.__session_id のチェック
        assert self.test_session_user._Session__session_id == 0

    def test_close(self):
        # 正常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        slave_id = 1
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         slave_id)
        test_port.write_object = MagicMock()
        test_port.flush = MagicMock()
        test_port.read_status = MagicMock()
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        self.test_session_user._Session__datasource.\
            get_client_connection = MagicMock(return_value=test_connection)
        self.test_session_user._Session__datasource.push_port = MagicMock()
        self.test_session_user._Session__datasource.\
            _DataSource__session_map[1] = self.test_session_user

        self.test_session_user.close()
        # session_idがinvalid=0にセットされたかチェック
        assert self.test_session_user._Session__session_id == 0
        # DataSourceからsessionが削除されたかチェック
        with pytest.raises(KeyError):
            self.test_session_user._Session__datasource.\
                _DataSource__session_map[1]

    def test_close_twice(self):
        # 正常系のテスト
        # 前準備
        # port, connectionインスタンスを作成し、使用されるメソッドをモックに置き換える
        slave_id = 1
        test_port = Port('hostname',
                         54321,
                         ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                         slave_id)
        test_port.write_object = MagicMock()
        test_port.flush = MagicMock()
        test_port.read_status = MagicMock()
        test_connection = Connection(self.data_source, 54321)
        test_connection.begin_worker = MagicMock(return_value=test_port)
        self.test_session_user._Session__datasource.\
            get_client_connection = MagicMock(return_value=test_connection)
        self.test_session_user._Session__datasource.push_port = MagicMock()
        self.test_session_user._Session__datasource.\
            _DataSource__session_map[1] = self.test_session_user

        self.test_session_user.close()
        self.test_session_user.close()
        # session_idがinvalid=0にセットされたままかチェック
        assert self.test_session_user._Session__session_id == 0
