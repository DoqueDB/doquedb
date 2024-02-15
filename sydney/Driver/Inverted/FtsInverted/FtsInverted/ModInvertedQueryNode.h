// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedQueryNode.h -- 検索式の内部表現ノードのインタフェイス
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedQueryNode_H__
#define __ModInvertedQueryNode_H__

#include "ModTypes.h"
#include "ModMap.h"
#include "ModVector.h"
#include "ModInvertedTypes.h"
#include "ModInvertedManager.h"
#include "ModInvertedException.h"
#include "ModInvertedQuery.h"
#include "ModInvertedQueryNode.h"
#include "ModInvertedSearchResult.h"
#include "ModInvertedRankingScoreCalculator.h"
#ifdef V1_6
#include "ModLanguageSet.h"
#endif // V1_6

class ModOstrStream;

class ModInvertedLocationListIterator;
class ModInvertedScoreCalculator;
class ModInvertedRankingRegexLeafNode;
class ModInvertedAtomicNode;
class ModInvertedAtomicAndNode;
class ModInvertedAtomicAndNotNode;
class ModInvertedAtomicOrNode;

class ModInvertedIDScorePair;
class ModInvertedQueryBaseNode;
class ModInvertedQueryLeafNode;
class ModInvertedTermLeafNode;
class ModInvertedSimpleTokenLeafNode;
class ModInvertedQueryInternalNode;
class ModInvertedOperatorAndNode;
class ModInvertedOperatorAndNotNode;
class ModInvertedOperatorEndNode;
class ModInvertedOperatorLocationNode;
class ModInvertedLocationListIterator;
class ModInvertedRegexLeafNode;
class ModInvertedRankingResultLeafNode;

//
// 中間ノードの処理順序を単純な予想文書頻度ではなく、予想コスト順とする
//
#define	SORTFACTOR

