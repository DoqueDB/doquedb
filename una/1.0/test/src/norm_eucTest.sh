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
##### FILE = etc #####

# TARGET N
# 雑多な文字種を対象とする基本動作
#
# PROCESS
# 雑多な文字種から成る文字列を対象とする正規化結果(前処理形式出力)と、
# 正解ファイルとを比較する。
#
# EFFECT
# 正規化結果が正解ファイルと一致する。

# TARGET N
# 雑多な文字種を対象とする基本動作
#
# PROCESS
# 雑多な文字種から成る文字列を対象とする正規化結果(正規化表記出力)と、
# 正解ファイルとを比較する。
#
# EFFECT
# 正規化結果が正解ファイルと一致する。

# TARGET N
# 雑多な文字種を対象とする基本動作
#
# PROCESS
# 雑多な文字種から成る文字列を対象とする展開結果と、正解ファイルと
# を比較する。
#
# EFFECT
# 展開結果が正解ファイルと一致する。

# TARGET N
# 雑多な文字種を対象とする基本動作
#
# PROCESS
# 雑多な文字種から成る文字列を対象とする正規化結果(前処理形式出力)
# から原表記を抽出し、入力ファイルと比較する。
#
# EFFECT
# 抽出結果が入力ファイルと一致する。

# TARGET N
# 雑多な文字種を対象とする基本動作
#
# PROCESS
# 雑多な文字種から成る文字列を対象とする正規化結果(前処理形式出力)
# から、正規化表記を抽出し、正規化結果(正規化表記出力)と比較する。
#
# EFFECT
# 抽出結果が正規化結果と一致する。

##### FILE = patkata #####

# TARGET N
# カタカナ文字列を対象とする基本動作
#
# PROCESS
# カタカナ文字列を対象とする正規化結果(前処理形式出力)と、正解ファイル
# とを比較する。
#
# EFFECT
# 正規化結果が正解ファイルと一致する。

# TARGET N
# カタカナ文字列を対象とする基本動作
#
# PROCESS
# カタカナ文字列を対象とする正規化結果(正規化表記出力)と、正解ファイル
# とを比較する。
#
# EFFECT
# 正規化結果が正解ファイルと一致する。

# TARGET N
# カタカナ文字列を対象とする基本動作
#
# PROCESS
# カタカナ文字列を対象とする展開結果と、正解ファイルとを比較する。
#
# EFFECT
# 展開結果が正解ファイルと一致する。

# TARGET N
# カタカナ文字列を対象とする基本動作
#
# PROCESS
# カタカナ文字列を対象とする正規化結果(前処理形式出力)から原表記を抽出
# し、入力ファイルと比較する。
#
# EFFECT
# 抽出結果が入力ファイルと一致する。

# TARGET N
# カタカナ文字列を対象とする基本動作
#
# PROCESS
# カタカナ文字列を対象とする正規化結果(前処理形式出力)から、正規化表記
# を抽出し、正規化結果(正規化表記出力)と比較する。
#
# EFFECT
# 抽出結果が正規化結果と一致する。

# TARGET N
# Ｃインターフェースと通常のインターフェースの差異を確認
#
# PROCESS
# Ｃインターフェースを通じての結果と通常のインターフェース
# を通じての結果を取得し比較する。
#
# EFFECT
# 差異がないこと。

prefix="./"
TEST=eucTest
DAT_DIR=../src/dat.norm/$TEST
ENCODING="-c euc"

FILES="etc kata"

for FILE in $FILES
do
  DAT_FILE=$DAT_DIR/$FILE

  case $FILE in
  etc)
    SEP=0x0a
    ;;
  *)
    SEP=0x2c
    ;;
  esac
  
  DIC_SPECIFY="-r ../unadic/norm"

  ANS_BOTH=../src/ans.norm/$TEST/$FILE.both
  ANS_NORM=../src/ans.norm/$TEST/$FILE.norm
  ANS_EXP=../src/ans.norm/$TEST/$FILE.exp

  OUT_BOTH=norm_$TEST.$FILE.both
  OUT_NORM=norm_$TEST.$FILE.norm
  OUT_ORG=norm_$TEST.$FILE.org
  OUT_XTR=norm_$TEST.$FILE.xtr
  OUT_EXP=norm_$TEST.$FILE.exp
  OUT_NORMI=norm_$TEST.$FILE.normI
  OUT_EXPI=norm_$TEST.$FILE.expI
	
  ${prefix}norm -l $DIC_SPECIFY $ENCODING -i $DAT_FILE -o $OUT_BOTH -a b;
  ${prefix}norm -l $DIC_SPECIFY $ENCODING -i $OUT_BOTH -o $OUT_ORG  -a O;
  ${prefix}norm -l $DIC_SPECIFY $ENCODING -i $OUT_BOTH -o $OUT_XTR  -a X;
  ${prefix}norm -l $DIC_SPECIFY $ENCODING -i $DAT_FILE -o $OUT_NORM -a n;
  ${prefix}norm -l $DIC_SPECIFY $ENCODING -i $DAT_FILE -o $OUT_EXP  -a e -S $SEP;
  ${prefix}norm -l $DIC_SPECIFY $ENCODING -i $DAT_FILE -o $OUT_NORMI -a n -I;
  ${prefix}norm -l $DIC_SPECIFY $ENCODING -i $DAT_FILE -o $OUT_EXPI -a e -S $SEP -I;

# 通常のインターフェースとCインターフェースを比較
  cmp -s $OUT_NORM $OUT_NORMI && rm $OUT_NORMI || \
  (echo "C norm FAILED: norm_"$TEST; ls -l $OUT_NORM $OUT_NORMI; echo);
  cmp -s $OUT_EXP $OUT_EXPI && rm $OUT_EXPI || \
  (echo "C exp FAILED: norm_"$TEST; ls -l $OUT_EXP $OUT_EXPI; echo);

# 原表記抽出結果と入力ファイルを比較
  cmp -s $DAT_FILE $OUT_ORG && rm $OUT_ORG || \
  (echo "extorg FAILED: norm_"$TEST; ls -l $DAT_FILE $OUT_ORG; echo);

# 正規化表記抽出結果と正規化結果を比較
  cmp -s $OUT_NORM $OUT_XTR && rm $OUT_XTR || \
  (echo "extract FAILED: norm_"$TEST; ls -l $OUT_NORM $OUT_XTR; echo);

# 前処理結果と正解を比較
  cmp -s $ANS_BOTH $OUT_BOTH && \
  (test -f $OUT_ORG || test -f $OUT_XTR || rm $OUT_BOTH) || \
  (echo "both FAILED: norm_"$TEST; ls -l $ANS_BOTH $OUT_BOTH; echo);

# 正規化結果と正解を比較
  cmp -s $ANS_NORM $OUT_NORM && \
  (test -f $OUT_XTR || rm $OUT_NORM) || \
  (echo "norm FAILED: norm_"$TEST; ls -l $ANS_NORM $OUT_NORM; echo);

# 展開結果と正解を比較
  cmp -s $ANS_EXP $OUT_EXP && rm $OUT_EXP || \
  (echo "exp FAILED: norm_"$TEST; ls -l $ANS_EXP $OUT_EXP; echo);
done
