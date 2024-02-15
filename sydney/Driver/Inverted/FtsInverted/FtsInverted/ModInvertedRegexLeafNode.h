// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedRegexLeafNode.h -- 正規表現に対応する末端ノード
// 
// Copyright (c) 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedRegexLeafNode_H__
#define __ModInvertedRegexLeafNode_H__

#include "ModUnicodeRegularExpression.h"		// 正規表現関係
#include "ModInvertedQueryLeafNode.h"
#include "ModInvertedQuery.h"

#include "ModInvertedAtomicNode.h"

class ModInvertedFile;
class ModInvertedDocumentIterator;

// 
// regexのデバック
// 	文書読み出し、パターンマッチングの回数を調べる
// #define REGEX_DEBUG

//
// CLASS
// ModInvertedRegexLeafNode -- 正規表現に対応する末端ノード
//
// NOTES
//
class ModInvertedRegexLeafNode 
	: public ModInvertedQueryLeafNode, ModInvertedAtomicNode
{
public:
	// コンストラクタ
	ModInvertedRegexLeafNode(const ModUnicodeString& termString_,const  ModUInt32 resultType_);

	// デストラクタ
	virtual ~ModInvertedRegexLeafNode();

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

	// 文書内頻度を得る
	virtual ModSize getTermFrequency(DocumentID documentID, 
									 Query::EvaluateMode mode);

	// 正規表現ノードの有効化
	// void validate(ModInvertedFile* const invertedFile);
	void validate(InvertedFile* invertedFile,
					const ModInvertedQuery::ValidateMode mode,
					ModInvertedQuery* rQuery);


	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery& query);

	virtual void contentString(ModUnicodeString& content) const;

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
			const ModBoolean withCalOrCombName,
			const ModBoolean withCalOrCombParam) const;

	void setTotalDocumentFrequency(const ModSize value) {
		ModInvertedQueryLeafNode::setTotalDocumentFrequency(value); }
	ModSize getTotalDocumentFrequency() const {
		return ModInvertedQueryLeafNode::getTotalDocumentFrequency(); }

#ifndef MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF
	void* operator new(unsigned int size, unsigned int dummy = 0) {
		return ModInvertedQueryLeafNode::operator new(size, dummy); }
	void operator delete(void* address, unsigned int size) {
		ModInvertedQueryLeafNode::operator delete(address, size); }
#endif // MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF


#if 1	// あいまい ----------------------------------------------------------
	// 文書頻度の見積もり値を返す
	virtual ModSize getEstimatedDocumentFrequency() const;

	// endNodeのアクセサ関数
	virtual QueryNode* getEndNode() const;
	virtual void setEndNode(QueryNode* endNode_);

	// 粗い evalaute  のための検索式ノードの取得
	virtual ModInvertedQueryNode* getQueryNodeForRoughEvaluation() const;

	// 粗い evaluate のための検索式ノードをセットする
	virtual void setQueryNodeForRoughEvaluation(QueryNode* node);

	// RankingOr,RankingAndの場合は子ノード数の分だけsocresをリザーブ
	virtual void reserveScores();

	// 有効化の最後に子ノードの数をチェックする
	virtual void checkQueryNode(ModInvertedQuery*, 
								const ModBoolean,
								const ModBoolean);

#ifdef DEBUG
	// sortFactor を表示(debug用)
	virtual void showSortFactor(ModUnicodeString& out);