//
// CLASS
// ModInvertedQueryNode -- 検索式の内部表現 (木構造) のノード
//
// NOTES
// 検索式の内部表現には、オペランドをオペレータで結んだ木構造を用いる。
// そのノードのクラス。
// 検索の実行、子ノードの管理を行なう。
// 検索の実行のついては、一部の例外を除き、Boolean検索結果やランキング
// 検索結果の他に、文書頻度、文書内頻度、文書内位置を得ることができる。
// 抽象クラスであり、実際の処理は派生クラスで行なう。
//
class
ModInvertedQueryNode : public ModInvertedObject
{
public:
	// クラス定義内の行を短くするための typedef
//	typedef ModInvertedDocumentID				DocumentID;
	typedef ModInvertedIDScorePair				IDScorePair;
	typedef ModInvertedDocumentScore			DocumentScore;
	typedef ModInvertedRankingScoreNegator		ScoreNegator;
	typedef ModInvertedRankingScoreCombiner		ScoreCombiner;
	typedef ModInvertedRankingScoreCalculator	ScoreCalculator;
	typedef ModInvertedAtomicNode				AtomicNode;
	typedef ModInvertedAtomicOrNode				AtomicOrNode;
	typedef ModInvertedFile						InvertedFile;
	typedef ModInvertedBooleanResult			BooleanResult;
	typedef ModInvertedQuery					Query;
	typedef ModInvertedQueryNode				QueryNode;
	typedef ModInvertedQueryBaseNode			BaseNode;
	typedef ModInvertedQueryLeafNode			LeafNode;
	typedef ModInvertedTermLeafNode				TermLeafNode;
	typedef ModInvertedSimpleTokenLeafNode		SimpleTokenLeafNode;
	typedef ModInvertedRegexLeafNode 			RegexLeafNode;
	typedef ModInvertedBooleanResultLeafNode	BooleanResultLeafNode;
	typedef ModInvertedQueryInternalNode		InternalNode;
	typedef ModInvertedOperatorOrNode			OrNode;
	typedef ModInvertedOperatorAndNode			AndNode;
	typedef ModInvertedOperatorAndNotNode		AndNotNode;
	typedef ModInvertedOperatorEndNode			EndNode;
	typedef ModInvertedOperatorLocationNode		LocationNode;
	typedef ModInvertedLocationListIterator		LocationIterator;
	typedef ModVector<ModInvertedLocationListIterator*>		LocationIterators;
	typedef ModMap<ModUnicodeString, QueryNode*, ModLess<ModUnicodeString> >
			QueryNodeMap;
	typedef ModMap<QueryNode*,int,ModLess<QueryNode*> >	QueryNodePointerMap;

	// ノードの種別
	enum NodeType {
		atomicNode					= 0x8000,		// アトミックフラグ
		atomicMask					= ~(atomicNode),// アトミックフラグマスク
		noType						= 0x0000,
		termLeafNode				= 0x0001,		// 検索語
		simpleTokenLeafNode			= 0x0002,		// 索引語
		booleanResultLeafNode		= 0x0011,		// Boolean検索結果
		rankingResultLeafNode		= 0x0012,		// ランキング検索結果
		regexLeafNode				= 0x0013,		// 正規表現
		internalNode				= 0x0100,		// 中間ノード
		operatorNotNode				= 0x0101,		// 補集合
		operatorAndNode				= 0x0102,		// 積集合
		operatorOrNode				= 0x0103,		// 和集合
		atomicOrNode				= operatorOrNode|atomicNode,
													// 和集合(アトミック)
		operatorAndNotNode			= 0x0104,		// 差集合
		operatorScaleNode			= 0x0105,		// Scale
		operatorLocationNode		= 0x0106,		// location
		operatorEndNode				= 0x0107,		// end
#ifdef V1_4	// 単語単位検索
		operatorWordNode			= 0x0108,		// word
#endif //  V1_4 単語単位検索
		unorderedNode				= 0x0010,		// window用 unordered flag
		windowNode					= 0x0300,		// 位置演算
		orderedOperatorWindowNode 	= 0x0301,
		orderedSimpleWindowNode 	= 0x0302,
		orderedDistanceNode			= 0x0303,
		unorderedWindowNode			= 0x0310,
		unorderedOperatorWindowNode	= 0x0311,
		unorderedSimpleWindowNode	= 0x0312
	};

	// 文書頻度や文書内頻度が計算されても見積もられてもいない (或いは
	// 計算できない) ことを表す特別な値。具体的な値は (unsigned)-1。
	static const ModSize UndefinedFrequency;
	static const QueryNode* emptySetNode;

	static const ModSize MaxSortFactor;

	static const Query::EvaluateMode defaultEMode;

	// デフォルトコンストラクタ
	ModInvertedQueryNode();

	// デストラクタ
	virtual ~ModInvertedQueryNode();

	// ラフノードの生成
	virtual ModInvertedQueryNode* createRoughNode(
		const ModInvertedQuery* Query,
		const ModInvertedQuery::ValidateMode mode) = 0;

	// 種別を得る
	virtual NodeType getType() const { return noType; }
#ifndef DEL_BOOL
	// 検索の一括実行
	virtual void retrieve(BooleanResult& queryResult,
						  Query::EvaluateMode mode) = 0;
#endif
	// 与えられた文書が検索条件を満たすかどうかの検査
	virtual ModBoolean evaluate(DocumentID documentID,
								Query::EvaluateMode mode) = 0;
	// 位置情報リストが必要版
	// ただし、位置情報リストが取得できない場合は、TFを取得する
	ModBoolean evaluate(DocumentID documentID,
						LocationIterator*& locations,
						ModSize& uiTF_,
						Query::EvaluateMode mode,
						ModInvertedQueryNode* givenEndNode = 0);
	// 位置情報リストが必要版 (TF不要版)
	ModBoolean evaluate(DocumentID documentID,
						LocationIterator*& locations,
						Query::EvaluateMode mode,
						ModInvertedQueryNode* givenEndNode = 0)
	{
		ModSize dummy = 0;
		return evaluate(documentID, locations, dummy, mode, givenEndNode);
	}

	// 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
	// 二番目の形式は、位置情報が必要な場合に用いられる。
	virtual ModBoolean lowerBound(DocumentID givenDocumentID,
								  DocumentID& foundDocumentID,
								  Query::EvaluateMode mode) = 0;

	virtual ModBoolean evaluateScore(const DocumentID documentID,
									 DocumentScore& score,
									 Query::EvaluateMode mode)  = 0;

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
	virtual ModBoolean evaluateScore(const DocumentID documentID,
									 DocumentScore& score,
									 LocationIterator*& locations,
									 Query::EvaluateMode mode,
									 ModInvertedQueryNode* givenEndNode = 0)=0;
#endif

	virtual ModBoolean lowerBoundScore(const DocumentID givenDocumentID,
									   DocumentID& foundDocumentID,
									   DocumentScore& score,
									   Query::EvaluateMode mode) = 0;

	// 文書頻度を見積もる
	virtual ModSize estimateDocumentFrequency() = 0;

	// 文書頻度を得る
	virtual ModSize getDocumentFrequency(Query::EvaluateMode mode) = 0;

	// 文書内頻度を得る
	virtual ModSize getTermFrequency(DocumentID documentID, 
									 Query::EvaluateMode mode) = 0;

	// 文書頻度による比較
	static ModBoolean lessFrequent(QueryNode* x, QueryNode* y);
	static ModBoolean moreFrequent(QueryNode* x, QueryNode* y);

	// sortFactor による比較 sortChildren での比較関数
	static ModBoolean lessSortFactor(QueryNode* x, QueryNode* y);

	// sortFactor の計算 - BaseNodeで実装
	virtual ModSize calcSortFactor() = 0;

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
							  const ModBoolean withCalOrCombName,
							  const ModBoolean withCalOrCombParam) const = 0;

	// ノードの内容を表わす文字列を返す
	virtual void contentString(ModUnicodeString& content) const = 0;

