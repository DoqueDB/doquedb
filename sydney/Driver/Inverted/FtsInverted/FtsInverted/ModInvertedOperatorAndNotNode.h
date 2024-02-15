// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorAndNotNode.h -- 差集合ノード
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

#ifndef __ModInvertedOperatorAndNotNode_H__
#define __ModInvertedOperatorAndNotNode_H__

#include "ModInvertedQuery.h"
#include "ModInvertedQueryInternalNode.h"

//
// CLASS
// ModInvertedOperatorAndNotNode -- 差集合演算を表すノード
//
// NOTES
// 検索式内部表現中間ノードクラスの派生クラスで、差集合演算を表す。
//
class
ModInvertedOperatorAndNotNode : public ModInvertedQueryInternalNode {
public:
	// コンストラクタ
	ModInvertedOperatorAndNotNode(const  ModUInt32 resultType_);

	// デストラクタ
	virtual ~ModInvertedOperatorAndNotNode();

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

#ifndef DEL_BOOL
	// もとのブーリアン検索用 retrieve/evalaute/lowerBound
	// 検索の一括実行
	void booleanRetrieve(BooleanResult& queryResult,
						 Query::EvaluateMode mode);
#endif
	// 与えられた文書が検索条件を満たすかどうかの検査
	ModBoolean booleanEvaluate(DocumentID documentID,
							   Query::EvaluateMode mode);

	// 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
	ModBoolean booleanLowerBound(DocumentID givenDocumentID,
								 DocumentID& foundDocumentID,
								 Query::EvaluateMode mode);

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

	// 文書頻度を見積もる
	ModSize estimateDocumentFrequency();

	// 子ノードの追加
	void insertChild(ModInvertedQueryNode* child);

	// 子ノードリストの平坦化
	virtual void flattenChildren(const QueryNodePointerMap& sharedNodeMap,
								 const ModBoolean isChildOfWindowNode);

	// queryNodeForRoughEvaluation の作成
	virtual void makeRoughPointer(const Query::ValidateMode,
								  QueryNodePointerMap&,
								  const ModInvertedQuery* Query);

	// 中間ノードの共有化
	virtual ModSize sharedQueryNode(QueryNodeMap& globalNodeMap,
									QueryNodePointerMap& nodePointerMap);

	// TermLeafNode を消去して SimpleToken/OrderedDistance にする
	virtual ModBoolean eraseTermLeafNode(QueryNode*& node, Query& query);

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
			const ModBoolean withCalOrCombName,
			const ModBoolean withCalOrCombParam) const;

	// スコア否定器をセット
	void setGivneNegatorAsScoreNegator(ScoreNegator* negator);

	// スコア否定器をセットする
	virtual void setScoreNegator(
				const ModUnicodeString& negatorName);

	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery& query);

	virtual void validate(InvertedFile* invertedFile,
				const ModInvertedQuery::ValidateMode mode,
				ModInvertedQuery* rQuery);

	// 有効化の最後に子ノードの数をチェックする
	virtual void checkQueryNode(ModInvertedQuery*,
								const ModBoolean,
								const ModBoolean);

	// ランキング検索のスコア計算第２ステップで使用するlowerBound
	ModBoolean lowerBoundScoreForSecondStep(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score);

	// Get search term list
	virtual void getSearchTermList(
		ModInvertedQuery::SearchTermList& vecSearchTerm_,
		ModSize uiSynonymID_) const;
	
protected:
	void retrieveScoreWithoutNegator(ModInvertedSearchResult* queryResult,
									Query::EvaluateMode mode);
	void retrieveScoreWithNegator(ModInvertedSearchResult* queryResult,
									Query::EvaluateMode mode);

	// 粗い evaluate 満足を前提とした、正確な再評価
	virtual ModBoolean reevaluate(DocumentID documentID);
	// 位置情報リストが必要版
	// ただし、位置情報リストが取得できない場合は、TFを取得する
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* givenQueryNode = 0);

	virtual ModBoolean booleanReevaluate(DocumentID documentID);
	virtual ModBoolean booleanReevaluate(
		DocumentID documentID,
		LocationIterator*& locations,
		ModSize& uiTF_,
		ModInvertedQueryNode* givenQueryNode = 0);

	DocumentID child0Upper;		// children[0] の lowerBound() 結果
	DocumentID child0Lower;		// children[0] の lowerBound() 入力値
	DocumentID child1Upper;		// children[1] の lowerBound() 結果
	DocumentID child1Lower;		// children[1] の lowerBound() 入力値

private:
	// スコア否定器
	ModInvertedRankingScoreNegator* scoreNegator;

	// コピーコンストラクタ (使用禁止)
	ModInvertedOperatorAndNotNode(const AndNotNode& original);
	AndNotNode& operator=(const AndNotNode& original);
};

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::ModInvertedOperatorAndNotNode -- And-Not ノードの生成
//
// NOTES
// And-Not ノードの生成
//
// ARGUMENTS
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedOperatorAndNotNode::ModInvertedOperatorAndNotNode(const  ModUInt32 resultType_)
	: ModInvertedQueryInternalNode(QueryNode::operatorAndNotNode,resultType_),
	  child0Upper(0), child0Lower(1), child1Upper(0), child1Lower(1),
	  scoreNegator(0)
{}

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::flattenChildren -- 子ノードリストの平坦化
//
// NOTES
// 子ノードリストの平坦化。
// ModInvertedQueryInternalNodeのflattenChildrenを呼ぶ。
//
// ARGUMENTS
// const QueryNodePointerMap& sharedNodeMap
//  OR標準形変換時に共有しているノードが登録されているマップ変数
//
// const ModBoolean isChildOfWindowNode
//      windowノードを親にもっている場合 ModTure となる
//
// RETURN
// なし
//
inline void
ModInvertedOperatorAndNotNode::flattenChildren(const QueryNodePointerMap& sharedNodeMap,
											   const ModBoolean isChildOfWindowNode)
{
	ModInvertedQueryInternalNode::flattenChildren(sharedNodeMap,isChildOfWindowNode);
}

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::getSearchTermList --
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
ModInvertedOperatorAndNotNode::getSearchTermList(
	ModInvertedQuery::SearchTermList& vecSearchTerm_,
	ModSize uiSynonymID_) const
{
	// Ignore not-node.
	return children[0]->getSearchTermList(vecSearchTerm_, uiSynonymID_);
}

#endif //__ModInvertedOperatorAndNotNode_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
