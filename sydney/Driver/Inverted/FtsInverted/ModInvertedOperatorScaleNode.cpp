// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorScaleNode.cpp -- ModInvertedOperatorScaleNodeの実装
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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

#include "ModInvertedOperatorScaleNode.h"
#include "ModInvertedBooleanResultLeafNode.h"
#include "ModInvertedRankingResultLeafNode.h"


// 
// FUNCTION public
// ModInvertedOperatorScaleNode::evaluateScore -- 与えられた文書の検索式に対する適合度の計算
// 
// NOTES
// 与えられた文書が検索式にどの程度適合しているかを表すスコアを求める。
// (実際には子ノードの評価を行っている)
// 
// 
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// DocumentScore& score,
//		スコア(結果格納用)
// Query::EvaluateMode mode
// 		評価モード	
// 
// RETURN
// ブーリアン検索条件(子ノード)にマッチしていれば True,
// アンマッチであれば False
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedOperatorScaleNode::evaluateScore(const DocumentID id,
											DocumentScore& score,
											Query::EvaluateMode mode)
{
	// 子ノードは一つ
	if (children[0]->evaluateScore(id, score, mode) == ModTrue) {
		// 子ノードのスコアをscale倍
		score *= scale;
		return ModTrue;
	} else {
		return ModFalse;
	}
}

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
// 
// FUNCTION public
// ModInvertedOperatorScaleNode::evaluateScore -- 与えられた文書の検索式に対する適合度の計算
// 
// NOTES
// 与えられた文書が検索式にどの程度適合しているかを表すスコアを求め、
// スコアが 0 でない場合には、与えられた出現位置情報オブジェクトに
// 出現位置情報を格納する。
// 
// ARGUMENTS
// ModInvertedDocumentID documentID
// 	文書ID
// ModInvertedTermPositionList& termPositions
//	出現位置情報オブジェクト
// ModInvertedQueryNode* givenEndNode
// 		ここでは未使用。orderedDistanceでのみ使用。
// 
// RETURN
// 求めたスコア
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedOperatorScaleNode::evaluateScore(const DocumentID documentID,
											DocumentScore& score,
											LocationIterator*& locations,
											Query::EvaluateMode mode,
											ModInvertedQueryNode* givenEndNode)
{
	// 子ノードは一つ
	if (children[0]
			->evaluateScore(documentID, score, locations, mode) == ModTrue) {
		// スコアをscale倍
		score *= scale;
		return ModTrue;
	} else {
		return ModFalse;
	}
}
#endif

// 
// FUNCTION public
// ModInvertedOperatorScaleNode::lowerBoundScore -- スコア付き lowerBound
// 
// NOTES
// 文書IDが与えられた値以上で、検索式との適合度が 0 でない文書の内、
// 文書ID最小のものを検索し、そのような文書が存在する場合は、与えられた
// 文書IDオブジェクトに結果を格納し、その文書の適合度を返す。
// 
// ARGUMENTS
// ModInvertedDocumentID givenID
//		文書IDの下限
// ModInvertedDocumentID& foundID
//		結果格納用の文書IDオブジェクト
// DocumentScore& score,
//		スコア(結果格納用)
// Query::EvaluateMode mode
//		評価モード 
// 
// RETURN
// 文書IDが foundDocumentID である文書について求めたスコア
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedOperatorScaleNode::lowerBoundScore(const DocumentID givenID,
													 DocumentID& foundID,
													 DocumentScore& score,
													 Query::EvaluateMode mode)
{
	// 子ノードは一つ
	if (children[0]->lowerBoundScore(givenID, foundID, score, mode) == ModTrue) {
		// スコアをscale倍
		score *= scale;
		return ModTrue;
	} else {
		return ModFalse;
	}
}

//
// FUNCTION public
// ModInvertedQueryNode::duplicate -- 自分のコピーを作成する
//
// NOTES
// 自分のコピーを作成する。Query::から呼ばれ、検索木を上からたどり、
// 検索木のコピーを作成する際に使用される。
// QueryNode::dupulicateをオーバーライド。TermLeafNodeのコピーを作る。
//
// ARGUMENTS
// const ModInvertedQuery&
//		クエリ。
//
// RETURN
// 生成したコピーのノードのポインタ
//
// EXCEPTIONS
// ModInvertedErrorQueryValidateFail
//
ModInvertedQueryNode*
ModInvertedOperatorScaleNode::duplicate(const ModInvertedQuery& rQuery)
{
	// コピーコンストラクタで自分の複製を作成
	ModInvertedOperatorScaleNode* node 
			= new ModInvertedOperatorScaleNode(*this,firstStepResult->getType());

	ModVector<ModInvertedQueryNode*>::Iterator child = this->children.begin();
	ModVector<ModInvertedQueryNode*>::Iterator end = this->children.end();

	// 子ノードのduplicateの結果
	ModInvertedQueryNode* newChild;

	// scaleNodeの子ノードは一つだけ
	newChild = children[0]->duplicate(rQuery);

	// 子ノードを追加
	node->insertChild(newChild);


	return node;
}