#ifdef DEBUG
	// sortFactor を表示(debug用)
	virtual void showSortFactor(ModUnicodeString& out) = 0;
#endif

	// 子ノードリストの平坦化 (例 #and(#and(X,Y),Z) → #and(X,Y,Z))
	virtual void flattenChildren(const QueryNodePointerMap& sharedNodeMap,
								 const ModBoolean isChildOfWindowNode) = 0;

	// 子ノード数を得る
	virtual ModSize getChildrenSize() const = 0;

	// 子ノードのソート
	virtual void sortChildren(const ModInvertedQuery::ValidateMode mode) = 0;

	// 中間ノードの共有化
	virtual ModSize sharedQueryNode(QueryNodeMap& globalNodeMap,
									QueryNodePointerMap& nodePointerMap) = 0;


	// TermLeafNodeの消去
	virtual ModBoolean eraseTermLeafNode(QueryNode*& node, Query& query) = 0;

	// queryNodeForRoughEvaluation の作成
	virtual void makeRoughPointer(const Query::ValidateMode,
								  QueryNodePointerMap&,
								  const ModInvertedQuery* Query) = 0;

	// 有効化の最後に子ノードの数をチェックする
	virtual void checkQueryNode(ModInvertedQuery*, 
								const ModBoolean,
								const ModBoolean) = 0;

	virtual ModInvertedSearchResult* getRankingResult() { return 0; }
	virtual void setRankingResult(ModInvertedSearchResult* firstStepResult_) { }
#if 0
	virtual void removeFromFirstStepResult(const ModInvertedSearchResult*);
