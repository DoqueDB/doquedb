# 
# Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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
##### FILE = alpha #####

# TARGET N
# ASCII 英字列を対象とする英単語正規化
#
# PROCESS
# ASCII 英字列(異表記正規化済み)を対象とする辞書引き結果と、正解ファイ
# ルとを比較する。
#
# EFFECT
# 辞書引き結果が正解ファイルと一致する。

# TARGET N
# ASCII 英字列を対象とする英単語正規化
#
# PROCESS
# ASCII 英字列(異表記正規化済み)を対象とする正規化結果と、正解ファイル
# とを比較する。
#
# EFFECT
# 正規化結果が正解ファイルと一致する。

# TARGET N
# ASCII 英字列を対象とする英単語正規化
#
# PROCESS
# ASCII 英字列(異表記正規化済み)を対象とする展開結果と、正解ファイルと
# を比較する。
#
# EFFECT
# 展開結果が正解ファイルと一致する。

##### FILE = cyrillic #####

# TARGET N
# キリル文字列を対象とする英単語正規化
#
# PROCESS
# キリル文字列(異表記正規化済み)を対象とする辞書引き結果と、正解ファイ
# ルとを比較する。
#
# EFFECT
# 辞書引き結果が正解ファイルと一致する。

# TARGET N
# キリル文字列を対象とする英単語正規化
#
# PROCESS
# キリル文字列(異表記正規化済み)を対象とする正規化結果と、正解ファイル
# とを比較する。
#
# EFFECT
# 正規化結果が正解ファイルと一致する。

# TARGET N
# キリル文字列を対象とする英単語正規化
#
# PROCESS
# キリル文字列(異表記正規化済み)を対象とする展開結果と、正解ファイルと
# を比較する。
#
# EFFECT
# 展開結果が正解ファイルと一致する。

##### FILE = etc #####

# TARGET N
# 対象外文字列を対象とする英単語正規化
#
# PROCESS
# 英単語正規化の対象外文字を含む文字列(異表記正規化済み)を対象とする辞
# 書引き結果と、正解ファイルとを比較する。
#
# EFFECT
# 辞書引き結果が正解ファイルと一致する。

# TARGET N
# 対象外文字列を対象とする英単語正規化
#
# PROCESS
# 英単語正規化の対象外文字を含む文字列(異表記正規化済み)を対象とする正
# 規化書引き結果と、正解ファイルとを比較する。
#
# EFFECT
# 正規化結果が正解ファイルと一致する。

# TARGET N
# 対象外文字列を対象とする英単語正規化
#
# PROCESS
# 英単語正規化の対象外文字を含む文字列(異表記正規化済み)を対象とする展
# 開規化書引き結果と、正解ファイルとを比較する。
#
# EFFECT
# 展開結果が正解ファイルと一致する。

prefix="./"
TEST=eucTest
DAT_DIR=../src/dat.stem/$TEST
ENCODING="-c euc"

FILES="alpha cyrillic etc bbed"

for FILE in $FILES
do
	DAT_FILE=$DAT_DIR/$FILE
	DIC_DIR=../unadic-n

	for ACT in look stem exp
	do
		ANS_DIR=../src/ans.stem/$TEST
		ANS_FILE=$ANS_DIR/$FILE.$ACT
		OUT_FILE=stem_$TEST.$FILE.$ACT

		${prefix}stem -r $DIC_DIR $ENCODING -a $ACT -n -i $DAT_FILE -o $OUT_FILE -S 0x2c;

		cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
		(echo $ACT" FAILED: stem_"$TEST; ls -l $ANS_FILE $OUT_FILE; echo);
	done
done
