// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedQueryBaseNode.h -- ベースノード
//								主に各ノード共通のメンバ変数の定義を行っている
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedQueryBaseNode_H__
#define __ModInvertedQueryBaseNode_H__

#include "ModOstrStream.h"

#include "ModInvertedQueryNode.h"


class
ModInvertedQueryBaseNode : public ModInvertedQueryNode {
public:
	// コンストラクタ
#ifdef SYD_INVERTED
	ModInvertedQueryBaseNode(const NodeType = noType,
							 const ModUInt32 resultType_=0,const ModSize = (unsigned)(-1));
#else
	ModInvertedQueryBaseNode(const NodeType = noType,
							const ModUInt32 resultType_=0,const ModSize = UndefinedFrequency);
#endif
	~ModInvertedQueryBaseNode();

	// ラフノードの生成
	ModInvertedQueryNode* createRoughNode(const ModInvertedQuery* Query,
					const ModInvertedQuery::ValidateMode mode);

	// sortFactor の計算
	virtual ModSize calcSortFactor();

	// 文書頻度を見積もる
	virtual ModSize estimateDocumentFrequency();

	// 文書頻度を得る
	virtual ModSize getDocumentFrequency(Query::EvaluateMode mode);

	// 文書内頻度を得る
	virtual ModSize getTermFrequency(DocumentID documentID,
									 Query::EvaluateMode mode);

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
							  const ModBoolean withCalOrCombName,
							  const ModBoolean withCalOrCombParam) const;

	// ノードの内容を表わす文字列を返す
	virtual void contentString(ModUnicodeString& content) const;

#ifdef DEBUG
	// sortFactor を表示(debug用)
	virtual void showSortFactor(ModUnicodeString& out);
#endif

	// 子ノードリストの平坦化 (例 #and(#and(X,Y),Z) → #and(X,Y,Z))
	virtual void flattenChildren(const QueryNodePointerMap& sharedNodeMap,
								 const ModBoolean isChildOfWindowNode);

	// 子ノードのソート
	virtual void sortChildren(const ModInvertedQuery::ValidateMode mode);

	// TermLeafNodeの消去
	virtual ModBoolean eraseTermLeafNode(QueryNode*& node, Query& query);

	// queryNodeForRoughEvaluation の作成
	virtual void makeRoughPointer(const Query::ValidateMode,
								  QueryNodePointerMap&,
								  const ModInvertedQuery* Query);

	// 有効化の最後に子ノードの数をチェックする
	virtual void checkQueryNode(ModInvertedQuery*, 
								const ModBoolean,
								const ModBoolean);

	virtual ModInvertedSearchResult* getRankingResult() 
	{
		return firstStepResult; 
	}
	virtual void setRankingResult(ModInvertedSearchResult* firstStepResult_) 
	{
		firstStepResult = firstStepResult_; 
	}

	// RankingOr,RankingAndの場合は子ノード数の分だけsocresをリザーブ
	virtual void reserveScores();

	// 粗い evaluate のアクセサ関数
	virtual void setQueryNodeForRoughEvaluation(QueryNode* node);
	virtual ModInvertedQueryNode* getQueryNodeForRoughEvaluation() const;

	// endNodeのアクセサ関数
	virtual void setEndNode(QueryNode* endNode_);
	virtual QueryNode* getEndNode() const;

	// 文書頻度の見積もり値を返す
	virtual ModSize getEstimatedDocumentFrequency() const;

	// 検索語の長さを得る
	virtual void getTermLength(ModSize& length) const {
				// ここに来るのはTermLeafNode以外のケース
				ModAssert(0)};

	// 全文書数のアクセサ関数
	virtual void setTotalDocumentFrequency(const ModSize freq);
	virtual ModSize getTotalDocumentFrequency() const;

	NodeType getType() const;

	// lowerBound の直前の結果を保存する変数（上限と下限）
	ModInvertedDocumentID upper;
	ModInvertedDocumentID lower;
	// スコア計算器とスコア合成器を同時に持つことは
	// ありえないのでunion化
	union {
		ScoreCalculator* scoreCalculator;
		ModInvertedRankingScoreCombiner* scoreCombiner;
	};

	// firstStepStatusのアクセサ関数
	virtual void setFirstStepStatus(FirstStepStatus _status);
	virtual FirstStepStatus getFirstStepStatus();

	virtual ModBoolean needDocumentFrequency() const;

	// needDFのアクセサ関数
	virtual void setNeedDF(const ModBoolean needDF_);
	virtual ModBoolean getNeedDF();

	// originalStringの共有化
	virtual void sharedOriginalString(QueryNode* eraseNode, QueryNode* sharedNode);

	void resetUpperLower() {
		this->upper = 0;
		this->lower = 0;
	}

