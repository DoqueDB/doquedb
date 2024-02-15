// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedQueryInternalNode.h -- 検索式の内部表現の中間ノード
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedQueryInternalNode_H__
#define __ModInvertedQueryInternalNode_H__

#include "ModVector.h"
#include "ModMap.h"

#ifndef SYD_INVERTED	// 定義されてなかったら
class ModInvertedUnorderedOperatorWindowLocationListIterator;
class ModInvertedUnorderedSimpleWindowLocationListIterator;
#endif

#include "ModInvertedQueryBaseNode.h"
#include "ModInvertedRankingScoreCombiner.h"

#ifdef SYD_INVERTED
class ModInvertedUnorderedOperatorWindowLocationListIterator;
class ModInvertedUnorderedSimpleWindowLocationListIterator;
#endif

class ModInvertedRankingScoreCombiner;
class ModInvertedLocationListIterator;
//
// CLASS
// ModInvertedQueryInternalNode -- 検索式の内部表現 (木構造) の中間ノード
//
// NOTES
// 検索式の内部表現には、オペランドをオペレータで結んだ木構造を用いる。
// その中間ノードのクラス。
// 検索式内部表現ノードクラスの派生クラスである。
// 木構造の中間ノードを表し、検索の実行、頻度の計算、子ノードの管理を行なう。
// これも抽象クラスであり、実際の処理は派生クラスで行なう。
//
class
ModInvertedQueryInternalNode :
	public ModInvertedQueryBaseNode
{

public:
	typedef ModInvertedUnorderedOperatorWindowLocationListIterator
		UnorderedOperatorLocationIterator;
	typedef ModInvertedUnorderedSimpleWindowLocationListIterator
		UnorderedSimpleLocationIterator;

	// コンストラクタ
	ModInvertedQueryInternalNode(const NodeType type = noType,const ModUInt32 resultType_=0);

	// デストラクタ
	virtual ~ModInvertedQueryInternalNode();

	// 子ノードの追加
	virtual void insertChild(QueryNode* child);

	// 子ノードの取り出し
	virtual ModVector<ModInvertedQueryNode*>* getChildren();

	// 子ノード数を得る
	virtual ModSize getChildrenSize() const;

	// 中間ノードの共有化
	virtual ModSize sharedQueryNode(QueryNodeMap& globalNodeMap,
									QueryNodePointerMap& nodePointerMap);

	// RoughPointerの内容を共有化する
	virtual void addRoughToGlobalNodeMap(QueryNodeMap& globalNodeMap,
										 QueryNodePointerMap& nodePointerMap);

	// 子ノードのソート
	virtual void sortChildren(const ModInvertedQuery::ValidateMode mode);

	// 子ノードのソートを行う前にRegexをソートの対象から外す

	// sortFactor の計算
	virtual ModSize calcSortFactor();

#ifdef DEBUG
	// sortFactor を表示(debug用)
	virtual void showSortFactor(ModUnicodeString& out);
#endif

	// TermLeafNode を消去して SimpleToken/OrderedDistance にする
	virtual ModBoolean eraseTermLeafNode(QueryNode*& node, Query& query);

	// 有効化の最後に子ノードの数をチェックする
	virtual void checkQueryNode(ModInvertedQuery*, 
								const ModBoolean,
								const ModBoolean);

	virtual void reserveScores();

	virtual void validate(InvertedFile* invertedFile,
						  const ModInvertedQuery::ValidateMode mode,
						  ModInvertedQuery* rQuery);

	// スコア合成器を得る
	ScoreCombiner* getScoreCombiner() const;

	// スコア計算器を得る
	// InternalNodeは計算器を持っていないので呼ばれない
	ScoreCalculator* getScoreCalculator() const { return 0; }

	// スコア計算器をセットする
	virtual void setScoreCalculator(
		ScoreCalculator* calculator);
	virtual void setScoreCalculator(
		const ModUnicodeString& calculatorName);

	// スコア合成器をセットする
	virtual void setScoreCombiner(
		const ModUnicodeString& combinerName);
	void setScoreCombiner(
		ScoreCombiner* scoreCombiner_);

	// ランキング検索のスコア計算第１ステップ
	virtual void doFirstStepInRetrieveScore(
			ModInvertedBooleanResult *expungedDocumentId,
			const ModInvertedDocumentID maxDocumentId);

	// ランキング検索のスコア計算第２ステップ
	void doSecondStepInRetrieveScore(
			ModInvertedSearchResult*& result);
	void doSecondStepInRetrieveScore();

	// 不要な位置情報反復子をpushする
	void pushFreeList(ModInvertedLocationListIterator* location);
	// 位置情報反復子を得る
	ModInvertedLocationListIterator* getFreeList();

protected:
	// friend class ModInvertedQuery;

#ifdef DEBUG
	// 文書頻度見積もり値付き出力(debug用)
	virtual void showEstimatedValue(ModUnicodeString& out);
#endif // DEBUG

	// 検索条件ノードを出力 ラフノード表示 on/off 可
	virtual void getQueryString(ModUnicodeString& out,
								  const ModBoolean asTermString = ModFalse,
								  const ModBoolean withCalOrCombName = ModTrue, 
								  const ModBoolean withCalOrCombParam = ModTrue,
								  const ModBoolean withRouh = ModFalse) const;

	// sharedQueryNode の補助関数
	void changeSimpleTypeNode(ModVector<ModInvertedQueryNode*>::Iterator child,
							  QueryNodePointerMap& nodePointerMap);

	// changeSimpleTypeNodeにより単純化可能なノードの組み合わせかチェックする
	ModBoolean checkSimpleTypeNode(ModInvertedQueryNode* upperNode,
								   ModInvertedQueryNode* lowerNode);

	// 破棄すべき中間ノードを登録する関数
	void addQueryNodePointerMap(
		ModInvertedQueryNode* node,
		QueryNodePointerMap& nodePointerMap);

	// makeRoughPointer() の下請け関数
	ModBoolean makeRoughMap(const Query::ValidateMode,
							QueryNode*,
							QueryNodePointerMap&,
							const ModInvertedQuery* Query);

	// makeRoughPointer() の下請け関数
	void makeRoughNode(const QueryNodePointerMap& map,
					   const ModInvertedQuery* Query,
					   const ModInvertedQuery::ValidateMode mode);

	// 子供のノード
	ModVector<ModInvertedQueryNode*> children;

	// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
	// Rankig InternalNodeはCombiner LeafNodeはCalculator
	// Atomic はCalculator
	virtual void getCalculatorOrCombinerName(ModUnicodeString& name,
											 const ModBoolean withParams) const;

	virtual void flattenChildren(const QueryNodePointerMap& sharedNodeMap,
								 const ModBoolean isChildOfWindowNode);

private:
	// BooleanResultLeafNode/RakingResultLeafNodeをラフノードに追加する際の
	// サイズの制限。件数がこの値より大きい場合は速度が低下する恐れがあるため
	// ラフノードには追加しない
	static ModSize sizeLimitOfResultLeafNodeToAddToRoughMap;

	// LocationListIteratorのキャッシュ
	// 不要になったものをリスト管理する
	ModInvertedLocationListIterator* freeList;
};


