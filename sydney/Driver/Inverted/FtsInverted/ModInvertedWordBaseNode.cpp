// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedWordBaseNode.cpp -- 文字検索系ノードの実装
// 
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModInvertedWordBaseNode.h"

#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedFile.h"
#include "Inverted/ModInvertedDocumentLengthFile.h"
#else
#include "ModInvertedFile.h"
#include "ModInvertedDocumentLengthFile.h"
#endif

#ifndef DEL_BOOL
//
// FUNCTION public
// ModInvertedWordBaseNode::retrieve -- 検索の一括実行
//
// NOTES
// 検索式を一括実行する。
//
// ARGUMENTS
// ModInvertedBooleanResult& queryResult
//  Boolean検索結果オブジェクト
// ModInvertedQuery::EvaluateMode mode
//  評価モード
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedWordBaseNode::retrieve(BooleanResult& queryResult,
								  Query::EvaluateMode mode)
{
	// 検索結果をまず空にする。
	queryResult.clear();

	estimateDocumentFrequency();

	if (estimatedDocumentFrequency != UndefinedFrequency) {
		// 出現頻度と同じ分だけ予約しておく
		queryResult.reserve(estimatedDocumentFrequency);
	}

	// retrieve 処理
	ModInvertedDocumentID currentID = 1;

	// 検索語を含む文書を探す
	while (lowerBound(currentID, currentID, mode | Query::roughEvaluationFlag)
			== ModTrue) {
		// 検索語を含む文書を発見
		if (reevaluate(currentID) == ModTrue) {
			queryResult.pushBack(currentID);
		}
		++currentID;
	}

	// retrieve 終了。estimatedDocumentFrequency に正確な文書頻度を設定する
	this->estimatedDocumentFrequency = queryResult.getSize();
	this->retrieved = ModTrue;

	// queryResultに割り当てられたメモリを最小限にするためreserve()を実行
	queryResult.reserve(estimatedDocumentFrequency);
}
#endif
//
// FUNCTION public
// ModInvertedOperatorLocationNode::makeRoughPointer -- queryNodeForRoughEvaluation の作成
//
// NOTES
// このノードにはラフノードは生成しない。ただし、子ノードのラフノード生成、
// 親ノードのラフノード生成用のmap生成のためmakeRoghMapを行う。
// (子ノードは一つしかないはめchildren[0]にのみ行う)
//
// makeRoughMap()で出来たmapの内容を親ノード用のparentMapに追加する。
//
// ARGUMENTS
// const validateMode mode
//      有効化モード
// ModVector<ModInvertedQueryNode*>& parentRoughPointer
//      親ノード用のroughPointerのchildrenになるVector (結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedWordBaseNode::makeRoughPointer(
	const Query::ValidateMode mode,
	QueryNodePointerMap& parentMap, const ModInvertedQuery* Query)
{
	QueryNodePointerMap tmpMap;

	// 自分の子ノードのラフノード生成・親ノードのためにマップ生成
	if (this->makeRoughMap(mode, children[0], tmpMap, Query) != ModTrue) {
		// 子ノードが空集合
		// roughPointer の内容も空集合にしておく

		// emptySetNode は実際にはQueryNodeで持っているが、
		// あいまい除去のためAtomicNodeにしている
		this->queryNodeForRoughEvaluation
			= const_cast<ModInvertedQueryNode*>(ModInvertedAtomicNode::emptySetNode);

		return;
	}

	// 自分にはラフノードは作らない

	// tmpMap を親ノード用の map に挿入する
	parentMap.insert(tmpMap.begin(), tmpMap.end());
}

//
// FUNCTION public
// ModInvertedWordBaseNode::lowerBound -- 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
//
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する。
//
// ARGUMENTS
// ModInvertedDocumentID givenDocumentID
//      文書ID
// ModInvertedDocumentID foundDocumentID
//      結果格納用の文書IDオブジェクト
// ModInvertedQuery::EvaluateMode mode
//      評価モード
//
// RETURN
// 与えられた文書がが条件を満たす場合ModTrue、満たさない場合ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedWordBaseNode::lowerBound(DocumentID givenDocumentID,
									DocumentID& foundDocumentID,
									Query::EvaluateMode mode)
{
	// 既に調査済み？
	if (givenDocumentID >= this->lower) {
		if (this->upper == ModSizeMax) {
			return ModFalse;        // 既にEndに達っしている
		}
		if (givenDocumentID <= this->upper) {
			foundDocumentID = this->upper;
			return ModTrue;
		}
	}

	DocumentID tmpID(givenDocumentID);

	// 子ノードに対し粗いlowerBound
	while (children[0]->lowerBound(tmpID, foundDocumentID,
								   mode | Query::roughEvaluationFlag)) {
		// locationNodeを満たしているかチェック
		if (reevaluate(foundDocumentID) == ModTrue) {
			this->upper = foundDocumentID;
			this->lower = givenDocumentID;
			return ModTrue;
		}
		tmpID = foundDocumentID + 1;
	}
	this->upper = ModSizeMax;
	this->lower = givenDocumentID;

	return ModFalse;
}

//
// FUNCTION public
// ModInvertedWordBaseNode::checkQueryNode -- 子ノードの数をチェックする
//
// NOTES
// 有効化の最後に呼び出されて、子ノードの数をチェックする。もし異常で
// あれば例外を投げる。
//
// ここでは InternalNode 用の定義がされている。子ノードの数が 1 以外の場合
// 異常と判断する。
//
// また、子ノードに対しても再帰的に本関数を呼び出す。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
void
ModInvertedWordBaseNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_
)
{
	if(needDF_ == ModTrue) {
		setNeedDF(ModTrue);
	}

	ModSize s = this->children.getSize();
	if (s != 1) {
		// 子ノードが 1 以外なのは異常
		ModErrorMessage << "children size is not 1." << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}

	ModUnicodeString termString;
	if (setStringInChildren_ == ModTrue) {
		getQueryString(termString, ModTrue, ModTrue, ModTrue, ModFalse);
		query_->insertTermNode(termString,
		static_cast<ModInvertedQueryInternalNode*>(this));

		ModSize avelen(query_->getAverageDocumentLength());
		if (avelen != 0) {
			ScoreCalculator* calculator = getScoreCalculator();
			; ModAssert(calculator != 0);
			calculator->setAverageDocumentLength(avelen);
		}
	}

	children[0]->checkQueryNode(query_, ModFalse, ModFalse);
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