// FUNCTION public
// ModInvertedOperatorScaleNode::prefixString
//				-- 演算子を表わす文字列(#andR)を返す
//
// NOTES
// QueryNodeで定義された内容をオーバライドする
// 演算子を表わす文字列(#andR)を返す
//
// ARGUMENTS
// ModString& prefix
//		演算子を表わす文字列を返す(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModInvertedOperatorScaleNode::prefixString(ModUnicodeString& prefix,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam) const
{
	ModOstrStream out;
	out << "#scale[" << this->scale << "]";
	prefix = out.getString();
}

//
// FUNCTION public
// ModInvertedOperatorScaleNode::makeRoughPointer -- queryNodeForRoughEvaluation の作成
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
//		有効化モード
// ModVector<ModInvertedQueryNode*>& parentRoughPointer
//		親ノード用のroughPointerのchildrenになるVector (結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModInvertedOperatorScaleNode::makeRoughPointer(
			const Query::ValidateMode mode,
			QueryNodePointerMap& parentMap, const ModInvertedQuery* Query)
{
	QueryNodePointerMap tmpMap;
	// children[0]->makeRoughPointer(mode, dummy, Query);

	// 自分の子ノードのラフノード生成・親ノードのためにマップ生成
	if (this->makeRoughMap(mode, children[0], tmpMap, Query) != ModTrue) {
		// 子ノードが空集合
		// roughPointer の内容も空集合にしておく
		this->queryNodeForRoughEvaluation = const_cast<ModInvertedQueryNode*>(QueryNode::emptySetNode);

		return;
	}

	// 自分にはラフノードは作らない 

	// tmpMap を親ノード用の map に挿入する
	parentMap.insert(tmpMap.begin(), tmpMap.end());
}

//
// FUNCTION public
// ModInvertedOperatorScaleNode::checkQueryNode -- 子ノードの数をチェックする
//
// NOTES
// 有効化の最後に呼び出されて、子ノードの数をチェックする。もし異常で
// あれば例外を投げる。
//
// ここでは RankingOperatorScaleNode 用の定義がされている。子ノードの数が 1
// 以外の場合異常と判断する。
//
// また、子ノードに対しても再帰的に本関数を呼び出す。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorQueryValidateFail
//		子ノード数が 1 以外であった
//
void
ModInvertedOperatorScaleNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_
	)
{
	ModSize s = this->children.getSize();
	if (s != 1) {
		// 子ノードが 1 以外なのは異常
		ModErrorMessage << "children size is not 1." << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}

	this->children[0]->checkQueryNode(query_, setStringInChildren_, needDF_);
}

//
// FUNCTION
// ModInvertedOperatorScaleNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
//
// NOTES
// ランキング検索で、スコア計算の第２ステップのみを実施する。
// 子ノードに対して第２ステップを実行し、得られたスコアをscale倍にする。
//
// ARGUMENTS
// ModInvertedRankingResult* result_
//      検索結果
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedOperatorScaleNode::doSecondStepInRetrieveScore(ModInvertedSearchResult *&result_)
{
	// 子ノード(一つ)に対して同関数を呼ぶ
	children[0]->doSecondStepInRetrieveScore(result_);

	// 子ノードの結果のスコアをscale倍する
	if(result_->getType() == ((1 << _SYDNEY::Inverted::FieldType::Score) |
							(1 << _SYDNEY::Inverted::FieldType::Rowid)))
	{
		// 通常のranking検索なので、高速版を使用する
		// 高速版
		ModInvertedSearchResultScore *r = (ModInvertedSearchResultScore *)result_;
		for (ModInvertedSearchResultScore::Iterator e(r->begin());
			 e != r->end(); ++e)
		{
			(*e).second *= scale;
		}
	}
	else
	{	// 汎用版
		for(ModSize i = 0 ; i < result_->getSize(); i++){
			result_->setScore(i,result_->getScore(i)*scale);
		}
	}
}

//
// FUNCTION
// ModInvertedOperatorScaleNode::lowerBoundScoreForSecondStep() -- ランキング検索の第２ステップで使用するlowerBound
//
// NOTES
// ランキング検索のスコア計算第２ステップで使用されるlowerBound。
// 子ノードに対してlowerBoundScoreForSecondStep()を行い、
// 得られたスコアをscale倍して返す。
//
// ARGUMENTS
// ModInvertedDocumentID givenID,
//      文書IDの下限
// ModInvertedDocumentID& foundID,
//      結果格納用の文書IDオブジェクト
// ModInvertedDocumentScore& score);
//      スコア(結果格納用)
//
// RETURN
// 求めたスコア
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedOperatorScaleNode::lowerBoundScoreForSecondStep(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
	// 子ノード(一つ)に対して同関数を呼ぶ
	if(children[0]->lowerBoundScoreForSecondStep(givenID, foundID, score)
		== ModFalse) {
		return ModFalse;
	}

	// 子ノードの結果のスコアをscale倍する
	score *= scale;

	return ModTrue;
}

//
// Copyright (c) 1999, 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
