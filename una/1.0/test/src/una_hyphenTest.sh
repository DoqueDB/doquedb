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
TEST=hyphenTest
DAT_DIR=../src/dat.una/$TEST
ANS_DIR=../src/ans.una/$TEST

# TARGET N
# 行末ハイフン処理
#
# PROCESS
# 任意に作成した行末ハイフンパターンを対象として、形態素解析結果と
# 正解ファイルとを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# 
# PROCESS
# 実データから収集した行末ハイフンパターンを対象として、形態素解析結果と
# 正解ファイルとを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
#
# PROCESS
# 英語辞書見出しのハイフネーションを対象として、形態素解析結果と
# 正解ファイルとを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

FILES="test trec eow"

for FILE in $FILES
do
	DIC_DIR=../unadic/una
	DAT_FILE=$DAT_DIR/$FILE

	for M in 0 1
	do
		ANS_FILE=$ANS_DIR/$FILE.m$M
		OUT_FILE=una_$TEST.$FILE.m$M

		${prefix}unacore -c euc -m $M -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S 0x23

		cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
		(echo $FILE.$M" FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
	done
done
