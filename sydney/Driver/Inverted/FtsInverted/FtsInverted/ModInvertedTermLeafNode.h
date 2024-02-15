// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedTermLeafNode.h -- 検索語に対応する末端ノード
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedTermLeafNode_H__
#define __ModInvertedTermLeafNode_H__

#include "ModInvertedTokenizer.h"
#include "ModInvertedQueryLeafNode.h"
#ifdef V1_6
#include "ModLanguageSet.h"
#endif // V1_6


class ModInvertedFile;
class ModInvertedQuery;
class ModInvertedQueryInternalNode;
class ModInvertedWordOrderedDistanceNode;
class ModInvertedOrderedDistanceNode;

//
// CLASS
// ModInvertedTermLeafNode -- 検索語に対応する末端ノード
//
// NOTES
// 検索式の内部表現には、オペランドをオペレータで結んだ木構造を用いる。
// その末端ノードで、検索語に対応するもののクラス。
//
class
ModInvertedTermLeafNode : public ModInvertedQueryLeafNode
{
public:
	// PathとLocationQueryPairVectorの型は同じ。
	// ModInvertedQueryNode.h, ModInvertedQuery.h を参照
	typedef ModVector< ModPair<ModSize, ModInvertedQueryNode* > >	Path;
	typedef ModPair<ModSize,Path*>			LocationPathPair;
	typedef ModPair<ModSize,QueryNode*>		LocationQueryPair;
	typedef ModVector<LocationQueryPair>	LocationQueryPairVector;
	typedef ModVector<LocationPathPair>		LocationPathPairVector;
	typedef ModMap<QueryNode*,int ,ModLess<QueryNode*> >	QueryNodeCountMap;
	typedef ModInvertedTermMatchMode MatchMode;

	ModInvertedTermLeafNode();
	ModInvertedTermLeafNode(const ModUnicodeString& termString_, 
						    const ModUInt32 resultType_,
#ifdef V1_6
							const ModLanguageSet& langSet_,
#endif // V1_6
							const MatchMode matchMode_ = ModInvertedTermStringMode
							);

	virtual ~ModInvertedTermLeafNode();

	// 検索の一括実行
	// ブーリアン検索用の評価関数：termLeafNodeはvalidateで
	// 削除されるのでこの関数が呼ばれることはない
#ifndef DEL_BOOL
	virtual void retrieve(BooleanResult& queryResult,
						  Query::EvaluateMode mode) 
		{ ; ModAssert(0); }
#endif
	// 与えられた文書が検索条件を満たすかどうかの検査
	// ブーリアン検索用の評価関数：termLeafNodeはvalidateで
	// 削除されるのでこの関数が呼ばれることはない
	virtual ModBoolean evaluate(DocumentID documentID, 
								Query::EvaluateMode mode)
		{ ; ModAssert(0); return ModFalse; }

	// 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
	// ブーリアン検索用の評価関数：termLeafNodeはvalidateで
	// 削除されるのでこの関数が呼ばれることはない
	virtual ModBoolean lowerBound(DocumentID givenDocumentID,
								  DocumentID& foundDocumentID,
								  Query::EvaluateMode mode)
		{ ; ModAssert(0); return ModFalse; }


	// ランキング検索用の評価関数：termLeafNodeはvalidateで
	// 削除されるのでこの関数が呼ばれることはない
	virtual ModBoolean evaluateScore(const DocumentID documentID,
									DocumentScore& score,
									Query::EvaluateMode mode)
		{ ; ModAssert(0); return ModFalse; }

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
	// ランキング検索用の評価関数：termLeafNodeはvalidateで
	// 削除されるのでこの関数が呼ばれることはない
	virtual ModBoolean evaluateScore(const DocumentID documentID,
									DocumentScore& score,
									LocationIterator*& locations,
									Query::EvaluateMode mode,
									ModInvertedQueryNode* givenEndNode = 0)
		{ ; ModAssert(0); return ModFalse; }
#endif		

	// ランキング検索用の評価関数：termLeafNodeはvalidateで
	// 削除されるのでこの関数が呼ばれることはない
	virtual ModBoolean lowerBoundScore(const DocumentID givenDocumentID,
									DocumentID& foundDocumentID,
									DocumentScore& score,
									Query::EvaluateMode mode)
		{ ; ModAssert(0); return ModFalse; }

	// 文書頻度を見積もる
	virtual ModSize estimateDocumentFrequency()
		{ ; ModAssert(0); return 0; }

	// 検索条件に適合する部分の長さの範囲を得る
	void getTermLength(ModSize& length) const;

	// 正確な評価ノードの操作
	void setQueryNodeForPreciseEvaluation(QueryNode* const node);
	ModInvertedQueryNode* getQueryNodeForPreciseEvaluation() const;

	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery& query);

	virtual void contentString(ModUnicodeString& content) const;

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
			const ModBoolean withCalOrCombName, 
			const ModBoolean withCalOrCombParam) const;

