// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedWordBaseNode.h -- 文字検索系ノードインタフェイスファイル
// 
// Copyright (c) 2002, 2004, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedWordBaseNode_H__
#define __ModInvertedWordBaseNode_H__

#include "ModInvertedQueryInternalNode.h"
#include "ModInvertedAtomicNode.h"

//
// CLASS
// ModInvertedWordBaseNode -- 文字検索系ノードの基底クラス
//
// NOTES
//
class
ModInvertedWordBaseNode :
	public ModInvertedQueryInternalNode, public ModInvertedAtomicNode
{ 
public:
	ModInvertedWordBaseNode();
	ModInvertedWordBaseNode(const NodeType nType,
				const ModUInt32 resultType_);

	// デストラクタ
	~ModInvertedWordBaseNode();

#ifndef DEL_BOOL
	// 検索の一括実行 BOOL
	virtual void retrieve(BooleanResult& queryResult,
						  Query::EvaluateMode mode);
#endif
	// 文書頻度を見積もる
	virtual ModSize estimateDocumentFrequency();

	// 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
	virtual ModBoolean lowerBound(DocumentID givenDocumentID,
								  DocumentID& foundDocumentID,
								  Query::EvaluateMode mode);

	// queryNodeForRoughEvaluation の作成
	virtual void makeRoughPointer(const Query::ValidateMode,
								  QueryNodePointerMap&,
								  const ModInvertedQuery* Query);

//----- あいまい Atomic Start -------------------------------------------------
	// ランキング検索のスコア計算第１ステップ
	void doFirstStepInRetrieveScore(
		ModInvertedBooleanResult *expungedDocumentId,
		const ModInvertedDocumentID maxDocumentId);

	// ランキング検索のスコア計算第２ステップ
	void doSecondStepInRetrieveScore(
		ModInvertedSearchResult* &result);
	void doSecondStepInRetrieveScore();

	// ランキング検索のスコア計算第２ステップで使用するlowerBound
	ModBoolean lowerBoundScoreForSecondStep(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score);

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
									 ModInvertedQueryNode* givenEndNode);
#endif

	// lowerBoundのランキング版（スコアも計算する）
	virtual ModBoolean lowerBoundScore(const DocumentID givenDocumentID,
									   DocumentID& foundDocumentID,
									   DocumentScore& score,
									   Query::EvaluateMode mode);

	// スコア合成器をセットする
	virtual void setScoreCombiner(
		ScoreCombiner* scoreCombiner_);
	virtual void setScoreCombiner(
		const ModUnicodeString& combinerName);

	// スコア計算器をセットする
	virtual void setScoreCalculator(
		ScoreCalculator* calculator);
	virtual void setScoreCalculator(
		const ModUnicodeString& calculatorName);

	// スコア計算器を得る
	virtual ScoreCalculator* getScoreCalculator() const {
		return scoreCalculator; }
	// スコア合成器を得る
	ScoreCombiner* getScoreCombiner() const {
		return 0; }
//----- あいまい Atomic End ---------------------------------------------------

