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
scalardata.py -- :class: `ScalarData` を基底とするクラスの実装モジュール.

:class: `DateData`
:class: `DateTimeData`
:class: `DecimalData`
:class: `DoubleData`
:class: `FLoatData`
:class: `Integer64Data`
:class: `IntegerData`
:class: `LanguageData`
:class: `NullData`
:class: `StringData`
"""
from typing import Union, TYPE_CHECKING, List
if TYPE_CHECKING:
    from .iostream import InputStream, OutputStream

import re
from decimal import Decimal
# 循環参照を回避するため、元の型をインポートする
from datetime import date, datetime

from ..exception import exceptions
from .serializable import Serializable
from .abstracts import ScalarData
from .language import LanguageSet
from .constants import ClassID, DataType
from ..common.unicodestr import UnicodeString


class DateData(ScalarData, Serializable):
    """date型をあらわすクラス.

    Args:
        value (Union[date, 'DateData']): 値

    Attributes:
        value (date): 値
    """

    @property
    def value(self) -> date:
        """値(date)のゲッター."""
        return self.__value

    @value.setter
    def value(self, value: date) -> None:
        """値(date)のセッター."""
        self.__value = value

    @property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.DATE_DATA.value

    def __init__(self,
                 value: Union[date, 'DateData'] = None
                 ) -> None:
        super().__init__(DataType.DATE.value)
        if isinstance(value, date):
            self.__value = value
        elif isinstance(value, DateData):
            self.__value = value.value
        elif value is None:
            self.__value = date(1000, 1, 1)
        else:
            raise exceptions.UnexpectedError('unexpected argument')

    def __eq__(self, other_: object) -> bool:
        result = False
        if isinstance(other_, DateData):
            result = (self.__value == other_.__value)

        return result

    def __str__(self) -> str:
        return self.__value.strftime('%Y-%m-%d')

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        year = input_.read_int()
        month = input_.read_int()
        day = input_.read_int()
        self.__value = date(year, month, day)

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        output_.write_int(self.__value.year)
        output_.write_int(self.__value.month)
        output_.write_int(self.__value.day)

    # 使わない機能なのでコメントアウトしている
    # (JDBCには実装されているメソッド)
    '''
    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return DateData(self)
    '''


class DateTimeData(ScalarData, Serializable):
    """datetime型をあらわすクラス.

    Args:
        value (Union[datetime, 'DateTimeData']): 値

    Attributes:
        value (datetime): 値
        millisecond (int): ミリ秒
    """

    @property
    def value(self) -> datetime:
        """値(datetime)のゲッター."""
        return self.__value

    @value.setter
    def value(self, value: datetime) -> None:
        """値(datetime)のセッター."""
        self.__value = value
        self.__millisecond = int(value.microsecond/1000)

    @property
    def millisecond(self) -> int:
        """値(datetime)のゲッター."""
        return self.__millisecond

    @property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.DATE_TIME_DATA.value

    def __init__(self,
                 value: Union[datetime,
                              'DateTimeData',
                              ] = None
                 ) -> None:
        super().__init__(DataType.DATE_TIME.value)
        if isinstance(value, datetime):
            self.__value = value
            self.__millisecond = int(value.microsecond/1000)
        elif isinstance(value, DateTimeData):
            self.__value = value.value
            self.__millisecond = value.millisecond
        elif value is None:
            self.__value = datetime(1000, 1, 1)
            self.__millisecond = 0
        else:
            raise exceptions.UnexpectedError('unexpected argument')

    def __eq__(self, other_: object) -> bool:
        result = False
        if isinstance(other_, DateTimeData):
            result = (self.__value == other_.__value)

        return result

    def __str__(self) -> str:
        s = self.__value.strftime('%Y-%m-%d %H:%M:%S.')
        if self.__millisecond < 10:
            s += '0'
        if self.__millisecond < 100:
            s += '0'
        s += str(self.__millisecond)
        return s

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        year = input_.read_int()
        month = input_.read_int()
        day = input_.read_int()
        hour = input_.read_int()
        minute = input_.read_int()
        second = input_.read_int()
        self.__millisecond = input_.read_int()
        input_.read_int()  # precisionは読み捨てる
        self.__value = datetime(year, month, day,
                                hour, minute, second,
                                self.__millisecond*1000)

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        output_.write_int(self.__value.year)
        output_.write_int(self.__value.month)
        output_.write_int(self.__value.day)
        output_.write_int(self.__value.hour)
        output_.write_int(self.__value.minute)
        output_.write_int(self.__value.second)
        output_.write_int(self.__millisecond)
        # precisionは常に3
        output_.write_int(3)

    # 使わない機能なのでコメントアウトしている
    # (JDBCには実装されているメソッド)
    '''
    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return DateTimeData(self)
    '''


class DecimalData(ScalarData, Serializable):
    """decimal型をあらわすクラス.

    Args:
        value (Union[str, 'DecimalData']): 値

    Attributes:
        value (str): 値
        _precision (int): 全体の桁数
        _scale (int): 小数点以下の桁数
    """
    __iDigitPerUnit = 9
    __iDigitMask = 100000000

    PLUS = '+'
    MINUS = '-'
    PERIOD = '.'

    @ property
    def value(self) -> str:
        """値(str)のゲッター."""
        return self.__value

    @property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.DECIMAL_DATA.value

    @ value.setter
    def value(self, value: str) -> None:
        """値(str)のセッター.

        Notes:
            文字列による数値表現は、先頭の文字が'+', '-'であり、数字と'.'で構成される場合のみを許容する。
            精度は倍精度浮動小数点数までが利用可能。
            正常例：'+100', '-0.12', '1.2000'
            異常例：'000.2', '0010', '.12', '12.', '1e3', '12+1'
        """
        # 数値かチェックする
        # 正規表現での確認
        if not re.fullmatch(r'^[-+]?\d+\.?\d+|[-+]?\d+', value):
            raise exceptions.ProgrammingError(f'{value} is not valid number')
        # 先頭に0が１個以上ある場合ははじく 0010, 00.2など
        elif re.fullmatch(r'[-+]?0\d+\.\d+|[-+]?0+\d+', value):
            raise exceptions.ProgrammingError(f'{value} is not valid number')
        # floatにキャストできるかで確認
        try:
            float(value)
        except ValueError:
            raise exceptions.ProgrammingError(f'{value} is not valid number')

        self._scale = 0
        # 精度の計算
        precision = len(value)
        if self.MINUS in value or self.PLUS in value:
            precision -= 1
        if self.PERIOD in value:
            precision -= 1
            # 整数値が 0 の場合はプレシジョンを１下げる
            if int(value.split(self.PERIOD)[0]) == 0:
                precision -= 1

            # スケールを計算
            try:
                decimal_value = value.split(self.PERIOD)[1]
            except IndexError:
                raise exceptions.UnexpectedError(f'{value} is invalid')
            self._scale = len(decimal_value)

        self._precision = precision

        self.__value = value

    def __init__(self,
                 value: Union[str, 'DecimalData'] = None) -> None:
        super().__init__(DataType.DECIMAL.value)
        if isinstance(value, str):
            # セッターで登録
            self.value = value
        elif isinstance(value, DecimalData):
            self._precision = value._precision
            self._scale = value._scale
            self.__value = value.value
        elif value is None:
            self._precision = self.__iDigitPerUnit
            self._scale = 0
            self.__value = '0'
        else:
            raise exceptions.UnexpectedError('unexpected argument')

    def __eq__(self, other_: object) -> bool:
        result = False
        if isinstance(other_, DecimalData):
            result = (Decimal(self.__value) == Decimal(other_.__value))
        return result

    def __str__(self) -> str:
        return str(self.__value)

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        # まずprecisionとscaleを読込む
        self._precision = input_.read_int()
        self._scale = input_.read_int()

        # 整数値の桁数を読込む
        integer_len = input_.read_int()
        # 小数点以下の桁数を読込む
        fraction_len = input_.read_int()

        # 正負を読込む
        negative = input_.read(1)

        vec_size = input_.read_int()
        vec_digit: List[int] = []
        for i in range(vec_size):
            vec_digit.append(input_.read_int())

        # 文字列を作成
        decimal_str = ''

        if negative != b'\x00':
            decimal_str += '-'

        # 整数値の作成
        if integer_len > 0:
            last = int(
                (integer_len + self.__iDigitPerUnit - 1) / self.__iDigitPerUnit
            )

            # 最初の値を作成
            unit_value = vec_digit[0]
            if unit_value > 0:
                decimal_str += str(unit_value)

            # 残りの値を作成
            for i in range(1, last):
                unit_value = vec_digit[i]
                decimal_str += str(unit_value).zfill(9)
        else:
            decimal_str += '0'

        # 小数点以下の値の作成
        if fraction_len > 0:
            decimal_str += '.'
            fraction_str = ''

            i = int(
                (integer_len + self.__iDigitPerUnit - 1) / self.__iDigitPerUnit
            )
            end = fraction_len
            while True:
                unit_value = vec_digit[i]
                fraction_str += str(unit_value).zfill(9)

                i += 1
                end -= self.__iDigitPerUnit
                # 終了条件
                if end <= 0:
                    break

            # 小数点以下の桁数に対して文字列が大きい場合は桁数内までで切る
            if len(fraction_str) > fraction_len:
                decimal_str += fraction_str[0:fraction_len]
            else:
                decimal_str += fraction_str

        self.__value = decimal_str

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        # 負のフラグ(bytes)
        if self.MINUS in self.__value:
            b_is_negative = b'\x01'
        else:
            b_is_negative = b'\x00'

        decimal_str = self.__value

        # 整数の桁数と小数点以下の桁数 (DecimalDataから)
        integer_len_dd = self._precision - self._scale
        fraction_len_dd = self._scale

        # 整数の桁数と小数点以下の桁数 (__valueから)
        integer_len = 0
        fraction_len = 0

        size = 0
        vec_digit: List[int] = []

        # ピリオド位置のインデックス
        period_index = decimal_str.find('.')
        # 整数値のみの場合
        if period_index < 0:
            period_index = len(decimal_str)
        # 少数値がある場合
        else:
            fraction_len = len(decimal_str) - period_index - 1

        integer_len = period_index

        check_zero = Decimal(self.__value)
        if check_zero.is_zero():
            # DecimalDataの数値を使用
            integer_len = integer_len_dd
            fraction_len = fraction_len_dd
            size = (integer_len + self.__iDigitPerUnit - 1) / self.__iDigitPerUnit\
                + (fraction_len + self.__iDigitPerUnit - 1) / self.__iDigitPerUnit
            for i in range(size):
                vec_digit.insert(i, 0)
        else:
            integer_str = decimal_str[0:integer_len]
            fraction_str = None

            # 小数点以下の部分を文字列に変換
            if fraction_len > 0:
                fraction_str = decimal_str[integer_len+1:]

            work_int_str = ''
            work_frac_str = None

            # 0埋めする
            for i in range(integer_len_dd - integer_len):
                work_int_str += '0'
            work_int_str += integer_str

            if fraction_len > 0:
                work_frac_str = ''
                work_frac_str += fraction_str
                # 0埋めする
                for i in range(fraction_len_dd - fraction_len):
                    work_frac_str += '0'
                # 最後のユニット分も0埋めする
                for i in range(self.__iDigitPerUnit):
                    work_frac_str += '0'

        int_digit_size = int((integer_len + self.__iDigitPerUnit -
                              1) / self.__iDigitPerUnit)
        frac_digit_size = int((
            fraction_len + self.__iDigitPerUnit - 1) / self.__iDigitPerUnit)
        size = int_digit_size + frac_digit_size
        vec_digit.clear()

        target = int_digit_size - 1
        end_position = len(work_int_str)

        # 整数値の処理
        i = integer_len
        while True:
            if i > self.__iDigitPerUnit:
                vec_digit.insert(target, int(
                    work_int_str[
                        end_position-self.__iDigitPerUnit:end_position])
                )
                end_position -= self.__iDigitPerUnit
                target -= 1
            else:
                vec_digit.insert(target, int(
                    work_int_str[:end_position])
                )

            target -= 1
            i -= self.__iDigitPerUnit
            # 終了条件
            if i <= 0:
                break

        # 少数値の処理
        if fraction_len > 0:
            target = int_digit_size
            begin_position = 0
            i = fraction_len
            while True:
                vec_digit.insert(target, int(
                    work_frac_str[begin_position:begin_position +
                                  self.__iDigitPerUnit])
                                 )
                begin_position += self.__iDigitPerUnit

                target += 1
                i -= self.__iDigitPerUnit
                # 終了条件
                if i <= 0:
                    break

        # TRへの書き込み
        output_.write_int(self._precision)
        output_.write_int(self._scale)

        output_.write_int(integer_len)
        output_.write_int(fraction_len)

        output_.write(b_is_negative)

        output_.write_int(size)

        # digitsの書き込み
        for i in range(size):
            output_.write_int(vec_digit[i])

    # 使わない機能なのでコメントアウトしている
    # (JDBCには実装されているメソッド)
    ''''
    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return DecimalData(self)
    '''


class DoubleData(ScalarData, Serializable):
    """double型をあらわすクラス.

    Args:
        value (Union[float, 'DoubleData']): 値

    Attributes:
        value (double): 値

    Notes:
        Python では float = 倍精度浮動小数点数(double)なので float で実装する.
    """

    @ property
    def value(self) -> float:
        """値(float)のゲッター."""
        return self.__value

    @ value.setter
    def value(self, value: float) -> None:
        """値(float)のセッター."""
        self.__value = value

    @ property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.DOUBLE_DATA.value

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.__value = input_.read_double()

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        output_.write_double(self.__value)

    def __init__(self,
                 value: Union[float, 'DoubleData'] = None) -> None:
        super().__init__(DataType.DOUBLE.value)
        if isinstance(value, float):
            self.__value = value
        elif isinstance(value, DoubleData):
            self.__value = value.value
        elif value is None:
            self.__value = 0.0
        else:
            raise exceptions.UnexpectedError

    def __eq__(self, other_: object) -> bool:
        result = False
        if isinstance(other_, DoubleData):
            result = (self.__value == other_.__value)

        return result

    def __str__(self) -> str:
        return str(self.__value)

    # 使わない機能なのでコメントアウトしている
    # (JDBCには実装されているメソッド)
    '''
    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return DoubleData(self)
    '''


class FloatData(ScalarData, Serializable):
    """float型をあらわすクラス.

    Args:
        value (Union[float, 'FloatData']): 値

    Attributes:
        value (float): 値

    Notes:
        Pythonには単精度不動小数点がないため、float(倍精度浮動小数点)で実装する.
    """

    @ property
    def value(self) -> float:
        """値(float)のゲッター."""
        return self.__value

    @ value.setter
    def value(self, value: float) -> None:
        """値(float)のセッター."""
        self.__value = value

    @ property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.FLOAT_DATA.value

    def __init__(self,
                 value: Union[float, 'FloatData'] = None
                 ) -> None:
        super().__init__(DataType.FLOAT.value)
        if isinstance(value, float):
            self.__value = value
        elif isinstance(value, FloatData):
            self.__value = value.value
        elif value is None:
            self.__value = 0.0
        else:
            raise exceptions.UnexpectedError('unexpected argument')

    def __eq__(self, other_: object) -> bool:
        result = False
        if isinstance(other_, FloatData):
            result = (self.__value == other_.__value)

        return result

    def __str__(self) -> str:
        return str(self.__value)

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.__value = input_.read_float()

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        output_.write_float(self.__value)

    # 使わない機能なのでコメントアウトしている
    # (JDBCには実装されているメソッド)
    '''
    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return FloatData(self)
    '''


class Integer64Data(ScalarData, Serializable):
    """long型をあらわすクラス.

    Args:
        value (Union[int, 'Integer64Data']): 値

    Attributes:
        value (int): 値

    Note:
        pythonのint型にビット幅はない.
        ``value``が64bitよりも大きい場合 :func: `write_object` でエラーとなる.
    """

    @ property
    def value(self) -> int:
        """値(int)のゲッター."""
        return self.__value

    @ value.setter
    def value(self, value: int) -> None:
        """値(int)のセッター."""
        self.__value = value

    @ property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.INTEGER64_DATA.value

    def __init__(self,
                 value: Union[int, 'Integer64Data'] = None
                 ) -> None:
        super().__init__(DataType.INTEGER64.value)
        if isinstance(value, int):
            self.__value = value
        elif isinstance(value, Integer64Data):
            self.__value = value.value
        elif value is None:
            self.__value = 0
        else:
            raise exceptions.UnexpectedError('unexpected argument')

    def __eq__(self, other_: object) -> bool:
        result = False
        if isinstance(other_, Integer64Data):
            result = (self.__value == other_.__value)

        return result

    def __str__(self) -> str:
        return str(self.__value)

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.__value = input_.read_long()

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        output_.write_long(self.__value)

    # 使わない機能なのでコメントアウトしている
    # (JDBCには実装されているメソッド)
    '''
    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return Integer64Data(self)
    '''


class IntegerData(ScalarData, Serializable):
    """int型をあらわすクラス.

    Args:
        value (Union[int, 'IntegerData']): 値

    Attributes:
        value (int): 値

    Note:
        pythonのint型にビット幅はない.
        ``value``が32bitよりも大きい場合 :func: `write_object` でエラーとなる.
    """

    @ property
    def value(self) -> int:
        """値(int)のゲッター."""
        return self.__value

    @ value.setter
    def value(self, value: int) -> None:
        """値(int)のセッター."""
        self.__value = value

    @ property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.INTEGER_DATA.value

    def __init__(self,
                 value: Union[int, 'IntegerData'] = None
                 ) -> None:
        super().__init__(DataType.INTEGER.value)
        if isinstance(value, int):
            self.__value = value
        elif isinstance(value, IntegerData):
            self.__value = value.value
        elif value is None:
            self.__value = 0
        else:
            raise exceptions.UnexpectedError('unexpected argument')

    def __eq__(self, other_: object) -> bool:
        result = False
        if isinstance(other_, IntegerData):
            result = (self.__value == other_.__value)

        return result

    def __str__(self) -> str:
        return str(self.__value)

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.__value = input_.read_int()

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        output_.write_int(self.__value)

    # 使わない機能なのでコメントアウトしている
    # (JDBCには実装されているメソッド)
    '''
    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return IntegerData(self)
    '''


class StringData(ScalarData, Serializable):
    """string型をあらわすクラス.

    Args:
        value (Union[str, 'StringData']): 値

    Attributes:
        value (str): 値
    """

    @ property
    def value(self) -> str:
        """値(str)のゲッター."""
        return self.__value

    @ value.setter
    def value(self, value: str) -> None:
        """値(str)のセッター."""
        self.__value = value

    @ property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.STRING_DATA.value

    def __init__(self,
                 value: Union[str, 'StringData'] = None
                 ) -> None:
        super().__init__(DataType.STRING.value)
        if isinstance(value, str):
            self.__value = value
        elif isinstance(value, StringData):
            self.__value = value.value
        elif value is None:
            self.__value = ''
        else:
            raise exceptions.UnexpectedError('unexpected argument')

    def __eq__(self, other_: object) -> bool:
        if isinstance(other_, StringData):
            return self.__value == other_.__value

        return False

    def __str__(self) -> str:
        return self.__value

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.__value = UnicodeString.read_object(input_)

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        UnicodeString.write_object(output_, self.__value)

    # 使わない機能なのでコメントアウトしている
    # (JDBCには実装されているメソッド)
    '''
    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return StringData(self)
    '''


class LanguageData(ScalarData, Serializable):
    """Language型をあらわすクラス.

    Args:
        value (Union[str, 'LanguageData']): 値

    Attributes:
        value (LanguageSet): 値
    """

    @ property
    def value(self) -> LanguageSet:
        """値(LanguageSet)のゲッター."""
        return self.__value

    @ property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.LANGUAGE_DATA.value

    def __init__(self,
                 value: Union[str, 'LanguageData'] = None
                 ) -> None:
        super().__init__(DataType.LANGUAGE.value)
        if isinstance(value, str):
            self.__value = LanguageSet(value)
        elif isinstance(value, LanguageData):
            self.__value = LanguageSet(value.value)
        elif value is None:
            self.__value = LanguageSet()
        else:
            raise exceptions.UnexpectedError('unexpected argument')

    def __eq__(self, other_: object) -> bool:
        result = False
        if isinstance(other_, LanguageData):
            result = (self.__value == other_.__value)

        return result

    def __str__(self) -> str:
        return str(self.__value)

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.__value.read_object(input_)

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        self.__value.write_object(output_)

    # 使わない機能なのでコメントアウトしている
    # (JDBCには実装されているメソッド)
    '''
    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return LanguageData(self)
    '''


class NullData(ScalarData, Serializable):
    """NULLをあらわすクラス.

    Attributes:
        nulldata (NullData): NullDataのインスタンス
    """
    @ property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.NULL_DATA.value

    # TODO: 未使用のためコメントアウト
    '''
    def get_instance(self) -> 'NullData':
        """NullDataのインスタンスを得る"""
        self.__nulldata = NullData()
        return self.__nulldata
    '''

    def __init__(self) -> None:
        super().__init__(DataType.NULL.value)

    def __eq__(self, other_: object) -> bool:
        return isinstance(other_, NullData)

    def __str__(self) -> str:
        return "(null)"

    def read_object(self, input_: 'InputStream') -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        pass

    def write_object(self, output_: 'OutputStream') -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        pass

    '''
    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return NullData.get_instance
    '''
