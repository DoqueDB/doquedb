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
#### UNAによる基本機能のテスト #####
#### 追加のテスト ####

prefix="./"
TEST=basicTest
DAT_DIR=../src/dat.una/$TEST

# TARGET N
# NlpAnalyzerによる動作モードごとの動作
#
# PROCESS
# 形態素解析、異表記正規化あり時の言語指定を日本語のみにした場合、
# データはステミングされることを検証する。
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# NlpAnalyzerによる動作モードごとの動作
#
# PROCESS
# 形態素解析、異表記正規化なし時の言語指定を日本語のみにした場合、
# データはステミングされないことを検証する。
# 解析結果と正解ファイルを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

FILES="test"

for FILE in $FILES
do
	DAT_FILE=$DAT_DIR/$FILE
	DICNLP_DIR=../unadic
	ANS_FILE=../src/ans.una/$TEST/$FILE
	OUT_FILE=una_$TEST.$FILE

	${prefix}nlpnorm -c euc -L ja -l -r $DICNLP_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

	ANS_FILE=../src/ans.una/$TEST/$FILE.x
	OUT_FILE=una_$TEST.$FILE.x
	
	${prefix}nlpnorm -c euc -x -L ja -l -r $DICNLP_DIR -i $DAT_FILE -o $OUT_FILE -S1 0x23

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
done