#endif
	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery& query) = 0;


	// RankingOr,RankingAndの場合は子ノード数の分だけsocresをリザーブ
	virtual void reserveScores() {}

	// 粗い evaluate のアクセサ関数
	virtual void setQueryNodeForRoughEvaluation(QueryNode* node) = 0;
	virtual ModInvertedQueryNode* getQueryNodeForRoughEvaluation() const = 0;

	// endNodeのアクセサ関数
	virtual void setEndNode(QueryNode* endNode_) = 0;
	virtual QueryNode* getEndNode() const = 0;

	// スコア計算器をセットする -- Leaf/Atomicで実装する
	virtual void setScoreCalculator(ScoreCalculator* calculator) = 0;
	virtual void setScoreCalculator(const ModUnicodeString& calculatorName) = 0;

	// スコア合成器をセットする -- Leaf/Atomicで実装する
	virtual void setScoreCombiner(ScoreCombiner* scoreCombiner) = 0;
	virtual void setScoreCombiner(const ModUnicodeString& combinerName) = 0;

	// スコア合成器を取得する -- Leaf/Atomicで実装する
	virtual ScoreCombiner* getScoreCombiner() const = 0;

	// スコア計算器を取得する -- Leaf/Atomicで実装する
	virtual ScoreCalculator* getScoreCalculator() const = 0;

	// スコア否定器をセットする
	virtual void setScoreNegator(const ModUnicodeString& negatorName) {}

	// 文書頻度の見積もり値を返す
	virtual ModSize getEstimatedDocumentFrequency() const = 0;

	// 子ノードの取り出し
	virtual ModVector<ModInvertedQueryNode*>* getChildren() = 0;

	virtual void validate(InvertedFile* invertedFile,
						  const ModInvertedQuery::ValidateMode mode,
						  ModInvertedQuery* rQuery) = 0; 

	// ショートワード用Orかどうかの判定
	virtual ModBoolean isShortWordOrNode() const { return ModFalse; }

	// 検索語の長さを得る
	virtual void getTermLength(ModSize& length) const = 0;

	// RoughPointerの内容を共有化する
	virtual void addRoughToGlobalNodeMap(QueryNodeMap& globalNodeMap,
										 QueryNodePointerMap& nodePointerMap) = 0;

	// 全文書数のアクセサ関数
	virtual void setTotalDocumentFrequency(const ModSize freq) = 0;
	virtual ModSize getTotalDocumentFrequency() const = 0;

	// termLeafNodeから生成されるorderedDistance/AtomicWindowでオーバーライド
	// originalTermStringをセットする
	virtual void setOriginalString(const ModUnicodeString& termString,
#ifdef V1_6
								   const ModLanguageSet& langSet_,
#endif // V1_6
								   const ModInvertedTermMatchMode& mmode_) {}
	virtual ModBoolean getOriginalString(ModUnicodeString& termString,
#ifdef V1_6
										 ModLanguageSet& langSet_,
#endif // V1_6
								   		 ModInvertedTermMatchMode& mmode_) const
		{ return ModFalse; }
	ModBoolean isConvertedFromTermLeafNode() const;

   // 検索式文字列を出力
	virtual void getQueryString(ModUnicodeString& out,
								const ModBoolean asTermString = ModFalse,
								const ModBoolean withCalOrCombName = ModTrue,
								const ModBoolean withCalOrCombParam = ModTrue,
								const ModBoolean withRouh = ModFalse) const = 0;

	// ランキング検索のスコア計算第１ステップ
	virtual void doFirstStepInRetrieveScore(
			ModInvertedBooleanResult *expungedDocumentId,
			const ModInvertedDocumentID maxDocumentId) = 0;
	// ランキング検索のスコア計算第２ステップ
	virtual void doSecondStepInRetrieveScore() = 0;
	virtual void doSecondStepInRetrieveScore(
			ModInvertedSearchResult*& result) = 0;

	virtual ModBoolean lowerBoundScoreForSecondStep(
			ModInvertedDocumentID givenID,
			ModInvertedDocumentID& foundID,
			ModInvertedDocumentScore& score) = 0;

	// ランキング検索のスコア計算状態を保持するenum
	enum FirstStepStatus {
		initial,	// 何も処理されてない
		firstDone,	// 第１ステップ終了
		secondDone	// 第２ステップ終了
	};

	// firstStepStatusのアクセサ関数
	virtual void setFirstStepStatus(FirstStepStatus _status) = 0;
	virtual FirstStepStatus getFirstStepStatus() = 0;

	virtual ModBoolean needDocumentFrequency() const = 0;

    // needDFのアクセサ関数
	virtual void setNeedDF(const ModBoolean needDF_) = 0;
	virtual ModBoolean getNeedDF() = 0;

	// 粗い evaluate 満足を前提とした、正確な再評価
	virtual ModBoolean reevaluate(DocumentID documentID) = 0;
	// 位置情報が必要版
	// ただし、位置情報リストが取得できない場合は、TFを取得する
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* givenEndNode = 0) = 0;
	// 位置情報リストが必要版 (TF不要版)
	ModBoolean reevaluate(DocumentID documentID,
						  LocationIterator*& locations,
						  ModInvertedQueryNode* givenEndNode = 0)
	{
		ModSize dummy = 0;
		return reevaluate(documentID, locations, dummy, givenEndNode);
	}

	virtual void resetUpperLower(){};

	// prepareを呼ぶ
	virtual void prepareScoreCalculator(const ModSize totalFrequency_,
										const ModSize documentFrequency_);
	// prepareExを呼ぶ
	virtual void prepareScoreCalculatorEx(const ModUInt64 totalTermFrequency_,
										  const ModUInt64 totalDocumentLength_,
										  const ModUInt32 queryTermFrequency_);
	virtual bool isExtendedScoreCalculator()
	{
		ScoreCalculator* calculator = this->getScoreCalculator();
		return calculator?calculator->isExtendedFirstStep():false;
	}

	virtual ModUInt64 getTotalTermFrequency()
	{
		ScoreCalculator* calculator = this->getScoreCalculator();
		return calculator?calculator->getTotalTermFrequency():0;
	}

	// Get search term list
	virtual void getSearchTermList(
		ModInvertedQuery::SearchTermList& vecSearchTerm_,
		ModSize uiSynonymID_) const;
	
