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
data.py -- :class: `Data` を基底とするクラスの実装モジュール.

:class: `BinaryData`
:class: `WordData`
"""
from typing import Union


from .abstracts import Data
from .serializable import Serializable
from .constants import ClassID, DataType
from .language import LanguageSet
from .iostream import InputStream, OutputStream
from ..exception import exceptions
from ..common.unicodestr import UnicodeString


class BinaryData(Data, Serializable):
    """バイナリ型をあらわすクラス

    Args:
        value (Union[bytes, 'BinaryData'])

    Attributes:
        value (bytes): 値
    """

    @property
    def value(self) -> bytes:
        """値のゲッター."""
        return self.__value

    @value.setter
    def value(self, value: bytes) -> None:
        """値のセッター."""
        self.__value = value

    @property
    def class_id(self) -> int:
        return ClassID.BINARY_DATA.value

    def __init__(self,
                 value: Union[bytes, 'BinaryData'] = None) -> None:
        super().__init__(DataType.BINARY.value)
        if isinstance(value, bytes):
            self.__value = value
        elif isinstance(value, BinaryData):
            self.__value = value.value
        elif value is None:
            self.__value = bytes(0)
        else:
            raise exceptions.UnexpectedError('unexpected argument')

    def __eq__(self, other_: object) -> bool:
        result = False
        if isinstance(other_, BinaryData):
            result = (self.__value == other_.__value)

        return result

    def __str__(self) -> str:
        s = 'size='
        s += str(len(self.__value))
        return s

    def read_object(self, input_: InputStream) -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        # unsigned intはないのでintで読込む
        _len = input_.read_int()
        self.__value = input_.read(_len)

    def write_object(self, output_: OutputStream) -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        # unsigned intはないのでintで書込む
        output_.write_int(len(self.__value))
        output_.write(self.__value)

    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return BinaryData(self)


class WordData(Data, Serializable):
    """ワードをあらわすクラス

    Args:
        term (str): 単語
        value (Optional['WordData'])

    Attributes:
        term (str): 単語
        language (LanguageSet): 言語
        category (int): カテゴリー番号
        scale (float): スケール
        docfrequency (int): 文書頻度

    TODO:
        未実装
    """
    # カテゴリ - 未定義
    CATEGORY_UNDEFINED = 0

    # カテゴリ - 必須
    CATEGORY_ESSENTIAL = 1
    # カテゴリ - 重要
    CATEGORY_IMPORTANT = 2
    # カテゴリ - 有用
    CATEGORY_HELPFUL = 3
    # カテゴリ - 必須関連語
    CATEGORY_ESSENTIAL_RELATED = 4
    # カテゴリ - 重要関連語
    CATEGORY_IMPORTANT_RELATED = 5
    # カテゴリ - 有用関連語
    CATEGORY_HELPFUL_RELATED = 6
    # カテゴリ - 禁止
    CATEGORY_PROHIBITIVE = 7
    # カテゴリ - 禁止関連語
    CATEGORY_PROHIBITIVE_RELATED = 8

    __CATEGORY_STRING = ['Undefined',
                         'Essential',
                         'Important',
                         'Helpful',
                         'EssentialRelated',
                         'ImportantRelated',
                         'HelpfulRelated',
                         'Prohibitive',
                         'ProhibitiveRelated']

    def __init__(self, value: Union[str, 'WordData'] = None
                 ) -> None:
        super().__init__(DataType.WORD.value)
        self.__language = LanguageSet()
        if isinstance(value, str):
            self.__term = value
            self.__category = self.CATEGORY_UNDEFINED
            self.__scale = 0.0
            self.__docfrequency = 0
        elif isinstance(value, WordData):
            self.__term = value.term
            self.__language = value.language
            self.__category = value.category
            self.__scale = value.scale
            self.__docfrequency = value.docfrequency
        elif value is None:
            self.__term = ''
            self.__category = self.CATEGORY_UNDEFINED
            self.__scale = 0.0
            self.__docfrequency = 0
        else:
            raise exceptions.ProgrammingError('bad argument')

    def __eq__(self, other_: object) -> bool:
        result = False
        if isinstance(other_, WordData):
            result = (self.__term == other_.term
                      and self.__language == other_.language
                      and self.__category == other_.category
                      and self.__scale == other_.scale
                      and self.__docfrequency == other_.docfrequency)

        return result

    def __str__(self) -> str:
        if self.__category != self.CATEGORY_UNDEFINED:
            # word(xxx) 用
            s = f"{self.__term} language {str(self.__language)} category {self.__CATEGORY_STRING[self.__category]} scale {self.__scale} df {self.__docfrequency}"
        else:
            # cluster(xxx).keyword 用
            s = f"{self.__term} scale {self.__scale}"

        return s

    def __hash__(self) -> int:
        return hash(self.__language) + self.__category << 4\
            | hash(self.__term)

    @ property
    def term(self) -> str:
        """単語のゲッター."""
        return self.__term

    @ term.setter
    def term(self, term: str) -> None:
        """単語のセッター."""
        self.__term = term

    @ property
    def languagestr(self) -> str:
        """言語文字列のゲッター."""
        return str(self.__language)

    @ languagestr.setter
    def languagestr(self, lang: str) -> None:
        """言語文字列のセッター."""
        self.__language.set(lang)

    @ property
    def language(self) -> LanguageSet:
        """言語のゲッター"""
        return self.__language

    @ language.setter
    def language(self, lang: LanguageSet) -> None:
        """言語のセッター."""
        self.__language.set(lang)

    @ property
    def category(self) -> int:
        """カテゴリーのゲッター."""
        return self.__category

    @ category.setter
    def category(self, category: int) -> None:
        """カテゴリーのセッター."""
        self.__category = category

    @ property
    def scale(self) -> float:
        """スケールのゲッター."""
        return self.__scale

    @ scale.setter
    def scale(self, scale: float) -> None:
        """スケールのセッター."""
        self.__scale = scale

    @ property
    def docfrequency(self) -> int:
        """文書頻度のゲッター."""
        return self.__docfrequency

    @ docfrequency.setter
    def docfrequency(self, docfrequency: int) -> None:
        """文書頻度のセッター."""
        self.__docfrequency = docfrequency

    @ property
    def class_id(self) -> int:
        """クラスIDのゲッター."""
        return ClassID.WORD_DATA.value

    def read_object(self, input_: InputStream) -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.__term = UnicodeString.read_object(input_)
        self.__language.read_object(input_)
        self.__category = input_.read_int()
        self.__scale = input_.read_double()
        self.__docfrequency = input_.read_int()

    def write_object(self, output_: OutputStream) -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        UnicodeString.write_object(output_, self.__term)
        self.__language.write_object(output_)
        output_.write_int(self.__category)
        output_.write_double(self.__scale)
        output_.write_int(self.__docfrequency)

    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return WordData(self)