#endif

	// queryNodeForRoughEvaluation の作成
	virtual void makeRoughPointer(const Query::ValidateMode,
											QueryNodePointerMap&,
											const ModInvertedQuery* Query);

	// TermLeafNodeの消去
	virtual ModBoolean eraseTermLeafNode(QueryNode*& node, Query& query);


	// 子ノードのソート
	virtual void sortChildren(const ModInvertedQuery::ValidateMode mode);

	// 子ノード数を得る
	virtual ModSize getChildrenSize() const;

	// 子ノードリストの平坦化 (例 #and(#and(X,Y),Z) → #and(X,Y,Z))
	virtual void flattenChildren(
				const QueryNodePointerMap& sharedNodeMap,
				const ModBoolean isChildOfWindowNode);

	// sortFactor の計算
	virtual ModSize calcSortFactor();

	virtual ModSize getDocumentFrequency(Query::EvaluateMode mode);

	// ラフノードの生成
	ModInvertedQueryNode* createRoughNode(const ModInvertedQuery* Query,
					const ModInvertedQuery::ValidateMode mode);

	void changeSimpleTypeNode(
			ModVector<ModInvertedQueryNode*>::Iterator child,
			QueryNodePointerMap& nodePointerMap);

	virtual void addRoughToGlobalNodeMap(QueryNodeMap& globalNodeMap,
			QueryNodePointerMap& nodePointerMap);

	virtual void getTermLength(ModSize& length) const;

	virtual ModVector<ModInvertedQueryNode*>* getChildren();

	virtual ModSize sharedQueryNode(QueryNodeMap& globalNodeMap,
									QueryNodePointerMap& nodePointerMap);

// ここまで 追加 2000/02/29

	// スコア計算器を得る
	ScoreCalculator* getScoreCalculator() const {
		return ModInvertedQueryLeafNode::getScoreCalculator(); }
	// スコア合成器を得る
	ScoreCombiner* getScoreCombiner() const {
		return ModInvertedQueryLeafNode::getScoreCombiner(); }

	// スコア合成器をセットする
	void setScoreCombiner(ScoreCombiner* scoreCombiner_) {
		ModInvertedQueryLeafNode::setScoreCombiner(scoreCombiner_); }
	virtual void setScoreCombiner(const ModUnicodeString& combinerName) {
		ModInvertedQueryLeafNode::setScoreCombiner(combinerName); }

	// スコア計算器をセットする
	virtual void setScoreCalculator(ScoreCalculator* calculator) {
		ModInvertedQueryLeafNode::setScoreCalculator(calculator); }
	virtual void setScoreCalculator(const ModUnicodeString& calculatorName) {
		ModInvertedQueryLeafNode::setScoreCalculator(calculatorName); }

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

	// firstStepStatusのアクセサ関数
	void setFirstStepStatus(FirstStepStatus _status);
	FirstStepStatus getFirstStepStatus();

	ModInvertedSearchResult* getRankingResult();

	ModBoolean needDocumentFrequency() const;

	// needDFのアクセサ関数
	void setNeedDF(const ModBoolean needDF_);
	ModBoolean getNeedDF();

#endif	// あいまい ----------------------------------------------------------

protected:
	// friend class ModInvertedQueryParser;
	// friend class ModInvertedQuery;

	ModBoolean reevaluate(DocumentID documentID);
	ModBoolean reevaluate(DocumentID documentID,
						  LocationIterator*& locations,
						  ModSize& uiTF_,
						  ModInvertedQueryNode* givenEndNode = 0);

	// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
	// RankingRegexLeafNodeはcalculator名を返す
	virtual void getCalculatorOrCombinerName(ModUnicodeString& name,
					const ModBoolean withParams) const;

#ifdef DEBUG
	// 文書頻度の見積もり値を出力 sortChildren() のデバッグ用
	void showEstimatedValue(ModUnicodeString& out);
#endif // DEBUG

	// 検索語文字列
	ModUnicodeString termString;

	// 正規表現オブジェクト
	ModUnicodeRegularExpression regularExpression;

	// 文書反復子
	ModInvertedDocumentIterator* iterator;

	// テキストデーターが異表記正規化されていた場合 ModTrue
	ModBoolean normalizing;

	// テキスト情報は RegexLocationListIterator が存在している間保存し
	// ておく必要がある。そのためlastTextに入れておく。
	ModUnicodeString lastText;

#if 1	// あいまい ----------------------------------------------------------

	virtual void getQueryString(ModUnicodeString& out, 
				const ModBoolean asTermString = ModFalse,
				const ModBoolean withCalOrCombName = ModTrue, 
				const ModBoolean withCalOrCombParam = ModTrue,
				const ModBoolean withRouh = ModFalse) const;

#endif	// あいまい ----------------------------------------------------------