protected:

	// 文書頻度の見積もり値
	ModSize estimatedDocumentFrequency;

	// estimatedDocumentFrequencyの状態
	// estimatedDocumentFrequencyは見積もり値であるが、retrieveを行う事により
	// 正確な値がセットされる。
	// retrieveを行い正確な値がセットされるとこのフラグはModTrueになる。
	ModBoolean retrieved;

	// 粗い evaluate のための検索式ノード
	QueryNode*  queryNodeForRoughEvaluation;

	// OperatorOrおよびAtomic系のノードで使用
	ModSize totalDocumentFrequency;


	// 子ノードのソートでキーになる値を保存する変数
	ModSize sortFactor;


	// 種別へのアクセサ関数
	void setType(NodeType type_);

	virtual void getQueryString(ModUnicodeString& out, 
								  const ModBoolean asTermString = ModFalse,
								  const ModBoolean withCalOrCombName = ModTrue, 
								  const ModBoolean withCalOrCombParam = ModTrue,
								  const ModBoolean withRouh = ModFalse) const;

#ifdef DEBUG
	// 文書頻度の見積もり値を出力
	virtual void showEstimatedValue(ModUnicodeString& out);
#endif // DEBUG

	// sharedQueryNode の補助関数
	virtual void changeSimpleTypeNode(
		ModVector<ModInvertedQueryNode*>::Iterator child,
		QueryNodePointerMap& nodePointerMap);

	ModInvertedSearchResult *firstStepResult; // 12/27/2005 by A.G
private:
	// ノード種別
	NodeType type;

	// スコア計算の状態を保持するenum
	FirstStepStatus firstStepStatus;

	// 自分の先祖がANDノードか、もしくは
	// ANDNOTの第２個ノードだった場合ModTrueになる
	ModBoolean needDF;
};

//
// FUNCTION public
// ModInvertedQueryNode::getType -- 種別を得る
//
// NOTES
// 検索式内部表現ノードオブジェクトの種別を得る。
//
// ARGUMENTS
// なし
//
// RETURN
// 種別
//
// EXCEPTIONS
// なし
//
inline ModInvertedQueryNode::NodeType
ModInvertedQueryBaseNode::getType() const
{
	return this->type;
}

//
// FUNCTION protected
// ModInvertedQueryBaseNode::setType -- 種別をセットする
//
// NOTES
// 検索式内部表現ノードオブジェクトの種別をセットする。
//
// ARGUMENTS
// ModInvertedQueryNode::NodeType type_
//		種別
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedQueryBaseNode::setType(ModInvertedQueryNode::NodeType type_)
{
	this->type = type_;
}

//
// FUNCTION public
// ModInvertedQueryNode::estimateDocumentFrequency -- 文書頻度を見積もる
//
// NOTES
// 文書頻度を見積もる
//
// ARGUMENTS
// なし
//
// RETURN
// 文書頻度
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedQueryBaseNode::estimateDocumentFrequency()
{
	return this->estimatedDocumentFrequency;
}

