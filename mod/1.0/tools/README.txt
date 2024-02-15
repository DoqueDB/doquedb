#
# Copyright (c) 2023 Ricoh Company, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

Unicode のツールについて

このツールは ModUnicodeCharTrait などが使用するテーブルファイルを作成する
プログラムである。

（必要なファイル）
少なくとも UnicodeData.txt(UnicodeData-1.1.5.txt, UnicodeData-2.1.9 など)
が必要である。必須ではないが Blocks.txt (Blocks-2.txtなど) もあるとうれしい。

最新の物は以下のサイトで手に入るだろう。

ftp://ftp.unicode.org/Public/

（コンパイル）
c.O ディレクトリなどで make を実行するとプログラム「maketable」が作成される。

（実行方法）

<UnicodeData.txt として ../src/UnicodeData-1.1.5.txt を使い、
 Blocks.txt としてデフォルト値(1.x.x でのブロックの範囲)を使う場合>

% ./maketable > ModUnicodeCharTrait.tbl

<UnicodeData.txt として UnicodeData-1.1.5.txt を使い、
 Blocks.txt としてデフォルト値(1.x.x でのブロックの範囲)を使う場合>

% ./maketable UnicodeData-1.1.5.txt > ModUnicodeCharTrait.tbl

<UnicodeData.txt として UnicodeData-2.1.9.txt を使い、
 Blocks.txt として Blocks-2.txt を使う場合>

% ./maketable UnicodeData-2.1.9.txt Blocks-2.txt > ModUnicodeCharTrait.tbl

（簡単なクラス説明）
UnicodeDataFile クラス
	セミコロンで区切られたフィールド値を格納したファイル

UnicodeDataRow クラス
	1行に含まれているセミコロンで区切られたフィールド値を
	文字列として取り出すことができる。

UnicodeDataRowCreater クラス
	UnicodeDataRow の派生クラスを選んで生成するクラス

UnicodeDataRowUnicodeData クラス
	UnicodeDataRow の派生クラス
	UnicodeDataRow を "UnicodeData.txt" に特化したもので、
	フィールド値を数値やブール値として取り出すことができる。

UnicodeDataRowBlocks クラス
	UnicodeDataRow の派生クラス
	UnicodeDataRow を "Blocks.txt" に特化したもので、
	フィールド値を数値として取り出すことができる。

