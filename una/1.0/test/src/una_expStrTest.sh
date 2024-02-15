# 
# Copyright (c) 2009, 2023 Ricoh Company, Ltd.
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
# Expand Strings testing
# Target Language and Analyzer :
#     ja : UNA
#
prefix="./"
TEST=expStrTest
DAT_DIR=../src/dat.una/$TEST

##########################################
# TARGET N
# 展開機能の基本機能テスト
#
# PROCESS
# ・データディレクトリに置かれている文字列単位の展開データに応じた展開処理を行う。
# ・新仕様で廃止した文字列展開モードが指定された場合(nlpexpstrの-eオプションで指定)、
#   設定値は無視して動作することを検証する。
# ・展開データに登録されている展開対象文字列を入力した時は展開結果のステータスが1、
#   展開結果に最長の展開文字列の展開文字列パターンが設定されることを検証する。
# ・展開データに登録されていない展開対象文字列を入力した時は展開結果のステータスが0、
#   展開結果に展開対象文字列が設定されることを検証する。
# ・展開対象文字列長パラメータで指定された文字列長より長い展開対象文字列を入力した時は
#   展開結果のステータスが2、展開結果に展開対象文字列が設定されることを検証する。
# ・展開パターン数パラメータで指定された展開パターン数より取得される展開パターン数が多い
#   時は展開結果のステータスが3、展開結果に展開対象文字列が設定されることを検証する。
# ・解析結果と正解ファイルを比較する。
#
# EFFECT
# 出力結果が正解ファイルと一致する。

FILES="regstr sentence"

for FILE in $FILES
do
	DAT_FILE=$DAT_DIR/$FILE
	DIC_DIR=../unadic
	DATAS="0 1"

	for DATA in $DATAS
	do
		ANS_FILE=../src/ans.una/$TEST/$FILE.$DATA.nlpexpstr
		OUT_FILE=nlpexpstr_$TEST.$DATA.$FILE
		${prefix}nlpexpstr -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -c utf8 -e $DATA;
		
		# 文字列展開結果と正解ファイルを比較
		cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
		(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
	done
done

##########################################
# TARGET N
#
# 最大展開文字列長パラメータがデフォルトより大きい(41)場合
# 最大展開文字列パターン数:デフォルト
#
# PROCESS
# ・最大展開文字列長パラメータに41を設定
# ・ModNlpAnalyzer::getExpandStringsを呼び出す
#
# EFFECT
# 出力結果が正解ファイルと一致する。

DIC_DIR=../unadic
ANS_FILE=../src/ans.una/$TEST/$FILE.nlpexpstr.s41
OUT_FILE=nlpexpstr_$TEST.$FILE.s41
${prefix}nlpexpstr -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -c utf8 -s 41;

# 文字列展開結果と正解ファイルを比較
cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

##########################################
# TARGET N
#
# 最大展開文字列長パラメータがデフォルトより小さい(39)の場合
#
# PROCESS
# ・最大展開文字列長パラメータに39を設定
# ・ModNlpAnalyzer::getExpandStringsを呼び出す
#
# EFFECT
# 出力結果が正解ファイルと一致する。

DIC_DIR=../unadic
ANS_FILE=../src/ans.una/$TEST/$FILE.nlpexpstr.s39
OUT_FILE=nlpexpstr_$TEST.$FILE.s39
${prefix}nlpexpstr -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -c utf8 -s 39;

# 文字列展開結果と正解ファイルを比較
cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

##########################################
# TARGET N
#
# 最大展開文字列パターン数パラメータがデフォルトより大きい(41)場合
#
# PROCESS
# ・最大展開文字列長パラメータに41を設定
# ・ModNlpAnalyzer::getExpandStringsを呼び出す
#
# EFFECT
# 出力結果が正解ファイルと一致する。

DIC_DIR=../unadic
ANS_FILE=../src/ans.una/$TEST/$FILE.nlpexpstr.u41
OUT_FILE=nlpexpstr_$TEST.$FILE.u41
${prefix}nlpexpstr -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -c utf8 -u 41;

# 文字列展開結果と正解ファイルを比較
cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

##########################################
# TARGET N
#
# 最大展開文字列パターン数パラメータがデフォルトより小さい(15)場合
#
# PROCESS
# ・最大展開文字列長パラメータに15を設定
# ・ModNlpAnalyzer::getExpandStringsを呼び出す
#
# EFFECT
# 出力結果が正解ファイルと一致する。

DIC_DIR=../unadic
ANS_FILE=../src/ans.una/$TEST/$FILE.nlpexpstr.u15
OUT_FILE=nlpexpstr_$TEST.$FILE.u15
${prefix}nlpexpstr -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -c utf8 -u 15;

# 文字列展開結果と正解ファイルを比較
cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);

##########################################
# TARGET N
#
# 複合語分割を指定した場合
#
# PROCESS
# ・複合語分割のモードを設定
# ・ModNlpAnalyzer::getExpandStringsを呼び出す
#
# EFFECT
# 出力結果が正解ファイルと一致する。

DIC_DIR=../unadic

for MODE in 1 2 5 6
do
	ANS_FILE=../src/ans.una/$TEST/$FILE.nlpexpstr.$MODE
	OUT_FILE=nlpexpstr_$TEST.$FILE.$MODE
	${prefix}nlpexpstr -l -r $DIC_DIR -i $DAT_FILE -o $OUT_FILE -c utf8 -m $MODE;

	# 展開結果と正解ファイルを比較
	cmp -s $ANS_FILE $OUT_FILE && rm $OUT_FILE || \
	(echo $FILE " FAILED:"; ls -l $ANS_FILE $OUT_FILE; echo);
done
