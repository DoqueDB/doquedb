# 
# Copyright (c) 2017, 2023 Ricoh Company, Ltd.
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
#### UNAによる複数辞書機能のテスト #####

prefix="./"
TEST=errMultiTest
DAT_DIR=../src/dat.una/$TEST

##### FILE = multidic #####

# TARGET N
# 複数辞書が優先度に従って参照されることを確認する
# 無効語が正しく処理されることを確認する
#
# PROCESS
# 解析結果と正解ファイルとを比較する
# 
# EFFECT
# 解析結果が正解ファイルと一致する

FILES="multidic"

for FILE in $FILES
do
	DAT_FILE=$DAT_DIR/$FILE

	for DICLIST in noprio notnumber prio0 prio256 badorder nobase nocomma nodics toomany nowrd noapp2
	do
		DICNLP_DIR=../multidic
		OUT_FILE=una_$TEST.$FILE.$DICLIST
		cp $DICNLP_DIR/una/diclist-$DICLIST.dat $DICNLP_DIR/una/diclist.dat

		${prefix}nlpblock -c euc -l -x -p -B -s : -r $DICNLP_DIR -i $DAT_FILE -o $OUT_FILE
		[ -e $OUT_FILE -a ! -s $OUT_FILE ] && echo "$DICLIST OK" && rm $OUT_FILE || \
		(echo $FILE " FAILED:"; ls -l $OUT_FILE; echo);

		cp $DICNLP_DIR/una/diclist-original.dat $DICNLP_DIR/una/diclist.dat
	done
done
