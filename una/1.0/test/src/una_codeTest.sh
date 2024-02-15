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
#### UNAのカバレッジテスト #####
#
# 以下のテストにおいては、正常に終了する(assertで落ちたりしない)ことが
# テストの終了条件であり、解析結果を比較する機能テストとは異なる。
# 種々のテストを行っているのは、カバレッジを確保するため
#

prefix="./"
TEST=codeTest

# TARGET N
# ModEnglishWordStemmer::ModEnglishWordStemmer(path, filename)
# 
# PROCESS
# 実行データの作成
#
# EFFECT
# 正常に処理終了する

DAT_DIR=../src/dat.stem/$TEST
PATH_DIR=$DAT_DIR/path
OUT_FILE=stemmer.dat.test

ANS_FILE=../src/ans.stem/codeTest/winstemmer.dat

${prefix}stem -c euc -M $PATH_DIR/path.txt2 $OUT_FILE

cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
(echo $FILE" FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

###   stem end   ###

# TARGET N
# ModEnglishWordStemmerDataPath::ModEnglishWordStemmerDataPath( ModCharString )
# ModEnglishWordStemmer::ModEnglishWordStemmer( ModEnglishWordStemmerDataPath, ModCharString )
# ModEnglishWordStemmer::ModEnglishWordStemmer( ModCharString )
# ModUnaMiddle::initialize( ModCharString )
# ModUnaMiddle::create()
# ModUnaMiddle::terminate()
#
# PROCESS
# 呼ばれていない関数のテスト
#
# EFFECT
# 正常に処理終了する

ANS_FILE1=../src/ans.una/codeTest/notcallfunc.ok
OUT_FILE1=notcallfunc.result
OUT_FILE2=stem.test
${prefix}notcallfunc

cmp -s $ANS_FILE1 $OUT_FILE1 && rm $OUT_FILE1 || \
(echo $FILE" FAILED:"; ls -l $ANS_FILE1 $OUT_FILE1; echo);

cmp -s $ANS_FILE $OUT_FILE2 && rm $OUT_FILE2 || \
(echo $FILE" FAILED:"; ls -l $ANS_FILE $OUT_FILE2; echo);
