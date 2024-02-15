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
TEST=errTest
DAT_FILE1=../src/dat.norm/errTest/err
DAT_FILE2=../src/dat.norm/errTest/pat16

# TARGET E
# 異表記正規化器の初期化エラー
#
# PROCESS
# 存在しない辞書ディレクトリを指定する。
#
# EFFECT
# ModExceptionが発生する。
rm -rf nowhere;
OUT=`${prefix}norm -x -r nowhere`
echo norm_$TEST: 001 $OUT;

# TARGET E
# 正規化範囲指定のエラー
#
# PROCESS
# 正規化開始位置が入力の文字数を超える。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -b 6 -i $DAT_FILE1`
echo norm_$TEST: 002-1 $OUT;

# TARGET E
# 正規化範囲指定のエラー
#
# PROCESS
# 正規化終了位置が入力の文字数を超える。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -e 6 -i $DAT_FILE1`
echo norm_$TEST: 002-2 $OUT;

# TARGET E
# 正規化範囲指定のエラー
#
# PROCESS
# 正規化終了位置が開始位置より前に指定される。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -b 3 -e 1 -i $DAT_FILE1`
echo norm_$TEST: 002-3 $OUT;

# TARGET E
# 展開範囲指定のエラー
#
# PROCESS
# 展開開始位置が入力の文字数を超える。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a e -b 6 -i $DAT_FILE1`
echo norm_$TEST: 003-1 $OUT;

# TARGET E
# 展開範囲指定のエラー
#
# PROCESS
# 展開終了位置が入力の文字数を超える。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a e -e 6 -i $DAT_FILE1`
echo norm_$TEST: 003-2 $OUT;

# TARGET E
# 展開範囲指定のエラー
#
# PROCESS
# 展開終了位置が開始位置より前に指定される。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a e -b 3 -e 1 -i $DAT_FILE1`
echo norm_$TEST: 003-3 $OUT;

# TARGET E
# 正規化時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字0と区切り文字1に同じ文字を指定して正規化する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -D0 0022 -D1 0022 -i $DAT_FILE1`
echo norm_$TEST: 004-1 $OUT;

# TARGET E
# 正規化時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字0と区切り文字2に同じ文字を指定して正規化する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -D0 0020 -D2 0020 -i $DAT_FILE1`
echo norm_$TEST: 004-2 $OUT;

# TARGET E
# 正規化時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字0とエスケープ文字に同じ文字を指定して正規化する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -D0 0020 -D3 0020 -i $DAT_FILE1`
echo norm_$TEST: 004-3 $OUT;

# TARGET E
# 正規化時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字1と区切り文字2に同じ文字を指定して正規化する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -D1 0020 -D2 0020 -i $DAT_FILE1`
echo norm_$TEST: 004-4 $OUT;

# TARGET E
# 正規化時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字1とエスケープ文字に同じ文字を指定して正規化する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -D1 0020 -D3 0020 -i $DAT_FILE1`
echo norm_$TEST: 004-5 $OUT;

# TARGET E
# 正規化時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字2とエスケープ文字に同じ文字を指定して正規化する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -D2 0020 -D3 0020 -i $DAT_FILE1`
echo norm_$TEST: 004-6 $OUT;

# TARGET E
# 正規化時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字0にカタカナブロックの文字（中点）を指定して正規化する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -D0 30FB -i $DAT_FILE1`
echo norm_$TEST: 004-7 $OUT;

# TARGET E
# 正規化時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字1にカタカナブロックの文字（中点）を指定して正規化する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -D1 30FB -i $DAT_FILE1`
echo norm_$TEST: 004-8 $OUT;

# TARGET E
# 正規化時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字2にカタカナブロックの文字（中点）を指定して正規化する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -D2 30FB -i $DAT_FILE1`
echo norm_$TEST: 004-9 $OUT;

# TARGET E
# 正規化時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# エスケープ文字にカタカナブロックの文字（中点）を指定して正規化する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -D3 30FB -i $DAT_FILE1`
echo norm_$TEST: 004-10 $OUT;

# TARGET E
# 情報抽出時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字0と区切り文字1に同じ文字を指定して情報抽出する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a O -D0 0020 -D1 0020 -i $DAT_FILE1`
echo norm_$TEST: 005-1 $OUT;

# TARGET E
# 情報抽出時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字0と区切り文字2に同じ文字を指定して情報抽出する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a O -D0 0020 -D2 0020 -i $DAT_FILE1`
echo norm_$TEST: 005-2 $OUT;

# TARGET E
# 情報抽出時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字0とエスケープ文字に同じ文字を指定して情報抽出する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a O -D0 0020 -D3 0020 -i $DAT_FILE1`
echo norm_$TEST: 005-3 $OUT;

# TARGET E
# 情報抽出時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字1と区切り文字2に同じ文字を指定して情報抽出する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a O -D1 0020 -D2 0020 -i $DAT_FILE1`
echo norm_$TEST: 005-4 $OUT;

# TARGET E
# 情報抽出時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字1とエスケープ文字に同じ文字を指定して情報抽出する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a O -D1 0020 -D3 0020 -i $DAT_FILE1`
echo norm_$TEST: 005-5 $OUT;

# TARGET E
# 情報抽出時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字2とエスケープ文字に同じ文字を指定して情報抽出する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a O -D2 0020 -D3 0020 -i $DAT_FILE1`
echo norm_$TEST: 005-6 $OUT;

# TARGET E
# 情報抽出時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字0にカタカナブロックの文字（中点）を指定して情報抽出する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a O -D0 30FB -i $DAT_FILE1`
echo norm_$TEST: 005-7 $OUT;

# TARGET E
# 情報抽出時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字1にカタカナブロックの文字（中点）を指定して情報抽出する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a O -D1 30FB -i $DAT_FILE1`
echo norm_$TEST: 005-8 $OUT;

# TARGET E
# 情報抽出時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# 区切り文字2にカタカナブロックの文字（中点）を指定して情報抽出する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a O -D2 30FB -i $DAT_FILE1`
echo norm_$TEST: 005-9 $OUT;

# TARGET E
# 情報抽出時の区切り文字/エスケープ文字指定のエラー
#
# PROCESS
# エスケープ文字にカタカナブロックの文字（中点）を指定して情報抽出する。
#
# EFFECT
# ModExceptionが発生する。
OUT=`${prefix}norm -x -a O -D3 30FB -i $DAT_FILE1`
echo norm_$TEST: 005-10 $OUT;
