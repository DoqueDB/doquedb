#!/bin/sh
# 
# Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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
###############################################################################
#
# mkExpStrData.sh
#
# 機能: 文字列展開データの作成
#
# 実行方法:
#    ・normデータ作成時にmake-normから呼び出され文字列展開データを作成する
#        コマンドは以下
#          make -f make-norm expstrdata
#
# 実行前提条件:
#   ・文字列展開データが設定されている
#   ・以下のプログラムが存在している
#        tools/bin/norm
#   ・normの実行用データが存在している
#        ./dic/norm
#
###############################################################################

echo "mkExpStrData.sh start."

###############################################################################
# ディレクトリの設定
###############################################################################
SRCDIR=../src-data/norm
UNADIR=../src-data/norm/una
TOOLDIR=../tools/perl
BINDIR=../tools/bin
WRKDIR=./nwork
DATADIR=.

EXPSTR_VERSION=dougigo0.1

EXE_NORM=$BINDIR/norm

###############################################################################
# データ作成
################################################################################

# UNA品詞データの前処理
iconv -f UTF8 -t UCS2 $UNADIR/unahin.utf8 -o $WRKDIR/unahin.ucs2

TMP_EXP=$WRKDIR/src_expSystem
DIC_SRC=$WRKDIR/expStrStr
perl $TOOLDIR/exp1.pl $SRCDIR/src_expSystem.txt > $TMP_EXP.tmp1
$EXE_NORM -l -r $DATADIR/$DS/norm -k < $TMP_EXP.tmp1 > $TMP_EXP.tmp2
perl $TOOLDIR/exp3pre.pl < $TMP_EXP.tmp2 | \
	perl $TOOLDIR/exp3.pl | \
	perl $TOOLDIR/sortu.pl | \
	perl $TOOLDIR/group.pl | \
	perl $TOOLDIR/ungroup.pl | \
	perl $TOOLDIR/sortu.pl | \
	perl $TOOLDIR/group.pl | \
	perl -pe "s/[01]://g" | \
	perl $TOOLDIR/tmp_exp3.pl | \
	iconv -f UTF8 -t UCS2 -o $DIC_SRC.tmp1
$BINDIR/addrecno $DIC_SRC.tmp1 $DIC_SRC.tmp2
$BINDIR/unasort $DIC_SRC.tmp2 $DIC_SRC.ucs2
$BINDIR/mkunadic -c -a $EXPSTR_VERSION $UNADIR/costmax.log $WRKDIR/unahin.ucs2 \
	$DIC_SRC.ucs2 $WRKDIR/expStrStrWrd.dic $WRKDIR/expStrStrApp.dic

echo "mkExpStrData.sh completed."
