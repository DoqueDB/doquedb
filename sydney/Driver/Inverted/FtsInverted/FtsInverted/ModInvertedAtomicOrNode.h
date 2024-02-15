// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedAtomicOrNode.h -- 和集合ノードのアトミック版インターフェース
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2004, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedAtomicOrNode_H__
#define __ModInvertedAtomicOrNode_H__

#include "ModInvertedAtomicNode.h"
#include "ModInvertedOperatorOrNode.h"
//
// CLASS
// ModInvertedAtomicOrNode -- アトミックなランキング検索を行なう和集合クラス
//
// NOTES
// このクラスは ModInvertedAtomicNode と ModInvertedOperarorOrNode を
// 多重継承したクラスであり、アトミックなランキング検索を行なう和集合
// ノードクラスである。
//
class
ModInvertedAtomicOrNode : 
	public ModInvertedOperatorOrNode, public ModInvertedAtomicNode
{
public:

	// コンストラクタ
	ModInvertedAtomicOrNode(const ModUInt32 resultType_= 0);

	// デストラクタ
	virtual ~ModInvertedAtomicOrNode();

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

#ifndef DEL_BOOL
	// 検索の一括実行
	void retrieve(BooleanResult& queryResult, Query::EvaluateMode mode);
#endif
	// 与えられた文書が検索条件を満たすかどうかの検査
	virtual ModBoolean evaluate(DocumentID documentID,
								Query::EvaluateMode mode);

	// 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
	virtual ModBoolean lowerBound(DocumentID givenDocumentID,
								  DocumentID& foundDocumentID,
								  Query::EvaluateMode mode);

	// 子ノードリストの平坦化 (例 #or(#or(X,Y),Z) → #or(X,Y,Z))
	// ただし、Atomicの場合はscoreCalculatorが同じ場合のみ可
	virtual void flattenChildren(const QueryNodePointerMap& sharedNodeMap,
								 const ModBoolean isChildOfWindowNode);

	// 文書内頻度を得る
	virtual ModSize getTermFrequency(DocumentID documentID, 
									 Query::EvaluateMode mode);

	// 出現頻度を得る
	virtual ModSize getDocumentFrequency(Query::EvaluateMode mode);

	// 文書頻度を見積もる
	virtual ModSize estimateDocumentFrequency();

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
			const ModBoolean withCalOrCombName,
			const ModBoolean withCalOrCombParam) const;

	// 文書頻度の見積もり値を返す
	virtual ModSize getEstimatedDocumentFrequency() const;

	void setTotalDocumentFrequency(const ModSize value) {
		ModInvertedOperatorOrNode::setTotalDocumentFrequency(value); }
	ModSize getTotalDocumentFrequency() const {
		return ModInvertedOperatorOrNode::getTotalDocumentFrequency(); }

#ifndef MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF
	void* operator new(unsigned int size, unsigned int dummy = 0) {
		return ModInvertedOperatorOrNode::operator new(size, dummy);
	}
	void operator delete(void* address, unsigned int size) {
		ModInvertedOperatorOrNode::operator delete(address, size);
	}
#endif // MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF


#if 1 	// あいまい ---------------------------------------------------------

	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery& query);

	// スコア計算器をセットする
	virtual void setScoreCalculator(
		ScoreCalculator* calculator);
	virtual void setScoreCalculator(
		const ModUnicodeString& calculatorName);

	// スコア合成器をセットする
	void setScoreCombiner(
		ScoreCombiner* scoreCombiner_);
	virtual void setScoreCombiner(
		const ModUnicodeString& combinerName);

	// スコア計算器を得る
	ScoreCalculator* getScoreCalculator() const { 
		return scoreCalculator; }

	// スコア合成器を得る
	ScoreCombiner* getScoreCombiner() const {
		return 0; }

	// endNodeのアクセサ関数
	virtual QueryNode* getEndNode() const;
	virtual void setEndNode(QueryNode* endNode_);

	// 粗い evalaute のための検索式ノードの取得
	virtual ModInvertedQueryNode* getQueryNodeForRoughEvaluation() const;
	virtual void setQueryNodeForRoughEvaluation(QueryNode* node);

	// RankingOr,RankingAndの場合は子ノード数の分だけsocresをリザーブ
	virtual void reserveScores();

	// 有効化の最後に子ノードの数をチェックする
	virtual void checkQueryNode(ModInvertedQuery*, 
								const ModBoolean,
								const ModBoolean);

	// 子ノード数を得る
	virtual ModSize getChildrenSize() const;

