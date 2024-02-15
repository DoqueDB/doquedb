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
TEST=threadTest
DAT_DIR=../src/dat.una/$TEST

# TARGET N
# NlpAnalyzerによるマルチスレッドの動作
#
# PROCESS
# 異表記正規化ありで解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによるマルチスレッドの動作
#
# PROCESS
# 異表記正規化なしで解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

FILE=test
DAT_FILE=$DAT_DIR/$FILE

DIC_DIR=../unadic

ANS_FILE=../src/ans.una/$TEST/$FILE
OUT_FILE=una_$TEST.$FILE

${prefix}nlpthread -c euc -l -L fr/ja+en -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

ANS_FILE=../src/ans.una/$TEST/$FILE.x
OUT_FILE=una_$TEST.$FILE.x

${prefix}nlpthread -c euc -x -l -L fr/ja+en -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

# TARGET N
# NlpAnalyzerによるマルチスレッドの動作
#
# PROCESS
# 異表記正規化あり・異表記展開ありで、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによるマルチスレッドの動作
#
# PROCESS
# 異表記正規化なし・異表記展開ありで、
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

FILE=etc
DAT_FILE=$DAT_DIR/$FILE

DIC_DIR=../unadic

ANS_FILE=../src/ans.una/$TEST/$FILE.e
OUT_FILE=una_$TEST.$FILE.e

${prefix}nlpthread -c euc -e -l -L fr/ja+en -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

OUT_FILE=una_$TEST.$FILE.e.x

${prefix}nlpthread -c euc -e -x -l -L fr/ja+en -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
