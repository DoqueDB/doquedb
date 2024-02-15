// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedWindowBaseNode.h -- 近傍演算ノードインタフェイスファイル
// 
// Copyright (c) 2002, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedWindowBaseNode_H__
#define __ModInvertedWindowBaseNode_H__

#include "ModInvertedOperatorAndNode.h"
#include "ModInvertedAtomicNode.h"

//
// CLASS
// ModInvertedWindowBaseNode -- 近傍演算ノードの基底クラス
//
// NOTES
//
class
ModInvertedWindowBaseNode :
	public ModInvertedOperatorAndNode, public ModInvertedAtomicNode
{
public:
	ModInvertedWindowBaseNode();
	ModInvertedWindowBaseNode(const NodeType nType,const ModUInt32 resultType_);

	// デストラクタ
	virtual ~ModInvertedWindowBaseNode();

	// 順序つき位置演算タイプに設定する
	void setOrderedType();
	// 順序無視位置演算タイプに設定する
	void setUnorderedType();

// あいまい AtomicNode Start ------------------------------------------------------

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

	// ランキング検索のスコア計算第１ステップ
	void doFirstStepInRetrieveScore(
		ModInvertedBooleanResult *expungedDocumentId,
		const ModInvertedDocumentID maxDocumentId);
	
	// ランキング検索のスコア計算第２ステップ
	void doSecondStepInRetrieveScore(
		ModInvertedSearchResult*& result);
	void doSecondStepInRetrieveScore();

	// ランキング検索のスコア計算第２ステップで使用するlowerBound
	ModBoolean lowerBoundScoreForSecondStep(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score);
// あいまい AtmicNode End --------------------------------------------------------

// あいまい OparatorAndNode Start ------------------------------------------------
	// 与えられた文書が検索条件を満たすかどうかの検査
	virtual ModBoolean evaluate(DocumentID documentID,
								Query::EvaluateMode mode);
	// 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
	virtual ModBoolean lowerBound(DocumentID givenDocumentID,
								  DocumentID& foundDocumentID,
								  Query::EvaluateMode mode);
	// 出現頻度を得る
	virtual ModSize getDocumentFrequency(Query::EvaluateMode mode);
	// 文書頻度を見積もる
	virtual ModSize estimateDocumentFrequency();

	virtual ModSize getEstimatedDocumentFrequency() const;

	void setTotalDocumentFrequency(const ModSize value) {
		ModInvertedOperatorAndNode::setTotalDocumentFrequency(value); }
	ModSize getTotalDocumentFrequency() const {
		return ModInvertedOperatorAndNode::getTotalDocumentFrequency(); }

	// endNodeのアクセサ関数
	virtual QueryNode* getEndNode() const;
	virtual void setEndNode(QueryNode* endNode_);

	// 粗い evalaute  のための検索式ノードの取得
	virtual ModInvertedQueryNode* getQueryNodeForRoughEvaluation() const;

	virtual void setQueryNodeForRoughEvaluation(QueryNode* node);

	// RankingOr,RankingAndの場合は子ノード数の分だけsocresをリザーブ
	virtual void reserveScores();

	// queryNodeForRoughEvaluation の作成
	virtual void makeRoughPointer(const Query::ValidateMode,
								  QueryNodePointerMap&,
								  const ModInvertedQuery* Query);

	// TermLeafNode を消去して SimpleToken/OrderedDistance にする
	virtual ModBoolean eraseTermLeafNode(QueryNode*& node, Query& query);

	// 子ノード数を得る
	virtual ModSize getChildrenSize() const;

	// ノードの内容を表わす文字列を返す
	virtual void contentString(ModUnicodeString& content) const;

#ifdef DEBUG
	// sortFactor を表示(debug用)
	virtual void showSortFactor(ModUnicodeString& out);
#endif // DEBUG

	// ラフノードの生成
	ModInvertedQueryNode* createRoughNode(const ModInvertedQuery* Query,
										  const ModInvertedQuery::ValidateMode mode);

	// sharedQueryNode の補助関数
	void changeSimpleTypeNode(
		ModVector<ModInvertedQueryNode*>::Iterator child,
		QueryNodePointerMap& nodePointerMap);

	// RoughPointerの内容を共有化する
	virtual void addRoughToGlobalNodeMap(QueryNodeMap& globalNodeMap,
										 QueryNodePointerMap& nodePointerMap);

	 // 検索語の長さを得る
	virtual void getTermLength(ModSize& length) const;

	virtual ModVector<ModInvertedQueryNode*>* getChildren();

	// originalTermStringをセットする
	void setOriginalString(const ModUnicodeString&,
#ifdef V1_6
						   const ModLanguageSet&,
#endif // V1_6
						   const ModInvertedTermMatchMode&);
	ModBoolean getOriginalString(ModUnicodeString&,
#ifdef V1_6
								 ModLanguageSet&,
#endif // V1_6
						   		 ModInvertedTermMatchMode&) const;

#ifndef MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF
	void* operator new(unsigned int size, unsigned int dummy = 0) {
		return ModInvertedOperatorAndNode::operator new(size, dummy);
	}
	void operator delete(void* address, unsigned int size) {
		ModInvertedOperatorAndNode::operator delete(address, size);
	}
#endif // MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF
// あいまい OparatorAndNode End --------------------------------------------------

// あいまい QueryBaseNode Start --------------------------------------------------
	// firstStepStatusのアクセサ関数
	void setFirstStepStatus(FirstStepStatus _status);
	FirstStepStatus getFirstStepStatus();

	ModInvertedSearchResult* getRankingResult();

	ModBoolean needDocumentFrequency() const;

	// needDFのアクセサ関数
	void setNeedDF(const ModBoolean needDF_);
	ModBoolean getNeedDF();
// あいまい QueryBaseNode End ----------------------------------------------------

	// 子ノードリストの平坦化 (実際には何もしない)
	virtual void flattenChildren(const QueryNodePointerMap& sharedNodeMap,
								 const ModBoolean isChildOfWindowNode);

	// スコア合成器をセットする
	void setScoreCombiner(
		ScoreCombiner* scoreCombiner_);
	virtual void setScoreCombiner(
		const ModUnicodeString& combinerName);

	// スコア計算器をセットする
	virtual void setScoreCalculator(
		ScoreCalculator* calculator);
	virtual void setScoreCalculator(
		const ModUnicodeString& calculatorName);

	// スコア計算器を得る
	ScoreCalculator* getScoreCalculator() const {
		return scoreCalculator; }

	// スコア合成器を得る
	ScoreCombiner* getScoreCombiner() const {
		return 0; }

#ifndef DEL_BOOL
	// 検索の一括実行
	virtual void retrieve(BooleanResult& queryResult,
						  Query::EvaluateMode mode);
#endif
	// 文書内頻度を得る
	virtual ModSize getTermFrequency(DocumentID documentID,
									 Query::EvaluateMode mode);

	static int getTermFrequencyMethod;        // 文書内頻度取得方法の指定

	// 中間ノードの共有化
	virtual ModSize sharedQueryNode(QueryNodeMap& globalNodeMap,
									QueryNodePointerMap& nodePointerMap);

	// 有効化
	virtual void validate(InvertedFile* invertedFile,
						  const ModInvertedQuery::ValidateMode mode,
						  ModInvertedQuery* rQuery);

	// 検索条件ノードを出力 ラフノード表示 on/off 可
	virtual void getQueryString(ModUnicodeString& out,
								const ModBoolean asTermString = ModFalse,
								const ModBoolean withCalOrCombName = ModTrue,
								const ModBoolean withCalOrCombParam = ModTrue,
								const ModBoolean withRouh = ModFalse) const;

	virtual void prefixString(ModUnicodeString& prefix,
							  const ModBoolean withCalOrCombName,
							  const ModBoolean withCalOrCombParam) const = 0; 

	void setChildForWindowNode(ModInvertedQueryInternalNode*& node_,
				   const ModInvertedQuery& rQuery_);

	ModBoolean isOrderedType() const;

	// 検索語の間隔の下限
	ModSize getMinimalDistance() const;
	// 検索語の間隔の上限
	ModSize getMaximalDistance() const;

	// 先頭要素と末尾要素の頭と頭の間の文字数の下限
	ModSize         minimalDistance;
	// 先頭要素と末尾要素の頭と頭の間の文字数の上限
	ModSize         maximalDistance;

	// 有効化の最後に子ノードの数をチェックする
	virtual void checkQueryNode(ModInvertedQuery*,
								const ModBoolean,
								const ModBoolean);

	// 子ノードリストの並べ替え
	void sortChildren(const ModInvertedQuery::ValidateMode mode);

	// 自分のコピーを作成する
	//virtual QueryNode* duplicate(const ModInvertedQuery& query);

protected:

	// 粗い evaluate 満足を前提とした、正確な再評価
	virtual ModBoolean reevaluate(DocumentID documentID) = 0;
	// 位置情報が必要版
	// ただし、位置情報リストが取得できない場合は、TFを取得する
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locationa,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* givenEndNode = 0) = 0;

	// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
	// Rankig InternalNodeはCombiner LeafNodeはCalculator
	// Atomic はCalculator
	virtual void getCalculatorOrCombinerName(
		ModUnicodeString& name,
		const ModBoolean withParams) const;