#ifdef DEBUG
	// sortFactor を表示(debug用)
	virtual void showSortFactor(ModUnicodeString& out);
#endif // DEBUG

	// ノードの内容を表わす文字列を返す
	virtual void contentString(ModUnicodeString& content) const;

	// sortFactor の計算
	virtual ModSize calcSortFactor();

	// ラフノードの生成
	ModInvertedQueryNode* createRoughNode(const ModInvertedQuery* Query,
				const ModInvertedQuery::ValidateMode mode);

	// TermLeafNode を消去して SimpleToken/OrderedDistance にする
	virtual ModBoolean eraseTermLeafNode(QueryNode*& node, Query& query);

	// queryNodeForRoughEvaluation の作成 - YOGA
	virtual void makeRoughPointer(const Query::ValidateMode,
								  QueryNodePointerMap&,
								  const ModInvertedQuery* Query);

	void changeSimpleTypeNode(ModVector<ModInvertedQueryNode*>::Iterator child,
							  QueryNodePointerMap& nodePointerMap);

	virtual void addRoughToGlobalNodeMap(QueryNodeMap& globalNodeMap,
										 QueryNodePointerMap& nodePointerMap);

	virtual ModBoolean isShortWordOrNode() const;

	virtual void getTermLength(ModSize& length) const;

	virtual ModVector<ModInvertedQueryNode*>* getChildren();

	virtual ModSize sharedQueryNode(QueryNodeMap& globalNodeMap,
									QueryNodePointerMap& nodePointerMap);

	// ランキング検索のスコア計算第１ステップ
	// スコア計算の第１ステップ
	void doFirstStepInRetrieveScore(
		ModInvertedBooleanResult *expungedDocumentId,
		const ModInvertedDocumentID maxDocumentId);

	// スコア計算の第２ステップ
	void doSecondStepInRetrieveScore(
		ModInvertedRankingResult*& result);
	void doSecondStepInRetrieveScore();

	// スコア計算の第２ステップで使用するlowerBound
	ModBoolean lowerBoundScoreForSecondStep(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score);

	// firstStepStatusのアクセサ関数
	void setFirstStepStatus(FirstStepStatus _status);
	FirstStepStatus getFirstStepStatus();
	
	ModInvertedSearchResult* getRankingResult();

	ModBoolean needDocumentFrequency() const;

	// needDFのアクセサ関数
	void setNeedDF(const ModBoolean needDF_);
	ModBoolean getNeedDF();

#endif	// あいまい ---------------------------------------------------------

	virtual void setOriginalString(const ModUnicodeString& termString,
#ifdef V1_6
								   const ModLanguageSet& langSet_,
#endif // V1_6
								   const ModInvertedTermMatchMode&  mmode_);
	ModBoolean getOriginalString(ModUnicodeString& termString,
#ifdef V1_6
								 ModLanguageSet& langSet_,
#endif // V1_6
								 ModInvertedTermMatchMode& mmode_) const;
	
	// Get search term list
	virtual void getSearchTermList(
		ModInvertedQuery::SearchTermList& vecSearchTerm_,
		ModSize uiSynonymID_) const;

protected:

	// 有効化
	virtual void validate(InvertedFile* invertedFile,
						  const ModInvertedQuery::ValidateMode mode,
						  ModInvertedQuery* rQuery);

#ifdef DEBUG
	// 文書頻度の見積もり値を出力
	virtual void showEstimatedValue(ModUnicodeString& out);
#endif

	// 検索条件ノードを出力 ラフノード表示 on/off 可
	virtual void getQueryString(ModUnicodeString& out, 
								  const ModBoolean asTermString = ModFalse,
								  const ModBoolean withCalOrCombName = ModTrue,
								  const ModBoolean withCalOrCombParam = ModTrue,
								  const ModBoolean withRouh = ModFalse) const;

	ModBoolean reevaluate(DocumentID documentID);
	ModBoolean reevaluate(DocumentID documentID,
						  LocationIterator*& locations,
						  ModSize& uiTF,
						  ModInvertedQueryNode* givenEndNode = 0);

	ModBoolean lowerBoundLocal(DocumentID givenDocumentID,
							   DocumentID& foundDocumentID,
							   Query::EvaluateMode mode,
							   ModVector<ModInvertedQueryNode*>& tmp);
	ModSize getTermFrequencyLocal(DocumentID givenDocumentID,
								  Query::EvaluateMode mode,
								  ModVector<ModInvertedQueryNode*>& tmp);


	// 子ノードリストの並べ替え
	void sortChildren(const ModInvertedQuery::ValidateMode mode);

	// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
	virtual void getCalculatorOrCombinerName(ModUnicodeString& name,
											 const ModBoolean withParams) const;

