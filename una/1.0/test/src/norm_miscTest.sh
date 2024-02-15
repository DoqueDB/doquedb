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
TEST=miscTest
DAT_DIR=../src/dat.norm/$TEST

# TARGET N
# 省略モード展開
#
# PROCESS
# 入力文字列に対する省略モードの展開結果と、正解ファイルとを比較する。
#
# EFFECT
# 展開結果が正解ファイルと一致する。

FILE=expchk
DAT_FILE=$DAT_DIR/$FILE
ENCODING="-c euc"
DIC_DIR=../unadic/norm
ANS_DIR=../src/ans.norm/$TEST
OUT_FILE=norm_$TEST.$FILE.chk
ANS_FILE=$ANS_DIR/$FILE.chk

${prefix}norm -l -r $DIC_DIR $ENCODING -i $DAT_FILE -o $OUT_FILE -a C;

# 展開結果と正解ファイルを比較
cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
(echo "expchk FAILED: norm_"$TEST; ls -l $ANS_FILE $OUT_FILE; echo);

# TARGET N
# 展開のみの実施
#
# PROCESS
# 入力文字列(原表記)に対する通常の展開結果(正規化も実施)と、入力文字列
# の正規化表記に対して展開のみを行なった結果とを比較する。
#
# EFFECT
# 両結果が一致する。

FILE=pat16mor
DAT_FILE=$DAT_DIR/$FILE
DIC_DIR=../unadic/norm
OUT_NORM=norm_$TEST.$FILE.norm
OUT_EXP=norm_$TEST.$FILE.exp
OUT_ONLY=norm_$TEST.$FILE.only

${prefix}norm -l -r $DIC_DIR $ENCODING -i $DAT_FILE -o $OUT_NORM -a n;
${prefix}norm -l -r $DIC_DIR $ENCODING -i $DAT_FILE -o $OUT_EXP -a e;
${prefix}norm -l -r $DIC_DIR $ENCODING -i $OUT_NORM -o $OUT_ONLY -a E;

# 展開結果と正解ファイルを比較
cmp -s $OUT_EXP $OUT_ONLY && rm $OUT_NORM $OUT_EXP $OUT_ONLY || \
(echo "exponly FAILED: norm_"$TEST; ls -l $OUT_EXP $OUT_ONLY; echo);

# TARGET N
# 正規化範囲指定
#
# PROCESS
# 入力文字列を範囲指定して正規化した結果(前処理形式出力)と、
# 正解ファイルとを比較する。
#
# EFFECT
# 正規化結果が正解ファイルと一致する。

# TARGET N
# 正規化範囲指定
#
# PROCESS
# 入力文字列を範囲指定して正規化した結果(正規化表記出力)と、
# 正解ファイルとを比較する。
#
# EFFECT
# 正規化結果が正解ファイルと一致する。

# TARGET N
# 正規化範囲指定
#
# PROCESS
# 入力文字列を範囲指定して展開した結果と、正解ファイルとを比較する。
#
# EFFECT
# 展開結果が正解ファイルと一致する。

FILE=normsub
DAT_FILE=$DAT_DIR/$FILE

DIC_DIR=../unadic/norm
ANS_DIR=../src/ans.norm/$TEST
OUT_BOTH=norm_$TEST.$FILE.both
OUT_NORM=norm_$TEST.$FILE.norm
OUT_EXP=norm_$TEST.$FILE.exp
ANS_BOTH=$ANS_DIR/$FILE.both
ANS_NORM=$ANS_DIR/$FILE.norm
ANS_EXP=$ANS_DIR/$FILE.exp

${prefix}norm -l -b 4 -e 12 -r $DIC_DIR $ENCODING -i $DAT_FILE -o $OUT_BOTH -a b;
${prefix}norm -l -b 4 -e 12 -r $DIC_DIR $ENCODING -i $DAT_FILE -o $OUT_NORM -a n;
${prefix}norm -l -b 4 -e 12 -r $DIC_DIR $ENCODING -i $DAT_FILE -o $OUT_EXP  -a e;

# 前処理結果と正解ファイルを比較
cmp -s $ANS_BOTH $OUT_BOTH && rm $OUT_BOTH || \
(echo "both-sub FAILED: norm_"$TEST; ls -l $ANS_BOTH $OUT_BOTH; echo);

# 正規化結果と正解ファイルを比較
cmp -s $ANS_NORM $OUT_NORM && rm $OUT_NORM || \
(echo "norm-sub FAILED: norm_"$TEST; ls -l $ANS_NORM $OUT_NORM; echo);

# 展開結果と正解ファイルを比較
cmp -s $ANS_EXP $OUT_EXP && rm $OUT_EXP || \
(echo "exp-sub FAILED: norm_"$TEST; ls -l $ANS_EXP $OUT_EXP; echo);

# TARGET N
# 分割長指定
#
# PROCESS
# 入力文字列に対して、分割長を指定して正規化した結果(前処理形式出力)と、
# 分割長を指定しないで正規化した結果(前処理形式出力)とを比較する。
#
# EFFECT
# 両者の結果が一致する。

# TARGET N
# 分割長指定
#
# PROCESS
# 入力文字列に対して、分割長を指定して正規化した結果(正規化表記出力)と、
# 分割長を指定しないで正規化した結果(正規化表記出力)とを比較する。
#
# EFFECT
# 両者の結果が一致する。

# TARGET N
# 分割長指定
#
# PROCESS
# 入力文字列に対象して、分割長を指定して展開した結果と、分割長を指定し
# ないで展開した結果とを比較する。
#
# EFFECT
# 両者の結果が一致する。

FILE=maxlen
DAT_FILE=$DAT_DIR/$FILE

DIC_DIR=../unadic/norm
ANS_DIR=../src/ans.norm/$TEST
OUT_BOTH=norm_$TEST.$FILE.both
OUT_NORM=norm_$TEST.$FILE.norm
OUT_BOTHL=norm_$TEST.$FILE.bothlen
OUT_NORML=norm_$TEST.$FILE.normlen

${prefix}norm -l -r $DIC_DIR $ENCODING -i $DAT_FILE -o $OUT_BOTH -a b;
${prefix}norm -l -r $DIC_DIR $ENCODING -i $DAT_FILE -o $OUT_NORM -a n;

${prefix}norm -l -r $DIC_DIR $ENCODING -i $DAT_FILE -o $OUT_BOTHL -a b -L 4;
${prefix}norm -l -r $DIC_DIR $ENCODING -i $DAT_FILE -o $OUT_NORML -a n -L 4;

# 前処理結果と正解ファイルを比較
cmp -s $OUT_BOTH $OUT_BOTHL && rm $OUT_BOTH $OUT_BOTHL || \
(echo "both-len FAILED: norm_"$TEST; ls -l $OUT_BOTH $OUT_BOTHL; echo);

# 正規化結果と正解ファイルを比較
cmp -s $OUT_NORM $OUT_NORML && rm $OUT_NORM $OUT_NORML || \
(echo "norm-len FAILED: norm_"$TEST; ls -l $OUT_NORM $OUT_NORML; echo);