private:
	// 使用禁止
	ModInvertedRegexLeafNode(const ModInvertedRegexLeafNode& original);
	ModInvertedRegexLeafNode& operator=(
		const ModInvertedRegexLeafNode& original);

#ifdef DEBUG
#ifdef REGEX_DEBUG
	// regexのデバック
	// 	文書読み出し、パターンマッチングの回数を調べる
	ModSize countRegexReevaluate1;
	ModSize countRegexReevaluate2;
	ModSize countRegexLowerBound;
	ModSize countRegexReadDoc;
	ModSize countRegexPatternMatch;
#endif // REGEX_DEBUG
#endif // DEBUG

};

//
// FUNCTION public
// ModInvertedRegexLeafNode::evaluateScore -- 与えられた文書のスコアを計~算する
//
// NOTES
// 与えられた文書のスコアを計算する。
// AtomicNode::evalauteScoreをコールする。
//
// ARGUMENTS
// const DocumentID documentID
//	  文書ID
// DocumentScore& score
//	  スコア（結果格納用）
// evaluationMode mode
//	  評価モード (デフォルトは ModInvertedQueryNode::defaultMode)
//
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
//
//
inline ModBoolean
ModInvertedRegexLeafNode::evaluateScore(const DocumentID documentID,
												DocumentScore& score,
												Query::EvaluateMode mode)
{
	return ModInvertedAtomicNode::evaluateScore(documentID, score, mode);
}

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
//
// FUNCTION public
// ModInvertedRegexLeafNode::evaluateScore -- 与えられた文書のスコアを計~算する（位置も計算する）
//
// NOTES
// 与えられた文書のスコアを計算する
// AtomicNode::evaluateScore(location付)をコールする。
//
// ARGUMENTS
// const DocumentID documentID
//	  文書ID
// DocumentScore& score
//	  スコア（結果格納用）
// LocationIterator*& locations,
//	  位置情報（結果格納用）
// evaluationMode mode
//	  評価モード (デフォルトは ModInvertedQueryNode::defaultMode)
// ModInvertedQueryNode* givenEndNode
//	  ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
//
inline ModBoolean
ModInvertedRegexLeafNode::evaluateScore(const DocumentID documentID,
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
// ModInvertedRegexLeafNode::lowerBoundScore -- lowerBoundのランキング版(スコアも計算する）
//
// NOTES
//
// ARGUMENTS
// ModInvertedDocumentID givenID
//	  文書ID
// ModInvertedDocumentID& foundID
//	  結果格納用の文書IDオブジェクト
// DocumentScore& score
//	  スコア（結果格納用）
// evaluationMode mode
//	  評価モード (デフォルトは ModInvertedQueryNode::defaultMode)
//
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
//
inline ModBoolean
ModInvertedRegexLeafNode::lowerBoundScore(const DocumentID givenID,
											DocumentID& foundID,
											DocumentScore& score,
											Query::EvaluateMode mode)
{
	return AtomicNode::lowerBoundScore(givenID, foundID, score, mode);
}


// 
// FUNCTION public
// ModInvertedRegexLeafNode::getTermFrequency -- 文書内頻度の取得
// 
// NOTES
// 条件を満たす語句の出現頻度を求める。
// 
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// 
// Query::EvaluateMode mode
// 		評価モード
// 
// RETURN
// 求めた文書内頻度
// 
inline ModSize
ModInvertedRegexLeafNode::getTermFrequency(ModInvertedDocumentID documentID,
											Query::EvaluateMode mode)
{
	return ModInvertedQueryBaseNode::getTermFrequency(documentID, mode);
}

//
// FUNCTION protected
// ModInvertedRegexLeafNode::getCalculatorOrCombinerName -- 自分の持って~いるscoreCalculatorまたはscoreCombiner名を返す
//
// NOTES
// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す。
// AtomicNode::getCalculatorOrCombinerName(string)をコール。
//
// ARGUMENTS
//  ModString& name
//	scoreCalculator名(結果格納用)。
// RETURN
//   なし
//
inline void
ModInvertedRegexLeafNode::getCalculatorOrCombinerName(
	ModUnicodeString& name,
	const ModBoolean withParams) const
{
	ModInvertedAtomicNode::getCalculatorOrCombinerName(name, withParams);
}

#if 1	// あいまい ------------------ PUBLIC --------------------------------
//
// FUNCTION public
// ModInvertedRegexLeafNode::getEstimatedDocumentFrequency -- 文書頻度の見積もり値を返す
//
// NOTES
// 文書頻度の見積もり値を返す。QueryNode::getEstimatedValueをオーバーライド。
// インターフェースはQueryNodeで定義したら、文書頻度の見積もり値は実際には
// BaseNodeが持っているため。
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
ModInvertedRegexLeafNode::getEstimatedDocumentFrequency() const
{
	return ModInvertedQueryBaseNode::getEstimatedDocumentFrequency();
}	

//
// FUNCTION public
// ModInvertedRegexLeafNode::getEndNode -- endNodeのアクセサ関数
//
// NOTES
// OrderedDistanceNodeのendNodeのアクセサ関数。OrderedDistance以外のばあいは
// 本関数がコールされる。OrderedDistance以外はendNodeを持たないため常に0を返す。
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
ModInvertedRegexLeafNode::getEndNode() const
{
	return ModInvertedQueryBaseNode::getEndNode();
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::setEndNode -- endNodeのアクセサ関数
//
// NOTES
// OrderedDistanceNodeのendNodeのアクセサ関数。OrderedDistance以外のばあいは
// 本関数がコールされる。なにもしない。
//
// ARGUMENTS
//  QueryNode* endNode_
//	セットするノード。
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedRegexLeafNode::setEndNode(ModInvertedQueryNode* endNode_)
{
	ModInvertedQueryBaseNode::setEndNode(endNode_);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::getQueryNodeForRoughEvaluation -- 粗い評価用ノード~の取得
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
ModInvertedRegexLeafNode::getQueryNodeForRoughEvaluation() const
{
	return ModInvertedQueryBaseNode::getQueryNodeForRoughEvaluation();
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::setQueryNodeForRoughEvaluation -- 粗い評価のためのノードをセットする
//
// NOTES
// 位置のつき合わせを省略した粗い評価のための検索式ノードへのポインタを
// セットする。
//
// ARGUMENTS
// ModInvertedQueryNode* queryNodeForRoughEvaluation_
//	  粗い評価のためのノードへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedRegexLeafNode::setQueryNodeForRoughEvaluation(QueryNode* node)
{
	ModInvertedQueryBaseNode::setQueryNodeForRoughEvaluation(node);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::reserveScores -- scoresをリザーブ
//
// NOTES
// 子ノードを辿りrankingOr,rankingAndの場合はメンバ変数であるscoresを
// リザーブする。本関数が呼ばれるのは、末端ノードの場合であるため
// 何もしない。
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
// EXCEPTIONS
// なし
//
inline void
ModInvertedRegexLeafNode::reserveScores() { 
	ModInvertedQueryBaseNode::reserveScores();
}

#ifdef DEBUG
// FUNCTION public
// ModInvertedRegexLeafNode::showSortFactor -- sortFactor を表示
//
// NOTES
// sortFactor を表示(debug用)。サブクラスへ継承される。
//
// ARGUMENTS
// ModOstrStream& out
//	  sortFactor 値を含む文字情報
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedRegexLeafNode::showSortFactor(ModUnicodeString& out)
{
	ModInvertedQueryBaseNode::showSortFactor(out);
}
#endif

//
// FUNCTION public
// ModInvertedRegexLeafNodeB:makeRoughPointer -- queryNodeForRoughEvaluation の~作成
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
//	  有効化（最適化）モード
// QueryNodePointerMap& parentRoughPointer
//	  rough pointer の内容となるノードのアドレスが入った Map 変数
//	  （結果格納用）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedRegexLeafNode::makeRoughPointer(
	const Query::ValidateMode mode,
	QueryNodePointerMap& parentRoughPointer,
	const ModInvertedQuery* Query)
{
	ModInvertedQueryBaseNode::makeRoughPointer(mode, parentRoughPointer, Query);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::eraseTermLeafNode -- TermLeafNodeの削除
//
// NOTES
// 最適化のために、TermLeafNodeを削除する。ただし実際に削除の処理を行
// なうのはOperatorAnd/OperatorOr/OperatorAndNotの各ノードだけである。
// OperatorAnd/OperatorOr/OperatorAndNotの各ノードではこの関数をオーバ
// ライドする。
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
inline ModBoolean
ModInvertedRegexLeafNode::eraseTermLeafNode(QueryNode*& node, Query& query)
{
	return ModInvertedQueryBaseNode::eraseTermLeafNode(node, query);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::sortChildren -- 子ノードのソート
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
ModInvertedRegexLeafNode::sortChildren(
	const ModInvertedQuery::ValidateMode mode)
{
	ModInvertedQueryBaseNode::sortChildren(mode);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::getChildrenSize -- 子ノード数の取得
//
// NOTES
// 子ノード数を取得する。常に 0 を返す。
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
ModInvertedRegexLeafNode::getChildrenSize() const
{
	return ModInvertedQueryLeafNode::getChildrenSize();
}

//
// FUNCTION public
// ModInvertedQueryBaseNode::flattenChildren -- 子ノードリストの平坦化
//
// NOTES
// このクラスではなにもしない。実際に平坦化を行なうのはAndとOrだけであ
// る。
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
ModInvertedRegexLeafNode::flattenChildren(
							const QueryNodePointerMap& sharedNodeMap,
							const ModBoolean isChildOfWindowNode)
{
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::calcSortFactor -- sortFactor の計算
//
// NOTES
// sortChildren() 関数で使用する sortFactor メンバ変数を計算する。ただ
// し QueryNode では単に estimatedDocumentFrequency を代入しているだけ。
// SimpleTokenLeafNode(末端ノード) はこの関数を継承して使う。中間ノー
// ドはこの関数をオーバライドする。
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
ModInvertedRegexLeafNode::calcSortFactor()
{
	// Regexは検索コストが高いため、MaxSortFactor とする。
	this->sortFactor =  ModInvertedAtomicNode::MaxSortFactor;
	return this->sortFactor;
}

//
// FUNCTION public
// ModInvertedRankingRegexLeafNode::getDocumentFrequency -- 文書頻度を得る
//
// NOTES
// 文書頻度を得る。RegexLeafNode::getDocumentFrequencyをコール
//
// ARGUMENTS
// なし
//
// RETURN
// 文書頻度を得る
//
inline ModSize
ModInvertedRegexLeafNode::getDocumentFrequency(Query::EvaluateMode mode)
{
	return ModInvertedQueryBaseNode::getDocumentFrequency(mode);
}

//
// FUNCTION private
// ModInvertedRankingQuery::createRoughNode -- モードに応じたRoughNodeの生成
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
//	  Query
//
// RETURN
// 生成した RoughNode のアドレス
//
// EXCEPTIONS
// なし
//
inline ModInvertedQueryNode*
ModInvertedRegexLeafNode::createRoughNode(
	const ModInvertedQuery* Query,
	const ModInvertedQuery::ValidateMode mode)
{
	// Regexでラフノードを作ることはない
	ModAssert(0);
	return 0;
}

// 追加 2000/02/29
//
// FUNCTION public
// ModInvertedRegexLeafNode::changeSimpleTypeNode
//
inline void
ModInvertedRegexLeafNode::changeSimpleTypeNode(
	ModVector<ModInvertedQueryNode*>::Iterator child,
	QueryNodePointerMap& nodePointerMap)
{
	ModInvertedQueryLeafNode::changeSimpleTypeNode(child, nodePointerMap);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::addRoughToGlobalNodeMap
//
inline void
ModInvertedRegexLeafNode::addRoughToGlobalNodeMap(
	QueryNodeMap& globalNodeMap, QueryNodePointerMap& nodePointerMap)
{
	ModInvertedQueryLeafNode::addRoughToGlobalNodeMap(
							globalNodeMap, nodePointerMap);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::getTermLength
//
inline void
ModInvertedRegexLeafNode::getTermLength(ModSize& length) const
{
	ModInvertedQueryBaseNode::getTermLength(length);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::getChildren -- 子ノード数の取得
//
inline ModVector<ModInvertedQueryNode*>*
ModInvertedRegexLeafNode::getChildren()
{
	return ModInvertedQueryLeafNode::getChildren();
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::sharedQueryNode
//
inline ModSize
ModInvertedRegexLeafNode::sharedQueryNode(QueryNodeMap& globalNodeMap,
	QueryNodePointerMap& nodePointerMap)
{
	return ModInvertedQueryLeafNode::sharedQueryNode(
									globalNodeMap,nodePointerMap);
}

// ここまで 追加 2000/02/29

//
// FUNCTION protected
// ModInvertedRegexLeafNode::getQueryString -- 検索条件ノードを出力
//
// NOTES
// 検索条件ノードを出力。ラフノードの内容を表示するかどうかを選択できる
// ModBooleanのデフォルト値は旧getQueryString()のもの
//
// ARGUMENTS
// ModOstrStream& out
//	  結果格納用オブジェクト
// ModBoolean withRouh
//	  ラフノードを表示するかどうかを示すフラグ（trueで表示）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedRegexLeafNode::getQueryString(ModUnicodeString& out,
	const ModBoolean asTermString,
	const ModBoolean withCalOrCombName, 
	const ModBoolean withCalOrCombParam,
	const ModBoolean withRouh) const
{
	ModInvertedQueryBaseNode::getQueryString(out,
			asTermString, withCalOrCombName, withCalOrCombParam, withRouh);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::doFirstStepInRetrieveScore(
//      const ModInvertedDocumentID maxDocumentId)
//
inline void
ModInvertedRegexLeafNode::doFirstStepInRetrieveScore(
	ModInvertedBooleanResult *expungedDocumentId,
	const ModInvertedDocumentID maxDocumentId)
{
	ModInvertedAtomicNode::doFirstStepInRetrieveScore(expungedDocumentId,maxDocumentId);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::doSecondStepInRetrieveScore(
//		ModInvertedRankingResult* result_)
//
inline void
ModInvertedRegexLeafNode::doSecondStepInRetrieveScore(
		ModInvertedSearchResult*& result_)
{
	ModInvertedAtomicNode::doSecondStepInRetrieveScore(result_);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::doSecondStepInRetrieveScore()
//
inline void
ModInvertedRegexLeafNode::doSecondStepInRetrieveScore()
{
	ModInvertedAtomicNode::doSecondStepInRetrieveScore();
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::lowerBoundScoreForSecondStep()
//
inline ModBoolean
ModInvertedRegexLeafNode::lowerBoundScoreForSecondStep(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
	return ModInvertedAtomicNode::lowerBoundScoreForSecondStep(
										givenID, foundID, score);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::setFirstStepStatus(FirstStepStatus _status)
//
inline void
ModInvertedRegexLeafNode::setFirstStepStatus(FirstStepStatus _status)
{
	ModInvertedQueryBaseNode::setFirstStepStatus(_status);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::getFirstStepStatus()
//
inline ModInvertedQueryNode::FirstStepStatus
ModInvertedRegexLeafNode::getFirstStepStatus()
{
	return ModInvertedQueryBaseNode::getFirstStepStatus();
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::getFirstStepStatus()
//
inline ModInvertedSearchResult*
ModInvertedRegexLeafNode::getRankingResult()
{
	return ModInvertedQueryBaseNode::getRankingResult();
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::needDocumentFrequency()
//
inline ModBoolean
ModInvertedRegexLeafNode::needDocumentFrequency() const
{
	return ModInvertedQueryBaseNode::needDocumentFrequency();
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::setNeedDF(const ModBoolean needDF_)
//
inline void
ModInvertedRegexLeafNode::setNeedDF(const ModBoolean needDF_)
{
	ModInvertedQueryBaseNode::setNeedDF(needDF_);
}

//
// FUNCTION public
// ModInvertedRegexLeafNode::getNeedDF()
//
inline ModBoolean
ModInvertedRegexLeafNode::getNeedDF()
{
	return ModInvertedQueryBaseNode::getNeedDF();
}

#endif	// あいまい ----------------------------------------------------------

#endif //__ModInvertedRegexLeafNode_H__

//
// Copyright (c) 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
