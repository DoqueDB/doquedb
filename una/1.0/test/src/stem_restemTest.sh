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
##### FILE = alpha #####

# TARGET N
# ASCII 英字列を対象とする正規化語形の再正規化
#
# PROCESS
# ASCII 英字列(異表記正規化済み)を対象とする正規化結果を再度正規化し、
# 両者を比較する。
#
# EFFECT
# 両者の正規化結果が同一である。

##### FILE = cyrillic #####

# TARGET N
# キリル文字列(異表記正規化済み)対象とする正規化語形の再正規化
#
# PROCESS
# キリル文字列(異表記正規化済み)を対象とする正規化結果を再度正規化し、
# 両者を比較する。
#
# EFFECT
# 両者の正規化結果が同一である。

prefix="./"
TEST=restemTest
DAT_DIR=../src/dat.stem/eucTest
ENCODING="-c euc"

FILES="alpha cyrillic"

for FILE in $FILES
do
	DAT_FILE=$DAT_DIR/$FILE
	DIC_DIR=../unadic-n
	ANS_DIR=../src/ans.stem/eucTest
	ANS_FILE=$ANS_DIR/$FILE.stem
	OUT_FILE=stem_$TEST.$FILE

	${prefix}stem -r $DIC_DIR $ENCODING -a s -i $ANS_FILE -o $OUT_FILE;

	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $ACT" FAILED: stem_"$TEST; ls -l $ANS_FILE $OUT_FILE; echo);
done
