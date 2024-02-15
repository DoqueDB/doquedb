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
DAT_DIR=../src/dat.stem/$TEST
PATH_DIR=$DAT_DIR/path

# TARGET E
# 辞書見出し語形ソースに不正な文字が含まれる
#
# PROCESS
# 不正な文字を含むファイルを辞書見出し語形ソースファイルに指定して、ス
# テマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.001-1 /dev/null`
echo stem_$TEST: 001-1 $OUT;

# TARGET E
# 辞書展開語形ソースに不正な文字が含まれる
#
# PROCESS
# 不正な文字を含むファイルを辞書展開語形ソースファイルに指定して、ステ
# マーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.001-2 /dev/null`
echo stem_$TEST: 001-2 $OUT;

# TARGET E
# 規則見出し語形ソースに不正な文字が含まれる
#
# PROCESS
# 不正な文字を含むファイルを規則見出し語形ソースファイルに指定して、ス
# テマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.001-3 /dev/null`
echo stem_$TEST: 001-3 $OUT;

# TARGET E
# 規則正規化語形ソースに不正な文字が含まれる
#
# PROCESS
# 不正な文字を含むファイルを規則正規化語形ソースファイルに指定して、ス
# テマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.001-4 /dev/null`
echo stem_$TEST: 001-4 $OUT;

# TARGET E
# 辞書見出し語形ソースの末尾が改行でない
#
# PROCESS
# 末尾に改行がないファイルを辞書見出し語形ソースファイルに指定して、ス
# テマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.002-1 /dev/null`
echo stem_$TEST: 002-1 $OUT;

# TARGET E
# 辞書展開語形ソースの末尾が改行でない
#
# PROCESS
# 末尾に改行がないファイルを辞書展開語形ソースファイルに指定して、ス
# テマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.002-2 /dev/null`
echo stem_$TEST: 002-2 $OUT;

# TARGET E
# 規則見出し語形ソースの末尾が改行でない
#
# PROCESS
# 末尾に改行がないファイルを規則見出し語形ソースファイルに指定して、ス
# テマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.002-3 /dev/null`
echo stem_$TEST: 002-3 $OUT;

# TARGET E
# 規則正規化語形ソースの末尾が改行でない
#
# PROCESS
# 末尾に改行がないファイルを規則正規化語形ソースファイルに指定して、ス
# テマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.002-4 /dev/null`
echo stem_$TEST: 002-4 $OUT;

# TARGET E
# 辞書インデックスソースに不正な文字が含まれる
#
# PROCESS
# 不正な文字を含むファイルを辞書インデックスソースファイルに指定して、
# ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.003-1 /dev/null`
echo stem_$TEST: 003-1 $OUT;

# TARGET E
# 規則インデックスソースに不正な文字が含まれる
#
# PROCESS
# 不正な文字を含むファイルを規則インデックスソースファイルに指定して、
# ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.003-2 /dev/null`
echo stem_$TEST: 003-2 $OUT;

# TARGET E
# 辞書インデックスソースのフィールド数が規定数に合わない
#
# PROCESS
# 規定数(=3)と異なるフィールド数のファイルを辞書インデックスソースファ
# イルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.004-1 /dev/null`
echo stem_$TEST: 004-1 $OUT;

# TARGET E
# 規則インデックスソースのフィールド数が規定数に合わない
#
# PROCESS
# 規定数(=2)と異なるフィールド数のファイルを規則インデックスソースファ
# イルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.004-2 /dev/null`
echo stem_$TEST: 004-2 $OUT;

# TARGET E
# 辞書インデックスソースのレコード数が見出し語形数より少ない
#
# PROCESS
# 見出し語形数より少ない行数のファイルを辞書インデックスソースファイル
# に指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.005-1 /dev/null`
echo stem_$TEST: 005-1 $OUT;

# TARGET E
# 規則インデックスソースのレコード数が見出し語形数より少ない
#
# PROCESS
# 見出し語形数より少ない行数のファイルを規則インデックスソースファイル
# に指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.005-2 /dev/null`
echo stem_$TEST: 005-2 $OUT;

# TARGET E
# 辞書インデックスソースのレコード数が見出し語形数より多い
#
# PROCESS
# 見出し語形数より多い行数のファイルを辞書インデックスソースファイル
# に指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.006-1 /dev/null`
echo stem_$TEST: 006-1 $OUT;

# TARGET E
# 規則インデックスソースのレコード数が見出し語形数より多い
#
# PROCESS
# 見出し語形数より多い行数のファイルを規則インデックスソースファイル
# に指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.006-2 /dev/null`
echo stem_$TEST: 006-2 $OUT;

# TARGET E
# 辞書インデックスソースの末尾が改行でない
#
# PROCESS
# 末尾に改行がないファイルを辞書インデックスソースファイルに指定して、
# ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.007-1 /dev/null`
echo stem_$TEST: 007-1 $OUT;

# TARGET E
# 規則インデックスソースの末尾が改行でない
#
# PROCESS
# 末尾に改行がないファイルを規則インデックスソースファイルに指定して、
# ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.007-2 /dev/null`
echo stem_$TEST: 007-2 $OUT;

