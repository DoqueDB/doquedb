#!/bin/sh
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
###############################################################################
#
# mkstem.sh
#
# 機能：ステミングデータの作成
#
# 実行方法：
#   mkstem.sh
#
###############################################################################

###############################################################################
# ディレクトリの設定
###############################################################################
SRCDIR=../src-data/stem
TOOLDIR=../tools/perl
WRKDIR=./swork

###############################################################################
# データ作成
###############################################################################

# データ追加
cat $SRCDIR/SRC_join.dat $SRCDIR/source_add.dat | \
    perl -aF: -nle 'print $F[0] if/^[^#]/' | \
    perl $TOOLDIR/sortu.pl > $WRKDIR/add.tmp

# データ削除
perl -aF: -nle 'print $F[0] if/^[^#]/' $SRCDIR/source_del.dat | \
    perl $TOOLDIR/sortu.pl > $WRKDIR/del.tmp

# 語形データ作成
comm -23 $WRKDIR/add.tmp $WRKDIR/del.tmp | \
    perl $TOOLDIR/src_inf_split.pl | \
    perl $TOOLDIR/src_inf_conv.pl | \
    perl $TOOLDIR/src_inf_check.pl | \
    perl $TOOLDIR/src_form.pl | perl $TOOLDIR/sortu.pl > $WRKDIR/src_form.dat

###############################################################################
# 語幹データ作成
###############################################################################

perl $TOOLDIR/src_lemma.pl $WRKDIR/src_form.dat | \
    perl $TOOLDIR/sortu.pl | perl $TOOLDIR/group.pl | \
    sed 's/,//g' > $WRKDIR/src_lemma.dat

###############################################################################
# 異表記データの作成
###############################################################################

# パターン１に基づく語形データからの異表記収集
perl $TOOLDIR/src_var1.pl -p $SRCDIR/var1.pat $WRKDIR/src_lemma.dat | \
    perl $TOOLDIR/sortu.pl > $WRKDIR/var1.dat

# パターン２に基づく語形データからの異表記収集
perl $TOOLDIR/src_var2.pl -p $SRCDIR/var2.pat $WRKDIR/src_lemma.dat | \
    perl $TOOLDIR/sortu.pl > $WRKDIR/var2.dat

# 抽出した異表記の統合
cat $SRCDIR/VAR_[rt]*.dat $SRCDIR/var_add.dat $WRKDIR/var[12].dat | \
    grep '^[^#]' | perl $TOOLDIR/sortu.pl > $WRKDIR/var.tmp
perl $TOOLDIR/src_var_check.pl -p $SRCDIR/var1.pat -q $SRCDIR/var2.pat \
	-d $SRCDIR/var_del.dat -l $WRKDIR/src_lemma.dat $WRKDIR/var.tmp | \
    perl $TOOLDIR/sortu.pl | \
    perl $TOOLDIR/group.pl | \
    perl -a -nle 'print $F[1]' | perl $TOOLDIR/sortu.pl | \
    perl $TOOLDIR/src_var.pl | perl $TOOLDIR/sortu.pl | \
    perl $TOOLDIR/group.pl | \
    perl -a -nle 'print $F[1]' | perl $TOOLDIR/sortu.pl | \
    perl $TOOLDIR/src_var.pl | \
    perl $TOOLDIR/sortu.pl | perl $TOOLDIR/group.pl > $WRKDIR/src_var.dat

###############################################################################
# 正規化データの作成
###############################################################################

# 語形データに"-ed","-ing"からの名詞語形を追加
perl $TOOLDIR/form.pl $WRKDIR/src_form.dat | \
    perl $TOOLDIR/sortu.pl | perl $TOOLDIR/group.pl > $WRKDIR/form.dat

# 屈折パターンデータ
perl $TOOLDIR/inf_form.pl $WRKDIR/src_form.dat | \
    perl $TOOLDIR/sortu.pl | perl $TOOLDIR/group.pl | \
    perl -a -nle 'print $F[1]' | perl $TOOLDIR/sortu.pl | \
    perl $TOOLDIR/src_var.pl | \
    perl $TOOLDIR/sortu.pl | perl $TOOLDIR/group.pl > $WRKDIR/inf.dat

# 正規化対象とする語形のチェック
join $SRCDIR/check_form.dat $WRKDIR/form.dat > $WRKDIR/form.tmp
perl $TOOLDIR/form_exc.pl -i $WRKDIR/inf.dat -t $WRKDIR/form.tmp \
    > $WRKDIR/form_exc.dat

# 屈折形正規化
perl $TOOLDIR/norm_inf.pl \
    -e $WRKDIR/form_exc.dat -s $WRKDIR/form.dat -i $WRKDIR/inf.dat | \
    perl $TOOLDIR/sortu.pl | perl $TOOLDIR/group.pl > $WRKDIR/norm_inf.dat

# 異表記正規化と曖昧語形の解消
perl $TOOLDIR/norm_var.pl -s $WRKDIR/src_var.dat $WRKDIR/norm_inf.dat \
    > $WRKDIR/norm_var.dat

# 正規化語形の平準化
perl $TOOLDIR/norm_level.pl $WRKDIR/norm_var.dat | \
    perl $TOOLDIR/sortu.pl > $WRKDIR/norm.dat

###############################################################################
# 展開データの作成
###############################################################################

perl $TOOLDIR/exp_inf.pl -n $WRKDIR/norm.dat $WRKDIR/inf.dat | \
    perl $TOOLDIR/sortu.pl | perl $TOOLDIR/group.pl | \
    grep "," > $WRKDIR/exp_inf.dat

perl $TOOLDIR/exp_merge_var.pl \
    -s $WRKDIR/src_var.dat -i $WRKDIR/exp_inf.dat | \
    perl $TOOLDIR/sortu.pl | perl $TOOLDIR/group.pl > $WRKDIR/exp_inf_var.dat

perl $TOOLDIR/exp_merge_inf.pl -i $WRKDIR/exp_inf_var.dat $WRKDIR/src_var.dat |
    perl $TOOLDIR/sortu.pl | perl $TOOLDIR/group.pl > $WRKDIR/exp_var_inf.dat

# 結合可能なデータの展開パターンは同一になる
join -a1 -a2 $WRKDIR/exp_var_inf.dat $WRKDIR/exp_inf_var.dat | \
    perl $TOOLDIR/print.pl | perl $TOOLDIR/sortu.pl > $WRKDIR/exp.dat

###############################################################################
# リソースファイル作成
###############################################################################

perl $TOOLDIR/rl_stem.pl $SRCDIR/norm_inf.rul | \
    perl $TOOLDIR/sortu.pl > $WRKDIR/rl_stem.src

perl $TOOLDIR/mk_dic.pl -e $WRKDIR/exp.dat $WRKDIR/norm.dat \
    > $WRKDIR/dic_stem.src

rm -f $WRKDIR/var.tmp $WRKDIR/form.tmp
