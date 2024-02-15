// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//	ModTerm.cpp -- ModTerm の実装
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#include "LibUna/UnaNameSpace.h"
#include "LibUna/ModTerm.h"
#include "LibUna/UnicodeChar.h"
#include "LibUna/ModNlpNpCost.h"

_UNA_USING

namespace {
	// リソースファイルのファイル名
	const ModUnicodeString _ParameterDict("parameter.utf8");
	const ModUnicodeString _StopDict("stopDict.utf8");
	const ModUnicodeString _StopRepreDict("stopRepreDict.utf8");
	const ModUnicodeString _TermTypeTable("termType.utf8");
	const ModUnicodeString _StopTypeDict("stopType.utf8");
}

//
// FUNCTION public
// ModTermResource::ModTermResource -- 検索語処理器リソースのコンストラクタ
//
// NOTES
//	 検索語処理器リソースのコンストラクタ。
//	 各リソースファイルを基に以下の辞書を生成しそのポインタをセット。
//	 --------------------------------------------------------------
//		属性				 説明			 リソースファイル
//	 --------------------------------------------------------------
//	 - parameterDict		パラメタ辞書	(dir/paramter.utf8)
//	 - stopDict				禁止辞書		(dir/stopDict.utf8)
//	 - stopRepreDict		禁止表現辞書	(dir/stopRepreDict.utf8)
//	 - termTypeTable		検索語タイプ表	(dir/termType.utf8)
//	 - stopTypeDict			禁止タイプ辞書	(dir/stopType.utf8)
//	 --------------------------------------------------------------
//
// ARGUMENTS
//	 const ModUnicodeString& dir	リソースの格納ディレクトリ
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
ModTermResource::ModTermResource(
	const ModUnicodeString& dir)	// リソースの格納ディレクトリ
{
	// 初期化
	parameterDict		= 0;
	stopDict			= 0;
	stopRepreDict		= 0;
	termTypeTable		= 0;
	stopTypeDict		= 0;

	try {
		// 読み込み
		parameterDict = new ModTermParameterFile(dir + _ParameterDict);
		stopDict = new ModTermWordFile(dir + _StopDict);
		stopRepreDict = new ModTermWordFile(dir + _StopRepreDict);
		termTypeTable = new ModTermTypeTable(dir + _TermTypeTable);
		stopTypeDict = new ModTermTypeFile(dir + _StopTypeDict);

	// 例外発生
	} catch (ModException& exception){
		// これまで作成したリソースを解放する
		if(stopDict != 0) {
			delete stopDict;
		}
		if(stopDict != 0) {
			delete stopRepreDict;
		}
		if(parameterDict != 0) {
			delete parameterDict;
		}
		if(termTypeTable != 0) {
			delete termTypeTable;
		}
		if(stopTypeDict != 0) {
			delete stopTypeDict;
		}
		// 再スロー
		ModRethrow(exception);
	}
}

//
// FUNCTION public
// ModTermResource::~ModTermResource-- デストラクタ
//
// NOTES
//	 デストラクタ。 各リソースを消去する。
//
// ARGUMENTS
//	 なし
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
ModTermResource::~ModTermResource()
{
	delete stopDict;
	delete stopRepreDict;
	delete parameterDict;
	delete termTypeTable;
	delete stopTypeDict;
}

//
// FUNCTION public
// ModTerm::ModTerm -- 検索語処理器コンストラクタ
//
// NOTES
//	 検索語処理器コンストラクタ。
//
//	 リソース中のパラメタファイルを基に各パラメタについて以下を行う。
//	 - 値域の検査
//	 - パラメタ値の設定
//	 パラメタ値がパラメタファイル中に見付からない場合はデフォルト値が
//	 設定される。
//
// ARGUMENTS
//	 const ModTermResource* _resource		リソース
//	 ModNlpLocalAnalyzer* _analyzer			自然言語解析器
//	 ModSize maxWordLen_					
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
ModTerm::ModTerm(
	const ModTermResource* _resource,		// リソース
	ModNlpLocalAnalyzer* _analyzer,			// 自然言語解析器
	ModSize maxWordLen_
)
	: resource(_resource),
		analyzer(_analyzer)
{
	double value;
	ModUnicodeString string;
	string.clear();
	ModVector<double> weightVector;
	weightVector.clear();

	// 禁止辞書使用の有無
	if(resource->parameterDict->getValue("useStopDict1", value) == ModTrue) {
		useStopDict1 = value ? ModTrue : ModFalse;
	} else {
		useStopDict1 = ModTrue;	// 使用する
	}

	// 失敗回避の有無
	if(resource->parameterDict->getValue("failsafe1", value) == ModTrue) {
		failsafe1 = value ? ModTrue : ModFalse;
	} else {
		failsafe1 = ModTrue; // 回避する
	}

	// 検索要求の最大長 (形態素数)
	if(resource->parameterDict->getValue("maxText1", value) == ModTrue) {
		maxText1 = (ModSize)value;
	} else {
		maxText1 = 0; // 無制限
	}

	// 禁止タイプ辞書使用の有無
	if(resource->parameterDict->getValue("useStopTypeDict1", value) == ModTrue) {
		useStopTypeDict1 = value ? ModTrue : ModFalse;
	} else {
		useStopTypeDict1 = ModTrue;	// 使用する
	}

	// 名詞句コストの算出方法
	if(resource->parameterDict->getValue("calcCostMode", value) == ModTrue) {	
		calcCostMode = (ModSize)value;
	} else {
		calcCostMode = 1;	// default mode(WordFreq)
	}

	// 名詞句コスト算出の重み係数1
	if(resource->parameterDict->getValue("costWeight1", value) == ModTrue) {
		costWeight1 = (ModSize)value;
	} else {
		costWeight1 = 1;	// default weight
	}

	// 名詞句コスト算出の重み係数2
	if(resource->parameterDict->getValue("costWeight2", value) == ModTrue) {
		costWeight2 = (ModSize)value;
	} else {
		costWeight2 = 1;	// default weight
	}
}