//
// FUNCTION public
// ModInvertedQueryInternalNode::insertChild -- 子ノードの追加
//
// NOTES
// 子ノードの追加
//
// ARGUMENTS
// ModInvertedQueryNode* child
//	追加する子ノードへのポインタ
//
// RETURN
// なし
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
inline void
ModInvertedQueryInternalNode::insertChild(ModInvertedQueryNode* child)
{
	this->children.pushBack(child);
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::getChildren -- 子ノードの取り出し
//
// NOTES
// 子ノードの取り出し
//
// ARGUMENTS
// なし
//
// RETURN
// 子ノードを表わすベクタへのポインター
// 
// EXCEPTIONS
// なし
// 
inline ModVector<ModInvertedQueryNode*>*
ModInvertedQueryInternalNode::getChildren()
{
	return &children;
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::getChildrenSize -- 子ノード数の取得
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
ModInvertedQueryInternalNode::getChildrenSize() const
{
	return children.getSize();
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::getScoreCombiner -- スコア合成器の取得
//
// NOTES
// スコア合成器の取得
//
// ARGUMENTS
// なし
//
// RETURN
// スコア合成器へのポインタ
// 
// EXCEPTIONS
// なし
// 
inline ModInvertedRankingScoreCombiner*
ModInvertedQueryInternalNode::getScoreCombiner() const
{
	return scoreCombiner;
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::setScoreCalculator -- スコア計算器をセットする
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
ModInvertedQueryInternalNode::setScoreCalculator(
	ScoreCalculator* calculator)
{
	; ModAssert(0);
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::setScoreCalculator -- スコア計算器をセットする
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
ModInvertedQueryInternalNode::setScoreCalculator(
	const ModUnicodeString& calculatorName)
{
	; ModAssert(0);
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::setScoreCombiner -- スコア合成器をセットする
//
// NOTES
// スコア合成器をセットする。
//
// ARGUMENTS
// const ModUnicodeString& combinerName
//	スコア計算器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedQueryInternalNode::setScoreCombiner(
	const ModUnicodeString& combinerName)
{
	scoreCombiner = ModInvertedRankingScoreCombiner::create(combinerName);
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::setScoreCombiner -- スコア合成器をセットする
//
// NOTES
// スコア合成器をセットする。
//
// ARGUMENTS
// const ModUnicodeString& combinerName
//  スコア計算器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedQueryInternalNode::setScoreCombiner(
	ScoreCombiner* scoreCombiner_)
{
	this->scoreCombiner = scoreCombiner_;
}

//
// FUNCTION protected
// ModInvertedQueryInternalNode::getCalculatorOrCombinerName -- 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
//
// NOTES
// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す。Rankingの
// InternalNodeはcombiner名を返す。
//
// ARGUMENTS
// ModString& name
//		scoreCalculator名(結果格納用)
//
// RETURN
// なし
// 
// EXCEPTIONS
// なし
// 
inline void
ModInvertedQueryInternalNode::getCalculatorOrCombinerName(
	ModUnicodeString& name,
	const ModBoolean withParams) const
{
	//	自分のcombiner名を返す
	if (this->scoreCombiner != 0) {
		this->scoreCombiner->getDescription(name, withParams);
	} else {
		name = "";
	}
}

#endif // __ModInvertedQueryInternalNode_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
