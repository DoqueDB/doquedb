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
test_connection.py -- src.doquedb.client.connection モジュールのテスト
"""
import pytest
from unittest.mock import MagicMock, patch

from src.doquedb.client.connection import Connection
from src.doquedb.client.datasource import DataSource, Port
from src.doquedb.client.constants import ProtocolVersion
from src.doquedb.port.constants import ConnectionSlaveID
from src.doquedb.client.object import Object
from src.doquedb.exception.exceptions import DatabaseError, UnexpectedError
from src.doquedb.common.scalardata import IntegerData


class TestConnection:
    def setup_method(self, method):
        # 前処理
        self.datasource = DataSource('localhost', 54321)
        self.port = Port('localhost',
                         54321,
                         ProtocolVersion.PROTOCOL_VERSION5.value,
                         1)

    def test_init(self):
        # 正常系のテスト
        # Connectionインスタンスを生成
        connection = Connection(self.datasource, self.port)

        # 各メンバ変数が正しくセットされたかチェック
        assert connection.type == Object.CONNECTION
        assert connection._Connection__datasource == self.datasource
        assert connection._Connection__port == self.port

    def test_begin_connection(self):
        # 正常系のテスト
        # 前準備
        self.port.write_object = MagicMock()
        self.port.flush = MagicMock()
        self.port.read_integerdata = MagicMock(return_value=IntegerData(2))
        with patch('src.doquedb.client.port.Port.open'), \
                patch('src.doquedb.client.port.Port.read_status'):
            # Connectionインスタンスを生成
            connection = Connection(self.datasource, self.port)

            new_connection = connection.begin_connection()
            # 返り値のチェック
            assert isinstance(new_connection, Connection)
            assert new_connection._Connection__datasource == self.datasource
            # 新しい通信ポートのチェック
            assert not new_connection._Connection__port.slave_id\
                == self.port.slave_id

    def test_begin_worker(self):
        # 正常系のテスト
        # 前準備
        self.port.write_object = MagicMock()
        self.port.flush = MagicMock()
        new_slave_id = 1
        worker_id = 1
        self.port.read_integerdata = MagicMock(side_effect=[
            IntegerData(new_slave_id),
            IntegerData(worker_id)])
        with patch('src.doquedb.client.port.Port.open'), \
                patch('src.doquedb.client.port.Port.read_status'):
            # Connectionインスタンスを生成
            connection = Connection(self.datasource, self.port)

            new_port = connection.begin_worker()
            assert new_port.worker_id == worker_id
            assert new_port.slave_id == new_slave_id

    def test_begin_worker_port_exist(self):
        # 正常系のテスト
        # datasourceに利用可能なportがある場合のテスト
        # 前準備
        self.port.write_object = MagicMock()
        self.port.flush = MagicMock()
        self.datasource._DataSource__portmap[1]\
            = self.port
        new_slave_id = 2
        worker_id = 1
        self.port.read_integerdata = MagicMock(side_effect=[
            IntegerData(new_slave_id),
            IntegerData(worker_id)])
        with patch('src.doquedb.client.port.Port.open'), \
                patch('src.doquedb.client.port.Port.read_status'):
            # Connectionインスタンスを生成
            connection = Connection(self.datasource, self.port)

            new_port = connection.begin_worker()
            assert new_port.worker_id == worker_id
            # datasourceにあるポートが用いられるので、new_slave_idではない
            assert new_port.slave_id == self.port.slave_id

    def test_begin_worker_protocol_error(self):
        # 異常系のテスト
        # 通信プロトコル周りでエラーが発生した場合
        # 前準備
        self.port.write_object = MagicMock()
        self.port.flush = MagicMock()
        self.datasource._DataSource__portmap[1]\
            = self.port
        self.port.read_integerdata = MagicMock(
            side_effect=DatabaseError('test'))
        with patch('src.doquedb.client.port.Port.open'), \
                patch('src.doquedb.client.port.Port.read_status'):
            # Connectionインスタンスを生成
            connection = Connection(self.datasource, self.port)

            with pytest.raises(DatabaseError):
                connection.begin_worker()
            # portが返却されたかチェック
            assert self.datasource._DataSource__portmap[1]\
                == self.port

    def test_begin_worker_invalid_slave_id_error(self):
        # 異常系のテスト
        # datasourceに登録されたportのslave idがANYだった場合
        # 前準備
        invalid_port = Port('localhost',
                            54321,
                            ProtocolVersion.PROTOCOL_VERSION5.value,
                            ConnectionSlaveID.ANY.value)
        invalid_port.write_object = MagicMock()
        invalid_port.flush = MagicMock()
        self.datasource._DataSource__portmap[1]\
            = invalid_port
        new_slave_id = 1
        worker_id = 1
        self.port.read_integerdata = MagicMock(side_effect=[
            IntegerData(new_slave_id),
            IntegerData(worker_id)])
        with patch('src.doquedb.client.port.Port.open'), \
                patch('src.doquedb.client.port.Port.read_status'):
            # Connectionインスタンスを生成
            connection = Connection(self.datasource, self.port)

            with pytest.raises(UnexpectedError):
                connection.begin_worker()
            # portが返却されていないことをチェック
            with pytest.raises(KeyError):
                self.datasource._DataSource__portmap[1]

    @ pytest.mark.skip(reason='未使用メソッド')
    def test_cancel_worker(self):
        # 正常系のテスト
        pass

    @ pytest.mark.skip(reason='未使用メソッド')
    def test_disconnect_port(self):
        # 正常系のテスト
        pass

    @ pytest.mark.skip(reason='未使用メソッド')
    def test_is_server_available(self):
        # 正常系のテスト
        pass

    @ pytest.mark.skip(reason='未実装メソッド')
    def test_is_database_available(self):
        # 正常系のテスト
        pass

    def test_close(self):
        # 正常系のテスト
        # 前準備
        self.port.write_object = MagicMock()
        self.port.flush = MagicMock()
        self.port.read_status = MagicMock()
        # Connectionインスタンスを生成
        connection = Connection(self.datasource, self.port)

        connection.close()
        assert connection._Connection__port is None

    def test_close_twice(self):
        # 正常系のテスト
        # 前準備
        self.port.write_object = MagicMock()
        self.port.flush = MagicMock()
        self.port.read_status = MagicMock()
        # Connectionインスタンスを生成
        connection = Connection(self.datasource, self.port)

        connection.close()
        # ２回呼んだ場合は何もしない
        connection.close()
        assert connection._Connection__port is None

    def test_close_error(self):
        # 準異常系のテスト
        # エラーは無視される
        # 前準備
        self.port.write_object = MagicMock(side_effect=DatabaseError('test'))
        self.port.flush = MagicMock()
        self.port.read_status = MagicMock()
        self.port.close = MagicMock(side_effect=UnexpectedError('test'))
        # Connectionインスタンスを生成
        connection = Connection(self.datasource, self.port)

        connection.close()
        assert connection._Connection__port is None