//
//	FUNCTION public
//	ModTerm::~ModTerm -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ModTerm::~ModTerm()
{
}

//
// FUNCTION public
// ModTerm::getTerm -- 名詞句の抽出
//
// NOTES
//	 poolTermを呼び出し名詞句を抽出する。
//
// ARGUMENTS
//	 const ModUnicodeString&		extractedConcept_	名詞句（正規化表記）
//	 ModVector<ModUnicodeString>&	normVector_			名詞句を構成する形態素（正規化表記）
//	 ModVector<ModUnicodeString>&	origVector_			名詞句を構成する形態素（元表記）
//	 ModVector<int>&				posVector_			名詞句を構成する形態素の品詞
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
ModBoolean
ModTerm::getTerm(
	ModUnicodeString&				normalized_,
	ModUnicodeString&				original_,
	double&							npCost_,
	ModVector<ModUnicodeString>*	normVector_,
	ModVector<ModUnicodeString>*	origVector_,
	ModVector<int>*					posVector_)
{
	normalized_.clear();
	original_.clear();
	if(normVector_){
		normVector_->clear();
		origVector_->clear();
		posVector_->clear();
	}
	npCost_ = 0;

	if (candidateIte == 0 ) {
		// extract nour phrase from the target document
		// reserve NP candidate in the vector
		analyzer->poolTerm(useStopDict1, maxText1, useStopTypeDict1);

		// 名詞句が１つも得られない事態を回避
		if(candidate.getSize() == 0 && failsafe1 == ModTrue) {

			// 禁止辞書を使用しない
			analyzer->poolTerm(ModFalse, maxText1, useStopTypeDict1);

			// それでも得られない場合はテキストを名詞句とする
			// 形態素毎のorg、norm、posはベクターに格納しない
			if(candidate.getSize() == 0 
				&& this->getStrTarget() != UnicodeChar::usCtrlRet
				&& this->getStrTarget() != UnicodeChar::usCtrlCr) {

				ModUnicodeString strTarget = this->getStrTarget();
				// delete 0x0a from end of the line
				ModUnicodeChar* r = strTarget.rsearch(0x0a);
				if (r != 0){
					strTarget.truncate(r);
				}

				// delete 0x0d from end of the line
				r = strTarget.rsearch(0x0d);
				if (r != 0){
					strTarget.truncate(r);
				}

				ModTermElement term(strTarget);
				candidate.insertTerm(term);
			}
		}

		// get the start position of the keyword candidate vector
		candidateIte = candidate.begin();
	}

	if (candidateIte == candidate.end())
		return ModFalse;

	// set NP extraction result;
	normalized_ = candidateIte->getTermNorm();
	original_ = candidateIte->getTermOrig();

	ModSize len = candidateIte->getOrigVector().getSize();
	if(len != 0){
	// set other NP information
		if(normVector_){
			ModUnicodeString norm;
			ModUnicodeString orig;
			unsigned int pos;

			for(ModSize i = 0; i < len; i++) {
				// normalized morphs
				norm = candidateIte->getNormVector()[i];
				normVector_->pushBack(norm);

				// orginal morphs
				orig = candidateIte->getOrigVector()[i];
				origVector_->pushBack(orig);

				// part of speech
				pos = candidateIte->getPosVector()[i];
				posVector_->pushBack(pos);
			}
		}

		// calculate npCost
		ModNlpNpCost nlpNpCost(this);
		npCost_ = nlpNpCost.calculateNpCost(&(candidateIte->getCostVector()), calcCostMode);
	}

	// turn to next keyword
	candidateIte++;

	return ModTrue;
}

//
// FUNCTION public
// ModTerm::resetTerm -- プールした名詞句をリセットする
//
// NOTES
//	 プールに登録した名詞句をリセットする。
//
// ARGUMENTS
//	 なし
//
// RETURN
//	 なし
//
// EXCEPTIONS
//
void
ModTerm::resetTerm()
{
	candidate.clear();
	candidateIte = 0;
}

//
// FUNCTION public
// ModTerm::setStrTarget -- 解析文字列のセット
//
// NOTES
//	 解析文字列をセットする。
//
// ARGUMENTS
//	 ModUnicodeString	strTarget_		解析文字列
//
// RETURN
//	 なし
//
// EXCEPTIONS
//	 none
//
void
ModTerm::setStrTarget(
		ModUnicodeString	strTarget_)
{
	strTarget = strTarget_;
}

//
// FUNCTION public
// ModTerm::getStrTarget -- 解析文字列の取得
//
// NOTES
//	 解析文字列を取得する。
//
// ARGUMENTS
//	 なし
//
// RETURN
//	 ModUnicodeString	strTarget		解析文字列
//
// EXCEPTIONS
//	 none
//
ModUnicodeString
ModTerm::getStrTarget()
{
	return strTarget;
}

//
// FUNCTION public
// ModTerm::insertTerm -- 名詞句の登録
//
// NOTES
//	 名詞句を登録する。
//
// ARGUMENTS
//	 なし
//
// RETURN
//	 const ModTermElement& element
//
// EXCEPTIONS
//	 none
//
void
ModTerm::insertTerm(const ModTermElement& element)
{
	candidate.insertTerm(element);
}

//
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