#ifdef V1_6
	ModLanguageSet langSet;
#endif // V1_6

private:

#ifdef DEL_BOOL
	// boolean
	void retrieve(ModInvertedBooleanResult *expungedDocumentId,
					const ModInvertedDocumentID maxDocumentId,
					ModInvertedBooleanResult *result);
#endif
	// ranking
	void retrieve(ModInvertedBooleanResult *expungedDocumentId,
					const ModInvertedDocumentID maxDocumentId,
					ModInvertedSearchResult *result);

	// 使用禁止
	ModInvertedAtomicOrNode(const AtomicOrNode& original);
	AtomicOrNode& operator=(const AtomicOrNode& original);
};


// 
// FUNCTION public
// ModInvertedAtomicOrNode::getDocumentFrequency -- 出現頻度を得る
// 
// NOTES
// superクラスであるQueryNodeの
// getDocumentFrequency() を呼び出す。
//
// ARGUMENTS
// Query::EvaluateMode mode
//		評価モード
// 
// RETURN
// 出現頻度
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModSize
ModInvertedAtomicOrNode::getDocumentFrequency(Query::EvaluateMode mode)
{
	return ModInvertedOperatorOrNode::getDocumentFrequency(mode);
}

// 
// FUNCTION public
// ModInvertedTermLeafNode::estimateDocumentFrequency -- 文書頻度の見積もり
// 
// NOTES
// 条件を満たす文書数を見積もる。superクラスであるTermLeafNodeの
// estimateDocumentFrequency() を呼び出す。
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
ModInvertedAtomicOrNode::estimateDocumentFrequency()
{
	return ModInvertedOperatorOrNode::estimateDocumentFrequency();
}

