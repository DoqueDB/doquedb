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
test_iostream.py -- src.doquedb.common.iostream モジュールのテスト
"""
import sys
import struct
import socket

import pytest
from unittest.mock import MagicMock, patch

from src.doquedb.common.iostream import InputStream, OutputStream
from src.doquedb.exception.exceptions import UnexpectedError
from src.doquedb.common.constants import ClassID
from src.doquedb.client.constants import StatusSet
from src.doquedb.common.serialdata import Status
from src.doquedb.common.scalardata import Integer64Data


class TestInputStream:
    def setup_method(self, method):
        # 前処理
        self.test_istream = socket.socket().makefile('rb')
        self.test_inputstream = InputStream(self.test_istream)

    def teardown_method(self, method):
        # 後処理
        self.test_inputstream.close()

    def test_init(self):
        assert self.test_inputstream._InputStream__istream == self.test_istream

    def test_read(self):
        # 正常系のテスト
        self.test_istream.read = MagicMock()

        self.test_inputstream.read(0)
        self.test_istream.read.assert_called_once_with(0)

    def test_read_error(self):
        # 異常系のテスト
        self.test_inputstream.close()
        with pytest.raises(UnexpectedError):
            self.test_inputstream.read(0)

    def test_read_int(self):
        # 正常系のテスト
        test_int = 1
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>i', test_int))
        assert self.test_inputstream.read_int() == 1
        # 最大値
        i_max = 2147483647
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>i', i_max))
        assert self.test_inputstream.read_int() == 2147483647
        # 最小値
        i_min = -2147483648
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>i', i_min))
        assert self.test_inputstream.read_int() == -2147483648

    def test_read_short(self):
        # 正常系のテスト
        test_int = 1
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>h', test_int))
        assert self.test_inputstream.read_short() == 1
        # 最大値
        i_max = 32767
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>h', i_max))
        assert self.test_inputstream.read_short() == 32767
        # 最小値
        i_min = -32768
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>h', i_min))
        assert self.test_inputstream.read_short() == -32768

    def test_read_long(self):
        # 正常系のテスト
        test_long = 1
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>q', test_long))
        assert self.test_inputstream.read_long() == 1
        # 最大値
        l_max = 9223372036854775807
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>q', l_max))
        assert self.test_inputstream.read_long() == 9223372036854775807
        # 最小値
        l_min = -9223372036854775807
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>q', l_min))
        assert self.test_inputstream.read_long() == -9223372036854775807

    def test_read_double(self):
        # 正常系のテスト
        test_double = 1.1
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>d', test_double))
        assert self.test_inputstream.read_double() == 1.1
        # 最大値
        d_max = sys.float_info.max
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>d', d_max))
        assert self.test_inputstream.read_double() == d_max
        # 最小値
        d_min = -d_max
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>d', d_min))
        assert self.test_inputstream.read_double() == d_min

    def test_read_float(self):
        # 正常系のテスト
        test_float = 0
        self.test_inputstream.read = MagicMock(
            return_value=struct.pack('>f', test_float))
        assert self.test_inputstream.read_float() == 0
        # 最大値
        f_max_binary = b'\x7f\x7f\xff\xff'
        f_max = struct.unpack('>f', f_max_binary)[0]
        self.test_inputstream.read = MagicMock(return_value=f_max_binary)
        assert self.test_inputstream.read_float() == f_max
        # 最小値
        f_min_binary = b'\xff\x7f\xff\xff'
        f_min = struct.unpack('>f', f_min_binary)[0]
        self.test_inputstream.read = MagicMock(return_value=f_min_binary)
        assert self.test_inputstream.read_float() == f_min

    def test_read_object(self):
        # 正常系のテスト
        mock_list = [ClassID.STATUS.value, StatusSet.SUCCESS.value]
        self.test_inputstream.read_int = MagicMock(side_effect=mock_list)
        test_object = self.test_inputstream.read_object()
        assert isinstance(test_object, Status)
        assert test_object.status == StatusSet.SUCCESS.value

    def test_read_object_arg(self):
        # 引数がある場合の正常系のテスト
        mock_list = [ClassID.STATUS.value, StatusSet.SUCCESS.value]
        self.test_inputstream.read_int = MagicMock(side_effect=mock_list)
        test_object = self.test_inputstream.read_object(Status())
        assert isinstance(test_object, Status)
        assert test_object.status == StatusSet.SUCCESS.value

    def test_read_object_different_arg(self):
        # 引数が受け取ったClass ID と別の場合のテスト
        mock_list = [ClassID.STATUS.value, StatusSet.SUCCESS.value]
        self.test_inputstream.read_int = MagicMock(side_effect=mock_list)
        test_object = self.test_inputstream.read_object(Integer64Data())
        assert isinstance(test_object, Status)
        assert not isinstance(test_object, Integer64Data)
        assert test_object.status == StatusSet.SUCCESS.value

    def test_close(self):
        # 正常系のテスト
        self.test_inputstream._InputStream__istream = MagicMock()

        with patch.object(self.test_inputstream._InputStream__istream,
                          'close') as mock_close:
            self.test_inputstream.close()
            # __istream.closeが呼ばれたかどうかを確認
            mock_close.assert_called_once()
            assert self.test_inputstream._InputStream__istream is None

            # ２回クローズしても問題ない
            self.test_inputstream.close()
            # __istream.closeは呼ばれないので、１回目のクローズで呼ばれた回数から変わらない
            mock_close.assert_called_once()
            assert self.test_inputstream._InputStream__istream is None


class TestOutputStream:
    def setup_method(self, method):
        # 前処理
        self.test_ostream = socket.socket().makefile('wb')
        self.test_outputstream = OutputStream(self.test_ostream)

    def teardown_method(self, method):
        # 後処理
        self.test_outputstream.close()

    def test_init(self):
        # 正常系のテスト
        assert self.test_outputstream._OutputStream__ostream ==\
            self.test_ostream

    def test_write(self):
        # 正常系のテスト
        # 通信が必要な部分をモックに変える
        self.test_ostream.write = MagicMock()
        self.test_outputstream.write(b'test')
        self.test_ostream.write.assert_called_once_with(
            b'test')

    def test_write_error(self):
        # 異常系のテスト
        self.test_outputstream.close()
        with pytest.raises(UnexpectedError):
            self.test_outputstream.write(b'')

    def test_write_int(self):
        # 正常系のテスト
        # 通信が必要な部分をモックに変える
        self.test_outputstream.write = MagicMock()
        self.test_outputstream.write_int(1)
        self.test_outputstream.write.assert_called_once_with(
            struct.pack('>i', 1))
        # 負の値のチェック
        self.test_outputstream.write = MagicMock()
        self.test_outputstream.write_int(-1)
        self.test_outputstream.write.assert_called_once_with(
            struct.pack('>i', -1))

    def test_write_int_error(self):
        # 異常系のテスト
        with pytest.raises(struct.error):
            over_value = 2147483648
            self.test_outputstream.write_int(over_value)

    def test_write_short(self):
        # 正常系のテスト
        # 通信が必要な部分をモックに変える
        self.test_outputstream.write = MagicMock()
        self.test_outputstream.write_short(1)
        self.test_outputstream.write.assert_called_once_with(
            struct.pack('>h', 1))
        # 負の値のチェック
        self.test_outputstream.write = MagicMock()
        self.test_outputstream.write_short(-1)
        self.test_outputstream.write.assert_called_once_with(
            struct.pack('>h', -1))

    def test_write_short_error(self):
        # 異常系のテスト
        with pytest.raises(struct.error):
            over_value = 32768
            self.test_outputstream.write_short(over_value)

    def test_write_long(self):
        # 正常系のテスト
        # 通信が必要な部分をモックに変える
        self.test_outputstream.write = MagicMock()
        self.test_outputstream.write_long(1)
        self.test_outputstream.write.assert_called_once_with(
            struct.pack('>q', 1))
        # 負の値のチェック
        self.test_outputstream.write = MagicMock()
        self.test_outputstream.write_long(-1)
        self.test_outputstream.write.assert_called_once_with(
            struct.pack('>q', -1))

    def test_write_long_error(self):
        # 異常系のテスト
        with pytest.raises(struct.error):
            over_value = 9223372036854775808
            self.test_outputstream.write_long(over_value)

    def test_write_object(self):
        # 正常系のテスト
        # 代表として:obj: `Status`の場合の１パターンのみテストする
        # 通信が必要な部分をモックに変える
        self.test_outputstream.write_int = MagicMock()
        test_status = Status(0)
        test_status.write_object = MagicMock()

        self.test_outputstream.write_object(test_status)
        self.test_outputstream.write_int.assert_called_once_with(
            ClassID.STATUS.value)
        test_status.write_object.assert_called_once_with(
            self.test_outputstream)

    def test_write_object_no_args(self):
        # 正常系のテスト
        # 引数を指定しなかった場合のテスト
        # 通信が必要な部分をモックに変える
        self.test_outputstream.write_int = MagicMock()

        self.test_outputstream.write_object()
        self.test_outputstream.write_int.assert_called_once_with(
            ClassID.NONE.value)

    def test_flush(self):
        # 正常系のテスト
        self.test_ostream.flush = MagicMock()

        self.test_outputstream.flush()
        self.test_ostream.flush.assert_called_once()

    def test_flush_error(self):
        # 異常系のテスト
        self.test_outputstream.close()

        with pytest.raises(UnexpectedError):
            self.test_outputstream.flush()

    def test_close(self):
        # 正常系のテスト
        self.test_outputstream.flush = MagicMock()
        self.test_outputstream._OutputStream__ostream = MagicMock()

        with patch.object(self.test_outputstream._OutputStream__ostream,
                          'close') as mock_close:
            self.test_outputstream.close()
            # flush(), __ostream.closeが呼ばれたかどうかを確認
            self.test_outputstream.flush.assert_called_once()
            mock_close.assert_called_once()
            assert self.test_outputstream._OutputStream__ostream is None

            # ２回クローズしても問題ない
            self.test_outputstream.close()
            # flush(), __ostream.closeは呼ばれないので、callされた回数は変わらない
            self.test_outputstream.flush.assert_called_once()
            mock_close.assert_called_once()
            assert self.test_outputstream._OutputStream__ostream is None
