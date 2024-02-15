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
prefix="./"
TEST=keywordTest
DAT_DIR=../src/dat.una/$TEST
ENCODING="-c euc"

# TARGET N
# NlpAnalyzerによるキーワード抽出の動作
#
# PROCESS
# 異表記正規化あり・キーワード抽出ありで、
# ステミング有(1,3,5,7)、下位構造展開有(1,2,5,6)、改行無視(4,5,6,7)を
# それぞれ指定し、解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによるキーワード抽出の動作
#
# PROCESS
# 異表記正規化あり・キーワード抽出あり・省メモリ指定ありで、
# ステミング有(1,3,5,7)、下位構造展開有(1,2,5,6)、改行無視(4,5,6,7)を
# それぞれ指定し、解析結果と正解ファイルを比較する。
#
# EFFECT
# 単語取得関数(getWord系列)は、省メモリ指定によって結果は変化しない
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによるキーワード抽出の動作
#
# PROCESS
# 異表記正規化なし・キーワード抽出ありで、
# ステミング有(1,3,5,7)、下位構造展開有(1,2,5,6)、改行無視(4,5,6,7)を
# それぞれ指定し、解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。
# キーワード抽出用の関数は指定によらず異表記正規化ありと解釈される

# TARGET N
# NlpAnalyzerによるキーワード抽出の動作
#
# PROCESS
# 異表記正規化なし・キーワード抽出あり、省メモリ指定ありで
# ステミング有(1,3,5,7)、下位構造展開有(1,2,5,6)、改行無視(4,5,6,7)を
# それぞれ指定し、解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。
# 単語取得関数(getWord系列)は、省メモリ指定によって結果は変化しない
# キーワード抽出用の関数は指定によらず異表記正規化ありと解釈される

FILE=test
DAT_FILE=$DAT_DIR/$FILE

DIC_DIR=../unadic

for Y in 0 1 2 3 4 5 6 7
do
	ANS_FILE=../src/ans.una/$TEST/$FILE.m$Y.k
	OUT_FILE=una_$TEST.$FILE.m$Y.k

	${prefix}nlpnorm $ENCODING -k -m $Y -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE 

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

	OUT_FILE=una_$TEST.$FILE.m$Y.k.mr

	${prefix}nlpnorm $ENCODING -k -m $Y -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE  -MR

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

	OUT_FILE=una_$TEST.$FILE.m$Y.k.x

	${prefix}nlpnorm $ENCODING -k -x -m $Y -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

	OUT_FILE=una_$TEST.$FILE.m$Y.k.x.mr

	${prefix}nlpnorm $ENCODING -k -x -m $Y -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -MR

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
done
