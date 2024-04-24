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
language.py -- 言語を扱うクラスのモジュール

:class: `LanguageTag`
:class: `LanguageSet`
"""
from typing import Union, List

import copy


from .iostream import InputStream, OutputStream
from .constants import Country, Language
from ..exception.exceptions import ProgrammingError, UnexpectedError


class LanguageTag:
    """言語種別と国・地域種別のペア（言語タグ）を表現するクラス.

    Args:
        language_code (int): 言語種別コード
        countrycode (int): 国・地域コード

    Attributes:
        language_code (int): 言語種別コード
        country_code (int): 国・地域コード
    """

    def __init__(self,
                 language_code: int = None,
                 country_code: int = None
                 ) -> None:
        self.__language_code = Language.UNDEFINED
        self.__country_code = Country.UNDEFINED

        # Country Codeのみの指定は許容しない
        if language_code is Language.UNDEFINED\
            or language_code is None\
                and country_code is not None:
            raise ProgrammingError(
                'language code can not be UNDEFINED or None while country code is defined')

        if language_code and language_code is not Language.UNDEFINED:
            if Language.is_valid(language_code):
                self.__language_code = language_code
            else:
                raise ProgrammingError(f'{language_code} is invalid')
        if country_code and country_code is not Country.UNDEFINED:
            if Country.is_valid(country_code):
                self.__country_code = country_code
            else:
                raise ProgrammingError(f'{country_code} is invalid')

    @property
    def language_code(self) -> int:
        """言語種別コードのゲッター
        """
        return self.__language_code

    @property
    def country_code(self) -> int:
        """国・地域コードのゲッター
        """
        return self.__country_code

    def __eq__(self, other_: object) -> bool:
        """言語タグが等しいか比較する.

        Args:
            other_ (object): 比較対象

        Returns:
            bool: True  等しい
                  False 等しくない
        """
        if isinstance(other_, LanguageTag):
            return self.__language_code == other_.language_code\
                and self.__country_code == other_.country_code

        return False

    def clone(self) -> object:
        """オブジェクトのコピーを作成して返す.

        Returns:
            object: コピーされたオブジェクト
        """
        return LanguageTag(self.__language_code, self.__country_code)

    def __hash__(self) -> int:
        """ハッシュコードを得る

        Returns:
            int: ハッシュコード
        """
        return self.__country_code << 4 | self.__language_code


class LanguageSet:
    """複数の言語タグ(以下言語セット)を扱うクラス.

    Args:
        arg (Union[str, 'LanguageSet']):
        language_set_symbol(str)           言語セットを示す文字列
        (LanguageSet)   言語セット

    Attributes:
        taglist (list): 言語タグを積むリスト
        size(int): 登録された言語タグの数
    """
    # 言語タグの区切り文字
    TAG_SEPARATOR = '+'
    # 言語種別コードと国・地域コードの区切り文字
    LANGUAGE_COUNTRY_SEPARATOR = '-'

    def __init__(self,
                 arg: Union[str, 'LanguageSet'] = None
                 ) -> None:
        self.__taglist: List[LanguageTag] = []
        if isinstance(arg, str) or isinstance(arg, LanguageSet):
            self.set(arg)
        elif arg is None:
            # 引数がない場合taglistは空
            pass
        else:
            raise ProgrammingError('bad argument')

    @property
    def size(self) -> int:
        """言語タグ数のゲッター"""
        return len(self.__taglist)

    @property
    def taglist(self) -> list:
        """言語セットのリストのゲッター"""
        return self.__taglist

    def __len__(self) -> int:
        return len(self.__taglist)

    def __eq__(self, other_: object) -> bool:
        """データが等しいか比較する.

        Args:
            other_ (object): 比較対象

        Returns:
            bool: True  等しい
                  False 等しくない
        """
        if isinstance(other_, LanguageSet):
            return self.__taglist == other_.taglist
        return False

    def __hash__(self) -> int:
        """ハッシュコードを得る

        Returns:
            int: ハッシュコード
        """
        hashcode = 0
        for i in range(len(self.__taglist)):
            hashcode <<= 4
            hashcode |= hash(self.__taglist[i])

        return hashcode

    def __str__(self) -> str:
        """文字列に変換する.

        Returns:
            str: 文字列
        """
        langset_symbol = ''

        number_of_lang_tags = len(self.__taglist)
        for i in range(number_of_lang_tags):
            lang_tag = self.__taglist[i]

            langset_symbol += Language.to_symbol(lang_tag.language_code)

            if lang_tag.country_code != Country.UNDEFINED:
                langset_symbol += self.LANGUAGE_COUNTRY_SEPARATOR
                langset_symbol += Country.to_symbol(lang_tag.country_code)

            if i < number_of_lang_tags - 1:
                langset_symbol += self.TAG_SEPARATOR

        return langset_symbol

    def set(self, arg: Union[str, 'LanguageSet'] = None) -> None:
        """言語セットを設定する.

        Args:
            arg　(Union[str], LanguageTag]):
            (str)          言語セットを示す文字列
            (LanguageSet)  言語セット
        """
        if isinstance(arg, LanguageSet):
            self.__taglist = copy.deepcopy(arg.taglist)
            return

        if isinstance(arg, str):
            symbol_len = len(arg)
            if symbol_len == 0:
                self.clear()
                return

            tmp_langset = LanguageSet()

            # 言語セットごとに分割
            lang_sets = arg.split(self.TAG_SEPARATOR)

            # 言語セットを``tmp_langset``に登録
            for items in lang_sets:
                lang_set = items.split(self.LANGUAGE_COUNTRY_SEPARATOR)
                lang_code = Language.UNDEFINED
                country_code = Country.UNDEFINED

                # 言語種別コードの作成
                lang_code = Language.to_code(lang_set[0])
                if Language.is_valid(lang_code) is False:
                    raise ProgrammingError(
                        f'illegal language symbol {lang_set[0]}.')

                # 国・地域別コードの作成
                if len(lang_set) == 2:
                    country_code = Country.to_code(lang_set[1])
                    if Country.is_valid(country_code) is False:
                        raise ProgrammingError(
                            f'illegal country symbol {lang_set[1]}.')

                lang_tag = LanguageTag(lang_code, country_code)
                tmp_langset.add(lang_tag)

            self.__taglist = tmp_langset.taglist

    def add(self, arg: Union[int, LanguageTag]) -> None:
        """言語セットに指定言語を追加する.

        Args:
            arg (Union[int, LanguageTag]):
            (int)           追加する言語の種別コード
            (LanguageTag)   追加する言語タグ

        Raises:
            ProgrammingError
            UnexpectedError
        """
        lang_tag_: LanguageTag = None

        if isinstance(arg, int):
            # 引数がintだった場合LanguageTagに変える
            lang_tag_ = LanguageTag(arg, Country.UNDEFINED)
        elif isinstance(arg, LanguageTag):
            lang_tag_ = arg
        else:
            raise ProgrammingError(f'bad argument {arg}.')

        # Language CodeがUNDEFINEDだった場合例外として処理
        if Language.is_valid(lang_tag_.language_code) is False:
            raise UnexpectedError(
                f'illegal language code {lang_tag_.language_code}.')

        number_of_lang_tags = len(self.__taglist)
        i = 0
        while True:
            # 終了条件
            if i >= number_of_lang_tags:
                # リストにないので末尾に追加する
                self.__taglist.append(lang_tag_)
                break

            lang_tag = self.__taglist[i]

            # 既に言語コードがリストにある場合
            if lang_tag_.language_code == lang_tag.language_code:
                while True:
                    lang_tag = self.__taglist[i]
                    if lang_tag_.language_code != lang_tag.language_code:
                        break

                    # 言語および国・地域コードが等しい言語タグが存在する場合、追加しない
                    if lang_tag_.country_code == lang_tag.country_code:
                        return

                    if lang_tag_.country_code < lang_tag.country_code:
                        break

                    i += 1
                    # 終了条件
                    if i >= number_of_lang_tags:
                        break

                # 途中に追加して終了
                self.__taglist.insert(i, lang_tag_)
                return

            elif lang_tag_.language_code < lang_tag.language_code:
                # 途中に追加して終了
                self.__taglist.insert(i, lang_tag_)
                return

            i += 1

    def clear(self) -> None:
        """言語セットをクリアする.
        """
        self.__taglist.clear()

    def is_contained(self, arg: Union[int, LanguageTag]) -> bool:
        """指定言語が含まれているかどうかを調べる.

        Args:
            arg  (Union[int, LanguageTag]):
            (int)           調べる対象言語の種別コード
            (LanguageTag)   調べる対象言語タグ

        Returns:
            bool: True  含まれている
                  False 含まれていない

        Raises:
            ProgrammingError
            UnexpectedError
        """
        number_of_lang_tags = len(self.__taglist)

        # 引数がlanguage code(int)の場合
        if isinstance(arg, int):
            if Language.is_valid(arg) is False:
                raise UnexpectedError(f'illegal language code {arg}.')

            for i in range(number_of_lang_tags):
                lang_tag = self.__taglist[i]

                if lang_tag.language_code == arg:
                    return True

            return False

        # 引数がLanguage Tag(LanguageTag)の場合
        elif isinstance(arg, LanguageTag):
            if Language.is_valid(arg.language_code) is False:
                raise UnexpectedError(
                    f'illegal language code {arg.language_code}.')

            return arg in self.__taglist

        else:
            raise ProgrammingError(f'bad argument {arg}.')

    def round(self) -> 'LanguageSet':
        """言語タグから国・地域コードを除いた言語セットを取得する.

        Returns:
            LangaugeSet: このオブジェクトの言語タグから国・地域コードを除いた言語セット
        """
        no_country_set = LanguageSet()

        number_of_lang_tags = len(self.__taglist)

        for i in range(number_of_lang_tags):
            lang_tag = self.__taglist[i]

            no_country_tag = LanguageTag(
                lang_tag.language_code, Country.UNDEFINED)
            no_country_set.add(no_country_tag)

        return no_country_set

    def read_object(self, input_: InputStream) -> None:
        """ストリームからオブジェクトを読込む.

        Args:
            input_ (InputStream): 入力用のストリーム
        """
        self.clear()

        # 言語タグ数
        number_of_lang_tags = input_.read_int()

        for i in range(number_of_lang_tags):
            # 言語コード
            lang_code = input_.read_short()

            # 国・地域コード
            country_code = input_.read_short()

            if not Language.is_valid(lang_code):
                raise UnexpectedError(f'unknown language code {lang_code}.')
            if country_code != Country.UNDEFINED\
                    and not Country.is_valid(country_code):
                raise UnexpectedError(f'unknown country code {country_code}.')

            lang_tag = LanguageTag(lang_code, country_code)

            self.add(lang_tag)

    def write_object(self, output_: OutputStream) -> None:
        """ストリームにオブジェクトを書き出す.

        Args:
            output_ (OutputStream): 出力用のストリーム
        """
        # 言語タグ数の書込み
        number_of_lang_tags = len(self.__taglist)
        output_.write_int(number_of_lang_tags)

        for i in range(number_of_lang_tags):
            lang_tag = self.__taglist[i]

            lang_code = lang_tag.language_code
            country_code = lang_tag.country_code

            # 言語コードと国・地域コードの書込み
            output_.write_short(lang_code)
            output_.write_short(country_code)
