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
test_connection.py -- src.doquedb.port.connection モジュールのテスト
"""
import pytest

from src.doquedb.exception.exceptions import UnexpectedError
from src.doquedb.port.connection import RemoteClientConnection
from src.doquedb.port.constants import ConnectionSlaveID
from src.doquedb.client.constants import ProtocolVersion


class TestConnection:
    """
    Notes:
        テストをする際、以下の設定のDoqueDB DB を用意する。
        ポート番号：54321
        認証方式: password
    """

    def setup_method(self, method):
        # 前処理
        self.conn: RemoteClientConnection = None

    def teardown_method(self, method):
        # 後処理
        if self.conn:
            self.conn.close()

    def test_open(self):
        # 正常動作時の検証
        self.conn = RemoteClientConnection(
            'localhost', 54321,
            ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
            ConnectionSlaveID.ANY.value)
        self.conn.open()

        # 正しいマスターIDを受け取ったか
        assert self.conn.master_id ==\
            ProtocolVersion.CURRENT_PROTOCOL_VERSION.value
        # 正しいスレーブIDを受け取ったか
        assert self.conn.slave_id >= int(ConnectionSlaveID.MINIMUM.value)\
            and self.conn.slave_id < int(ConnectionSlaveID.MAXIMUM.value)

    def test_open_twice(self):
        # ２回オープンした場合
        self.conn = RemoteClientConnection(
            'localhost', 54321,
            ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
            ConnectionSlaveID.ANY.value)
        # コネクションのオープン（１回目）
        self.conn.open()
        with pytest.raises(UnexpectedError):
            # コネクションのオープン（２回目）
            self.conn.open()
        assert self.conn.is_opened is True

    def test_open_bad_hostname(self):
        # 誤ったホスト名を指定
        self.conn = RemoteClientConnection(
            'unexpected name', 54321,
            ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
            ConnectionSlaveID.ANY.value)
        with pytest.raises(IOError):
            # コネクションのオープン
            self.conn.open()
        assert self.conn.is_opened is False

    def test_open_bad_portnum(self):
        # 誤ったポート番号を指定
        self.conn = RemoteClientConnection(
            'localhost', 12345,
            ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
            ConnectionSlaveID.ANY.value)
        with pytest.raises(IOError):
            # コネクションのオープン
            self.conn.open()
        assert self.conn.is_opened is False

    # サーバーからの応答待ちになって止まるためスキップ
    """
    def test_open_bad_slave(self):
        # 誤ったスレーブIDを指定
        self.conn = RemoteClientConnection(
            'localhost', 54321,
            ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
            ConnectionSlaveID.ANY.value+1)
        with pytest.raises(InterfaceError):
            # コネクションのオープン
            self.conn.open()
            assert self.conn.is_opened is False
    """

    def test_close(self):
        # 正常動作時の検証
        self.conn = RemoteClientConnection(
            'localhost', 54321, 4, ConnectionSlaveID.ANY.value)
        self.conn.open()
        self.conn.close()
        assert self.conn.is_opened is False

    def test_close_twice(self):
        # close()を２回読んでも正しく動作する
        self.conn = RemoteClientConnection(
            'localhost', 54321, 4, ConnectionSlaveID.ANY.value)
        self.conn.open()
        self.conn.close()
        self.conn.close()
        assert self.conn.is_opened is False