#ifdef DEBUG
	// 文書頻度の見積もり値を出力
	virtual void showEstimatedValue(ModUnicodeString& out);
#endif // DEBUG
};

//
// FUNCTION protected
// ModInvertedWindowBaseNode::ModInvertedWindowBaseNode -- 
//
// NOTES
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedWindowBaseNode::ModInvertedWindowBaseNode()
{}

//
// FUNCTION protected
// ModInvertedWindowBaseNode::ModInvertedWindowBaseNode -- ANDノードを生成する
//
// NOTES
// 積集合(AND)ノードを生成する。NodeType引数付き。
// OrderedDistanceNode や WindowNode から呼び出される。
//
// ARGUMENTS
// const NodeType nType
//      ノードタイプ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedWindowBaseNode::ModInvertedWindowBaseNode(const NodeType nType,
		const ModUInt32 resultType_)
	: ModInvertedOperatorAndNode(nType,resultType_)
{}

//
// FUNCTION public
// ModInvertedWindowBaseNode::ModInvertedOrderedDistanceNode -- 間隔演算ノードを破棄する
//
// NOTES
// 間隔演算(正確な距離を指定した距離演算)ノードを破棄する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedWindowBaseNode::~ModInvertedWindowBaseNode()
{
	if(scoreCalculator != 0) {
		delete scoreCalculator;
		scoreCalculator = 0;
	}
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::setOrderedType -- 順序つき位置演算タイプに設定する
//
// NOTES
// 順序つき位置演算タイプに設定する。この関数は
// RankingSimpleWindowNode や AtomicSimpleWindowNode でも使うので、
// Unordered を示す bit をオフするだけにする。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
inline void
ModInvertedWindowBaseNode::setOrderedType()
{
	NodeType tmpType = BaseNode::getType();
	tmpType = NodeType(tmpType & ~AtomicNode::unorderedNode);
	BaseNode::setType(tmpType);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::setUnorderedType -- 順序無視演算タイプに設定する
//
// NOTES
// 順序無視位置演算タイプに設定する。この関数は
// RankingSimpleWindowNode や AtomicSimpleWindowNode でも使うので、
// Unordered を示す bit をオンするだけにする。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
inline void
ModInvertedWindowBaseNode::setUnorderedType()
{
	NodeType tmpType = BaseNode::getType();
	tmpType = NodeType(tmpType | AtomicNode::unorderedNode);
	BaseNode::setType(tmpType);
}

// あいまい AtomicNode Start ------------------------------------------------------
//

//
// FUNCTION public
// ModInvertedWindowBaseNode::evaluateScore -- 与えられた文書のスコアを計算する
//
// NOTES
// 与えられた文書のスコアを計算する
//
// ARGUMENTS
// const DocumentID documentID
//    文書ID
// DocumentScore& score
//    スコア（結果格納用）
// evaluationMode mode
//    評価モード
//
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModInvertedWindowBaseNode::evaluateScore(const DocumentID documentID,
										   DocumentScore& score,
										   Query::EvaluateMode mode)
{
	return AtomicNode::evaluateScore(documentID, score, mode);
}

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
//
// FUNCTION public
// ModInvertedWindowBaseNode::evaluateScore -- 与えられた文書のスコアを計算する（位置も計算する）
//
// NOTES
// 与えられた文書のスコアを計算する
//
// ARGUMENTS
// const DocumentID documentID
//    文書ID
// DocumentScore& score
//    スコア（結果格納用）
// LocationIterator*& locations,
//    位置情報（結果格納用）
// evaluationMode mode
//    評価モード
// ModInvertedQueryNode* givenEndNode
//    ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModInvertedWindowBaseNode::evaluateScore(const DocumentID documentID,
										   DocumentScore& score,
										   LocationIterator*& locations,
										   Query::EvaluateMode mode,
										   ModInvertedQueryNode* givenEndNode)
{
	return AtomicNode::evaluateScore(documentID, score, locations, mode);
}
#endif

//
// FUNCTION public
// ModInvertedWindowBaseNode::lowerBoundScore -- lowerBoundのランキング版（スコアも計算する）
//
// NOTES
//
// ARGUMENTS
// ModInvertedDocumentID givenID
//    文書ID
// ModInvertedDocumentID& foundID
//    結果格納用の文書IDオブジェクト
// DocumentScore& score
//    スコア（結果格納用）
// evaluationMode mode
//    評価モード
//
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModInvertedWindowBaseNode::lowerBoundScore(const DocumentID givenID,
										   DocumentID& foundID,
										   DocumentScore& score,
										   Query::EvaluateMode mode)
{
	return AtomicNode::lowerBoundScore(givenID, foundID, score, mode);
}

//
// FUNCTION protected
// ModInvertedWindowBaseNode::getCalculatorOrCombinerName -- 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
//
// NOTES
// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す。
// AtomicNode::getCalculatorOrCombinerName(string)をコール。
//
// ARGUMENTS
// ModString& name
//    scoreCalculator名(結果格納用)。
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline void
ModInvertedWindowBaseNode::getCalculatorOrCombinerName(
	ModUnicodeString& name,
	const ModBoolean withParams) const
{
	ModInvertedAtomicNode::getCalculatorOrCombinerName(name, withParams);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::doFirstStepInRetrieveScore(
//      const ModInvertedDocumentID maxDocumentId)
//
inline void
ModInvertedWindowBaseNode::doFirstStepInRetrieveScore(
	ModInvertedBooleanResult *expungedDocumentId,
	const ModInvertedDocumentID maxDocumentId)
{
	ModInvertedAtomicNode::doFirstStepInRetrieveScore(expungedDocumentId,maxDocumentId);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::doSecondStepInRetrieveScore(
//      ModInvertedRankingResult* result_)
//
inline void
ModInvertedWindowBaseNode::doSecondStepInRetrieveScore(
	ModInvertedSearchResult* &result_)
{
	ModInvertedAtomicNode::doSecondStepInRetrieveScore(result_);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::doSecondStepInRetrieveScore()
//
inline void
ModInvertedWindowBaseNode::doSecondStepInRetrieveScore()
{
	ModInvertedAtomicNode::doSecondStepInRetrieveScore();
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::lowerBoundScoreForSecondStep()
//
inline ModBoolean
ModInvertedWindowBaseNode::lowerBoundScoreForSecondStep(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
	return ModInvertedAtomicNode::lowerBoundScoreForSecondStep(
									givenID, foundID, score);
}
// あいまい AtomicNode End --------------------------------------------------------

// あいまい OparatorAndNode Start ------------------------------------------------
//
// FUNCTION public
// ModInvertedWindowBaseNode::evaluate -- 与えられた文書が検索式を満たすかどうかの検査
//
inline ModBoolean
ModInvertedWindowBaseNode::evaluate(ModInvertedDocumentID documentID,
									  Query::EvaluateMode mode)
{
	return ModInvertedOperatorAndNode::evaluate(documentID, mode);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::lowerBound -- 検索式を満たす文書のうち、文書IDが与え~られた値以上で最小の文書の検索
//
inline ModBoolean
ModInvertedWindowBaseNode::lowerBound(ModInvertedDocumentID givenID,
										  ModInvertedDocumentID& foundID,
										  Query::EvaluateMode mode)
{
	return ModInvertedOperatorAndNode::lowerBound(givenID, foundID, mode);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getDocumentFrequency -- 出現頻度を得る
//
inline ModSize
ModInvertedWindowBaseNode::getDocumentFrequency(Query::EvaluateMode mode)
{
	return ModInvertedOperatorAndNode::getDocumentFrequency(mode);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::estimateDocumentFrequency -- 文書頻度の見積もり
//
inline ModSize
ModInvertedWindowBaseNode::estimateDocumentFrequency()
{
	return ModInvertedOperatorAndNode::estimateDocumentFrequency();
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getEstimatedDocumentFrequency -- 文書頻度の見積もり値を返す
//
inline ModSize
ModInvertedWindowBaseNode::getEstimatedDocumentFrequency() const
{
	return ModInvertedOperatorAndNode::getEstimatedDocumentFrequency();
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getEndNode -- endNodeのアクセサ関数
//
inline ModInvertedQueryNode*
ModInvertedWindowBaseNode::getEndNode() const
{
	return ModInvertedOperatorAndNode::getEndNode();
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::setEndNode -- endNodeのアクセサ関数
//
inline void
ModInvertedWindowBaseNode::setEndNode(ModInvertedQueryNode* endNode_)
{
	ModInvertedOperatorAndNode::setEndNode(endNode_);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getQueryNodeForRoughEvaluation -- 粗い評価用ノードの取得
//
inline ModInvertedQueryNode*
ModInvertedWindowBaseNode::getQueryNodeForRoughEvaluation() const
{
	return ModInvertedOperatorAndNode::getQueryNodeForRoughEvaluation();
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::setQueryNodeForRoughEvaluation -- 粗い評価のためのノードをセットする
//
inline void
ModInvertedWindowBaseNode::setQueryNodeForRoughEvaluation(QueryNode* node)
{
	ModInvertedOperatorAndNode::setQueryNodeForRoughEvaluation(node);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::reserveScores -- scoresをリザーブ
//
inline void
ModInvertedWindowBaseNode::reserveScores()
{
	ModInvertedOperatorAndNode::reserveScores();
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::makeRoughPointer -- queryNodeForRoughEvaluationの作成
//
// NOTES
// 自分にはラフを設定せず、子ノードに対し再帰的に makeRoughPointer を呼び出す。
//
// ARGUMENTS
// const Query::ValidateMode mode
//    有効化モード
// QueryNodePointerMap& parentMap,
//    ノードマップ
// const ModInvertedQuery* Query
//    クエリ
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline void
ModInvertedWindowBaseNode::makeRoughPointer(const Query::ValidateMode mode,
QueryNodePointerMap& parentMap,
const ModInvertedQuery* Query)
{
	ModInvertedOperatorAndNode::makeRoughPointer(mode, parentMap, Query);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::eraseTermLeafNode -- TermLeafNodeの消去
//
inline ModBoolean
ModInvertedWindowBaseNode::eraseTermLeafNode(QueryNode*& node, Query& query)
{
	return ModInvertedQueryInternalNode::eraseTermLeafNode(node, query);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getChildrenSize -- 子ノード数の取得
//
inline ModSize
ModInvertedWindowBaseNode::getChildrenSize() const
{
	return ModInvertedOperatorAndNode::getChildrenSize();
}

#ifdef DEBUG
//
// FUNCTION public
// ModInvertedWindowBaseNode::showSortFactor -- sortFactor を表示
//
inline void
ModInvertedWindowBaseNode::showSortFactor(ModUnicodeString& out)
{
	ModInvertedOperatorAndNode::showSortFactor(out);
}
#endif

//
// FUNCTION public
// ModInvertedWindowBaseNodeBaseNode::contentString -- ノードの内容を表わす文字列を返す
//
inline void
ModInvertedWindowBaseNode::contentString(ModUnicodeString& content) const
{
	ModInvertedOperatorAndNode::contentString(content);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::createRoughNode -- モードに応じたRoughNodeの生成
//
inline ModInvertedQueryNode*
ModInvertedWindowBaseNode::createRoughNode(
	const ModInvertedQuery* Query,
	const ModInvertedQuery::ValidateMode mode)
{
	return ModInvertedOperatorAndNode::createRoughNode(Query, mode);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::changeSimpleTypeNode
//
inline void
ModInvertedWindowBaseNode::changeSimpleTypeNode(
	ModVector<ModInvertedQueryNode*>::Iterator child,
	QueryNodePointerMap& nodePointerMap)
{
	ModInvertedQueryInternalNode::changeSimpleTypeNode(child, nodePointerMap);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::addRoughToGlobalNodeMap
//
inline void
ModInvertedWindowBaseNode::addRoughToGlobalNodeMap(
	QueryNodeMap& globalNodeMap, QueryNodePointerMap& nodePointerMap)
{
	ModInvertedQueryInternalNode::addRoughToGlobalNodeMap(
		globalNodeMap, nodePointerMap);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getTermLength
//
inline void
ModInvertedWindowBaseNode::getTermLength(ModSize& length) const
{
	ModInvertedQueryInternalNode::getTermLength(length);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getChildren
//
inline ModVector<ModInvertedQueryNode*>*
	ModInvertedWindowBaseNode::getChildren()
{
	return ModInvertedQueryInternalNode::getChildren();
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::setOriginalString()
//
inline void
ModInvertedWindowBaseNode::setOriginalString(
	const ModUnicodeString& termString,
#ifdef V1_6
	const ModLanguageSet& langSet_,
#endif // V1_6
	const ModInvertedTermMatchMode& mmode_)
{
	ModInvertedQueryBaseNode::setOriginalString(termString,
#ifdef V1_6
												langSet_,
#endif // V1_6
												mmode_);
}

inline ModBoolean
ModInvertedWindowBaseNode::getOriginalString(
	ModUnicodeString& termString,
#ifdef V1_6
	ModLanguageSet& langSet_,
#endif // V1_6
	ModInvertedTermMatchMode& mmode_) const
{
	return ModInvertedQueryBaseNode::getOriginalString(termString,
#ifdef V1_6
													   langSet_,
#endif // V1_6
													   mmode_);
}

#ifdef DEBUG
//
// FUNCTION protected
// ModInvertedWindowBaseNode::showEstimatedValue -- 文書頻度の見積もり値を出力
//
inline void
ModInvertedWindowBaseNode::showEstimatedValue(ModUnicodeString& out)
{
	ModInvertedOperatorAndNode::showEstimatedValue(out);
}
#endif //  DEBUG
// あいまい OparatorAndNode End --------------------------------------------------


// あいまい QueryBaseNode Start --------------------------------------------------
//
// FUNCTION public
// ModInvertedWindowBaseNode::setFirstStepStatus(FirstStepStatus _status)
//
inline void
ModInvertedWindowBaseNode::setFirstStepStatus(FirstStepStatus _status)
{
	ModInvertedQueryBaseNode::setFirstStepStatus(_status);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getFirstStepStatus()
//
inline ModInvertedQueryNode::FirstStepStatus
ModInvertedWindowBaseNode::getFirstStepStatus()
{
	return ModInvertedQueryBaseNode::getFirstStepStatus();
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getRankingResult()
//
inline ModInvertedSearchResult*
ModInvertedWindowBaseNode::getRankingResult()
{
	return ModInvertedQueryBaseNode::getRankingResult();
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::needDocumentFrequency()
//
inline ModBoolean
ModInvertedWindowBaseNode::needDocumentFrequency() const
{
	return ModInvertedQueryBaseNode::needDocumentFrequency();
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::setNeedDF(const ModBoolean needDF_)
//
inline void
ModInvertedWindowBaseNode::setNeedDF(const ModBoolean needDF_)
{
	ModInvertedQueryBaseNode::setNeedDF(needDF_);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getNeedDF()
//
inline ModBoolean
ModInvertedWindowBaseNode::getNeedDF()
{
	return ModInvertedQueryBaseNode::getNeedDF();
}
// あいまい QueryBaseNode End ----------------------------------------------------

//
// FUNCTION public
// ModInvertedWindowBaseNode::flattenChildren -- 子ノードリストの平坦化
//
// NOTES
// ここは子ノードにand/orがくることがないので何もしない
//
inline void
ModInvertedWindowBaseNode::flattenChildren(const QueryNodePointerMap& sharedNodeMap,
											 const ModBoolean isChildOfWindowNode)
{
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::setScoreCombiner -- スコア合成器をセットする
//
// NOTES
// スコア合成器をセットする。
//
// ARGUMENTS
// const ModUnicodeString& combinerName
//    スコア計算器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedWindowBaseNode::setScoreCombiner(
	const ModUnicodeString& calculatorName)
{
	// Atomic系ノードにCombinerをセットする事はない
	ModAssert(0);
}

//
// FUNCTION public
// ModInvertedSimpleWidnowNode::setScoreCombiner -- スコア合成器をセットする
//
// NOTES
// スコア合成器をセットする。
//
// ARGUMENTS
// const ScoreCombiner* scoreCombiner_
//     スコア合成器
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedWindowBaseNode::setScoreCombiner(
	ScoreCombiner* scoreCombiner_)
{
	// Atomic系のノードに合成器をセットする事はない
	ModAssert(0);
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::setScoreCalculator -- スコア計算器をセットする
//
inline void
ModInvertedWindowBaseNode::setScoreCalculator(
ScoreCalculator* calculator)
{
	scoreCalculator = calculator;
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::setScoreCalculator -- スコア計算器をセットする
//
// NOTES
// スコア計算器をセットする。
//
// ARGUMENTS
// const ModUnicodeString& calculatorName
//    スコア計算器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedWindowBaseNode::setScoreCalculator(
	const ModUnicodeString& calculatorName)
{
	scoreCalculator = ModInvertedRankingScoreCalculator::create(calculatorName);
}

// FUNCTION public
// ModInvertedWindowBaseNode::isOrderedType -- 順序つき位置演算タイプかの検査
//
// NOTES
// 順序つき位置演算タイプかを検査する。
//
// ARGUMENTS
// なし
//
// RETURN
// 順序つき位置演算タイプであれば ModTrue、そうでなければ ModFalse
//
inline ModBoolean
ModInvertedWindowBaseNode::isOrderedType() const
{
	return (BaseNode::getType()&AtomicNode::unorderedNode) ? ModFalse : ModTrue;
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getMinimalDistance -- 検索語の間隔の下限を得る
//
// NOTES
//
// ARGUMENTS
// なし
//
// RETURN
// minimalDistanceを返す
//
inline ModSize
ModInvertedWindowBaseNode::getMinimalDistance() const
{
	return minimalDistance;
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::getMaximalDistance -- 検索語の間隔の下限を得る
//
// NOTES
//
// ARGUMENTS
// なし
//
// RETURN
// maxmalDistanceを返す
//
inline ModSize
ModInvertedWindowBaseNode::getMaximalDistance() const
{
	return maximalDistance;
}

#endif //__ModInvertedWindowBaseNode_H__

//
// Copyright (c) 2002, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
