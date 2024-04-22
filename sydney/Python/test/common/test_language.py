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
test_language.py -- src.doquedb.common.language モジュールのテスト
"""
import pytest

from src.doquedb.common.language import LanguageTag, LanguageSet
from src.doquedb.common.constants import Language, Country
from src.doquedb.exception.exceptions import ProgrammingError, UnexpectedError


class TestLanguageTag:
    def setup_method(self, method):
        # 前処理
        self.test_language_tag = LanguageTag()

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_language_tag.language_code == Language.UNDEFINED
        assert self.test_language_tag.country_code == Country.UNDEFINED
        # 引数がある場合
        self.test_language_tag = LanguageTag(Language.JA, Country.JP)
        assert self.test_language_tag.language_code == Language.JA
        assert self.test_language_tag.country_code == Country.JP
        # 引数がlanguage_Codeのみの場合
        self.test_language_tag = LanguageTag(Language.JA)
        assert self.test_language_tag.language_code == Language.JA
        assert self.test_language_tag.country_code == Country.UNDEFINED

    def test_init_error(self):
        # 異常系のテスト
        # language_code=UNDEFINED, country_code!=UNDEFINEDの場合
        with pytest.raises(ProgrammingError):
            self.test_language_tag = LanguageTag(
                Language.UNDEFINED, Country.JP)
        # language_codeに引数を渡さず、country_codeだけ引数を渡した場合
        with pytest.raises(ProgrammingError):
            self.test_language_tag = LanguageTag(country_code=Country.JP)
        # language_code!=validの場合
        with pytest.raises(ProgrammingError):
            self.test_language_tag = LanguageTag(-1, Country.JP)
        # country_code!=validの場合
        with pytest.raises(ProgrammingError):
            self.test_language_tag = LanguageTag(Language.JA, -1)

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        self.test_language_tag = LanguageTag(Language.JA, Country.JP)
        assert self.test_language_tag == LanguageTag(Language.JA, Country.JP)
        # 型が違う場合
        assert self.test_language_tag != int(Language.JA)
        # 型が同じで``value``が違う場合
        test_wrong_value_language = LanguageTag(Language.EN, Country.JP)
        test_wrong_value_country = LanguageTag(Language.JA, Country.US)
        assert self.test_language_tag != test_wrong_value_language
        assert self.test_language_tag != test_wrong_value_country

    def test_hash(self):
        self.test_language_tag = LanguageTag(Language.JA, Country.JP)
        assert hash(self.test_language_tag) == Country.JP << 4 | Language.JA


class TestLanguageSet:
    def setup_method(self, method):
        # 前処理
        self.test_language_set = LanguageSet()

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_language_set.taglist == []
        # 引数がstr型の場合
        self.test_language_set = LanguageSet('ja-jp')
        assert self.test_language_set.taglist == [LanguageTag(
            Language.JA, Country.JP)]
        # 引数がLanguageSet型の場合
        self.test_language_set = LanguageSet(LanguageSet('en'))
        assert self.test_language_set.taglist == [LanguageTag(Language.EN)]

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        assert self.test_language_set == LanguageSet()
        # 型が違う場合
        assert self.test_language_set != LanguageTag()
        # 型が同じで``value``が違う場合
        test_wrong_value = LanguageSet('en-us')
        assert self.test_language_set != test_wrong_value

    def test_hash(self):
        # 正常系のテスト
        self.test_language_set = LanguageSet('ja-jp')
        hashcode = Country.JP << 4 | Language.JA
        assert hash(self.test_language_set) == hashcode
        # 言語タグを追加
        self.test_language_set.add(LanguageTag(Language.EN, Country.US))
        # ENの方がリストのインデックスが先に来るので EN→JAでhash値を計算
        hashcode = Country.US << 4 | Language.EN
        hashcode <<= 4
        hashcode |= (Country.JP << 4 | Language.JA)
        assert hash(self.test_language_set) == hashcode

    def test_str(self):
        # 正常系のテスト
        self.test_language_set = LanguageSet('ja-jp')
        self.test_language_set.add(LanguageTag(Language.ZH, Country.CN))
        assert str(self.test_language_set) == 'ja-jp+zh-cn'

    def test_set_str(self):
        # 正常系のテスト
        # 引数が文字列の場合
        self.test_language_set.set('en')
        assert self.test_language_set.taglist[0] == LanguageTag(Language.EN)

        self.test_language_set.set('ja-jp')
        # 元のリストはクリアされる
        assert self.test_language_set.taglist[0] == LanguageTag(
            Language.JA, Country.JP)

        self.test_language_set.set('ja-jp+en+zh-cn')
        taglist = [LanguageTag(Language.EN),
                   LanguageTag(Language.JA, Country.JP),
                   LanguageTag(Language.ZH, Country.CN)]
        assert self.test_language_set.taglist == taglist

    def test_set_langset(self):
        # 正常系のテスト
        # 引数が文字列の場合
        lang_set = LanguageSet()
        lang_set.add(LanguageTag(Language.EN, Country.UA))
        self.test_language_set.set(lang_set)
        assert self.test_language_set.taglist[0] == LanguageTag(
            Language.EN, Country.UA)

    def test_set_error(self):
        # 異常系のテスト
        # 誤った言語シンボルを指定した場合
        with pytest.raises(ProgrammingError, match=r'.*ea.*'):
            self.test_language_set.set('ea')

        # 誤った国・地域シンボルを指定した場合
        with pytest.raises(ProgrammingError, match=r'.*ub.*'):
            self.test_language_set.set('en-ub')

    def test_add(self):
        # 正常系のテスト
        # 引数が言語種別コードだった場合
        self.test_language_set.add(Language.EN)
        assert self.test_language_set.taglist == [LanguageTag(Language.EN)]

        # 引数が言語タグだった場合
        self.test_language_set.add(LanguageTag(Language.JA, Country.CA))
        assert self.test_language_set.taglist == [
            LanguageTag(Language.EN),
            LanguageTag(Language.JA, Country.CA)]

    def test_add_error(self):
        # 異常系のテスト
        # 引数が誤っている場合
        with pytest.raises(ProgrammingError, match=r'.*ea.*'):
            self.test_language_set.add('ea')

        # Language CodeがUNDEFINEDの場合
        with pytest.raises(ProgrammingError):
            self.test_language_set.add(Language.UNDEFINED)

    def test_is_contained(self):
        # 正常系のテスト
        self.test_language_set = LanguageSet('en+ja-jp+zh-cn')

        # 引数が言語種別コードの場合
        assert self.test_language_set.is_contained(Language.EN)
        assert not self.test_language_set.is_contained(Language.EL)

        # 引数が言語タグの場合
        assert self.test_language_set.is_contained(
            LanguageTag(Language.ZH, Country.CN))
        assert not self.test_language_set.is_contained(
            LanguageTag(Language.JA, Country.CN))

    def test_is_contained_error(self):
        # 異常系のテスト
        self.test_language_set = LanguageSet('en+ja-jp+zh-cn')

        # 引数の型が違う場合
        with pytest.raises(ProgrammingError):
            self.test_language_set.is_contained('ja')

        # 引数が言語種別コードの場合
        with pytest.raises(UnexpectedError):
            self.test_language_set.is_contained(-1)

        # 引数が言語タグの場合
        with pytest.raises(UnexpectedError):
            self.test_language_set.is_contained(
                LanguageTag())

    def test_round(self):
        # 正常系のテスト
        self.test_language_set = LanguageSet('en-us+ja-jp+zh-cn')

        no_country = self.test_language_set.round()
        assert no_country == LanguageSet('en+ja+zh')

    def test_read_object(self):
        # 正常系のテスト
        pass

    def test_read_object_error(self):
        # 異常系のテスト
        pass

    def test_write_object(self):
        # 正常系のテスト
        pass
