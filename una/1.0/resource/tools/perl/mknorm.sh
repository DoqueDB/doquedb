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
# mknorm.sh
#
# 機能: 文字変換データの作成
#
# 実行方法:
#   mknorm.sh
#
###############################################################################

###############################################################################
# ディレクトリの設定
###############################################################################
SRCDIR=../src-data/norm;
TOOLDIR=../tools/perl;
WRKDIR=./nwork;

###############################################################################
# データ作成
###############################################################################
perl $TOOLDIR/euc2ucs.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/euc2ucs.dat
perl $TOOLDIR/euc2ucs-jj.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/euc2ucs-jj.dat

perl $TOOLDIR/TargetLatin.pl -s $SRCDIR| perl $TOOLDIR/sortu.pl > $WRKDIR/TargetLatin.dat
perl $TOOLDIR/TargetCyrillic.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/TargetCyrillic.dat
perl $TOOLDIR/TargetGreek.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/TargetGreek.dat
perl $TOOLDIR/TargetKana.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/TargetKana.dat
perl $TOOLDIR/TargetControl.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/TargetControl.dat
perl $TOOLDIR/TargetCombining.pl -s $SRCDIR -d $WRKDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/TargetCombining.dat

perl $TOOLDIR/NormKmap.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/NormKmap.dat
perl $TOOLDIR/NormCode.pl -s $SRCDIR -d $WRKDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/NormCode.dat

perl $TOOLDIR/DecompLatin.pl -s $SRCDIR -d $WRKDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/DecompLatin.dat
perl $TOOLDIR/DecompGreek.pl -s $SRCDIR -d $WRKDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/DecompGreek.dat
perl $TOOLDIR/DecompCyrillic.pl -s $SRCDIR -d $WRKDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/DecompCyrillic.dat

perl $TOOLDIR/Cap2Small.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/Cap2Small.dat
perl $TOOLDIR/Full2Half.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/Full2Half.dat
perl $TOOLDIR/Half2Full.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/Half2Full.dat
perl $TOOLDIR/Small2Basic.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/Small2Basic.dat
perl $TOOLDIR/Old2New.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/Old2New.dat
perl $TOOLDIR/Hira2Kata.pl -s $SRCDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/Hira2Kata.dat

perl $TOOLDIR/CombiKana.pl -s $SRCDIR -d $WRKDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/CombiKana.dat
perl $TOOLDIR/CombiLatin.pl -s $SRCDIR -d $WRKDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/CombiLatin.dat
perl $TOOLDIR/CombiGreek.pl -s $SRCDIR -d $WRKDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/CombiGreek.dat
perl $TOOLDIR/CombiCyrillic.pl -s $SRCDIR -d $WRKDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/CombiCyrillic.dat

perl $TOOLDIR/MakeCombiMap.pl -d $WRKDIR | perl $TOOLDIR/sortu.pl | sed 's/^ *//' > $WRKDIR/combiMap.dat
perl $TOOLDIR/MakePreMap.pl -d $WRKDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/preMap.dat
perl $TOOLDIR/MakePostMap.pl -d $WRKDIR | perl $TOOLDIR/sortu.pl > $WRKDIR/postMap.dat
