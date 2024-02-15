// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorScaleNode.h -- ModInvertedOperatorScaleNode の宣言
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedOperatorScaleNode_H__
#define __ModInvertedOperatorScaleNode_H__

#include "ModInvertedQueryInternalNode.h"
#include "ModInvertedQueryNode.h"
#include "ModInvertedTermLeafNode.h"
#include "ModOstrStream.h"

//
// CLASS
// ModInvertedRankingScale -- ランキングスケール演算子ノード
//
// NOTES
//ランキング検索式内部表現ノードクラスと検索式内部表現中間ノードクラスの
// 派生クラスである。
// あらかじめ指定された因子に基づき、子ノードで算出されたスコアを変更する。
//
class
ModInvertedOperatorScaleNode
	: public ModInvertedQueryInternalNode
{
public:
	// コンストラクタ
	ModInvertedOperatorScaleNode(const double,const  ModUInt32 resultType_);
	ModInvertedOperatorScaleNode(
		const ModInvertedOperatorScaleNode& originalNode,
		const  ModUInt32 resultType_);

#ifndef DEL_BOOL
	// 検索の一括実行
	virtual void retrieve(BooleanResult& queryResult,
						  Query::EvaluateMode mode);
#endif
	// 与えられた文書が検索条件を満たすかどうかの検査
	virtual ModBoolean evaluate(DocumentID documentID,
								Query::EvaluateMode mode);

	// 与えられた文書のスコアを計算する
	virtual ModBoolean evaluateScore(const DocumentID documentID,
									 DocumentScore& score,
									 Query::EvaluateMode mode);

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
	// 与えられた文書のスコアを計算する(位置も計算)
	virtual ModBoolean evaluateScore(const DocumentID documentID,
									 DocumentScore& score,
									 LocationIterator*& locations,
									 Query::EvaluateMode mode,
									 ModInvertedQueryNode* givenEndNode = 0);
#endif

	// 与えられた文書が検索条件を満たすかどうかの検査
	virtual ModBoolean lowerBound(DocumentID givenID,
								  DocumentID& foundID,
								  Query::EvaluateMode mode);

	// lowerBoundのランキング版（スコアも計算する）
	virtual ModBoolean lowerBoundScore(const DocumentID givenID,
									   DocumentID& foundID, DocumentScore& score,
									   Query::EvaluateMode mode);

	// 文書頻度を見積もる
	virtual ModSize estimateDocumentFrequency();

	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery& query);

	// 演算子を表す文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
							  const ModBoolean withCalOrCombName,
							  const ModBoolean withCalOrCombParam) const;

	// 子ノードリストの平坦化
	virtual void flattenChildren(const QueryNodePointerMap& sharedNodeMap,
								 const ModBoolean isChildOfWindowNode);

	// queryNodeForRoughEvaluation の作成
	virtual void makeRoughPointer(const Query::ValidateMode,
								  QueryNodePointerMap&,
								  const ModInvertedQuery* Query);

	// 文書頻度を得る
	virtual ModSize getDocumentFrequency(Query::EvaluateMode mode);

	// 有効化の最後に子ノードの数をチェックする
	virtual void checkQueryNode(ModInvertedQuery*, 
								const ModBoolean,
								const ModBoolean);

	// スコア計算の第２ステップ
	void doSecondStepInRetrieveScore(
		ModInvertedSearchResult* &result);
	void doSecondStepInRetrieveScore(){
		ModInvertedQueryInternalNode::doSecondStepInRetrieveScore();
	}

	// Get search term list
	virtual void getSearchTermList(
		ModInvertedQuery::SearchTermList& vecSearchTerm_,
		ModSize uiSynonymID_) const;
	
