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
TEST=originTest
DAT_DIR=../src/dat.una/$TEST
ENCODING="-c euc"
FILE=test
DAT_FILE=$DAT_DIR/$FILE

# TARGET N
# NlpAnalyzerによる原表記取得・ベクター型結果取得の動作
#
# PROCESS
# 異表記正規化あり・ベクター型結果取得ありで、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによる原表記取得・ベクター型結果取得の動作
#
# PROCESS
# 異表記正規化なし・ベクター型結果取得ありで、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

DIC_DIR=../unadic

ANS_FILE=../src/ans.una/$TEST/$FILE.blockV
OUT_FILE=una_$TEST.$FILE.blockV

${prefix}nlpblock $ENCODING -v -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

ANS_FILE=../src/ans.una/$TEST/$FILE.blockV.x
OUT_FILE=una_$TEST.$FILE.blockV.x

${prefix}nlpblock $ENCODING -v -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -x

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

# TARGET N
# NlpAnalyzerによる原表記取得・ベクター型結果取得の動作
#
# PROCESS
# 異表記正規化あり・原表記取得あり・ベクター型結果取得ありで、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによる原表記取得・ベクター型結果取得の動作
#
# PROCESS
# 異表記正規化なし・原表記取得あり・ベクター型結果取得ありaで、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

DIC_DIR=../unadic

ANS_FILE=../src/ans.una/$TEST/$FILE.block.O
OUT_FILE=una_$TEST.$FILE.block.O

${prefix}nlpblock $ENCODING -O -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

ANS_FILE=../src/ans.una/$TEST/$FILE.block.O.x
OUT_FILE=una_$TEST.$FILE.block.O.x

${prefix}nlpblock $ENCODING -O -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -x

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

# TARGET N
# NlpAnalyzerによる品詞名取得・ベクター型結果取得の動作
#
# PROCESS
# 異表記正規化あり/なしで、ステミングあり、
# 下位構造展開有、改行無視をそれぞれ指定し、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

DIC_DIR=../unadic

ANS_FILE=../src/ans.una/$TEST/$FILE.block.H
OUT_FILE=una_$TEST.$FILE.block.H

${prefix}nlpblock $ENCODING -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -H

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

ANS_FILE=../src/ans.una/$TEST/$FILE.block.Hx
OUT_FILE=una_$TEST.$FILE.block.Hx

${prefix}nlpblock $ENCODING -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -H -x

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
