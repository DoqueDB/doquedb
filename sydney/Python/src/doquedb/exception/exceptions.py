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
exceptions.py -- DB-API 2.0 の例外
"""

from typing import Optional


class Warning(Exception):
    """警告のための例外クラス
    """
    pass


class Error(Exception):
    """警告以外の全ての例外クラスのベースとなる例外クラス    """
    pass


class UnexpectedError(Error):
    """予期しないエラーの例外クラス

    Args:

        msg (str): エラーの説明文
        code(:obj:`int`, optional): エラーコード
    """
    pass


class InterfaceError(Error):
    """インターフェースに関連する例外クラス
    """
    pass


class DatabaseError(Error):
    """データベースから取得した例外のベースとなる例外クラス

    Args:
        error_message (str): エラーの説明文
        state_code (str): SQLのステート
        error_code(int): エラーコード

    Attributes:
        error_message (str): エラーの説明文
        state_code (Optional[str]): SQLのステート
        error_code(Optional[int): エラーコード
    """

    def __init__(self, error_message: str,
                 state_code: Optional[str] = None,
                 error_code: Optional[int] = None):
        self.error_message = error_message
        if state_code is not None:
            self.state_code = state_code
        if error_code is not None:
            self.error_code = error_code

    def __str__(self) -> str:
        return self.error_message


class DataError(DatabaseError):
    """データの処理に関連する例外

    Args:
        error_message (str): エラーの説明文

    Attributes:
        error_message (str): エラーの説明文
    """

    def __init__(self, error_message):
        super().__init__(error_message)

    def __str__(self) -> str:
        return super().__str__()


class OperationalError(DatabaseError):
    """データベースのオペレーションに関連する例外

    Args:
        error_message (str): エラーの説明文

    Attributes:
        error_message (str): エラーの説明文
    """

    def __init__(self, error_message):
        super().__init__(error_message)

    def __str__(self) -> str:
        return super().__str__()


class IntegrityError(DatabaseError):
    """データベースのリレーションの整合性に関連する例外

    Args:
        error_message (str): エラーの説明文

    Attributes:
        error_message (str): エラーの説明文
    """

    def __init__(self, error_message):
        super().__init__(error_message)

    def __str__(self) -> str:
        return super().__str__()


class InternalError(DatabaseError):
    """データベースの内部エラーに関連する例外

    Args:
        error_message (str): エラーの説明文

    Attributes:
        error_message (str): エラーの説明文
    """

    def __init__(self, error_message):
        super().__init__(error_message)

    def __str__(self) -> str:
        return super().__str__()


class ProgrammingError(DatabaseError):
    """プログラミングエラーに関連する例外

    Args:
        error_message (str): エラーの説明文

    Attributes:
        error_message (str): エラーの説明文
    """

    def __init__(self, error_message):
        super().__init__(error_message)

    def __str__(self) -> str:
        return super().__str__()


class NotSupportedError(DatabaseError):
    """サポート外の機能に対する例外

    Args:
        error_message (str): エラーの説明文

    Attributes:
        error_message (str): エラーの説明文
    """

    def __init__(self, error_message):
        super().__init__(error_message)

    def __str__(self) -> str:
        return super().__str__()
