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
test_constants.py -- src.doquedb.port.constants モジュールのテスト
"""

from src.doquedb.port.constants import ConnectionSlaveID


class TestConnectionSlaveID:
    def test_is_normal_limit_value(self):
        # 代表値 100のチェック
        valid_value = 100
        assert ConnectionSlaveID.is_normal(valid_value), \
            'value check failed'

        # 下限値のチェック
        assert ConnectionSlaveID.is_normal(ConnectionSlaveID.MINIMUM.value), \
            'minimum value check failed'
        assert not ConnectionSlaveID.is_normal(
            ConnectionSlaveID.MINIMUM.value-1), \
            'lower value check failed'
        # 上限値のチェック
        assert ConnectionSlaveID.is_normal(ConnectionSlaveID.MAXIMUM.value-1), \
            'maximum value check failed'
        assert not ConnectionSlaveID.is_normal(
            ConnectionSlaveID.MAXIMUM.value), \
            'greater value check failed'
