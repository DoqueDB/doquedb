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
prefix="./"
TEST=stdTest

# TARGET N
# UNA前処理における全角/半角標準化
#
# PROCESS
# 全角文字列を対象とする形態素解析結果と半角文字列を対象とする形態素
# 解析結果とを比較する。
#
# EFFECT
# 両者の解析結果が同一である。

DIC_DIR=../unadic/una
DAT_DIR=../src/dat.una/$TEST

FILE1=kata
FILE2=kata_han

DAT_FILE1=$DAT_DIR/$FILE1
DAT_FILE2=$DAT_DIR/$FILE2

OUT_FILE1=una_$TEST.$FILE1
OUT_FILE2=una_$TEST.$FILE2

${prefix}una -c euc -l -r $DIC_DIR -i $DAT_FILE1 -o $OUT_FILE1 -S 0x23
${prefix}una -c euc -l -r $DIC_DIR -i $DAT_FILE2 -o $OUT_FILE2 -S 0x23

cmp -s $OUT_FILE1 $OUT_FILE2 && rm $OUT_FILE1 $OUT_FILE2 || \
	(echo $FILE1"/"$FILE2" FAILED:"; ls -l $OUT_FILE1 $OUT_FILE2; echo);