//----- あいまい Query Start --------------------------------------------------
	// 出現頻度を得る
	virtual ModSize getDocumentFrequency(Query::EvaluateMode mode);

	// 子ノードリストの平坦化 (例 #and(#and(X,Y),Z) → #and(X,Y,Z))
	virtual void flattenChildren(
		const QueryNodePointerMap& sharedNodeMap,
		const ModBoolean isChildOfWindowNode);

	// ノードの内容を表わす文字列を返す
	virtual void contentString(ModUnicodeString& content) const;

	// 文書頻度の見積もり値を返す
	virtual ModSize getEstimatedDocumentFrequency() const;

	// TermLeafNode を消去して SimpleToken/OrderedDistance にする
	virtual ModBoolean eraseTermLeafNode(QueryNode*& node, Query& query);

	virtual void addRoughToGlobalNodeMap(QueryNodeMap& globalNodeMap,
										 QueryNodePointerMap& nodePointerMap);

	// 検索語の長さを得る
	virtual void getTermLength(ModSize& length) const;

	virtual ModVector<ModInvertedQueryNode*>* getChildren();

	// 子ノード数を得る
	virtual ModSize getChildrenSize() const;

	// 中間ノードの共有化
	virtual ModSize sharedQueryNode(QueryNodeMap& globalNodeMap,
									QueryNodePointerMap& nodePointerMap);

	// firstStepStatusのアクセサ関数
	void setFirstStepStatus(FirstStepStatus _status);
	FirstStepStatus getFirstStepStatus();

	ModInvertedSearchResult* getRankingResult();

	ModBoolean needDocumentFrequency() const;

	// needDFのアクセサ関数
	void setNeedDF(const ModBoolean needDF_);
	ModBoolean getNeedDF();

	// ラフノードの生成
	ModInvertedQueryNode* createRoughNode(const ModInvertedQuery* Query,
					const ModInvertedQuery::ValidateMode mode);

	// endNodeのアクセサ関数
	virtual QueryNode* getEndNode() const;
	virtual void setEndNode(QueryNode* endNode_);

	// 粗い evalaute  のための検索式ノードの取得
	virtual ModInvertedQueryNode* getQueryNodeForRoughEvaluation() const;
	virtual void setQueryNodeForRoughEvaluation(QueryNode* node);

	// RankingOr,RankingAndの場合は子ノード数の分だけsocresをリザーブ
	virtual void reserveScores();

	void changeSimpleTypeNode(
		ModVector<ModInvertedQueryNode*>::Iterator child,
		QueryNodePointerMap& nodePointerMap);

	void setTotalDocumentFrequency(const ModSize value) {
		ModInvertedQueryInternalNode::setTotalDocumentFrequency(value); }
	ModSize getTotalDocumentFrequency() const {
			return ModInvertedQueryInternalNode::getTotalDocumentFrequency(); }

#ifndef MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF
	void* operator new(unsigned int size, unsigned int dummy = 0) {
		return ModInvertedQueryInternalNode::operator new(size, dummy); }
	void operator delete(void* address, unsigned int size) {
		ModInvertedQueryInternalNode::operator delete(address, size); }
#endif // MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF

#ifdef DEBUG
	// sortFactor を表示(debug用)
		virtual void showSortFactor(ModUnicodeString& out);
#endif

//----- あいまい Query End ----------------------------------------------------

	// 子ノードのソート
	virtual void sortChildren(const ModInvertedQuery::ValidateMode mode);

	// sortFactor の計算
	virtual ModSize calcSortFactor();

	// 有効化の最後に子ノードの数をチェックする
	virtual void checkQueryNode(ModInvertedQuery*,
								const ModBoolean,
								const ModBoolean);

	// Get search term list
	virtual void getSearchTermList(
		ModInvertedQuery::SearchTermList& vecSearchTerm_,
		ModSize uiSynonymID_) const;
	
protected:
	// 粗い evaluate 満足を前提とした、正確な再評価
	virtual ModBoolean reevaluate(DocumentID documentID) = 0;
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* givenEndNode = 0) = 0;

	// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
	// Rankig InternalNodeはCombiner LeafNodeはCalculator
	// Atomic はCalculator
	virtual void getCalculatorOrCombinerName(ModUnicodeString& name,
											 const ModBoolean withParams) const;

//----- あいまい Query Start --------------------------------------------------
	// 検索条件ノードを出力 ラフノード表示 on/off 可
	virtual void getQueryString(ModUnicodeString& out,
								const ModBoolean asTermString = ModFalse,
								const ModBoolean withCalOrCombName = ModTrue,
								const ModBoolean withCalOrCombParam = ModTrue,
								const ModBoolean withRouh = ModFalse) const;

#ifdef DEBUG
	// 文書頻度の見積もり値を出力
	virtual void showEstimatedValue(ModUnicodeString& out);