protected:
	virtual ModBoolean reevaluate(DocumentID documentID);
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* givenEndNode = 0);

	// 子ノードリストの並べ替え
	void sortChildren(const ModInvertedQuery::ValidateMode mode);

	// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
	// Rankig InternalNodeはCombiner LeafNodeはCalculator
	// Atomic はCalculator
	virtual void getCalculatorOrCombinerName(ModUnicodeString& name,
											 const ModBoolean withParams) const;

	// ランキング検索のスコア計算第２ステップで使用するlowerBound
	ModBoolean lowerBoundScoreForSecondStep(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score);

private:
	// スコア係数
	DocumentScore scale;
};

// 
// FUNCTION public
// ModInvertedOperatorScaleNode::ModInvertedOperatorScaleNode -- コンストラクタ
// 
// NOTES
// 新しい検索式表現ノードオブジェクトを生成する。
//
// ARGUMENTS
// doble scale_
//		スケール因子
// 
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedOperatorScaleNode::ModInvertedOperatorScaleNode(
	const double scale_,const  ModUInt32 resultType_)
	:ModInvertedQueryInternalNode(QueryNode::operatorScaleNode,resultType_), scale(scale_)
{
}

//
// FUNCTION public
// ModInvertedOperatorScaleNode::ModInvertedOperatorScaleNode -- コンストラクタ(コピーコンストラクタ)
//
// NOTES
// コピーコンストラクタ。originalNodeのコピーを作成する。
//
// ARGUMENTS
// const ModInvertedOperatorScaleNode& originalNode
//		オリジナルのModInvertedOperatorScaleNode
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedOperatorScaleNode::ModInvertedOperatorScaleNode(
	const ModInvertedOperatorScaleNode& originalNode,const  ModUInt32 resultType_)
:ModInvertedQueryInternalNode(QueryNode::operatorScaleNode,resultType_)
{
	this->scale = originalNode.scale;
}

//
// FUNCTION public
// ModInvertedOperatorScaleNode::retrieve -- 検索の一括実行
//
// NOTES
// 検索式を一括実行する(実際には子ノードの評価)。
//
// ARGUMENTS
// ModInvertedBooleanResult& queryResult
//		Boolean検索結果オブジェクト
// ModInvertedQuery::EvaluateMode mode
//		評価モード 
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
#ifndef DEL_BOOL
inline void
ModInvertedOperatorScaleNode::retrieve(BooleanResult& queryResult,
									   Query::EvaluateMode mode)
{
	children[0]->retrieve(queryResult, mode);
}
#endif
// 
// FUNCTION public
// ModInvertedOperatorScaleNode::lowerBound -- 検索式を満たす文書のうち、文書IDが与えられた値以上で最小の文書の検索
// 
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する(実際には子ノードを検査)。
// 
// ARGUMENTS
// ModInvertedDocumentID givenID
//		文書IDの下限
// ModInvertedDocumentID& foundID
//		結果格納用の文書IDオブジェクト
// Mode evaluationMode
//		評価モード
// 
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModInvertedOperatorScaleNode::lowerBound(DocumentID givenID,
										 DocumentID& foundID,
										 Query::EvaluateMode mode)
{
	return children[0]->lowerBound(givenID, foundID, mode);
}

//
// FUNCTION public
// ModInvertedOperatorScaleNode::estimateDocumentFrequency -- 文書頻度の見積もり
// 
// NOTES
// 条件を満たす文書数を見積もる。
// 
// ARGUMENTS
// なし
// 
// RETURN
// 見積もった文書頻度
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModSize
ModInvertedOperatorScaleNode::estimateDocumentFrequency()
{
	if (this->estimatedDocumentFrequency == UndefinedFrequency) {
		estimatedDocumentFrequency = children[0]->estimateDocumentFrequency();
	}
	return this->estimatedDocumentFrequency;
}

//
// FUNCTION public
// ModInvertedOperatorScaleNode::getDocumentFrequency -- 文書頻度を得る
//
// NOTES
// 子ノードのgetDocumentFrequencyを実行
//
// ARGUMENTS
// Query::EvaluateMode mode
//		評価モード
//
// RETURN
// 求めた文書頻度
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModSize
ModInvertedOperatorScaleNode::getDocumentFrequency(Query::EvaluateMode mode)
{
	return children[0]->getDocumentFrequency(mode);
}