#ifdef V1_6 
	ModLanguageSet getLangSet(){ return langSet; }
#endif // V1_6

	// 検索タイプのアクセス関数
	void setMatchMode(const MatchMode);
	const MatchMode getMatchMode() const;

	// Get search condition
	virtual void getSearchTermList(
		ModInvertedQuery::SearchTermList& vecSearchTerm_,
		ModSize uiSynonymID_) const;
	
protected:
	friend class ModInvertedQueryInternalNode;
	friend class ModInvertedQuery;
	friend class ModInvertedOperatorAndNode;
	friend class ModInvertedOperatorAndNotNode;
	friend class ModInvertedOperatorOrNode;
	friend class ModInvertedOperatorEndNode;
	friend class ModInvertedOperatorLocationNode;
	friend class ModInvertedOperatorScaleNode;
	
	virtual ModBoolean reevaluate(DocumentID documentID)
		{ ; ModAssert(0); return ModFalse; }
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* givenEndNode = 0)
		{ ; ModAssert(0); return ModFalse; }

	// 有効化する
	void validate(ModInvertedFile* invertedFile,
				  const Query::ValidateMode mode,
				  Query* rQuery);


	// ラフノード作成
	virtual ModInvertedQueryNode* createRoughNodeForTermLeafNode(
		const ModInvertedQuery& query,
		const Query::ValidateMode mode);

	// 検索語文字列
	ModUnicodeString termString;

	// 正しい評価用の検索式ノード
	QueryNode* queryNodeForPreciseEvaluation;

	// 検索語の長さ (ShortWordでは-1,NormalWordではそれ以外の値になる)
	ModSize termLength;

private:
	//
	
	
	// 通常の検索語ノードの有効化(validateの下請け関数)
	void validateForNormalWord(ModInvertedQuery& query,
						ModInvertedFile* invertedFile,
						const Query::ValidateMode mode,
						const ModInvertedLocationListMap& tokenMap,
						ModSize length,
						ModInvertedQueryNode*& preciseNode,
						ModInvertedQueryNode*& roughNode,
						ModSize& location,
						ModInvertedSmartLocationList*& newEmptyLocationList);

	// 検索語が short word だった場合の有効化関数
	void validateForShortWord(const ModUnicodeString& term,
							  const ModUnicodeString& from,
							  const ModUnicodeString& to,
							  ModInvertedFile* file,
							  ModInvertedQueryNode*& node,
							  ModInvertedQuery& query,
							  const Query::ValidateMode mode);

#ifdef V1_4	// 単語単位検索
	// 単語単位検索の有効化
	void validateForWordMode(ModInvertedQueryNode*& node,
							const ModSize tokenLength,
							ModInvertedQuery& query,
							ModInvertedFile* invertedFile,
							const Query::ValidateMode mode,
							ModInvertedSmartLocationList* emptyLoationList);

