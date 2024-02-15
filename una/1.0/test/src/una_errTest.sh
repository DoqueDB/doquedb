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
DAT_DIR=../src/dat.una/$TEST
FILE=exp
ENCODING="-c euc"

###  エラー用  ##

# TARGET E
# ModUnaMiddle::load(dicPath_, appPath_, conPath_, kkrPath_, egtPath_, untPath_, uncPath_, nrmPath_
#                    app2.Path_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは connect.tbl
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.una1
DAT_FILE=$DAT_DIR/$FILE

OUT_FILE=nlpnorm_$TEST.$FILE.contbl
OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
echo nlpnorm_$TEST: 1 $OUT;
rm $OUT_FILE

# TARGET E
# ModUnaMiddle::load(dicPath_, appPath_, conPath_, kkrPath_, egtPath_, untPath_, uncPath_, nrmPath_
#                    app2.Path_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは engmk.tbl
#
# EFFECT
# Exception で処理終了


ERR_DICNLP_DIR=../errdic.una2

OUT_FILE=nlpnorm_$TEST.$FILE.engtbl
OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
echo nlpnorm_$TEST: 2 $OUT;
rm $OUT_FILE

# TARGET E
# ModUnaMiddle::load(dicPath_, appPath_, conPath_, kkrPath_, egtPath_, untPath_, uncPath_, nrmPath_
#                    app2.Path_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは unastd.tbl
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.una3

OUT_FILE=nlpnorm_$TEST.$FILE.stdtbl
OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
echo nlpnorm_$TEST: 3 $OUT;
rm $OUT_FILE

# TARGET E
# ModUnaMiddle::load(dicPath_, appPath_, conPath_, kkrPath_, egtPath_, untPath_, uncPath_, nrmPath_
#                    app2.Path_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは unawrd.dic
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.una4

OUT_FILE=nlpnorm_$TEST.$FILE.wrddic
OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
echo nlpnorm_$TEST: 4 $OUT;
rm $OUT_FILE

# TARGET E
# ModUnaMiddle::load(dicPath_, appPath_, conPath_, kkrPath_, egtPath_, untPath_, uncPath_, nrmPath_
#                    app2.Path_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは unkcost.tbl
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.una5

OUT_FILE=nlpnorm_$TEST.$FILE.costtbl
OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
echo nlpnorm_$TEST: 5 $OUT;
rm $OUT_FILE

# TARGET E
# ModUnaMiddle::load(dicPath_, appPath_, conPath_, kkrPath_, egtPath_, untPath_, uncPath_, nrmPath_
#                    app2.Path_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは unkmk.tbl
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.una6

	OUT_FILE=nlpnorm_$TEST.$FILE.mktbl
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 6 $OUT;
	rm $OUT_FILE

# TARGET E
# ModNormRule::load(ruleDicPath_, ruleAppPath_, expandDicPath_, expandAppPath_, connectTblPath_, 
#                   preMapPath_, postMapPath_, combiMapPath_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは combiMap.dat
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.norm1

	OUT_FILE=nlpnorm_$TEST.$FILE.conMap
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 7 $OUT;
	rm $OUT_FILE

# TARGET E
# ModNormRule::load(ruleDicPath_, ruleAppPath_, expandDicPath_, expandAppPath_, connectTblPath_, 
#                   preMapPath_, postMapPath_, combiMapPath_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは connect.tbl
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.norm2

	OUT_FILE=nlpnorm_$TEST.$FILE.conn
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 8 $OUT;
	rm $OUT_FILE

# TARGET E
# ModNormRule::load(ruleDicPath_, ruleAppPath_, expandDicPath_, expandAppPath_, connectTblPath_, 
#                   preMapPath_, postMapPath_, combiMapPath_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは expApp.dic
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.norm3

	OUT_FILE=nlpnorm_$TEST.$FILE.expApp
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 9 $OUT;
	rm $OUT_FILE

