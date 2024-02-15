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
TEST=kanaTest
DAT_DIR=../src/dat.una/$TEST
ANS_DIR=../src/ans.una/$TEST

# TARGET N
# 仮名合字パターンの形態素解析
#
# PROCESS
# ひらがな/カタカナと濁点/半濁点の連続を対象とする形態素解析結果と
# 正解ファイルとを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

# TARGET N
# 長音処理パターンの形態素解析
#
# PROCESS
# ひらがな/カタカナと長音(または類似記号)の連続を対象とする形態素解析
# 結果と正解ファイルとを比較する。
#
# EFFECT
# 解析結果が正解ファイルと一致する。

FILES="chouon_hira chouon_khan chouon_kzen combi_chouon_hira combi_chouon_khan combi_chouon_kzen combi_hira combi_khan combi_kzen"

for FILE in $FILES
do
	DIC_DIR=../unadic/una
	DAT_FILE=$DAT_DIR/$FILE
	ANS_FILE=$ANS_DIR/$FILE
	OUT_FILE=una_$TEST.$FILE

	${prefix}una -c utf8 -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -S 0x23

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE" FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
done

# TARGET N
# 未登録語記号によるカタカナ文字列解析結果への影響
#
# PROCESS
# カタカナ文字列を対象とする形態素解析結果と、各文字列の先頭に
# 空白を付加した場合のカタカナ部分の形態素解析結果とを比較する。
#
# EFFECT
# 両者の解析結果が同一である。

DIC_DIR=../unadic/una
FILE="kata"
DAT_FILE=$DAT_DIR/$FILE
OUT_FILE1=una_$TEST.$FILE-1
OUT_FILE2=una_$TEST.$FILE-2
WORK_FILE1=una_$TEST.$FILE-W1
WORK_FILE2=una_$TEST.$FILE-W2

cp -f $DAT_FILE $WORK_FILE1
perl -pi.bak -e 's/^//' $WORK_FILE1
${prefix}una -c euc -r $DIC_DIR -S 0x23 -i $WORK_FILE1 -o $OUT_FILE1
perl -pi.bak -e 's/^//' $OUT_FILE1

cp -f $DAT_FILE $WORK_FILE2
perl -pi.bak -e 's/^/ /' $WORK_FILE2
${prefix}una -c euc -r $DIC_DIR -S 0x23 -i $WORK_FILE2 -o $OUT_FILE2
perl -pi.bak -e 's/^# //' $OUT_FILE2

cmp -s $OUT_FILE1 $OUT_FILE2 && \
rm -f $OUT_FILE1 $OUT_FILE2 $WORK_FILE1 $WORK_FILE2 \
$OUT_FILE1.bak $OUT_FILE2.bak $WORK_FILE1.bak $WORK_FILE2.bak || \
(echo $FILE" FAILED:"; ls -l $OUT_FILE1 $OUT_FILE2; echo);