#endif // V1_4 単語単位検索

	// tokenaizeの結果であるtokenMapの内容をTermLeafNodeのroughPointerにセット
	void tokenMapToRoughPointer(const ModInvertedLocationListMap& tokenMap,
								const LocationQueryPairVector& allLeaf,
								const LocationQueryPairVector& bestPath,
								ModInvertedQueryNode*& roughNode,
								ModInvertedQuery& query,
								ModInvertedQueryNode*& preciseNode,
								const Query::ValidateMode mode,
								ModInvertedFile* file);

	// コピーコンストラクタ・代入 (使用禁止)
	ModInvertedTermLeafNode(const TermLeafNode& original);
	TermLeafNode& operator=(const TermLeafNode& original);

	// 最適の索引語列を得る (validate() の補助関数)
	static ModBoolean getBestPath(LocationQueryPairVector::Iterator
								  iterator,
								  const LocationQueryPairVector::Iterator
								  endIterator,
								  ModSize startPosition,
								  const ModSize endPosition,
								  LocationPathPairVector& pathVector);

	// シンプルな索引語列を得る (validate() の補助関数)
	static ModBoolean getSimplePath(
		const LocationQueryPairVector& vecPairLocationQuery_,
		LocationPathPairVector& vecPairLocationPath_);

	// LocationPathPairを追加する (getSimplePath()の補助関数)
	static ModBoolean addPairLocationQuery(
		const LocationQueryPair& pairLocationQuery_,
		ModUnicodeString& cstrTerm_,
		const ModVector<ModUnicodeString>& vecTerm_,
		Path*& pPath_);

	// path を取り出す (validate() と getBestPath() の補助関数)
	static Path* findPath(LocationPathPairVector& pathVector,
						  ModSize startPosition);

	// pathVector を消す (validate() と getBestPath() の補助関数)
	static void erasePathVector(LocationPathPairVector& pathVector);

	// トークン分割の比較
	static ModBoolean betterPath(const Path& x, const Path& y);

	// 出現位置による比較
	static ModBoolean former(const LocationQueryPair& x,
							 const LocationQueryPair& y);

	// 文書頻度による比較
	static ModBoolean lessFrequent(const LocationQueryPair& x,
								   const LocationQueryPair& y);

	// OrderedDistanceNodeを生成する
	virtual ModInvertedOrderedDistanceNode* createOrderedDistanceNode(
		const Query& query,
		const ModInvertedFile* invertedFile,
		const Query::ValidateMode mode) const;

	virtual ModInvertedWordOrderedDistanceNode* createWordOrderedDistanceNode(
		const Query& query,
		const ModInvertedFile* invertedFile,
		const Query::ValidateMode mode) const;

	void createRoughNodeForTermLeafNode2(
		const ModInvertedQuery& query,
		ModVector<ModInvertedQueryNode*>::Iterator iteratorForRoughNodes,
		const ModVector<ModInvertedQueryNode*>::Iterator roughEnd,
		const ModSize roughSize,
		const Query::ValidateMode mode);

	// 検索タイプに応じたOrNodeを作成する
	ModInvertedOperatorOrNode*
	createOrNode(
		const ModInvertedQuery& query,
		const ModInvertedFile* invertedFile,
		const Query::ValidateMode mode);

	// SimpleTokenLeafNodeを設定する
	void setSimpleTokenLeafNodeForNormalWord(
		ModInvertedQueryNode* pQueryNode_,
		const ModInvertedQuery& cQuery_,
		const ModInvertedFile* pInvertedFile_,
		Query::ValidateMode uiValidateMode_);

	// Check match mode
	MatchMode checkMatchMode(ModInvertedFileIndexingType iIndexingType_,
							 MatchMode iMatchMode_) const;

	// Get tokenize mode
	ModInvertedTokenizer::TokenizeMode getTokenizeMode(
		ModInvertedFileIndexingType iIndexingType_,
		MatchMode iMatchMode_,
		Query::ValidateMode uiValidateMode_) const;

	MatchMode matchMode;				// 検索タイプ
#ifdef V1_6
	ModLanguageSet langSet;
#endif // V1_6
};

// inline
// ModInvertedTermLeafNode::operator const ModInvertedTermLeafNode*() const
// {
//	return this;
// }

