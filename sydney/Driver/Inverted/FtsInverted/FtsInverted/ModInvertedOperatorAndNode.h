// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorAndNode.h -- 積集合(AND)ノード
// 
// Copyright (c) 1998, 1999, 2000, 2001, 2002, 2004, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedOperatorAndNode_H__
#define __ModInvertedOperatorAndNode_H__

#include "ModInvertedQueryInternalNode.h"

//
// CLASS ModInvertedOperatorAndNode -- 積集合演算を表すノード
//
// NOTES
// 検索式内部表現中間ノードクラスの派生クラスで、積集合演算を表す。
//
class
ModInvertedOperatorAndNode : public ModInvertedQueryInternalNode
{
public:
	// コンストラクタ
	ModInvertedOperatorAndNode(const ModUInt32 resultType_=0);

	// デストラクタ
	virtual ~ModInvertedOperatorAndNode();

#ifndef DEL_BOOL
	// 検索の一括実行
	virtual void retrieve(BooleanResult& queryResult,
						  Query::EvaluateMode mode);
#endif
	// 与えられた文書が検索条件を満たすかどうかの検査
	virtual ModBoolean evaluate(DocumentID documentID,
								Query::EvaluateMode mode);

	// 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
	virtual ModBoolean lowerBound(DocumentID givenDocumentID,
								  DocumentID& foundDocumentID,
								  Query::EvaluateMode mode);

#if 1	// NEW

	// 与えられた文書のスコアを計算する
	virtual ModBoolean evaluateScore(const DocumentID documentID,
								DocumentScore& score,
								Query::EvaluateMode mode);

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
	// 与えられた文書のスコアを計算する（位置も計算する）
	virtual ModBoolean evaluateScore(const DocumentID documentID,
								DocumentScore& score,
								LocationIterator*& locations,
								Query::EvaluateMode mode,
								ModInvertedQueryNode* givenEndNode = 0);
#endif

	// lowerBoundのランキング版（スコアも計算する）
	virtual ModBoolean lowerBoundScore(const DocumentID givenDocumentID,
								DocumentID& foundDocumentID,
								DocumentScore& score,
								Query::EvaluateMode mode);

	// RankingOr,RankingAndの場合は子ノード数の分だけsocresをリザーブ
	// virtual void reserveScores(const ModInvertedQueryNode* node);
	virtual void reserveScores();

#endif	// NEW

	// 文書頻度を見積もる
	virtual ModSize estimateDocumentFrequency();

	// 子ノードリストの平坦化 (例 #and(#and(X,Y),Z) → #and(X,Y,Z))
	virtual void flattenChildren(const QueryNodePointerMap& sharedNodeMap,
								 const ModBoolean isChildOfWindowNode);

	// queryNodeForRoughEvaluation の作成
	virtual void makeRoughPointer(const Query::ValidateMode,
								  QueryNodePointerMap&,
								  const ModInvertedQuery* Query);

	// sortFactor の計算
	virtual ModSize calcSortFactor();

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
			const ModBoolean withCalOrCombName,
			const ModBoolean withCalOrCombParam) const;

	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery& query);

	// 有効化
	virtual void validate(InvertedFile* invertedFile,
			const ModInvertedQuery::ValidateMode mode,
			ModInvertedQuery* rQuery);

	// ランキング検索のスコア計算第１ステップ
	virtual void doFirstStepInRetrieveScore(
		ModInvertedBooleanResult *expungedDocumentId,
		const ModInvertedDocumentID maxDocumentId);

	// ランキング検索のスコア計算第２ステップ
	virtual void doSecondStepInRetrieveScore(
		ModInvertedRankingResult* &result);
	virtual void doSecondStepInRetrieveScore();

	// ランキング検索のスコア計算第２ステップで使用するlowerBound
	virtual ModBoolean lowerBoundScoreForSecondStep(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score);

    virtual void checkQueryNode(ModInvertedQuery*,
								const ModBoolean,
								const ModBoolean);

	// Get search term list
	virtual void getSearchTermList(
		ModInvertedQuery::SearchTermList& vecSearchTerm_,
		ModSize uiSynonymID_) const;
	
protected:
	// 粗い evaluate 満足を前提とした、正確な再評価
	// 二番目の形式は、位置情報が必要な場合に用いられる。
	virtual ModBoolean reevaluate(DocumentID documentID);
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* giveEndNode = 0);

	// 子ノードリストの並べ替え
	virtual void sortChildren(const ModInvertedQuery::ValidateMode mode);

	// コンストラクタ (2)
	ModInvertedOperatorAndNode(const NodeType nType,const  ModUInt32 resultType_);

private:
	// 子ノードのスコア
	ModVector<DocumentScore> scores;

	// コピーコンストラクタ (使用禁止)
	ModInvertedOperatorAndNode(const AndNode& original);
	AndNode& operator=(const AndNode& original);
};

//
// FUNCTION public
// ModInvertedOperatorAndNode::ModInvertedOperatorAndNode -- ANDノードを生成する
//
// NOTES
// 積集合(AND)ノードを生成する。
//
// ARGUMENTS
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedOperatorAndNode::ModInvertedOperatorAndNode(const  ModUInt32 resultType_)
	: ModInvertedQueryInternalNode(ModInvertedQueryNode::operatorAndNode,resultType_)
{ }

//
// FUNCTION protected
// ModInvertedOperatorAndNode::ModInvertedOperatorAndNode -- ANDノードを生成する
//
// NOTES
// 積集合(AND)ノードを生成する。NodeType引数付き。
// OrderedDistanceNode や WindowNode から呼び出される。
//
// ARGUMENTS
// const NodeType nType
//		ノードタイプ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedOperatorAndNode::ModInvertedOperatorAndNode(const NodeType nType,const  ModUInt32 resultType_)
	: ModInvertedQueryInternalNode(nType,resultType_)
{}

#endif //__ModInvertedOperatorAndNode_H__

//
// Copyright (c) 1998, 1999, 2000, 2001, 2002, 2004, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
