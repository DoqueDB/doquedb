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
test_resultset.py -- src.doquedb.client.resultset モジュールのテスト
"""
import pytest

from src.doquedb.common.serialdata import Status
from src.doquedb.common.arraydata import DataArrayData, ResultSetMetaData
from unittest.mock import MagicMock
from src.doquedb.client.object import Object
from src.doquedb.client.resultset import ResultSet
from src.doquedb.client.port import Port
from src.doquedb.client.datasource import DataSource
from src.doquedb.client.constants import ProtocolVersion, StatusSet
from src.doquedb.exception.exceptions import (ProgrammingError,
                                              UnexpectedError)
from src.doquedb.common.scalardata import IntegerData, StringData
from src.doquedb.common.constants import ClassID


def mock_read_object(tuple_):
    # test_get_next_tuple_data()のためのモック関数
    if not isinstance(tuple_, DataArrayData):
        raise UnexpectedError
    tuple_[0].value = 1
    tuple_[1].value = 'test'

    return tuple_


class TestResultSet:
    def setup_method(self, method):
        # 前処理
        self.datasource = DataSource('localhost', 54321)
        self.slave_id = 1
        self.port = Port('localhost',
                         54321,
                         ProtocolVersion.PROTOCOL_VERSION5.value,
                         self.slave_id)
        self.test_resultset = ResultSet(self.datasource,
                                        self.port)

    def test_init(self):
        # 正常系のテスト
        assert self.test_resultset.type == Object.RESULT_SET
        assert self.test_resultset._ResultSet__datasource == self.datasource
        assert self.test_resultset._ResultSet__port == self.port
        assert self.test_resultset._ResultSet__status == StatusSet.DATA.value
        assert self.test_resultset._ResultSet__metadata is None
        assert self.test_resultset._ResultSet__tupledata is None
        assert self.test_resultset._ResultSet__row is None
        assert self.test_resultset._ResultSet__rowcount == 0
        assert self.test_resultset._ResultSet__is_closed is False

    def test_get_status_status(self):
        # 正常系のテスト
        # 実行ステータス以外の場合
        self.test_resultset.get_next_tuple = MagicMock()

        # SUCCESS
        self.test_resultset._ResultSet__status = StatusSet.SUCCESS.value
        status = self.test_resultset.get_status()
        assert status == StatusSet.SUCCESS.value
        # get_next_tupleが呼ばれていないかチェック
        self.test_resultset.get_next_tuple.assert_not_called()

        # UNDEFINED
        self.test_resultset._ResultSet__status = StatusSet.UNDEFINED.value
        status = self.test_resultset.get_status()
        assert status == StatusSet.UNDEFINED.value
        # get_next_tupleが呼ばれていないかチェック
        self.test_resultset.get_next_tuple.assert_not_called()

        # CANCELED
        self.test_resultset._ResultSet__status = StatusSet.CANCELED.value
        status = self.test_resultset.get_status()
        assert status == StatusSet.CANCELED.value
        # get_next_tupleが呼ばれていないかチェック
        self.test_resultset.get_next_tuple.assert_not_called()

        # ERROR
        self.test_resultset._ResultSet__status = StatusSet.ERROR.value
        status = self.test_resultset.get_status()
        assert status == StatusSet.ERROR.value
        # get_next_tupleが呼ばれていないかチェック
        self.test_resultset.get_next_tuple.assert_not_called()

    def test_get_status_data(self):
        # 正常系のテスト
        # 実行ステータス以外の場合
        # META_DATA
        mock_list = [ResultSetMetaData(), Status(Status.status_map['SUCCESS'])]
        self.port.read_object = MagicMock(side_effect=mock_list)
        status = self.test_resultset.get_status()
        assert status == StatusSet.SUCCESS.value

        # DATA
        mock_list = [DataArrayData(), Status(Status.status_map['SUCCESS'])]
        self.port.read_object = MagicMock(side_effect=mock_list)
        status = self.test_resultset.get_status()
        assert status == StatusSet.SUCCESS.value

        # END_OF_DATA
        mock_list = [None, Status(Status.status_map['SUCCESS'])]
        self.port.read_object = MagicMock(side_effect=mock_list)
        status = self.test_resultset.get_status()
        assert status == StatusSet.SUCCESS.value

        # HAS_MORE_DATA
        mock_list = [Status(Status.status_map['HAS_MORE_DATA']),
                     Status(Status.status_map['SUCCESS'])]
        self.port.read_object = MagicMock(side_effect=mock_list)
        status = self.test_resultset.get_status()
        assert status == StatusSet.SUCCESS.value

    def test_get_next_tuple_metadata(self):
        # 正常系のテスト
        # MetaDataを取得した場合
        # 前準備
        metadata = ResultSetMetaData()
        self.test_resultset._ResultSet__port.read_object = MagicMock(
            return_value=metadata)

        self.test_resultset.get_next_tuple()
        assert self.test_resultset._ResultSet__metadata == metadata
        assert self.test_resultset._ResultSet__tupledata == DataArrayData()
        assert self.test_resultset._ResultSet__status\
            == StatusSet.META_DATA.value

    def test_get_next_tuple_data(self):
        # 正常系のテスト
        # Dataを取得した場合
        # 前準備
        # __tupledataの中身をセットする
        tuple_data = DataArrayData()
        tuple_data.add_element(IntegerData())
        tuple_data.add_element(StringData())
        self.test_resultset._ResultSet__tupledata = tuple_data
        row = DataArrayData()
        self.test_resultset._ResultSet__port.read_object = MagicMock(
            side_effect=mock_read_object)

        self.test_resultset.get_next_tuple(row)
        assert row.class_id == ClassID.DATA_ARRAY_DATA.value
        assert isinstance(row[0], IntegerData)
        assert row[0].value == 1
        assert isinstance(row[1], StringData)
        assert row[1].value == 'test'
        assert self.test_resultset._ResultSet__status\
            == StatusSet.DATA.value

    def test_get_next_tuple_else_pattern(self):
        # 正常系のテスト
        # 前準備
        status = Status(Status.status_map['SUCCESS'])
        self.test_resultset._ResultSet__port.read_object = MagicMock(
            return_value=status)

        self.test_resultset.get_next_tuple()
        assert self.datasource._DataSource__portmap[self.slave_id] == self.port
        assert self.test_resultset._ResultSet__port is None
        assert self.test_resultset._ResultSet__status\
            == StatusSet.SUCCESS.value

    def test_next(self):
        # 正常系のテスト
        self.test_resultset.get_next_tuple = MagicMock(
            side_effect=[StatusSet.META_DATA.value, StatusSet.DATA.value])

        has_data = self.test_resultset.next()
        # 読込に成功したかチェック
        assert has_data is True
        # ``rowcount``が増加したかチェック
        assert self.test_resultset._ResultSet__rowcount == 1

    def test_next_error(self):
        # 異常系のテスト
        self.test_resultset.get_next_tuple = MagicMock(
            side_effect=UnexpectedError)

        with pytest.raises(UnexpectedError):
            self.test_resultset.next()

    @ pytest.mark.skip(reason='未使用の機能')
    def test_cancel(self):
        # 正常系のテスト
        pass

    def test_get_row_as_tuple(self):
        # 正常系のテスト
        # 前準備
        self.test_resultset._ResultSet__row = DataArrayData()

        # １行の場合
        self.test_resultset._ResultSet__row.add_element(IntegerData(1))
        tuple_1row = self.test_resultset.get_row_as_tuple()
        assert tuple_1row == (1,)

        # ２行の場合
        self.test_resultset._ResultSet__row.add_element(StringData('test'))
        tuple_2rows = self.test_resultset.get_row_as_tuple()
        assert tuple_2rows == (1, 'test')

    def test_get_row_as_tuple_no_row(self):
        # 異常系のテスト
        # 前準備
        self.test_resultset._ResultSet__row = DataArrayData()

        # 行がない場合
        with pytest.raises(ProgrammingError):
            self.test_resultset.get_row_as_tuple()

    def test_close(self):
        # 正常系のテスト
        # 前準備
        self.test_resultset.get_status = MagicMock()
        self.test_resultset._ResultSet__status = StatusSet.DATA.value
        self.test_resultset._ResultSet__metadata = ResultSetMetaData()
        self.test_resultset._ResultSet__row = DataArrayData()
        self.test_resultset._ResultSet__rowcount = 1
        self.test_resultset._ResultSet__port = None

        self.test_resultset.close()
        self.test_resultset.get_status.assert_not_called()
        assert self.test_resultset._ResultSet__status\
            == StatusSet.UNDEFINED.value
        assert self.test_resultset._ResultSet__metadata is None
        assert self.test_resultset._ResultSet__row is None
        assert self.test_resultset._ResultSet__rowcount == 0
        assert self.test_resultset._ResultSet__is_closed is True

    def test_close_status_is_data(self):
        # 正常系のテスト
        # 前準備
        self.test_resultset.get_status = MagicMock(
            return_value=StatusSet.SUCCESS.value)
        self.test_resultset._ResultSet__status = StatusSet.DATA.value
        self.test_resultset._ResultSet__metadata = ResultSetMetaData()
        self.test_resultset._ResultSet__row = DataArrayData()
        self.test_resultset._ResultSet__rowcount = 1

        self.test_resultset.close()
        assert self.test_resultset._ResultSet__status\
            == StatusSet.UNDEFINED.value
        assert self.test_resultset._ResultSet__metadata is None
        assert self.test_resultset._ResultSet__row is None
        assert self.test_resultset._ResultSet__rowcount == 0
        assert self.test_resultset._ResultSet__is_closed is True

    def test_close_twice(self):
        # 正常系のテスト
        # 前準備
        self.test_resultset.get_status = MagicMock()
        self.test_resultset._ResultSet__status = StatusSet.DATA.value
        self.test_resultset._ResultSet__metadata = ResultSetMetaData()
        self.test_resultset._ResultSet__row = DataArrayData()
        self.test_resultset._ResultSet__rowcount = 1
        self.test_resultset._ResultSet__port = None

        self.test_resultset.close()
        # ２回閉じても問題ない
        self.test_resultset.close()
        self.test_resultset.get_status.assert_not_called()
        assert self.test_resultset._ResultSet__status\
            == StatusSet.UNDEFINED.value
        assert self.test_resultset._ResultSet__metadata is None
        assert self.test_resultset._ResultSet__row is None
        assert self.test_resultset._ResultSet__rowcount == 0
        assert self.test_resultset._ResultSet__is_closed is True
