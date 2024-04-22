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
prepare_statement.py -- クライアントのプリペアステートメント
"""
from typing import Optional, TYPE_CHECKING

if TYPE_CHECKING:
    from ..common.arraydata import DataArrayData
    from ..client.session import Session

from ..client.object import Object
from ..client.resultset import ResultSet


class PrepareStatement(Object):
    """プリペアステートメントクラス

    Args:
        prepare_id: プリペアID
    """

    def __init__(self,
                 prepare_id: int
                 ) -> None:
        super().__init__(Object.PREPARE_STATEMENT)
        self.__prepare_id = prepare_id

    @property
    def prepare_id(self) -> int:
        """プリペアIDのゲッター"""
        return self.__prepare_id

    @staticmethod
    def create(session: 'Session', statement: str) -> 'PrepareStatement':
        """プリペアステートメントを作成する.

        Args:
            session (Session): セッション
            statement (str): SQL文

        Returns:
            PrepareStatement: 作成したプリペアステートメント
        """
        return session.create_prepare_statement(statement)

    def execute(self,
                session: 'Session',
                parameter: Optional['DataArrayData'] = None
                ) -> ResultSet:
        """プリペアードステートメントの実行

        Args:
            parameter (Optional[DataArrayData]): パラメータ

        Returns:
            ResultSet: リザルトセット
        """
        return session.execute_prepare(self, parameter)

    def close(self, session: 'Session') -> None:
        """プリペアステートメントのクローズ
        """

        if self.prepare_id != 0:
            try:
                session.erase_prepare_statement(self.__prepare_id)
            except Exception:
                # 例外は無視する
                pass

            self.__prepare_id = 0