#endif // DEBUG
//----- あいまい Query End ----------------------------------------------------
};

//
// FUNCTION protected
// ModInvertedWordBaseNode::ModInvertedWordBaseNode -- コンストラクタ
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
ModInvertedWordBaseNode::ModInvertedWordBaseNode()
{}

//
// FUNCTION protected
// ModInvertedWindowBaseNode::ModInvertedWindowBaseNode -- コンストラクタ
//
// NOTES
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
ModInvertedWordBaseNode::ModInvertedWordBaseNode(const NodeType nType,
	const ModUInt32 resultType_)
	: ModInvertedQueryInternalNode(nType,resultType_)
{
}

//
// FUNCTION public
// ModInvertedWordBaseNode::~ModInvertedWordBaseNode -- デストラクタ
//
// NOTES
// デストラクタ
//
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedWordBaseNode::~ModInvertedWordBaseNode()
{
	if(scoreCalculator != 0) {
		delete scoreCalculator;
		scoreCalculator = 0;
	}
}

//
// FUNCTION protected
// ModInvertedWordBaseNode::estimateDocumentFrequency -- 文書頻度の推定
//
// NOTES
// 文書頻度を推定する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModSize
ModInvertedWordBaseNode::estimateDocumentFrequency()
{
	if (this->estimatedDocumentFrequency == AtomicNode::UndefinedFrequency) {
		estimatedDocumentFrequency =  children[0]->estimateDocumentFrequency();
	}
	return this->estimatedDocumentFrequency;
}