//
//
// FUNCTION public
// ModInvertedRankingOperatorSclaeNode::flattenChildren -- 子ノードリストの平坦化
//
// NOTES
// 実際に平坦化を行なうのはAndとOrだけなので、このクラスではなにもせず、
// 子ノードのflattenChildrenを呼ぶだけ
//
// ARGUMENTS
// const QueryNodePointerMap& sharedNodeMap
//		OR標準形変換時に共有しているノードが登録されているマップ変数
// const ModBoolean isChildOfWindowNode
//		windowノードを親にもっている場合 ModTrue となる
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline void
ModInvertedOperatorScaleNode::flattenChildren(
	const QueryNodePointerMap& sharedNodeMap,
	const ModBoolean isChildOfWindowNode)
{
	ModInvertedQueryInternalNode::flattenChildren(sharedNodeMap,isChildOfWindowNode);
}

//
// FUNCTION public
// ModInvertedOperatorScaleNode::evaluate -- 与えられた文書が検索式を満たすかどうかの検査
//
// NOTES
// 与えられた文書が検索式を満たすかどうか検査する(実際には子ノードの検査)。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//      文書ID
// Mode mode
//      評価モード
//
// RETURN
// 与えられた文書が検索式を満たす場合 true、満たさない場合 false
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModInvertedOperatorScaleNode::evaluate(DocumentID documentID,
									   Query::EvaluateMode mode)
{
	// 子ノードは一つ
	return children[0]->evaluate(documentID, mode);
}

//
// FUNCTION public
// ModInvertedOperatorScaleNode::getSearchTermList --
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
inline void
ModInvertedOperatorScaleNode::getSearchTermList(
	ModInvertedQuery::SearchTermList& vecSearchTerm_,
	ModSize uiSynonymID_) const
{
	// 子ノードは一つ
	return children[0]->getSearchTermList(vecSearchTerm_, uiSynonymID_);
}

//
// FUNCTION protected
// ModInvertedOperatorScaleNode::reevaluate -- 正確な再評価
//
// NOTES
// 粗い evaluate 満足を前提とした、正確な再評価
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedLocationListIterator*& locations
//		位置情報（結果格納用）
// ModInvertedQueryNode* giveEndNode
//		ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
inline ModBoolean
ModInvertedOperatorScaleNode::reevaluate(DocumentID documentID)
{
	return children[0]->reevaluate(documentID);
}

inline ModBoolean
ModInvertedOperatorScaleNode::reevaluate(DocumentID documentID,
										 LocationIterator*& locations,
										 ModSize& uiTF_,
										 ModInvertedQueryNode* givenEndNode)
{
	return children[0] ->reevaluate(documentID, locations, uiTF_);
}

//
// FUNCTION protected
// ModInvertedOperatorScaleNode::sortChildren -- 子ノードリストの並べ替え
//
// NOTES
// Scaleノードは子ノードが一つしかないため並べ替えは不要。子ノードのsortChlidren
// を行う。
//
// ARGUMENTS
// const ModInvertedQuery::ValidateMode mode
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline void
ModInvertedOperatorScaleNode::sortChildren(
	const ModInvertedQuery::ValidateMode mode)
{
	children[0]->sortChildren(mode);
}

//
// FUNCTION protected
// ModInvertedOperatorScaleNode::getCalculatorOrCombinerName -- 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
//
// NOTES
// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す。RankingScaleは
// Rankingであるが、calcultor/combinerは用いるため空文字列を返す。
//
// ARGUMENTS
// ModString& name
//		scoreCalculator名(結果格納用)。
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedOperatorScaleNode::getCalculatorOrCombinerName(
	ModUnicodeString& name,
	const ModBoolean withParams) const
{
	// calcultor/combinerは用いないため空文字列を返す。
	name = "";
}

#endif //__ModInvertedOperatorScaleNode_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