//
// FUNCTION protected
// ModInvertedAtomicOrNode::getCalculatorOrCombinerName -- 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
//
// NOTES
// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す。
// AtomicNode::getCalculatorOrCombinerName(string)をコール。
//
// ARGUMENTS
// ModString& name
//		scoreCalculator名(結果格納用)。
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline void
ModInvertedAtomicOrNode::getCalculatorOrCombinerName(ModUnicodeString& name,
	const ModBoolean withParams) const
{
	ModInvertedAtomicNode::getCalculatorOrCombinerName(name, withParams);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::getEstimatedDocumentFrequency -- 文書頻度の見積もり値を返す
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
ModInvertedAtomicOrNode::getEstimatedDocumentFrequency() const
{
	return ModInvertedQueryBaseNode::getEstimatedDocumentFrequency();
}

#if 1	// あいまい

//
// FUNCTION public
// ModInvertedAtomicOrNode::setScoreCalculator -- スコア計算器をセットする
//
// NOTES
// スコア計算器をセットする。
//
// ARGUMENTS
// ScoreCalculator* calculator
//		スコア計算器へのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedAtomicOrNode::setScoreCalculator(
	ScoreCalculator* calculator)
{
	scoreCalculator = calculator;
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::setScoreCalculator -- スコア計算器をセットする
//
// NOTES
// スコア計算器をセットする。
//
// ARGUMENTS
// const ModUnicodeString& calculatorName
//		スコア計算器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedAtomicOrNode::setScoreCalculator(
	const ModUnicodeString& calculatorName)
{
	scoreCalculator = ModInvertedRankingScoreCalculator::create(calculatorName);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::setScoreCombiner -- スコア合成器をセットする
//
// NOTES
// スコア合成器をセットする。
//
// ARGUMENTS
// const ScoreCombiner* scoreCombiner_
//		スコア合成器
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedAtomicOrNode::setScoreCombiner(
	ScoreCombiner* scoreCombiner_)
{
	; ModAssert(0);
}

//
// FUNCTION public
// ModInvertedAtocmiOrNode::setScoreCombiner -- スコア合成器をセットする
//
// NOTES
// スコア合成器をセットする。
//
// ARGUMENTS
// const ModUnicodeString& combinerName
//		スコア計算器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedAtomicOrNode::setScoreCombiner(const ModUnicodeString& combinerName)
{
	; ModAssert(0);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::getEndNode -- endNodeのアクセサ関数
//
// NOTES
// OrderedDistanceNodeのendNodeのアクセサ関数。OrderedDistance以外のばあいは
// 本関数がコールされる。OrderedDistance以外はendNodeを持たないため常に0を返す。
// (BaseNode::getEndNodeをコール)
//
// ARGUMENTS
// なし
//
// RETURN
// 常に 0
//
// EXCEPTIONS
// なし
//
inline ModInvertedQueryNode*
ModInvertedAtomicOrNode::getEndNode() const
{
	return ModInvertedQueryBaseNode::getEndNode();
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::setEndNode -- endNodeのアクセサ関数
//
// NOTES
// OrderedDistanceNodeのendNodeのアクセサ関数。OrderedDistance以外のばあいは
// 本関数がコールされる。なにもしない。
// (BaseNode::setEndNodeをコール)
//
// ARGUMENTS
// QueryNode* endNode_
//		セットするノード。
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedAtomicOrNode::setEndNode(ModInvertedQueryNode* endNode_)
{
	ModInvertedQueryBaseNode::setEndNode(endNode_);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::getQueryNodeForRoughEvaluation -- 粗い評価用ノードの取得
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
ModInvertedAtomicOrNode::getQueryNodeForRoughEvaluation() const
{
	return ModInvertedQueryBaseNode::getQueryNodeForRoughEvaluation();
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::setQueryNodeForRoughEvaluation -- 粗い評価のためのノードをセットする
//
// NOTES
// 位置のつき合わせを省略した粗い評価のための検索式ノードへのポインタを
// セットする。BaseNode::setQueryNodeForRoughEvaluation()をコール
//
// ARGUMENTS
// ModInvertedQueryNode* queryNodeForRoughEvaluation_
//		粗い評価のためのノードへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedAtomicOrNode::setQueryNodeForRoughEvaluation(QueryNode* node)
{
	ModInvertedQueryBaseNode::setQueryNodeForRoughEvaluation(node);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::reserveScores -- scoresをリザーブ
//
// NOTES
// 子ノードを辿りrankingOr,rankingAndの場合はメンバ変数であるscoresを
// リザーブする。本関数がコールされるのはrankingOr,rankingAnd以外の
// 中間ノードの場合であるため、自分の子ノードをたどる。
// (InternalNode::reserveScore()をコール)
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
ModInvertedAtomicOrNode::reserveScores()
{
	ModInvertedQueryInternalNode::reserveScores();
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::getChildrenSize -- 子ノード数の取得
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
ModInvertedAtomicOrNode::getChildrenSize() const
{
	return ModInvertedQueryInternalNode::getChildrenSize();
}

#ifdef DEBUG
//
// FUNCTION public
// ModInvertedAtomicOrNode::showSortFactor -- sortFactor を表示
//
// NOTES
// sortFactor を表示(debug用)。中間ノード用。QueryNodeで定義されている
// 内容をオーバライドしている。
//
// ARGUMENTS
// ModUnicodeString&
//		sortFactor 値を含む文字情報
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedAtomicOrNode::showSortFactor(ModUnicodeString& out)
{
	ModInvertedQueryInternalNode::showSortFactor(out);
}
#endif // DEBUG

//
// FUNCTION public
// ModInvertedAtomicOrBaseNode::contentString -- ノードの内容を表わす文字列を返す
//
// NOTES
// ノードの内容を表わす文字列を返す
//
// ただし QueryNode ではなにもしない。派生クラスで必要があればオーバラ
// イドする
//
// ARGUMENTS
// ModString& prefix
//		ノードの内容を表わす文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedAtomicOrNode::contentString(ModUnicodeString& content) const
{
	ModInvertedQueryBaseNode::contentString(content);
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::calcSortFactor -- sortFactor の計算
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
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedAtomicOrNode::calcSortFactor()
{
	return ModInvertedOperatorOrNode::calcSortFactor();
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::createRoughNode -- モードに応じたRoughNodeの生成
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
//		クエリ
//
// RETURN
// 生成した RoughNode のアドレス
//
// EXCEPTIONS
// なし
//
inline ModInvertedQueryNode*
ModInvertedAtomicOrNode::createRoughNode(
	const ModInvertedQuery* Query,
	const ModInvertedQuery::ValidateMode mode)
{
	return ModInvertedQueryBaseNode::createRoughNode(Query, mode);
}

#ifdef DEBUG
//
// FUNCTION protected
// ModInvertedAtomicOrNode::showEstimatedValue -- 文書頻度の見積もり値を出力
//
// NOTES
// 文書頻度の見積もり値を出力(あいまい除去のため作成)。
//
// ARGUMENTS
// ModUnicodeString&
//		結果格納用オブジェクト
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedAtomicOrNode::showEstimatedValue(ModUnicodeString& out)
{
	ModInvertedQueryInternalNode::showEstimatedValue(out);
}
#endif

//
// FUNCTION protected
// ModInvertedAtomicOrNode::getQueryString -- 検索条件ノードを出力
//
// NOTES
// 検索条件ノードを出力(あいまい除去のため作成)。
// ModBooleanのデフォルト値は旧getQueryStringのもの
//
// ARGUMENTS
// ModOstrStream& out
//		結果格納用オブジェクト
// ModBoolean withCalOrCombName,
//		スコア計算器・合成器の表示ON/OFF
// ModBoolean withCalOrCombParam,
//		スコア計算器・合成器のパラメータ表示ON/OFF
// ModBoolean withRouh
//		ラフノードを表示するかどうかを示すフラグ（trueで表示）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedAtomicOrNode::getQueryString(ModUnicodeString& out,
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
// ModInvertedAtomicOrNode::eraseTermLeafNode -- TermLeafNode消去
//
// NOTES
// TermLeafNode を消去して SimpleToken/OrderedDistance にする
// QueryNode の eraseTermLeafNode をオーバライドしている
//
// 子ノードのTermLeafNodeが EmptySetNode であった場合その子ノードは削
// 除する。削除して子ノードが1つしかない状態になったら、残った子ノードを
// 昇格させるため引数 node に代入して ModFalse で戻る。子ノード数が 0
// になったら、引数 node は 0 にして ModFalse で戻る。それ以外は
// ModTrue で戻る。
//
// ARGUMENTS
// QueryNode*& node
//		子ノードが1つしかなくなって昇格させる場合のノードポインタ (結果格納用)
// Query& query
//		クエリ
//
// RETURN
// この関数の呼び出し側で後処理が必要な場合 ModFalse を返す、特に必要
// ない場合 ModTrue を返す
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModInvertedAtomicOrNode::eraseTermLeafNode(QueryNode*& node, Query& query)
{
	ModBoolean retVal;
	node = 0;

	retVal = ModInvertedOperatorOrNode::eraseTermLeafNode(node, query);

	if(retVal == ModFalse) {
		// AtomicOrの場合スコアが変わってしまう可能性が有るので
		// 子ノードがひとつであっても昇格させない
		if(node != 0) {
			node = 0;
			retVal = ModTrue;
		}
	}
	return retVal;
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::makeRoughPointer -- queryNodeForRoughEvaluation の作成
//
// NOTES
// 自分にはラフを設定せず、子ノードに対し再帰的に makeRoughPointer を呼び出す。
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
inline void
ModInvertedAtomicOrNode::makeRoughPointer(const Query::ValidateMode mode,
										  QueryNodePointerMap& parentMap,
										  const ModInvertedQuery* Query)
{
	ModInvertedOperatorOrNode::makeRoughPointer(mode, parentMap,Query);
}

// 追加 2000/02/29
//
// FUNCTION public
// ModInvertedAtomicOrNode::changeSimpleTypeNode
//
inline void
ModInvertedAtomicOrNode::changeSimpleTypeNode(
	ModVector<ModInvertedQueryNode*>::Iterator child,
	QueryNodePointerMap& nodePointerMap)
{
	ModInvertedOperatorOrNode::changeSimpleTypeNode(child, nodePointerMap);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::addRoughToGlobalNodeMap
//
inline void
ModInvertedAtomicOrNode::addRoughToGlobalNodeMap(
	QueryNodeMap& globalNodeMap, QueryNodePointerMap& nodePointerMap)
{
	ModInvertedOperatorOrNode::addRoughToGlobalNodeMap(
										globalNodeMap, nodePointerMap);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::isShortWordOrNode
//
inline ModBoolean
ModInvertedAtomicOrNode::isShortWordOrNode() const
{
	return ModInvertedOperatorOrNode::isShortWordOrNode();
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::getTermLength
//
inline void
ModInvertedAtomicOrNode::getTermLength(ModSize& length) const
{
	ModInvertedOperatorOrNode::getTermLength(length);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::getChildren
//
inline ModVector<ModInvertedQueryNode*>*
ModInvertedAtomicOrNode::getChildren()
{
	return ModInvertedOperatorOrNode::getChildren();
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::sharedQueryNode
//
inline ModSize
ModInvertedAtomicOrNode::sharedQueryNode(QueryNodeMap& globalNodeMap,
	QueryNodePointerMap& nodePointerMap)
{
	return ModInvertedOperatorOrNode::sharedQueryNode(
		globalNodeMap,nodePointerMap);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::doSecondStepInRetrieveScore(
//		ModInvertedRankingResult* result_)
//
inline void
ModInvertedAtomicOrNode::doSecondStepInRetrieveScore(
		ModInvertedSearchResult*& result_)
{
	ModInvertedAtomicNode::doSecondStepInRetrieveScore(result_);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::doSecondStepInRetrieveScore()
//
inline void
ModInvertedAtomicOrNode::doSecondStepInRetrieveScore()
{
	ModInvertedAtomicNode::doSecondStepInRetrieveScore();
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::lowerBoundScoreForSecondStep()
//
inline ModBoolean
ModInvertedAtomicOrNode::lowerBoundScoreForSecondStep(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
	return ModInvertedAtomicNode::lowerBoundScoreForSecondStep(
										givenID, foundID, score);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::setFirstStepStatus(FirstStepStatus _status)
//
inline void
ModInvertedAtomicOrNode::setFirstStepStatus(FirstStepStatus _status)
{
	ModInvertedQueryBaseNode::setFirstStepStatus(_status);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::getFirstStepStatus()
//
inline ModInvertedQueryNode::FirstStepStatus
ModInvertedAtomicOrNode::getFirstStepStatus()
{
	return ModInvertedQueryBaseNode::getFirstStepStatus();
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::getFirstStepStatus()
//
inline ModInvertedRankingResult*
ModInvertedAtomicOrNode::getRankingResult()
{
	return ModInvertedQueryBaseNode::getRankingResult(); 
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::needDocumentFrequency()
//
inline ModBoolean
ModInvertedAtomicOrNode::needDocumentFrequency() const
{
	return ModInvertedQueryBaseNode::needDocumentFrequency();
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::setNeedDF(const ModBoolean needDF_)
//
inline void
ModInvertedAtomicOrNode::setNeedDF(const ModBoolean needDF_)
{
	ModInvertedQueryBaseNode::setNeedDF(needDF_);
}

//
// FUNCTION public
// ModInvertedAtomicOrNode::getNeedDF()
//
inline ModBoolean
ModInvertedAtomicOrNode::getNeedDF()
{
	return ModInvertedQueryBaseNode::getNeedDF();
}

#endif	// 1 あいまい

//
// FUNCTION public
// ModInvertedAtomicOrNode::setOriginalString() -- 自分が生成された元のTermLeafNodeの
//												   文字列をメンバ変数originalTermStringにセットする
//
// NOTES
// 自分が生成された元のTermLeafNodeの文字列をメンバ変数originalTermStringに
// セットする。OperatorOr::setOriginalString()をオーバーライド。
//
// ARGUMENTS
// ModUnicodeString termString
//      セットする文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedAtomicOrNode::setOriginalString(const ModUnicodeString& termString,
#ifdef V1_6
										   const ModLanguageSet& langSet_,
#endif // V1_6
										   const ModInvertedTermMatchMode& mmode_)
{
	ModInvertedOperatorOrNode::setOriginalString(termString,
#ifdef V1_6
												 langSet_,
#endif // V1_6
										  		 mmode_);
}

//
// FUNCTION public
// ModInvertedAtomicOrLeafNode::getOriginalString() -- 自分が生成された元のTermLeafNodeの~文字列を返す。
//
// NOTES
// 自分が生成された元のTermLeafNodeの文字列を返す。元の文字列はメンバ変数
// originalTermStringにeraseTermLeafNodeQuery()でセットされる。
//
// originalTermStringがセットされていない場合はFalseを返す。
//
// ARGUMENTS
// ModUnicodeString& termString
//      結果格納用
//
// RETURN
// originalTermStringがある場合True/ない場合False
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedAtomicOrNode::getOriginalString(ModUnicodeString& termString,
#ifdef V1_6
										   ModLanguageSet& langSet_,
#endif // V1_6
										   ModInvertedTermMatchMode& mmode_) const
{
	return ModInvertedOperatorOrNode::getOriginalString(termString,
#ifdef V1_6
														langSet_,
#endif // V1_6
														mmode_);
}
#endif // __ModInvertedAtomicOrNode_H__

//
// Copyright (c) 1999, 2000, 2001, 2002, 2004, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
