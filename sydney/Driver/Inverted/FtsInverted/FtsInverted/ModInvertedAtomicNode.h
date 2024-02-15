// vi:set ts=4 sw=4:
//
// ModInvertedAtomicNode.h -- アトミック検索用ノードの基底クラス
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedAtomicNode_H__
#define __ModInvertedAtomicNode_H__

#include "ModInvertedTypes.h"
#include "ModInvertedQueryNode.h"
#include "ModInvertedQuery.h"

class ModInvertedLocationListIterator;

//
// MACRO
// MOD_INV_USE_DF --
//
// NOTES
// retrieveTF/evaluateTF モードが指定されている場合、スコア計算対象文書を
// 決定した際の文書数を DF として使用する
//
#define MOD_INV_USE_DF

//
// MACRO
// MOD_INV_SIMPLE_DF_EVALUATION --
//
// NOTES
//
#define MOD_INV_SIMPLE_DF_EVALUATION

//
// CLASS
// ModInvertedAtomicNode -- アトミック検索用ノード
//
// NOTES
// アトミック検索を実現するためのノード。
// 実際には QueryNode との多重継承クラスとなるので、このクラスは InvertedObject
// のサブクラスとしない。
//
class
ModInvertedAtomicNode : public ModInvertedQueryNode
{
public:

	ModInvertedAtomicNode();

	virtual ~ModInvertedAtomicNode();

	virtual ModBoolean evaluateScore(const ModInvertedDocumentID documentID,
								ModInvertedDocumentScore& score,
								ModInvertedQuery::EvaluateMode inMode);
#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
	virtual ModBoolean evaluateScore(const ModInvertedDocumentID documentID,
								ModInvertedDocumentScore& score,
								ModInvertedLocationListIterator*& locations,
								ModInvertedQuery::EvaluateMode inMode,
								ModInvertedQueryNode* givenEndNode = 0);
#endif

	virtual ModBoolean lowerBoundScore(
								const ModInvertedDocumentID givenDocumentID,
								ModInvertedDocumentID& foundDocumentID,
								ModInvertedDocumentScore& score,
								ModInvertedQuery::EvaluateMode inMode);

	// ランキング検索のスコア計算第１ステップ
	virtual void doFirstStepInRetrieveScore(
		ModInvertedBooleanResult *expungedDocumentId,
		const ModInvertedDocumentID maxDocumentId);

	// ランキング検索のスコア計算第２ステップ
	virtual void doSecondStepInRetrieveScore(
		ModInvertedSearchResult*& result);

	void doSecondStepInRetrieveScore_Basic();

	void doSecondStepInRetrieveScore_Extended();

	virtual void doSecondStepInRetrieveScore();


	virtual ModBoolean lowerBoundScoreForSecondStep(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score)
	{
		return (this->*_lowerBoundScoreForSecondStep)(givenID,foundID,score);
	}

protected:
	// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
	// Rankig InternalNodeはCombiner LeafNodeはCalculator
	// Atomic はCalculator
	virtual void getCalculatorOrCombinerName(ModUnicodeString& name,
											 const ModBoolean withParams) const;

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

	// iteratorアクセスによる高速版
	// 検索結果型がModInvertedSearchResultScore時の呼ばれるメンバー関数
	ModBoolean lowerBoundScoreForSecondStep_highspeed(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score);

	// メンバー関数アクセスによる通常版
	// 検索結果型がModInvertedSearchResultScore以外の時に呼ばれるメンバー関数
	ModBoolean lowerBoundScoreForSecondStep_normal(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score);

	// ランキング検索のスコア計算第２ステップで使用するlowerBound
	ModBoolean (ModInvertedAtomicNode::*_lowerBoundScoreForSecondStep)(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score);

	// lowerBoundScoreForSecondStepの高速化用
	ModSize _position;
	ModInvertedSearchResultScore::Iterator _riterator;
	ModInvertedSearchResultScore::Iterator _rend;
	ModInvertedDocumentID _givenID;
};

#endif // __ModInvertedAtomicNode_H__

//
// Copyright (c) 1999, 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
