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
test_port.py -- src.doquedb.client.port モジュールのテスト
"""

import pytest

from unittest.mock import MagicMock, call, patch

from src.doquedb.port.connection import RemoteClientConnection
from src.doquedb.client.object import Object
from src.doquedb.client.port import Port
from src.doquedb.common.serialdata import Status, ErrorLevel, ExceptionData
from src.doquedb.common.scalardata import IntegerData, StringData
from src.doquedb.client.constants import ProtocolVersion
from src.doquedb.port.constants import ConnectionSlaveID
from src.doquedb.exception.exceptions import OperationalError


class TestPort:
    def setup_method(self, method):
        # 前処理
        self.test_port = Port('localhost', 54321,
                              ProtocolVersion.CURRENT_PROTOCOL_VERSION.value,
                              ConnectionSlaveID.ANY.value)

    def teardown_method(self, method):
        # 後処理
        self.test_port.close()

    def test_init(self):
        # 正常系のテスト
        # 引数チェック
        assert isinstance(self.test_port._Port__connection,
                          RemoteClientConnection)
        assert self.test_port._Port__connection.\
            _RemoteClientConnection__hostname == 'localhost'
        assert self.test_port._Port__connection.\
            _RemoteClientConnection__portnum == 54321
        assert self.test_port._Port__connection.\
            master_id == ProtocolVersion.CURRENT_PROTOCOL_VERSION.value
        assert self.test_port._Port__connection.\
            slave_id == ConnectionSlaveID.ANY.value
        # 初期値チェック
        assert self.test_port.type == Object.PORT
        assert self.test_port._Port__worker_id == -1
        assert self.test_port._Port__reuse is False

    def test_open(self):
        # 正常系のテスト
        self.test_port._Port__connection.open = MagicMock()
        self.test_port.open()
        self.test_port._Port__connection.open.assert_called_once()

    def test_close(self):
        # 正常系のテスト
        self.test_port._Port__connection.close = MagicMock()
        self.test_port.close()
        self.test_port._Port__connection.close.assert_called_once()

    def test_read_object(self):
        # 正常系のテスト
        # 引数がない場合
        self.test_port._Port__connection.read_object = MagicMock(
            return_value=IntegerData())
        test_object = self.test_port.read_object()
        # オブジェクトを読み込めたかチェック
        assert test_object == IntegerData()
        # ``__connection.read_object``が１回しか呼ばれなかったかチェック
        self.test_port._Port__connection.read_object.assert_called_once_with(
            None)

    def test_read_object_args(self):
        # 正常系のテスト
        # 引数がある場合
        self.test_port._Port__connection.read_object = MagicMock(
            return_value=IntegerData(2))
        test_object = self.test_port.read_object(IntegerData())
        # オブジェクトを読み込めたかチェック
        assert test_object == IntegerData(2)
        # 正しく引数が渡されたかチェック
        self.test_port._Port__connection.read_object.assert_called_once_with(
            IntegerData())

    def test_read_object_errorlevel(self):
        # 異常系のテスト
        # ErrorLevelを受け取った場合
        # 前準備
        error_level = ErrorLevel()
        error_level.level = ErrorLevel.USER
        exception_data = ExceptionData()
        self.test_port._Port__connection.read_object = MagicMock(
            side_effect=[error_level, exception_data])

        with patch('src.doquedb.exception.raise_error.RaiseClassInstance.raise_exception') as re:
            test_object = self.test_port.read_object()
            # オブジェクトを読み込めたかチェック
            assert isinstance(test_object, ExceptionData)
            # __reuseが正しくセットされたかチェック
            assert self.test_port._Port__reuse == ErrorLevel.USER
            # ErrorLevelとExceptionDataの読込で２回呼ばれたかチェック
            assert self.test_port._Port__connection.read_object.call_count == 2
            # 正しく引数が渡されたかチェック
            self.test_port._Port__connection.read_object.assert_has_calls(
                [call(None), call(None)])
            # rasie_exception()に正しくExceptionDataが渡されたかチェックする
            re.assert_called_once_with(exception_data)

    def test_read_object_exceptiondata(self):
        # 異常系のテスト
        # ExceptionDataを受け取った場合
        # 前準備
        exception_data = ExceptionData()
        self.test_port._Port__connection.read_object = MagicMock(
            return_value=exception_data)

        with patch('src.doquedb.exception.raise_error.RaiseClassInstance.raise_exception') as re:
            test_object = self.test_port.read_object()
            # オブジェクトを読み込めたかチェック
            assert isinstance(test_object, ExceptionData)
            # ExceptionDataの読込で１回だけ呼ばれたかチェック
            self.test_port._Port__connection.read_object.\
                assert_called_once_with(None)
            # rasie_exception()に正しくExceptionDataが渡されたかチェックする
            re.assert_called_once_with(exception_data)

    def test_read_integerdata(self):
        # 正常系のテスト
        self.test_port.read_object = MagicMock(
            return_value=IntegerData())
        test_object = self.test_port.read_integerdata()
        # オブジェクトを読み込めたかチェック
        assert test_object == IntegerData()
        # ``read_object``が１回だけ呼ばれたかチェック
        self.test_port.read_object.assert_called_once()

    def test_read_integerdata_error(self):
        # 異常系のテスト
        self.test_port.read_object = MagicMock(
            return_value=StringData())
        with pytest.raises(OperationalError):
            self.test_port.read_integerdata()

    def test_read_stringdata(self):
        # 正常系のテスト
        self.test_port.read_object = MagicMock(
            return_value=StringData())
        test_object = self.test_port.read_stringdata()
        # オブジェクトを読み込めたかチェック
        assert test_object == StringData()
        # ``read_object``が１回だけ呼ばれたかチェック
        self.test_port.read_object.assert_called_once()

    def test_read_stringdata_error(self):
        # 異常系のテスト
        self.test_port.read_object = MagicMock(
            return_value=IntegerData())
        with pytest.raises(OperationalError):
            self.test_port.read_stringdata()

    def test_read_status(self):
        # 正常系のテスト
        self.test_port.read_object = MagicMock(
            return_value=Status())
        test_status = self.test_port.read_status()
        # オブジェクトを読み込めたかチェック
        assert test_status == Status().status
        # ``read_object``が１回だけ呼ばれたかチェック
        self.test_port.read_object.assert_called_once()

    def test_read_status_error(self):
        # 異常系のテスト
        self.test_port.read_object = MagicMock(
            return_value=IntegerData())
        with pytest.raises(OperationalError):
            self.test_port.read_status()

    def test_write_object(self):
        # 正常系のテスト
        # 引数がない場合
        self.test_port._Port__connection.write_object = MagicMock()
        self.test_port.write_object()
        # 正しく引数が渡されたかチェック
        self.test_port._Port__connection.write_object.assert_called_once_with(
            None)

    def test_write_object_args(self):
        # 正常系のテスト
        # 引数がある場合
        self.test_port._Port__connection.write_object = MagicMock()
        self.test_port.write_object(IntegerData())
        # 正しく引数が渡されたかチェック
        self.test_port._Port__connection.write_object.assert_called_once_with(
            IntegerData())

    def test_flush(self):
        # 正常系のテスト
        self.test_port._Port__connection.flush = MagicMock()
        self.test_port.flush()
        # __connection.flush()が一回だけ呼ばれたかチェック
        self.test_port._Port__connection.flush.assert_called_once()

    def test_reset(self):
        # 正常系のテスト
        self.test_port._Port__reuse = True
        self.test_port.reset()
        assert self.test_port._Port__reuse is False
