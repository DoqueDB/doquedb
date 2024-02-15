// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorOrNode.h -- 和集合ノード
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

#ifndef __ModInvertedOperatorOrNode_H__
#define __ModInvertedOperatorOrNode_H__

#include "ModInvertedQueryInternalNode.h"
#include "ModInvertedQuery.h"
#include "ModInvertedRankingScoreCalculator.h"
#include "ModInvertedSearchResult.h"
         
class ModInvertedOrLocationListIterator;

//
// CLASS
// ModInvertedOperatorOrNode -- 和集合演算を表すノード
//
// NOTES
// 検索式内部表現中間ノードクラスの派生クラスで、和集合演算を表す。
//
class
ModInvertedOperatorOrNode : public ModInvertedQueryInternalNode
{
public:
	typedef ModInvertedSearchResultScore RankingResult;
	typedef ModVector<ModInvertedLocationListIterator*> LocationIterators;
	typedef ModInvertedOrLocationListIterator OrLocationIterator;

	// コンストラクタ
	ModInvertedOperatorOrNode(const  ModUInt32 resultType_);

	// デストラクタ
	virtual ~ModInvertedOperatorOrNode();

#ifndef DEL_BOOL
	// 検索の一括実行
	void retrieve(BooleanResult& queryResult,
				  Query::EvaluateMode mode);
#endif
	// 与えられた文書が検索条件を満たすかどうかの検査
	virtual ModBoolean evaluate(DocumentID documentID,
								Query::EvaluateMode mode);

	// 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
	virtual ModBoolean lowerBound(DocumentID givenDocumentID,
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

	virtual ModSize sharedQueryNode(QueryNodeMap& globalNodeMap,
										QueryNodePointerMap& nodePointerMap);

	// 文書頻度を見積もる
	ModSize estimateDocumentFrequency();

	// 子ノードリストの平坦化 (例 #or(#or(X,Y),Z) → #or(X,Y,Z))
	virtual void flattenChildren(const QueryNodePointerMap& sharedNodeMap,
								 const ModBoolean isChildOfWindowNode);

	// shortWordLength をセットする
	void setShortWordLength(const ModSize shortWordLength_);

	// shortWordLength を取得する
	ModSize getShortWordLength() const;

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
							  const ModBoolean withCalOrCombName,
							  const ModBoolean withCalOrCombParam) const;

	// RankingOr,RankingAndの場合は子ノード数の分だけsocresをリザーブ
	virtual void reserveScores();

	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery& query);

	// 有効化
	virtual void validate(InvertedFile* invertedFile,
						  const ModInvertedQuery::ValidateMode mode,
						  ModInvertedQuery* rQuery);

	// queryNodeForRoughEvaluation の作成
	virtual void makeRoughPointer(const Query::ValidateMode,
								  QueryNodePointerMap&,
								  const ModInvertedQuery* Query);

	// TermLeafNode を消去して SimpleToken/OrderedDistance にする
	virtual ModBoolean eraseTermLeafNode(QueryNode*& node, Query& query);

	virtual ModBoolean isShortWordOrNode() const;

	// originalTermStringを取得する
	virtual void setOriginalString(const ModUnicodeString& termString,
#ifdef V1_6
								   const ModLanguageSet& langSet_,
#endif // V1_6
								   const ModInvertedTermMatchMode& mmode_);
	virtual ModBoolean getOriginalString(ModUnicodeString& termString,
#ifdef V1_6
										 ModLanguageSet& langSet_,
#endif // V1_6
								   		 ModInvertedTermMatchMode& mmode_) const;

	// 子ノードリストの並べ替え
	void sortChildren(const ModInvertedQuery::ValidateMode mode);

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
	// 粗い evaluate 満足を前提とした、正確な再評価
	// 二番目の形式は、位置情報が必要な場合に用いられる。
	ModBoolean reevaluate(DocumentID documentID);
	ModBoolean reevaluate(DocumentID documentID,
						  LocationIterator*& locations,
						  ModSize& uiTF_,
						  ModInvertedQueryNode* givenEndNode = 0);

	static ModBoolean lessSortFactor(QueryNode* x, QueryNode* y);
	static RankingResult::Iterator setUnion(const RankingResult&,
											RankingResult::ConstIterator,
											RankingResult::ConstIterator,
											RankingResult::Iterator,
											RankingResult::Iterator,
											ScoreCombiner&,
											const ModInvertedDocumentScore = 1.0);

	// ユーザが検索語として入力した部分の short word の長さ
	ModSize shortWordLength;

#ifdef V1_6
	ModLanguageSet langSet;
#endif // V1_6
	ModInvertedTermMatchMode mmode;

private:
	// 子ノードのスコア
	ModVector<DocumentScore> scores;

	// オリジナルのTermLeafNodeの文字列(TermLeafNodeから生成された場合のみ保持)
	ModUnicodeString originalTermString;

	// 使用禁止
	ModInvertedOperatorOrNode(const OrNode& original);
	ModInvertedOperatorOrNode& operator=(const OrNode& original);
};

//
// FUNCTION public
// ModInvertedOperatorOrNode::setShortWordLength -- shortWordLength をセットする
//
// NOTES
// 引数 shortWordLength は short word の長さを表わす。
//
// short word の長さとはユーザが検索語として入力した部分の長さのことで
// ある。例えば、“光”と入力されshort wordとして展開された結果が次の
// ようになった場合、shortWordLengthは 1 である。
//
// #or(光,光線,光源,…)
//
// メンバ変数shortWordは、このORノードがshort word用のORノードだった場
// 合は0以外、short wordでない場合は0になっている。コンストラクタで0に
// 初期化している
//
// ARGUMENTS
// ModSize shortWordLength_
//		short word の検索語として入力された部分の長さ。デフォルトは 0
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedOperatorOrNode::setShortWordLength(const ModSize shortWordLength_)
{
	this->shortWordLength = shortWordLength_;
}

//
// FUNCTION public
// ModInvertedOperatorOrNode::getShortWordLength -- shortWordLength を取得する
//
// NOTES
// shortWordLength は short word の長さを表わす。
//
// ARGUMENTS
// なし
//
// RETURN
// shortWordLength
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedOperatorOrNode::getShortWordLength() const
{
	return this->shortWordLength;
}

//
// FUNCTION public
// ModInvertedOperatorOrNode::isShortWordOrNode -- shortWord用Orかどうかの判定
//
// NOTES
//
// ARGUMENTS
// なし
//
// RETURN
// shortWord用Orの場合はModTrue/それ以外の場合はModFalseを返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedOperatorOrNode::isShortWordOrNode() const
{
	return (getShortWordLength() != 0) ? ModTrue : ModFalse;
}

#endif //__ModInvertedOperatorOrNode_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