protected:
	// VC++ では以下の friend 宣言が必要
	friend class ModInvertedOperatorScaleNode;
	friend class ModInvertedAtomicOrNode;
	friend class ModInvertedQuery;
	friend class ModInvertedQueryBaseNode;
	friend class ModInvertedQueryInternalNode;
	friend class ModInvertedOperatorNotNode;
	friend class ModInvertedOperatorAndNode;
	friend class ModInvertedOperatorOrNode;
	friend class ModInvertedOperatorAndNotNode;
	friend class ModInvertedOperatorLocationNode;
	friend class ModInvertedOperatorEndNode;
#ifdef V1_4	// 単語単位検索
	friend class ModInvertedOperatorWordNode;
#endif // V1_4 単語単位検索
	friend class ModInvertedOrderedDistanceNode;
	friend class ModInvertedTermLeafNode;
	friend class ModInvertedSimpleWindowNode;
	friend class ModInvertedOperatorWindowNode;

#ifdef DEBUG
	// 文書頻度の見積もり値を出力
	virtual void showEstimatedValue(ModUnicodeString& out) = 0;
#endif //  DEBUG

	// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
	virtual void getCalculatorOrCombinerName(ModUnicodeString& name,
						const ModBoolean withParams) const = 0;

	// sharedQueryNode の補助関数
	virtual void changeSimpleTypeNode(
							ModVector<ModInvertedQueryNode*>::Iterator child,
							QueryNodePointerMap& nodePointerMap) = 0;

private:
#ifndef SYD_INVERTED	// 定義されてなかったら
	// 使用禁止
	ModInvertedQueryNode(const QueryNode& original);
	QueryNode& operator=(const QueryNode& original);
#endif
	static const BooleanResultLeafNode EmptySetNode;
protected:
};


#if 0
// 
// FUNCTION public
// ModInvertedAtomicNodeType -- アトミック検索用ノードタイプへ変換
// 
// NOTES
// ノードタイプをアトミック検索用ノードタイプへ変換するinline関数。
// アトミックノード Bit を ON にする。
// 
// ARGUMENTS
// ModInvertedQueryNode::NodeType nodeType
//	ノードタイプ
// 
// RETURN
// 変換したノードタイプ
// 
// EXCEPTIONS
// なし
//
inline ModInvertedQueryNode::NodeType
ModInvertedAtomicNodeType(ModInvertedQueryNode::NodeType nodeType)
{
	return (ModInvertedQueryNode::NodeType)
		(nodeType | ModInvertedQueryNode::atomicNode);
}
#endif

// 
// FUNCTION public
// ModInvertedAtomicMask -- アトミックをマスクする
// 
// NOTES
// アトミック検索用ノードタイプを示すビットをマスク（クリア）するinline関数
// (ランキングのビットはなくなった)
// 
// 
// ARGUMENTS
// ModInvertedQueryNode::NodeType nodeType
//	ノードタイプ
// 
// RETURN
// マスクしたノードタイプ
// 
// EXCEPTIONS
// なし
//
inline ModInvertedQueryNode::NodeType
ModInvertedAtomicMask(ModInvertedQueryNode::NodeType nodeType)
{
	return (ModInvertedQueryNode::NodeType)
		(nodeType & ~(ModInvertedQueryNode::atomicNode));
}


// 
// FUNCTION public
// ModInvertedIsAtomicQueryNode -- 検索タイプの判断
// 
// NOTES
// 検索タイプがアトミック検索か判断する。
// (アトミック検索用ノードタイプを示すビットが立っているか判断する)
// 
// 
// ARGUMENTS
// ModInvertedQueryNode::NodeType nodeType
//	ノードタイプ
// 
// RETURN
// アトミック検索の場合はModTrue、それ以外はModFalse
// 
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedIsAtomicQueryNode(ModInvertedQueryNode::NodeType nodeType)
{
	return ((nodeType & ModInvertedQueryNode::atomicNode) != 0) ?
		ModTrue : ModFalse;
}

// 
// FUNCTION public
// ModInvertedIsInternalNode -- ノードタイプの判断
// 
// NOTES
// 中間ノードか検査する
// 中間ノードのビットチェックし中間ノードならModTrue、LeafNodeなら
// ModFalseを返す
// 
// ARGUMENTS
// ModInvertedQueryNode::NodeType nodeType
//	ノードタイプ
// 
// RETURN
// 中間ノードの場合はModTrue、それ以外はModFalse
// 
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedIsInternalNode(ModInvertedQueryNode::NodeType nodeType)
{
	return ((nodeType & ModInvertedQueryNode::internalNode) != 0) ?
		ModTrue : ModFalse;
}

