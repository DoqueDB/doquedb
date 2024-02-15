// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedQueryLeafNode.h -- 検索式の内部表現の末端ノード
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedQueryLeafNode_H__
#define __ModInvertedQueryLeafNode_H__

#include "ModInvertedQueryBaseNode.h"
#include "ModInvertedSearchResult.h"


//
// CLASS
// ModInvertedQueryLeafNode -- 検索式の内部表現 (木構造) の末端ノード
//
// NOTES
// 検索式の内部表現には、オペランドをオペレータで結んだ木構造を用いる。
// その末端ノードのクラス。
// 検索式内部表現ノードクラスの派生クラスである。
// 木構造の末端ノードを表し、検索の実行を行なう。
// これも抽象クラスであり、実際の処理は派生クラスで行なう。
//
class
ModInvertedQueryLeafNode :
	public ModInvertedQueryBaseNode
{
public:
	// コンストラクタ
	ModInvertedQueryLeafNode(const NodeType = noType,
							 const ModUInt32 resultType_=0,
#ifdef SYD_INVERTED
							 const ModSize = (unsigned)(-1)
#else
							 const ModSize = UndefinedFrequency
#endif
	);

	// デストラクタ
	virtual ~ModInvertedQueryLeafNode();

	// 子ノード数を得る
	virtual ModSize getChildrenSize() const;

	// 子ノードの取り出し
	virtual ModVector<ModInvertedQueryNode*>* getChildren();

	// 中間ノードの共有化
	virtual ModSize sharedQueryNode(QueryNodeMap& globalNodeMap,
									QueryNodePointerMap& nodePointerMap) 
		{ ; ModAssert(0); return 0; }

	// sharedQueryNode の補助関数
	void changeSimpleTypeNode(ModVector<ModInvertedQueryNode*>::Iterator child,
							  QueryNodePointerMap& nodePointerMap)
		{/* LeafNodeでは何もしない */}

	virtual void validate(InvertedFile* invertedFile,
						  const ModInvertedQuery::ValidateMode mode,
						  ModInvertedQuery* rQuery)
		{/* LeafNodeでは何もしない(Result/Term/Regexでオーバーライド) */}

	// RoughPointerの内容を共有化する
	virtual void addRoughToGlobalNodeMap(QueryNodeMap& globalNodeMap,
										 QueryNodePointerMap& nodePointerMap);

	// スコア合成器を得る
	// LeafNodeは合成器を持っていないので呼ばれない
	virtual ScoreCombiner* getScoreCombiner() const { return 0; }

	// スコア計算器を得る
	ScoreCalculator* getScoreCalculator() const;

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

	// ランキング検索のスコア計算第１ステップ
	virtual void doFirstStepInRetrieveScore(
		ModInvertedBooleanResult *expungedDocumentId,
		const ModInvertedDocumentID maxDocumentId){}

	// ランキング検索のスコア計算第２ステップ
	virtual void doSecondStepInRetrieveScore(
		ModInvertedSearchResult*& result){}
	virtual void doSecondStepInRetrieveScore(){}

	// ランキング検索のスコア計算第２ステップで使用するlowerBound
	virtual ModBoolean lowerBoundScoreForSecondStep(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score){ return ModFalse; }

protected:

	// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
	virtual void getCalculatorOrCombinerName(ModUnicodeString& name,
											 const ModBoolean withParams) const;

private:
};

//
// FUNCTION public
// ModInvertedQueryLeafNode::ModInvertedQueryLeafNode -- 末端ノードの生成
//
// NOTES
// 新しい検索式内部表現ノードオブジェクトを生成する。
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
ModInvertedQueryLeafNode::ModInvertedQueryLeafNode(
	const NodeType type_,
	const ModUInt32 resultType_,
	const ModSize documentFrequency_
	)
	: ModInvertedQueryBaseNode(type_,resultType_, documentFrequency_)
{}

//
// FUNCTION public
// ModInvertedQueryLeafNode::~ModInvertedQueryLeafNode -- 末端ノードの破棄
//
// NOTES
// 検索式内部表現ノードオブジェクトを破棄する。
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
ModInvertedQueryLeafNode::~ModInvertedQueryLeafNode()
{
	if(scoreCalculator != 0){
		delete scoreCalculator;
		scoreCalculator = 0;
	}
}

