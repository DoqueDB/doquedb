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
test_data.py -- src.doquedb.common.data モジュールのテスト
"""
from src.doquedb.common.data import BinaryData, WordData
from src.doquedb.common.constants import ClassID
from src.doquedb.common.language import LanguageSet


class TestBinaryData:
    def setup_method(self, method):
        # 前処理
        self.test_binary_data = BinaryData()

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_binary_data.class_id == ClassID.BINARY_DATA

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_binary_data.value == bytes(0)

        # 引数がbytes型の場合
        self.test_binary_data = BinaryData(bytes(1))
        assert self.test_binary_data.value == bytes(1)

        # 引数がBinary Data型の場合
        self.test_binary_data = BinaryData(BinaryData(bytes(100)))
        assert self.test_binary_data.value == bytes(100)

    def test_eq(self):
        # 正常系のテスト
        # 一致する場合
        assert self.test_binary_data == BinaryData(bytes(0))
        # 型が違う場合
        assert self.test_binary_data != bytes(0)
        # 値が違う場合
        assert self.test_binary_data != BinaryData(bytes(200))

    def test_str(self):
        # 正常系のテスト
        assert str(self.test_binary_data) == 'size=0'

    def test_clone(self):
        # 正常系のテスト
        clone = self.test_binary_data.clone()
        assert self.test_binary_data == clone
        # アドレスは別
        assert id(self.test_binary_data) != id(clone)

    def test_read_object(self):
        # 正常系のテスト
        pass

    def test_write_object(self):
        # 正常系のテスト
        pass


class TestWordData:
    def setup_method(self, method):
        # 前処理
        self.test_word_data = WordData()

    def test_class_id(self):
        # 正常系のテスト
        assert self.test_word_data.class_id == ClassID.WORD_DATA

    def test_init(self):
        # 正常系のテスト
        # 引数がない場合
        assert self.test_word_data.language == LanguageSet()
        assert self.test_word_data.term == ''
        assert self.test_word_data.category == WordData.CATEGORY_UNDEFINED
        assert self.test_word_data.scale == 0.0
        assert self.test_word_data.docfrequency == 0

        # 引数がstr型の場合
        self.test_word_data = WordData('test_str')
        assert self.test_word_data.language == LanguageSet()
        assert self.test_word_data.term == 'test_str'
        assert self.test_word_data.category == WordData.CATEGORY_UNDEFINED
        assert self.test_word_data.scale == 0.0
        assert self.test_word_data.docfrequency == 0

        # 引数がWordData型の場合
        # 引数の準備
        word_data = WordData('test_word_data')
        word_data.language = LanguageSet('en')
        word_data.term = 'test_word_data'
        word_data.category = WordData.CATEGORY_HELPFUL_RELATED
        word_data.scale = 2
        word_data.docfrequency = 1

        self.test_word_data = WordData(word_data)
        assert self.test_word_data.language == LanguageSet('en')
        assert self.test_word_data.term == 'test_word_data'
        assert self.test_word_data.category == WordData.CATEGORY_HELPFUL_RELATED
        assert self.test_word_data.scale == 2
        assert self.test_word_data.docfrequency == 1

    def test_eq(self):
        # 正常系のテスト
        other = WordData()

        assert self.test_word_data == other

    def test_str(self):
        # 正常系のテスト
        # CATEGORY_UNDEFINEDの場合
        assert str(self.test_word_data) == ' scale 0.0'

        # その他のCATEGORYの場合
        # 準備
        self.test_word_data.term = 'test_str'
        self.test_word_data.language = LanguageSet('ja')
        self.test_word_data.category = WordData.CATEGORY_IMPORTANT
        self.test_word_data.scale = 0.5
        self.test_word_data.docfrequency = 2
        assert str(self.test_word_data)\
            == 'test_str language ja category Important scale 0.5 df 2'

    def test_hash(self):
        # 正常系のテスト
        self.test_word_data.term = 'hash'
        self.test_word_data.language = LanguageSet('zh')
        hashcode = hash(LanguageSet('zh')) + self.test_word_data.category << 4\
            | hash('hash')
        assert hash(self.test_word_data) == hashcode

    def test_clone(self):
        # 正常系のテスト
        clone = self.test_word_data.clone()
        assert self.test_word_data == clone
        # アドレスは別
        assert id(self.test_word_data) != id(clone)

    def test_read_object(self):
        # 正常系のテスト
        pass

    def test_write_object(self):
        # 正常系のテスト
        pass
