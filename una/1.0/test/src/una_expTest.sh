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
TEST=expTest
DAT_DIR=../src/dat.una/$TEST
ENCODING="-c euc"

# TARGET N
# UnaAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記展開ありで、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# UnaAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記展開あり・包含関係候補削除で、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

FILES="etc kata alpha"

for FILE in $FILES
do
	DAT_FILE=$DAT_DIR/$FILE
	DIC_DIR=../unadic/una
	NDIC_DIR=../unadic-n

	ANS_FILE=../src/ans.una/$TEST/$FILE.sep.e
	OUT_FILE=una_$TEST.$FILE.sep.e
	${prefix}unanorm $ENCODING -e -l -r1 $DIC_DIR -r2 $NDIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

	ANS_FILE=../src/ans.una/$TEST/$FILE.sep.eC
	OUT_FILE=una_$TEST.$FILE.sep.eC
	${prefix}unanorm $ENCODING -e -C -l -r1 $DIC_DIR -r2 $NDIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
done

# TARGET N
# NlpAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記正規化あり・異表記展開ありで、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記正規化なし・異表記展開ありで、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記正規化あり・異表記展開あり・包含関係候補削除で、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記正規化なし・異表記展開あり・包含関係候補削除で、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

for FILE in $FILES
do
	DAT_FILE=$DAT_DIR/$FILE
	DIC_DIR=../unadic
	ANS_FILE=../src/ans.una/$TEST/$FILE.e
	OUT_FILE=una_$TEST.$FILE.e

	${prefix}nlpnorm $ENCODING -e -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23
	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

	OUT_FILE=una_$TEST.$FILE.e.x

	${prefix}nlpnorm $ENCODING -e -x -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

	ANS_FILE=../src/ans.una/$TEST/$FILE.eC
	OUT_FILE=una_$TEST.$FILE.eC

	${prefix}nlpnorm $ENCODING -e -C -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

	OUT_FILE=una_$TEST.$FILE.eC.x

	${prefix}nlpnorm $ENCODING -e -C -x -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
done

# TARGET N
# UnaAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記展開あり、ステミング有(1,3,5,7)、下位構造展開有(1,2,5,6)、
# 改行無視(4,5,6,7)をそれぞれ指定し、解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# UnaAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記展開あり・包含関係候補削除で、ステミング有(1,3,5,7)、
# 下位構造展開有(1,2,5,6)、改行無視(4,5,6,7)をそれぞれ指定し、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# UnaAnalyzerによる異表記展開の動作
#
# PROCESS
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# UnaAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記展開あり・包含関係候補削除で、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

FILE=exp
DAT_FILE=$DAT_DIR/$FILE
DIC_DIR=../unadic/una
NDIC_DIR=../unadic-n

ANS_FILE=../src/ans.una/$TEST/$FILE.sep.e
OUT_FILE=una_$TEST.$FILE.sep.e

${prefix}unanorm $ENCODING -e -r1 $DIC_DIR -r2 $NDIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

ANS_FILE=../src/ans.una/$TEST/$FILE.sep.eC
OUT_FILE=una_$TEST.$FILE.sep.eC

${prefix}unanorm $ENCODING -e -C -r1 $DIC_DIR -r2 $NDIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

# TARGET N
# NlpAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記正規化あり・異表記展開ありで、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記正規化なし・異表記展開ありで、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記正規化あり・異表記展開あり・包含関係候補削除で、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによる異表記展開の動作
#
# PROCESS
# 異表記正規化なし・異表記展開あり・包含関係候補削除で、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

DIC_DIR=../unadic

ANS_FILE=../src/ans.una/$TEST/$FILE.e
OUT_FILE=una_$TEST.$FILE.e

${prefix}nlpnorm $ENCODING -e -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

OUT_FILE=una_$TEST.$FILE.e.x

${prefix}nlpnorm $ENCODING -e -x -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

ANS_FILE=../src/ans.una/$TEST/$FILE.eC
OUT_FILE=una_$TEST.$FILE.eC

${prefix}nlpnorm $ENCODING -e -C -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

OUT_FILE=una_$TEST.$FILE.eC.x

${prefix}nlpnorm $ENCODING -e -x -C -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