# TARGET E
# 辞書見出し語形オフセットに不正な値が記述される
#
# PROCESS
# 見出し語形オフセットの値が不正に記述されたファイルを辞書インデックス
# ソースファイルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.008-1 /dev/null`
echo stem_$TEST: 008-1 $OUT;

# TARGET E
# 規則見出し語形オフセットに不正な値が記述される
#
# PROCESS
# 見出し語形オフセットの値が不正に記述されたファイルを規則インデックス
# ソースファイルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.008-2 /dev/null`
echo stem_$TEST: 008-2 $OUT;

# TARGET E
# 辞書正規化語形オフセットに不正な値が記述される
#
# PROCESS
# 正規化語形オフセットの値が不正に記述されたファイルを辞書インデックス
# ソースファイルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.009-1 /dev/null`
echo stem_$TEST: 009-1 $OUT;

# TARGET E
# 規則正規化語形オフセットに不正な値が記述される
#
# PROCESS
# 正規化語形オフセットの値が不正に記述されたファイルを規則インデックス
# ソースファイルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.009-2 /dev/null`
echo stem_$TEST: 009-2 $OUT;

# TARGET E
# 辞書展開語形オフセットに不正な値が記述される
#
# PROCESS
# 展開語形オフセットの値が不正に記述されたファイルを辞書インデックス
# ソースファイルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.010 /dev/null`
echo stem_$TEST: 010 $OUT;

# TARGET E
# 辞書見出し語形オフセットが記述されていない
#
# PROCESS
# 見出し語形オフセットの値が記述されていないファイルを辞書インデックス
# ソースファイルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.011-1 /dev/null`
echo stem_$TEST: 011-1 $OUT;

# TARGET E
# 規則見出し語形オフセットが記述されていない
#
# PROCESS
# 見出し語形オフセットの値が記述されていないファイルを規則インデックス
# ソースファイルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.011-2 /dev/null`
echo stem_$TEST: 011-2 $OUT;

# TARGET E
# 辞書正規化語形オフセットが記述されていない
#
# PROCESS
# 正規化語形オフセットの値が記述されていないファイルを辞書インデックス
# ソースファイルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.012-1 /dev/null`
echo stem_$TEST: 012-1 $OUT;

# TARGET E
# 規則正規化語形オフセットが記述されていない
#
# PROCESS
# 正規化語形オフセットの値が記述されていないファイルを規則インデックス
# ソースファイルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.012-2 /dev/null`
echo stem_$TEST: 012-2 $OUT;

# TARGET E
# 辞書展開語形オフセットが記述されていない
#
# PROCESS
# 展開語形オフセットの値が記述されていないファイルを辞書インデックス
# ソースファイルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path_err.013 /dev/null`
echo stem_$TEST: 013 $OUT;

# TARGET E
# 書き込み不可のファイルが指定される
#
# PROCESS
# 書き込み不可のファイルをデータ書き出し用ファイルに指定して、ステマー
# を初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path.test $DAT_DIR/unwritable`
echo stem_$TEST: 014 $OUT;

# TARGET E
# 存在しないパスファイルが指定される
#
# PROCESS
# 存在しないファイルをソースデータのパスファイルに指定して、ステマーを
# 初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M path.NIL`
echo stem_$TEST: 015 $OUT;

# TARGET E
# 指定されたソースファイルの数が規定数より少ない
#
# PROCESS
# 規定数(=6)より少ない数のファイル名を記述したファイルでステマーを初期
# 化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path.less`
echo stem_$TEST: 016 $OUT;

# TARGET E
# 指定されたソースファイルの数が規定数より多い
#
# PROCESS
# 規定数(=6)より多い数のファイル名を記述したファイルでステマーを初期化
# する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -M $PATH_DIR/path.more`
echo stem_$TEST: 017 $OUT;

# TARGET E
# 存在しないデータファイルが指定される
#
# PROCESS
# 存在しないファイルをデータファイルに指定して、ステマーを初期化する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -r $DAT_DIR/NIL`
echo stem_$TEST: 018 $OUT;

# TARGET E
# 不正なファイルが指定される
#
# PROCESS
# 不正なファイル(古い形式)をデータファイルに指定して、ステマーを初期化
# する。
#
# EFFECT
# ModException が発生する。
OUT=`${prefix}stem -x -r $DAT_DIR`
echo stem_$TEST: 019 $OUT;

# TARGET L
# 辞書引き中のメモリ不足
#
# PROCESS
# メモリ制限を64bytesに設定し、56KBを超える入力を与える。
#
# EFFECT
# ModExceptionが発生する。
#OUT=`${prefix}stem -x -a l -m 64 -i $DAT_DIR/memtest`
#echo stem_$TEST: 020 $OUT;

# TARGET L
# 正規化中のメモリ不足
#
# PROCESS
# メモリ制限を64bytesに設定し、56KBを超える入力を与える。
#
# EFFECT
# ModExceptionが発生する。
#OUT=`${prefix}stem -x -m 64 -i $DAT_DIR/memtest`
#echo stem_$TEST: 021 $OUT;

# TARGET L
# 展開中のメモリ不足
#
# PROCESS
# メモリ制限を64bytesに設定し、56KBを超える入力を与える。
#
# EFFECT
# ModExceptionが発生する。
#OUT=`${prefix}stem -x -a e -m 64 -i $DAT_DIR/memtest`
#echo stem_$TEST: 022 $OUT;
