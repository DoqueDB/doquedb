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
test_raise_error.py -- src.doquedb.exception.raise_error モジュールのテスト
"""
from src.doquedb.common.serialdata import ExceptionData
import pytest

from src.doquedb.exception.raise_error import RaiseClassInstance
from src.doquedb.exception.errorcode import ErrorCode
from src.doquedb.exception.exceptions import UnexpectedError
from src.doquedb.exception.database_exceptions import ConnectionNotExist


class TestRaiseClassInstance:
    def test_raise_exception(self):
        # 正常系のテスト
        # 存在するエラーを指定した場合
        with pytest.raises(ConnectionNotExist):
            RaiseClassInstance.raise_exception(ExceptionData(
                errno_=ErrorCode.ConnectionNotExist.value))

        # 存在しないエラーを指定した場合
        with pytest.raises(UnexpectedError, match=r'invalid error code 0'):
            RaiseClassInstance.raise_exception(ExceptionData())