//----- あいまい Atomic Start -------------------------------------------------
//
// FUNCTION public
// ModInvertedWordBaseNode::doFirstStepInRetrieveScore(
//      const ModInvertedDocumentID maxDocumentId)
//
inline void
ModInvertedWordBaseNode::doFirstStepInRetrieveScore(
	ModInvertedBooleanResult *expungedDocumentId,
	const ModInvertedDocumentID maxDocumentId)
{
	ModInvertedAtomicNode::doFirstStepInRetrieveScore(expungedDocumentId,maxDocumentId);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::doSecondStepInRetrieveScore(
//      ModInvertedRankingResult*& result_)
//
inline void
ModInvertedWordBaseNode::doSecondStepInRetrieveScore(
	ModInvertedSearchResult*& result_)
{
	ModInvertedAtomicNode::doSecondStepInRetrieveScore(result_);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::doSecondStepInRetrieveScore()
//
inline void
ModInvertedWordBaseNode::doSecondStepInRetrieveScore()
{
	ModInvertedAtomicNode::doSecondStepInRetrieveScore();
}

//
// FUNCTION public
// ModInvertedWordBaseNode::lowerBoundScoreForSecondStep()
//
inline ModBoolean
ModInvertedWordBaseNode::lowerBoundScoreForSecondStep(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
	return ModInvertedAtomicNode::lowerBoundScoreForSecondStep(
									givenID, foundID, score);
}

//
// FUNCTION protected
// ModInvertedWordBaseNode::getCalculatorOrCombinerName -- 自分の持ってい~るscoreCalculatorまたはscoreCombiner名を返す
//
// NOTES
// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す。
// AtomicNode::getCalculatorOrCombinerName(string)をコール。
//
// ARGUMENTS
// ModString& name
//      scoreCalculator名(結果格納用)。
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline void
ModInvertedWordBaseNode::getCalculatorOrCombinerName(
	ModUnicodeString& name,
	const ModBoolean withParams) const
{
	ModInvertedAtomicNode::getCalculatorOrCombinerName(name, withParams);
}
//----- あいまい Atomic End ---------------------------------------------------

//----- あいまい Query Start --------------------------------------------------
//
// FUNCTION public
// ModInvertedAtomicLocationNode::getDocumentFrequency -- 出現頻度を得る
//
// NOTES
// superクラスであるQueryNodeの
// getDocumentFrequency() を呼び出す。
//
// ARGUMENTS
// なし
//
// RETURN
// 出現頻度
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModSize
ModInvertedWordBaseNode::getDocumentFrequency(Query::EvaluateMode mode)
{
	return ModInvertedQueryBaseNode::getDocumentFrequency(mode);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::flattenChildren -- 子ノードリストの平坦化
//
// NOTES
// 実際に平坦化を行なうのはAndとOrだけなので、このクラスではなにもせず、
// 子ノードのflattenChildrenを呼ぶだけ
//
// ARGUMENTS
// const QueryNodePointerMap& sharedNodeMap
//  OR標準形変換時に共有しているノードが登録されているマップ変数
//
// const ModBoolean isChildOfWindowNode
//  windowノードを親にもっている場合 ModTure となる
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedWordBaseNode::flattenChildren(
	const QueryNodePointerMap& sharedNodeMap,
	const ModBoolean isChildOfWindowNode)
{
	ModInvertedQueryInternalNode::flattenChildren(sharedNodeMap,isChildOfWindowNode);
}

//
// FUNCTION public
// ModInvertedWordBaseBaseNode::contentString -- ノードの内容を表わす文字列を返す
//
// NOTES
// ノードの内容を表わす文字列を返す
//
// ただし QueryNode ではなにもしない。派生クラスで必要があればオーバラ
// イドする
//
// ARGUMENTS
// ModString& prefix
//    ノードの内容を表わす文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedWordBaseNode::contentString(ModUnicodeString& content) const
{
	ModInvertedQueryBaseNode::contentString(content);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::getEstimatedDocumentFrequency -- 文書頻度の見積もり値を返す
//
// NOTES
// 文書頻度の見積もり値を返す。
// BaseNode::getEstimatedDocumentFrequency()をコール。
//
//
// ARGUMENTS
// なし
//
// RETURN
// 文書頻度の見積もり値
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedWordBaseNode::getEstimatedDocumentFrequency() const
{
	return ModInvertedQueryBaseNode::getEstimatedDocumentFrequency();
}

//
// FUNCTION public
// ModInvertedWordBaseNode::eraseTermLeafNode -- TermLeafNodeの消去
//
// NOTES
//  あいまい解消のため。実際にはInternalNode::eraseTermLeafNodeを行う
//
// ARGUMENTS
// QueryNode*& node
//
// Query& query
//      Query
//
// RETURN
// この関数の呼び出し側で後処理が必要な場合 ModFalse を返す、特に必要
// ない場合 ModTrue を返す
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModInvertedWordBaseNode::eraseTermLeafNode(QueryNode*& node,
												   Query& query)
{
   return ModInvertedQueryInternalNode::eraseTermLeafNode(node, query);
}

//
// FUNCTION public
// ModInvertedWordBase::addRoughToGlobalNodeMap
//
inline void
ModInvertedWordBaseNode::addRoughToGlobalNodeMap(
	QueryNodeMap& globalNodeMap, QueryNodePointerMap& nodePointerMap)
{
	ModInvertedQueryInternalNode::addRoughToGlobalNodeMap(
									globalNodeMap, nodePointerMap);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::getTermLength
//
inline void
ModInvertedWordBaseNode::getTermLength(ModSize& length) const
{
	ModInvertedQueryInternalNode::getTermLength(length);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::getChildren
//
inline ModVector<ModInvertedQueryNode*>*
ModInvertedWordBaseNode::getChildren()
{
	return ModInvertedQueryInternalNode::getChildren();
}

//
// FUNCTION public
// ModInvertedWordBaseNode::getChildrenSize -- 子ノード数の取得
//
// NOTES
// 子ノード数を取得する。
//
// ARGUMENTS
// なし
//
// RETURN
// 子ノード数
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedWordBaseNode::getChildrenSize() const
{
	return ModInvertedQueryInternalNode::getChildrenSize();
}

//
// FUNCTION public
// ModInvertedWordBaseNode::sharedQueryNode
//
inline ModSize
ModInvertedWordBaseNode::sharedQueryNode(QueryNodeMap& globalNodeMap,
												 QueryNodePointerMap& nodePointerMap)
{
	return ModInvertedQueryInternalNode::sharedQueryNode(
											globalNodeMap,nodePointerMap);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::setFirstStepStatus(FirstStepStatus _status)
//
inline void
ModInvertedWordBaseNode::setFirstStepStatus(FirstStepStatus _status)
{
	ModInvertedQueryBaseNode::setFirstStepStatus(_status);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::getFirstStepStatus()
//
inline ModInvertedQueryNode::FirstStepStatus
ModInvertedWordBaseNode::getFirstStepStatus()
{
	return ModInvertedQueryBaseNode::getFirstStepStatus();
}

//
// FUNCTION public
// ModInvertedWordBaseNode::getFirstStepStatus()
//
inline ModInvertedSearchResult*
ModInvertedWordBaseNode::getRankingResult()
{
	return ModInvertedQueryBaseNode::getRankingResult();
}

//
// FUNCTION public
// ModInvertedWordBaseNode::needDocumentFrequency()
//
inline ModBoolean
ModInvertedWordBaseNode::needDocumentFrequency() const
{
	return ModInvertedQueryBaseNode::needDocumentFrequency();
}

//
// FUNCTION public
// ModInvertedWordBaseNode::setNeedDF(const ModBoolean needDF_)
//
inline void
ModInvertedWordBaseNode::setNeedDF(const ModBoolean needDF_)
{
	ModInvertedQueryBaseNode::setNeedDF(needDF_);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::getNeedDF()
//
inline ModBoolean
ModInvertedWordBaseNode::getNeedDF()
{
	return ModInvertedQueryBaseNode::getNeedDF();
}

//
// FUNCTION public
// ModInvertedWordBase::createRoughNode -- モードに応じたRoughNodeの生成
//
// NOTES
//
// 検索タイプに応じてラフノードを生成する。
// ブーリアン検索の場合はAndNode
// ランキング検索の場合はRankingAndNode
// アトミック検索の場合はAtmicAndNode
// を生成しアドレスを返す。
//
// ARGUMENTS
// const ModInvertedQuery* Query
//    Query
//
// RETURN
// 生成した RoughNode のアドレス
//
// EXCEPTIONS
// なし
//
inline ModInvertedQueryNode*
ModInvertedWordBaseNode::createRoughNode(
	const ModInvertedQuery* Query,
	const ModInvertedQuery::ValidateMode mode)
{
	return ModInvertedQueryBaseNode::createRoughNode(Query, mode);
}

//
// FUNCTION protected
// ModInvertedOpertorLocationNode::getQueryString -- 検索条件ノードを出力
//
// NOTES
// 検索条件ノードを出力(あいまい除去のため作成)。
// ModBooleanのデフォルト値は旧getQueryString()のもの
//
// ARGUMENTS
// ModOstrStream& out
//    結果格納用オブジェクト
// ModBoolean withRouh
//    ラフノードを表示するかどうかを示すフラグ（trueで表示）
//
// RETURN
// なし
//
inline void
ModInvertedWordBaseNode::getQueryString(ModUnicodeString& out,
	const ModBoolean asTermString,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam,
	const ModBoolean withRouh) const
{
	ModInvertedQueryInternalNode::getQueryString(out,
				asTermString, withCalOrCombName, withCalOrCombParam, withRouh);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::getEndNode -- endNodeのアクセサ関数
//
// NOTES
// OrderedDistanceNodeのendNodeのアクセサ関数。OrderedDistance以外のばあいは
// 本関数がコールされる。OrderedDistance以外はendNodeを持たないため常に0を返す。
// (BaseNode::getEndNodeをコール)
//
// ARGUMENTS
//  なし
//
// RETURN
// 常に 0
//
// EXCEPTIONS
// なし
//
inline ModInvertedQueryNode*
ModInvertedWordBaseNode::getEndNode() const
{
	return ModInvertedQueryBaseNode::getEndNode();
}

//
// FUNCTION public
// ModInvertedWordBaseNode::setEndNode -- endNodeのアクセサ関数
//
// NOTES
// OrderedDistanceNodeのendNodeのアクセサ関数。OrderedDistance以外のばあいは
// 本関数がコールされる。なにもしない。
// (BaseNode::setEndNodeをコール)
//
// ARGUMENTS
//  QueryNode* endNode_
//  セットするノード。
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedWordBaseNode::setEndNode(ModInvertedQueryNode* endNode_)
{
	ModInvertedQueryBaseNode::setEndNode(endNode_);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::getQueryNodeForRoughEvaluation -- 粗い評価用ノードの取得
//
// NOTES
// 粗い評価用ノードを取得する。
//
// ARGUMENTS
// なし
//
// RETURN
// 粗い評価用ノード
//
// EXCEPTIONS
// なし
//
inline ModInvertedQueryNode*
ModInvertedWordBaseNode::getQueryNodeForRoughEvaluation() const
{
	return ModInvertedQueryBaseNode::getQueryNodeForRoughEvaluation();
}

//
// FUNCTION public
// ModInvertedWordBaseNode::setQueryNodeForRoughEvaluation -- 粗い評価のためのノードをセットする
//
// NOTES
// 位置のつき合わせを省略した粗い評価のための検索式ノードへのポインタを
// セットする。BaseNode::setQueryNodeForRoughEvaluation()をコール
//
// ARGUMENTS
// ModInvertedQueryNode* queryNodeForRoughEvaluation_
//    粗い評価のためのノードへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedWordBaseNode::setQueryNodeForRoughEvaluation(QueryNode* node)
{
	ModInvertedQueryBaseNode::setQueryNodeForRoughEvaluation(node);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::reserveScores -- scoresをリザーブ
//
// NOTES
// 子ノードを辿りrankingOr,rankingAndの場合はメンバ変数であるscoresを
// リザーブする。本関数がコールされるのはrankingOr,rankingAnd以外の
// 中間ノードの場合であるため、自分の子ノードをたどる。
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
inline void
ModInvertedWordBaseNode::reserveScores()
{
	ModInvertedQueryInternalNode::reserveScores();
}

//
// FUNCTION public
// ModInvertedWordBaseNode::changeSimpleTypeNode
//
inline void
ModInvertedWordBaseNode::changeSimpleTypeNode(
	ModVector<ModInvertedQueryNode*>::Iterator child,
	QueryNodePointerMap& nodePointerMap)
{
	ModInvertedQueryInternalNode::changeSimpleTypeNode(child, nodePointerMap);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::lowerBoundScore -- lowerBoundのランキング版（スコアも計算す~る）
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
ModInvertedWordBaseNode::lowerBoundScore(const DocumentID givenID,
										 DocumentID& foundID,
										 DocumentScore& score,
										 Query::EvaluateMode mode)
{
	return ModInvertedAtomicNode::lowerBoundScore(givenID, foundID, score, mode);
}

// FUNCTION public
// ModInvertedWordBaseNode::evaluateScore -- 与えられた文書のスコアを計算する
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
ModInvertedWordBaseNode::evaluateScore(const DocumentID documentID,
									   DocumentScore& score,
									   Query::EvaluateMode mode)
{
	return AtomicNode::evaluateScore(documentID, score, mode);
}

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
//
// FUNCTION public
// ModInvertedWordBaseNode::evaluateScore -- 与えられた文書のスコアを計算する（位置も計~算する）
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
//
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModInvertedWordBaseNode::evaluateScore(const DocumentID documentID,
									   DocumentScore& score,
									   LocationIterator*& locations,
									   Query::EvaluateMode mode,
									   ModInvertedQueryNode* giveEndNode)
{
	return AtomicNode::evaluateScore(documentID, score, locations, mode);
}
#endif


#ifdef DEBUG
//
// FUNCTION public
// ModInvertedWordBaseNode::showSortFactor -- sortFactor を表示
//
// NOTES
// sortFactor を表示(debug用)。中間ノード用。QueryNodeで定義されている
// 内容をオーバライドしている。
//
// ARGUMENTS
// ModUnicodeString& out
//            sortFactor 値を含む文字情報
//
// RETURN
//    なし
//
inline void
ModInvertedWordBaseNode::showSortFactor(ModUnicodeString& out)
{
	ModInvertedQueryInternalNode::showSortFactor(out);
}

//
// FUNCTION protected
// ModInvertedWordBaseNode::showEstimatedValue -- 文書頻度の見積もり値を出力
//
// NOTES
// 文書頻度の見積もり値を出力(あいまい除去のため作成)。
//
// ARGUMENTS
// ModUnicodeString& out
//  結果格納用オブジェクト
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedWordBaseNode::showEstimatedValue(ModUnicodeString& out)
{
	ModInvertedQueryInternalNode::showEstimatedValue(out);
}
#endif // DEBUG

//----- あいまい Query End ----------------------------------------------------

//
// FUNCTION public
// ModInvertedWordBaseNode::setScoreCombiner -- スコア合成器をセットする
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
ModInvertedWordBaseNode::setScoreCombiner(
	ScoreCombiner* scoreCombiner_)
{
	// Atomic系のノードに合成器をセットする事はない
	ModAssert(0);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::setScoreCalculator -- スコア計算器をセットする
//
// NOTES
// スコア計算器をセットする。
//
// ARGUMENTS
// ScoreCalculator* calculator
//    スコア計算器へのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedWordBaseNode::setScoreCalculator(
	ScoreCalculator* calculator)
{
	scoreCalculator = calculator;
}

//
// FUNCTION public
// ModInvertedWordBaseNode::setScoreCalculator -- スコア計算器をセットする
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
ModInvertedWordBaseNode::setScoreCalculator(
			const ModUnicodeString& calculatorName)
{
	scoreCalculator = ModInvertedRankingScoreCalculator::create(calculatorName);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::setScoreCombiner -- スコア合成器をセットする
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
ModInvertedWordBaseNode::setScoreCombiner(
	const ModUnicodeString& calculatorName)
{
	// Atomic系ノードにCombinerをセットする事はない
	ModAssert(0);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::sortChildren -- 子ノードのソート
//
// NOTES
// 子ノードのソート。ただしなにもしない。
// And/Or/OperatorWindow/SimpleWindowでオーバライドする。
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
inline void
ModInvertedWordBaseNode::sortChildren(
	const ModInvertedQuery::ValidateMode mode)
{
	children[0]->sortChildren(mode);
}

//
// FUNCTION public
// ModInvertedWordBaseNode::calcSortFactor -- sortFactor の計算
//
// NOTES
// sortChildren() 関数で使用する sortFactor メンバ変数を計算する。
// QueryNode::calcSortFactor() をオーバライドする。中間ノードの
// sortFactor は、各子ノードの sortFactor の和となる。
//
// ARGUMENTS
// なし
//
// RETURN
// 計算した sortFactor 値。
//
inline ModSize
ModInvertedWordBaseNode::calcSortFactor()
{
	return children[0]->calcSortFactor();
}

//
// FUNCTION public
// ModInvertedWordBaseNode::getSearchTermList --
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
ModInvertedWordBaseNode::getSearchTermList(
	ModInvertedQuery::SearchTermList& vecSearchTerm_,
	ModSize uiSynonymID_) const
{
	// 子ノードは一つ
	return children[0]->getSearchTermList(vecSearchTerm_, uiSynonymID_);
}

#endif //__ModInvertedWordBaseNode_H__

//
// Copyright (c) 2002, 2004, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