# TARGET E
# ModNormRule::load(ruleDicPath_, ruleAppPath_, expandDicPath_, expandAppPath_, connectTblPath_, 
#                   preMapPath_, postMapPath_, combiMapPath_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは expWrd.dic
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.norm4

	OUT_FILE=nlpnorm_$TEST.$FILE.expWrd
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 10 $OUT;
	rm $OUT_FILE

# TARGET E
# ModNormRule::load(ruleDicPath_, ruleAppPath_, expandDicPath_, expandAppPath_, connectTblPath_, 
#                   preMapPath_, postMapPath_, combiMapPath_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは postMap.dat
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.norm5

	OUT_FILE=nlpnorm_$TEST.$FILE.postMap
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 11 $OUT;
	rm $OUT_FILE

# TARGET E
# ModNormRule::load(ruleDicPath_, ruleAppPath_, expandDicPath_, expandAppPath_, connectTblPath_, 
#                   preMapPath_, postMapPath_, combiMapPath_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは preMap.dat
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.norm6

	OUT_FILE=nlpnorm_$TEST.$FILE.preMap
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 12 $OUT;
	rm $OUT_FILE

# TARGET E
# ModNormRule::load(ruleDicPath_, ruleAppPath_, expandDicPath_, expandAppPath_, connectTblPath_, 
#                   preMapPath_, postMapPath_, combiMapPath_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは ruleApp.dic
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.norm7

	OUT_FILE=nlpnorm_$TEST.$FILE.ruleApp
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 13 $OUT;
	rm $OUT_FILE

# TARGET E
# ModNormRule::load(ruleDicPath_, ruleAppPath_, expandDicPath_, expandAppPath_, connectTblPath_, 
#                   preMapPath_, postMapPath_, combiMapPath_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは ruleWrd.dic
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.norm8

	OUT_FILE=nlpnorm_$TEST.$FILE.ruleWrd
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 14 $OUT;
	rm $OUT_FILE

# TARGET E
# ModNormRule::load(ruleDicPath_, ruleAppPath_, expandDicPath_, expandAppPath_, connectTblPath_, 
#                   preMapPath_, postMapPath_, combiMapPath_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは unkcost.tbl
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.norm9

	OUT_FILE=nlpnorm_$TEST.$FILE.uncost
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 15 $OUT;
	rm $OUT_FILE

# TARGET E
# ModNormRule::load(ruleDicPath_, ruleAppPath_, expandDicPath_, expandAppPath_, connectTblPath_, 
#                   preMapPath_, postMapPath_, combiMapPath_)
# PROCESS
# 取り込むファイル名を変更して、データファイルをロードする
# 対象ファイルは unkmk.tbl
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.norm10

	OUT_FILE=nlpnorm_$TEST.$FILE.unmk
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 16 $OUT;
	rm $OUT_FILE

# TARGET E
#データディレクトリがない場合
#
# PROCESS
# 対象データセットディレクトリはerrdic.nodir
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.nodir

	OUT_FILE=nlpnorm_$TEST.$FILE.nodir
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 18 $OUT;
	rm $OUT_FILE

# TARGET E
#データディレクトリはあるがnormディレクトリがない場合
#
# PROCESS
# 対象データセットディレクトリはerrdic.nonorm
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.nonorm

	OUT_FILE=nlpnorm_$TEST.$FILE.nonorm
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 19 $OUT;
	rm $OUT_FILE

# TARGET E
#データディレクトリはあるがunaディレクトリがない場合
#
# PROCESS
# 対象データセットディレクトリはerrdic.nouna
#
# EFFECT
# Exception で処理終了

ERR_DICNLP_DIR=../errdic.nouna

	OUT_FILE=nlpnorm_$TEST.$FILE.nouna
	OUT=`${prefix}nlpnorm $ENCODING -X -m 1 -r $ERR_DICNLP_DIR -i $DAT_FILE -o $OUT_FILE`
	echo nlpnorm_$TEST: 20 $OUT;
	rm $OUT_FILE
