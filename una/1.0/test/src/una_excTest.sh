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
TEST=excTest
DAT_DIR=../src/dat.una/$TEST
ANS_DIR=../src/ans.una/$TEST

# TARGET N
# 音標符号付き文字を含むテキストの形態素解析
#
# PROCESS
# 任意に作成した音標符号付き文字を含む文字列を対象として、
# 形態素解析結果と正解ファイルとを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
#
# PROCESS
# 実データから収集した音標符号付き文字を含む文字列を対象として、
# 形態素解析結果と正解ファイルとを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

FILES="test uni"

for FILE in $FILES
do
	DAT_FILE=$DAT_DIR/$FILE

	DIC_DIR=../unadic/una
	ANS_FILE=$ANS_DIR/$FILE
	OUT_FILE=una_$TEST.$FILE
	
	${prefix}una -c utf8 -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S 0x23

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE" FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
done