// RankingQueryNodeより移動
// FUNCTION public
// ModInvertedQueryBaseNode::prefixString -- 演算子を表わす文字列を返す
//
// NOTES
// ノードの演算子等を表わす文字列を返す。
// (例) #and, #or, #regex
//
// ただし QueryNode ではなにもしない。派生クラスで必要があればオーバライドする
//
// ARGUMENTS
// ModString& prefix
//	  演算子を表わす文字列を返す(結果格納用)
//
// RETURN
//  なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedQueryBaseNode::prefixString(ModUnicodeString& prefix,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam) const
{
	// 特になにもしない
}

//
// FUNCTION public
// ModInvertedQueryBaseNode::contentString -- ノードの内容を表わす文字列を返す
//
// NOTES
// ノードの内容を表わす文字列を返す
//
// ただし QueryNode ではなにもしない。派生クラスで必要があればオーバラ
// イドする
//
// ARGUMENTS
// ModString& prefix
//	  ノードの内容を表わす文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedQueryBaseNode::contentString(ModUnicodeString& content) const
{
	// 特になにもしない
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
ModInvertedQueryBaseNode::flattenChildren(
							const QueryNodePointerMap& sharedNodeMap,
							const ModBoolean isChildOfWindowNode)
{}

//
// FUNCTION public
// ModInvertedQueryBaseNode::sortChildren -- 子ノードのソート
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
ModInvertedQueryBaseNode::sortChildren(const ModInvertedQuery::ValidateMode mode)
{}

//
// FUNCTION public
// ModInvertedQueryBaseNode::eraseTermLeafNode -- TermLeafNodeの削除
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
ModInvertedQueryBaseNode::eraseTermLeafNode(QueryNode*& node, Query& query)
{
	return ModTrue;
}

//
// FUNCTION public
// ModInvertedQueryNodeBase::makeRoughPointer -- queryNodeForRoughEvaluation の作成
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
ModInvertedQueryBaseNode::makeRoughPointer(
	const Query::ValidateMode mode,
	QueryNodePointerMap& parentRoughPointer,
	const ModInvertedQuery* Query)
{}

//
// FUNCTION public
// ModInvertedQueryBaseNode::checkQueryNode -- 子ノードの数をチェックする
//
// NOTES
// 有効化の最後に呼び出されて、子ノードの数をチェックする。もし異常で
// あれば例外を投げる。
//
// ここでは LeafNode 用の定義がされている。LeafNodeには子ノードがない
// ので、needDFに関しての処理だけを行う。
// この関数は QueryInternalNode によってオーライドされる。
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
ModInvertedQueryBaseNode::checkQueryNode(
	ModInvertedQuery* query,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_
	)
{
	if( needDF_ == ModTrue) {
		setNeedDF(needDF_);
	}
}

//
// FUNCTION public
// ModInvertedQueryBaseNode::reserveScores -- scoresをリザーブ
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
ModInvertedQueryBaseNode::reserveScores() { }

//
// FUNCTION public
// ModInvertedQueryBaseNode::getEndNode -- endNodeのアクセサ関数
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
ModInvertedQueryBaseNode::getEndNode() const
{
	return 0;
}
//
// FUNCTION public
// ModInvertedQueryBaseNode::setEndNode -- endNodeのアクセサ関数
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
ModInvertedQueryBaseNode::setEndNode(ModInvertedQueryNode* endNode_)
{
}

//
// FUNCTION public
// ModInvertedQueryBaseNode::getQueryNodeForRoughEvaluation -- 粗い評価用ノードの取得
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
ModInvertedQueryBaseNode::getQueryNodeForRoughEvaluation() const
{
	return this->queryNodeForRoughEvaluation;
}
//
// FUNCTION public
// ModInvertedQueryBaseNode::setQueryNodeForRoughEvaluation -- 粗い評価のためのノードをセットする
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
ModInvertedQueryBaseNode::setQueryNodeForRoughEvaluation(QueryNode* node)
{
	queryNodeForRoughEvaluation = node;
}


//
// FUNCTION public
// ModInvertedQueryBaseNode::getEstimatedDocumentFrequency -- 文書頻度の見積もり値を返す
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
ModInvertedQueryBaseNode::getEstimatedDocumentFrequency() const
{
	return estimatedDocumentFrequency;
}

//
// FUNCTION public
// ModInvertedQueryBaseNode::setTotalDocumentFrequency -- 全文書頻度を設定
//
// NOTES
// 全文書頻度を設定する。
//
// ARGUMENTS
// なし
//
// RETURN
// 総文書頻度
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedQueryBaseNode::setTotalDocumentFrequency(const ModSize value)
{
	totalDocumentFrequency = value;
}

//
// FUNCTION public
// ModInvertedQueryBaseNode::getTotalDocumentFrequency -- 全文書頻度を返す
//
// NOTES
// 全文書頻度を返す。
//
// ARGUMENTS
// なし
//
// RETURN
// 全文書頻度
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedQueryBaseNode::getTotalDocumentFrequency() const
{
	return totalDocumentFrequency;
}

//
// FUNCTION protected
// ModInvertedQueryBaseNode::changeSimpleTypeNode -- 中間ノードの単純化
//
// NOTES
// 中間ノードの単純化。
//
// #and(条件) → 条件
// #or(条件) → 条件
// #and-not(条件,()) → 条件
// #and-not((),条件) → ()
//
// というような children が1つしかない中間ノード単純化する関数。
// ここに来るのはLeafNodeのケースなので何もしない
//
// ARGUMENTS
// ModVector<ModInvertedQueryNode*>::Iterator child
//              単純化する中間ノードの反復子
// QueryNodePointerMap& nodePointerMap
//              OR標準形変換により共有されているノードが登録されているMap
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline void
ModInvertedQueryBaseNode::changeSimpleTypeNode(
					ModVector<ModInvertedQueryNode*>::Iterator child,
					QueryNodePointerMap& nodePointerMap)
{
}

//
// FUNCTION public
// ModInvertedQueryNode::setFirstStepStatus -- 種別をセットする。
//
// NOTES
// firstStepStatusの種別をセットする。
//
// ARGUMENTS
// FirstStepStatus _status
//		種別
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedQueryBaseNode::setFirstStepStatus(FirstStepStatus _status)
{
	firstStepStatus = _status;
}

//
// FUNCTION protected
// ModInvertedQueryBaseNode::getFirstStepStatus -- 種別を得る
//
// NOTES
// firstStepStatusの種別を得る。
//
// ARGUMENTS
// なし
//
// RETURN
// 種別
//
// EXCEPTIONS
// なし
//
inline ModInvertedQueryNode::FirstStepStatus
ModInvertedQueryBaseNode::getFirstStepStatus()
{
	return firstStepStatus;
}

//
// FUNCTION public
// ModInvertedQueryBaseNode::needDocumentFrequency() -- 他転置に対してヒット件数を問い合わせる必要性を提示する
//
// NOTES
// 有効化の時点でヒットしなかっため削除されたノードは、ある条件において
// もう一方の転置ではヒットする可能性がある。
// その際にはもう一方の転置に検索文字列のヒット件数を問い合わせる必要があるが、
// その必要性を提示する。
//
// ARGUMENTS
// なし
//
// RETURN
// 自分の先祖がANDであるか、もしくはANDNOTの第２子ノードであれば
// ModTrue、なければModFalseを返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean 
ModInvertedQueryBaseNode::needDocumentFrequency() const
{
	return needDF;
}

//
// FUNCTION public
// ModInvertedQueryBaseNode::setNeedDF -- 
//
// NOTES
// needDFの値をセットする
//
// ARGUMENTS
// const ModBoolean needDF_
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedQueryBaseNode::setNeedDF(const ModBoolean needDF_)
{
	needDF = needDF_;
}

//
// FUNCTION protected
// ModInvertedQueryBaseNode::getNeedDF -- 
//
// NOTES
// needDFの値を得る
//
// ARGUMENTS
// なし
//
// RETURN
// needDFの値
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedQueryBaseNode::getNeedDF()
{
	return needDF;
}

#endif //__ModInvertedQueryBaseNode_H__

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