//
// FUNCTION public
// ModInvertedQueryInternalNode::getChildrenSize -- 子ノード数の取得
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
ModInvertedQueryLeafNode::getChildrenSize() const
{
	// LeafNodeは子ノードを持たないので0を返す。
	return 0;
}

//
// FUNCTION public
// ModInvertedQueryLeafNode::getChildren -- 子ノードの取り出し
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
ModInvertedQueryLeafNode::getChildren()
{
	// LeafNodeは子ノードを持たないので0を返す
	return 0;
}
//
// FUNCTION public
// ModInvertedQueryLeafNode::addRoughToGlobalNodeMap -- RoughPointerの内容を共有化する
//
// NOTES
// (中間ノードの)粗い評価用に作られたノードの内容(queryNodeForRoughEvaluation)を
// GlobalNodeMapへ登録し、中間ノードとの共有化が行なえるようにする。
// ここに来るのはLeafNodeのケースであるため、何もしない。
//
// ARGUMENTS
// QueryNodeMap& globalNodeMap
//		中間ノードの共有化を行なううえで使用するMap変数
// QueryNodePointerMap& nodePointerMap
//		共有化の結果必要なくなったノードを登録するMap変数
//
// RETURN
//		なし
//
inline void
ModInvertedQueryLeafNode::addRoughToGlobalNodeMap(
	QueryNodeMap& globalNodeMap,
	QueryNodePointerMap& nodePointerMap)
{
	// LeafNodeは何もしない
}

//
// FUNCTION public
// ModInvertedQueryLeafNode::setScoreCalculator -- スコア計算器をセットする
//
// NOTES
// スコア計算器をセットする。
//
// ARGUMENTS
// ScoreCalculator* calculator
//		スコア計算器
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedQueryLeafNode::setScoreCalculator(
	ScoreCalculator* calculator)
{
	this->scoreCalculator = calculator;
}


//
// FUNCTION public
// ModInvertedQueryLeafNode::setScoreCalculator -- スコア計算器をセットする
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
ModInvertedQueryLeafNode::setScoreCalculator(
	const ModUnicodeString& calculatorName)
{
	scoreCalculator = ModInvertedRankingScoreCalculator::create(calculatorName);
}

//
// FUNCTION public
// ModInvertedQueryLeafNode::setScoreCombiner -- スコア合成器をセットする
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
ModInvertedQueryLeafNode::setScoreCombiner(
	ScoreCombiner* scoreCombiner_)
{
	// LeafNodeにCombinerをセットする事はない
	; ModAssert(0);
}

//
// FUNCTION public
// ModInvertedQueryLeafNode::setScoreCombiner -- スコア合成器をセットする
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
ModInvertedQueryLeafNode::setScoreCombiner(
	const ModUnicodeString& calculatorName)
{
	// LeafNodeにCombinerをセットする事はない
	; ModAssert(0);
}

//
// FUNCTION public
// ModInvertedQueryLeafNode::getScoreCalculator -- スコア計算器を得る
//
// NOTES
// スコア計算器を得る。
//
// ARGUMENTS
// なし
//
// RETURN
// スコア計算器へのポインタ
//
// EXCEPTIONS
// なし
//
inline ModInvertedRankingScoreCalculator*
ModInvertedQueryLeafNode::getScoreCalculator() const
{
	return this->scoreCalculator;
}

//
// FUNCTION protected
// ModInvertedQueryLeafNode::getCalculatorOrCombinerName -- 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
//
// NOTES
// LeafNodeはRankingTermLeafNode以外はcalculator/combinerをしようしないため
// 空文字列を返す。LeafNodeはRankingTermLeafNodeでオーバーライド
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
ModInvertedQueryLeafNode::getCalculatorOrCombinerName(
	ModUnicodeString& name,
	const ModBoolean withParams) const
{
	// 自分のcalculator名を返す
	ScoreCalculator* sc = this->getScoreCalculator();

	if (sc != 0) {
		sc->getDescription(name, withParams);
	} else {
		name = "";
	}
}

#endif // __ModInvertedQueryLeafNode_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