//
// FUNCTION public
// ModInvertedQueryNode::ModInvertedQueryNode -- 検索式内部表現ノードの生成
//
// NOTES
// 新しい検索式内部表現ノードオブジェクトを生成する。
//
// ARGUMENTS
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedQueryNode::ModInvertedQueryNode()
{
}

//
// FUNCTION public
// ModInvertedQueryNode::lessFrequent -- 予想文書頻度による比較
//
// NOTES
// ノードの予想文書頻度による比較。
//
// ARGUMENTS
// ModInvertedQueryNode* x
//		ノード１
// ModInvertedQueryNode* y
//		ノード２
// 
// RETURN
// ノード１の予想文書頻度が少ない場合 ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedQueryNode::lessFrequent(ModInvertedQueryNode* x,
								   ModInvertedQueryNode* y)
{
	return x->estimateDocumentFrequency() < y->estimateDocumentFrequency() ?
		ModTrue : ModFalse;
}

//
// FUNCTION public
// ModInvertedQueryNode::moreFrequent -- 予想文書頻度による比較
//
// NOTES
// ノードの予想文書頻度による比較。
//
// ARGUMENTS
// ModInvertedQueryNode* x
//		ノード１
// ModInvertedQueryNode* y
//		ノード２
// 
// RETURN
// ノード１の予想文書頻度が多い場合 ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedQueryNode::moreFrequent(ModInvertedQueryNode* x,
								   ModInvertedQueryNode* y)
{
	return x->estimateDocumentFrequency() > y->estimateDocumentFrequency() ?
		ModTrue : ModFalse;
}

// 
// FUNCTION public
// ModInvertedQueryNode::makeRoughPointer -- queryNodeForRoughEvaluation の作成
// 
// NOTES
// queryNodeForRoughEvaluation の作成
//
// queryNodeForRoughEvaluation は OR node 以外の Internal node に作る。
// ただし OrderedDistanceNode は TermLeafNode の削除処理で 
// queryNodeForRoughEvaluation が作成されるので本関数では作成しない。
//
// 本関数は何もせず 0 を返すだけだが And node, And-Not node によってオー
// バライドされる。つまり実際に queryNodeForRoughEvaluation を作るのは 
// And と And-Not の makeRoughPointer() である。
// 
// ARGUMENTS
// Query::ValidateMode mode
//		有効化（最適化）モード
// QueryNodePointerMap& parentRoughPointer
//		rough pointer の内容となるノードのアドレスが入った Map 変数
//		（結果格納用）
// 
// RETURN
// なし
// 
// EXCEPTIONS
// なし
//
inline void
ModInvertedQueryNode::makeRoughPointer(
	const Query::ValidateMode mode,
	QueryNodePointerMap& parentRoughPointer,
	const ModInvertedQuery* Query) 
{}

//
// FUNCTION public
// ModInvertedQueryNode::prepareScoreCalculator -- calculatorに対してprepareを呼ぶ
//
// NOTES
// 自身が持っているcalculatorに対してprepare()を呼ぶ。
//
// 但し、booleanResultLeafNode、rankingResultLeafNodeは計算器を持っていないので、
// 本関数をオーバーライドし、中では何もしないようにする。
//
// const ModSize totalFrequency
//      全文書数（N）
// const ModSize documentFrequency
//      出現数（df）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedQueryNode::prepareScoreCalculator(
	const ModSize totalFrequency_,
	const ModSize documentFrequency_)
{
	this->getScoreCalculator()->prepare(totalFrequency_,
										documentFrequency_);
}
inline void
ModInvertedQueryNode::prepareScoreCalculatorEx(
	const ModUInt64 totalTermFrequency_,
	const ModUInt64 totalDocumentLength_,
	const ModUInt32 queryTermFrequency_)
{
	this->getScoreCalculator()->prepareEx(totalTermFrequency_,
										totalDocumentLength_,
										queryTermFrequency_);
}
//
// VC++ でのコンパイルのために必要な関数
// 
inline void
ModDestroy(ModInvertedQueryNode**) 
{}

/*
inline void
ModInvertedQueryNode::setFirstStepStatus(FirstStepStatus _status)
{
	BaseNode::setFirstStepStatus(_status);
}
*/

#endif //__ModInvertedQueryNode_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