//
// FUNCTION public
// ModInvertedTermLeafNode::ModInvertedTermLeafNode -- 検索語に対応するノードの生成
//
// NOTES
// 検索語に対応する検索式表現ノードオブジェクトを新しく生成する。
//
// termLengthを0に初期化しているが、これは子ノードのTermLengthが違う場合。
// ShortWordではかならずしも一定の語長であるとはかぎらないので-1、
// それ以外（すべてのtermLengthが同じ）場合はその語長を設定する。
//
// ARGUMENTS
// const ModUnicodeString& termString_;
//		検索語文字列
//
// RETURN
// なし
// 
// EXCEPTIONS
// なし
//
inline
ModInvertedTermLeafNode::ModInvertedTermLeafNode()
	: termString(), ModInvertedQueryLeafNode(termLeafNode),
	  queryNodeForPreciseEvaluation(0), termLength(0)
{}

inline
ModInvertedTermLeafNode::ModInvertedTermLeafNode(
	const ModUnicodeString& termString_, 
	const ModUInt32 resultType_,
#ifdef V1_6
	const ModLanguageSet& langSet_,
#endif // V1_6
	const MatchMode matchMode_
	)
	: termString(termString_), 
	  ModInvertedQueryLeafNode(termLeafNode,resultType_,ModSize(-1)),
	  queryNodeForPreciseEvaluation(0), termLength(0),
	  matchMode(matchMode_)
#ifdef V1_6
	  , langSet(langSet_)
#endif // V1_6
{
}

// 
// FUNCTION public
// ModInvertedTermLeafNode::setQueryNodeForPrecieseEvaluation -- 正確な評価用ノードの設定
// 
// NOTES
// 正確な評価用ノードを設定する。
//
// ARGUMENTS
// ModInvertedQueryNode* const queryNodeForPreciseEvaluation_
//		正確な評価用ノード
//
// RETURN
// なし
// 
// EXCEPTIONS
// なし
//
inline void
ModInvertedTermLeafNode::setQueryNodeForPreciseEvaluation(
	ModInvertedQueryNode* const queryNodeForPreciseEvaluation_)
{
	this->queryNodeForPreciseEvaluation
	  = queryNodeForPreciseEvaluation_;
}

// 
// FUNCTION public
// ModInvertedTermLeafNode::getQueryNodeForPreciseEvaluation -- 正確な評価用ノードの取得
// 
// NOTES
// 正確な評価用ノードを取得する。
//
// ARGUMENTS
// なし
//
// RETURN
// 正確な評価用ノード
// 
// EXCEPTIONS
// なし
//
inline ModInvertedQueryNode*
ModInvertedTermLeafNode::getQueryNodeForPreciseEvaluation() const
{
	return this->queryNodeForPreciseEvaluation;
}

//
// FUNCTION public
// ModInvertedTermLeafNode::getTermLength -- 検索語の長さを得る
//
// NOTES
// トークナイザで分割後（正規化も考慮したうえで）の検索語の長さを返す
//
// ARGUMENTS
// ModSize& length
//	検索語の長さ(結果保存用)
//
// RETURN
// 検索語の長さを得る
// 
// EXCEPTIONS
// なし
//
inline void
ModInvertedTermLeafNode::getTermLength(ModSize& length) const
{
	length = this->termLength;
}

//
// FUNCTION public
// ModInvertedTermLeafNode::setMatchMode -- 検索タイプをセットする
//
// NOTES
// 検索タイプをセットする
//
// ARGUMENTS
// MatchMode _matchMode
//		検索タイプ
//
// RETURN
// なし
// 
// EXCEPTIONS
// なし
//
inline void
ModInvertedTermLeafNode::setMatchMode(const MatchMode _matchMode)
{
	matchMode = _matchMode;
}

//
// FUNCTION public
// ModInvertedTermLeafNode::getMatchMode -- 検索タイプを取得
//
// NOTES
// 検索タイプを取得する
//
// ARGUMENTS
// なし
//
// RETURN
// 検索タイプ
// 
// EXCEPTIONS
// なし
//
inline const ModInvertedTermLeafNode::MatchMode
ModInvertedTermLeafNode::getMatchMode() const 
{
	return matchMode;
}

#endif //__ModInvertedTermLeafNode_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
