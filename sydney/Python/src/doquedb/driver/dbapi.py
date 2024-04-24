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
dbapi.py -- DB-API 2.0 (PEP-249) の型オブジェクトやコンストラクタの実装モジュール
"""

import datetime
import time

from ..common.scalardata import LanguageData, DecimalData
from ..common.data import WordData

"""Python DB-API 2.0

apilevel (str): DB-APIのバージョン
threadsafety (int): 複数スレッドに対する対応設定
paramstyle (str): 変数埋め込み時のフォーマットスタイル.
"""
apilevel = '2.0'
threadsafety = 0
paramstyle = 'qmark'


"""型オブジェクト

STRING (str): データベースの文字列カラムの型
BINARY (bytes):  データベースのバイナリカラムの型
NUMBER (float)): データベースの数値カラムの型
DATETIME (datetime.datetime): データベースの日付/時間カラムの型
DECIMAL (DecimalData): データベースの"Decimal"カラムの型
LANGUAGE (LanguageData): データベースの言語カラムの型
WORD (WordData): データベースの単語タイプの索引の型
ROWID (int): データベースの"Row ID"カラムの型
"""
STRING = str
BINARY = bytes
NUMBER = float
DATETIME = datetime.datetime
DECIMAL = DecimalData
LANGUAGE = LanguageData
WORD = WordData
ROWID = int


"""コンストラクタ

Binary (BINARY)
Date (datetime.date)
Time (datetime.time)
TimeStamp (datetime.datetime)
Language (LanguageData)
Decimal (DecimalData)
"""
Binary = BINARY
Date = datetime.date
Time = datetime.time
Timestamp = datetime.datetime
Language = LanguageData
Decimal = DecimalData
def DateFromTicks(ticks): return Date(*time.localtime(ticks)[:3])
def TimeFromTicks(ticks): return Time(*time.localtime(ticks)[3:6])
def TimestampFromTicks(ticks): return Timestamp(*time.localtime(ticks)[:6])
