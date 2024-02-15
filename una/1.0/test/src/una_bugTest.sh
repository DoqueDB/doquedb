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
TEST=bugTest
DAT_DIR=../src/dat.una/$TEST

# TARGET N
# NlpAnalyzerによるバイナリファイルの解析
#
# PROCESS
# バイナリファイルを入力ファイルとした場合の動作を確認する。
#
# EFFECT
# 処理が途中で落ちない。

FILE=binary
DAT_FILE=$DAT_DIR/$FILE
OUT_FILE=una_$TEST.$FILE
DIC_DIR="../unadic"

${prefix}nlpnorm -c euc -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE
RETURN=$?
case $RETURN in
0|1)
	;;
*)
	echo FAILED;
	;;
esac
rm -f $OUT_FILE
