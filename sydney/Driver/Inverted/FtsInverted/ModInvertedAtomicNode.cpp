// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedAtomicNode.cpp -- アトミック検索用ノードの基底クラス実装
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModInvertedAtomicNode.h"

#include "ModInvertedSearchResult.h"
//#include "ModInvertedRankingScoreCalculator.h"
#include "ModInvertedIDScorePair.h"
#include "ModInvertedLocationListIterator.h"
#include "ModInvertedFileCapsule.h"

// 
// FUNCTION public
// ModInvertedAtomicNode::ModInvertedAtomicNode -- アトミックノード生成
// 
// NOTES
// 現在は何も行なっていない 
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
ModInvertedAtomicNode::ModInvertedAtomicNode()
	: _givenID(0),_position(-1)
#ifndef SYD_USE_LARGE_VECTOR
	  , _riterator(0)
#endif
{
}

// 
// FUNCTION public
// ModInvertedAtomicNode::~ModInvertedAtomicNode -- 
// 
// NOTES
// 現在は何も行なっていない 
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
ModInvertedAtomicNode::~ModInvertedAtomicNode()
{}
// 
// FUNCTION public
// ModInvertedAtomicNode::evaluateScore -- 与えられた文書のスコアを計算する
// 
// NOTES
// 与えられた文書のスコアを計算する
//
// 引数 mode の Query::evaluateTF ビットの on/off により処理方式が分れる。
//
// on の場合出現頻度を計算するために getDocumentFrequency() を実行する。
// getDocumentFrequency() を実行すると booleanResult が得られるので、
// 文書毎にスコアを全て計算してしまう。（メンバ変数 rankingResult を完
// 成させる。）計算した中から今回の条件を満足した文書のスコアを返す。2
// 回目以降の呼び出しに対しては、rankingResultの中から条件を満足した文
// 書のスコアを返す。
//
// off の場合引数 documentID で与えられた文書について条件を満足してい
// るか検索し、満足している場合スコアを計算して返す。
//
// ARGUMENTS
// const DocumentID documentID
//		文書ID
// DocumentScore& score
//		スコア（結果格納用）
// evaluationMode mode
//      評価モード (デフォルトは ModInvertedQueryNode::defaultMode)
// 
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// 
// 
ModBoolean
ModInvertedAtomicNode::evaluateScore(const ModInvertedDocumentID documentID,
									 ModInvertedDocumentScore& score,
									 ModInvertedQuery::EvaluateMode mode)
{
	ModInvertedRankingScoreCalculator* scoreCalculator = getScoreCalculator();

	// tf の計算は後 (default)
	// 文書内出現数を計算
#ifndef MOD_INV_SIMPLE_DF_EVALUATION
	if (((mode & Query::getTFbyMinEvaluationMode) != 0) &&
		(((mode & Query::getDFbyMinEvaluationMode) == 0) &&
		 ((mode & Query::getDFbyRoughEvaluationMode) == 0))) {
		// getTFbyMinEvaluationModeがセットされていて、
		// getDFbyMinEvaluationMode/getDFbyRoughEvaluationModeが
		// セットされていない場合は正確な評価が必要
		if (evaluate(documentID, mode) != ModTrue) {
			return ModFalse;
		}
	}
#endif
	ModSize tf = this->getTermFrequency(documentID, mode);
	if (tf == 0) {
		return ModFalse;
	}

	// 毎回 secondStep を計算するのは無駄 (1999/11/19)
	if (scoreCalculator->getPrepareResult() == 0) {
		// scoreCalculator->prepare(totalDocumentFrequency,
		scoreCalculator->prepare(this->getTotalDocumentFrequency(),
								 getDocumentFrequency(mode));
	}

	ModBoolean exist;
	score = (*scoreCalculator)(tf, documentID, exist);

	return exist;
}

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
// 
// FUNCTION public
// ModInvertedAtomicNode::evaluateScore -- 与えられた文書のスコアを計算する（位置も計算する）
// 
// NOTES
// 与えられた文書のスコアを計算する
//
// ARGUMENTS
// const DocumentID documentID
//		文書ID
// DocumentScore& score
//		スコア（結果格納用）
// LocationIterator*& locations,
//		位置情報（結果格納用）
// evaluationMode mode
//      評価モード (デフォルトは ModInvertedQueryNode::defaultMode)
// 
// ModInvertedQueryNode* givenEndNode
// 		ここでは未使用。orderedDistanceでのみ使用。
// 
// 
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// 
ModBoolean
ModInvertedAtomicNode::evaluateScore(const ModInvertedDocumentID documentID,
									 ModInvertedDocumentScore& score,
									 ModInvertedLocationListIterator*& locations,
									 ModInvertedQuery::EvaluateMode mode,
									 ModInvertedQueryNode* givenEndNode)
{
	locations = 0;

	//
	// とりあえず簡単に locations なしの evaluateScore を用いいてるが、
	// 高速化のためには書き直した方が良い
	//

	if (evaluateScore(documentID, score, mode) == ModTrue) {
		if (reevaluate(documentID, locations) == ModTrue) {
			return ModTrue;
		}
	}

	return ModFalse;
}
#endif

// 
// FUNCTION public
// ModInvertedAtomicNode::lowerBoundScore -- lowerBoundのランキング版（スコアも計算する）
// 
// NOTES
// 引数 mode の Query::evaluateTF ビットの on/off により処理方式が分れる。
//
// on の場合出現頻度を計算するために getDocumentFrequency() を実行する。
// getDocumentFrequency() を実行すると booleanResult が得られるので、
// 文書毎にスコアを全て計算してしまう。（メンバ変数 rankingResult を完
// 成させる。）計算した中から今回の条件を満足した文書のスコアを返す。2
// 回目以降の呼び出しに対しては、rankingResultの中から条件を満足した文
// 書のスコアを返す。
//
// off の場合引数で与えられた文書ID以降条件を満足している文書があるか
// 検索し、満足している場合スコアを計算して返す。
//
// ARGUMENTS
// ModInvertedDocumentID givenID
//      文書ID
// ModInvertedDocumentID& foundID
//		結果格納用の文書IDオブジェクト
// DocumentScore& score
//		スコア（結果格納用）
// evaluationMode mode
//      評価モード (デフォルトは ModInvertedQueryNode::defaultMode)
// 
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
// 
ModBoolean
ModInvertedAtomicNode::lowerBoundScore(const ModInvertedDocumentID givenID,
									   ModInvertedDocumentID& foundID,
									   ModInvertedDocumentScore& score,
									   ModInvertedQuery::EvaluateMode mode)
{
	ModInvertedRankingScoreCalculator* scoreCalculator = getScoreCalculator();

	ModSize tf;
	ModInvertedDocumentID id = givenID;

	while (1) {

		// まず粗く検索
		if (this->lowerBound(id, foundID,
							 mode|ModInvertedQuery::roughEvaluationFlag)
			!= ModTrue) {
			return ModFalse;		// もう条件を満足する文書はない
		}

		// 文書内出現数を計算
#ifdef MOD_INV_SIMPLE_DF_EVALUATION
		tf = this->getTermFrequency(foundID, mode);
		if (tf == 0) {
			// 検索条件を不満足
			id = foundID + 1;
		} else {
			break;					// 検索条件を満足した
		}
#else
		if (((mode & ModInvertedQuery::getTFbyMinEvaluationMode) != 0) &&
			(((mode & ModInvertedQuery::getDFbyMinEvaluationMode) == 0) &&
			 ((mode & ModInvertedQuery::getDFbyRoughEvaluationMode) == 0))) {
			// getTFbyMinEvaluationModeがセットされていて、
			// getDFbyMinEvaluationMode/getDFbyRoughEvaluationModeが
			// セットされていない場合は正確な評価が必要
			if (evaluate(foundID, mode) != ModTrue) {
				id = foundID + 1;
			} else {
				tf = this->getTermFrequency(foundID, mode);
				if (tf == 0) {
					// 検索条件を不満足
					id = foundID + 1;
				} else {
					break;					// 検索条件を満足した
				}
			}
		} else {
			tf = this->getTermFrequency(foundID, mode);
			if (tf == 0) {
				// 検索条件を不満足
				id = foundID + 1;
			} else {
				break;					// 検索条件を満足した
			}
		}
#endif
	}

	// 毎回 secondStep を計算するのは無駄 (1999/11/19)
	if (scoreCalculator->getPrepareResult() == 0) {
		scoreCalculator->prepare(getTotalDocumentFrequency(),
								 getDocumentFrequency(mode));
#ifdef DEBUG
		ModDebugMessage << "prepare secondStep: " << foundID << ' '
						<< scoreCalculator->getPrepareResult() << ' '
						<< intptr_t(this) << ModEndl;
#endif
	}

	ModBoolean exist;
	score = (*scoreCalculator)(tf, foundID, exist);

	return exist;
}

//
// FUNCTION protected
// ModInvertedAtomicNode::getCalculatorOrCombinerName -- 自分の持っているscoreCalculatorまたはscoreCombiner名を返す
//
// NOTES
// 自分の持っているscoreCalculatorまたはscoreCombiner名を返す。Atomicは
// calcuator名
//
// ARGUMENTS
// ModString& name
//		scoreCalculator名(結果格納用)。
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedAtomicNode::getCalculatorOrCombinerName(ModUnicodeString& name,
	const ModBoolean withParams) const
{
	//  自分のcalculator名を返す
	ModInvertedRankingScoreCalculator* sc = this->getScoreCalculator();

	if (sc != 0) {
		sc->getDescription(name, withParams);
	} else {
		name = "";
	}
}

#ifdef DEL_BOOL
void
ModInvertedAtomicNode::retrieve(ModInvertedBooleanResult *expungedDocumentId,
								const ModInvertedDocumentID maxDocumentId,
								ModInvertedBooleanResult *result)
{
	ModInvertedDocumentID ID(1);
	Query::EvaluateMode mode =
		ModInvertedFileCapsuleRankingSearch::defaultEvaluateMode;

	ModInvertedVector<ModInvertedDocumentID>::Iterator iter =  expungedDocumentId)->begin();

	while (lowerBound(ID, ID, mode|ModInvertedQuery::roughEvaluationFlag)
		== ModTrue) {
loop:	// 文書ID削除検査
		if(ID >= *iter)
		{
			if( ID == *iter){
				++ID;
				++iter;
				continue;
			}
			if( *iter <= maxDocumentId)
			{
				++iter;
			}
			else
				break;
			goto loop;
		}

		if (reevaluate(ID) == ModTrue) 
		{
			result->pushBack(ID);
		}
		++ID;
	}
}
#endif

void 
ModInvertedAtomicNode::retrieve(ModInvertedBooleanResult *expungedDocumentId,
								const ModInvertedDocumentID maxDocumentId,
								ModInvertedSearchResult *result)
{
	ModInvertedDocumentID ID(1);

	Query::EvaluateMode mode =
		ModInvertedFileCapsuleRankingSearch::defaultEvaluateMode;

	ModInvertedRankingScoreCalculator* scoreCalculator = getScoreCalculator();

	ModInvertedVector<ModInvertedDocumentID>::Iterator iter = expungedDocumentId->begin();

	while (lowerBound(ID, ID, mode|ModInvertedQuery::roughEvaluationFlag)
		== ModTrue) {
loop:	// 文書ID削除検査
		if(ID >= *iter)
		{
			if( ID == *iter){
				++ID;
				++iter;
				continue;
			}
			if( *iter <= maxDocumentId)
			{
				++iter;
			}
			else
				break;
			goto loop;
		}
		
		ModUInt32	tf = this->getTermFrequency(ID, mode);
		if( tf != 0 )
		{
			ModBoolean exist;
				// tf / ( X + tf ) 部分を計算する
			ModInvertedDocumentScore score = scoreCalculator->firstStep(tf, ID, exist);
			if(exist == ModTrue)
				result->pushBack(ID, score,tf);
		}
		++ID;
	}
}
//
// FUNCTION protected
// ModInvertedAtomicNode::doFirstStepInRetrieveScore -- スコア計算の第１ステップを実行する
//
// NOTES
// ランキング検索で、スコア計算の第１ステップのみを実施する。
//
// ARGUMENTS
// const ModInvertedDocumentID maxDocumentId
//		ドキュメントID
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedAtomicNode::doFirstStepInRetrieveScore(
	ModInvertedBooleanResult  *expungedDocumentId,
	const ModInvertedDocumentID maxDocumentId)
{
	if(getFirstStepStatus() == firstDone) {
		// すでに呼ばれている
		return;
	}

	ModInvertedSearchResult* result = getRankingResult();

#ifdef MOD_INV_SIMPLE_DF_EVALUATION

	// terminatorとして、(maxDocumentId + 1)をセットする
	expungedDocumentId->pushBack(maxDocumentId + 1);
	//
#ifdef DEL_BOOL
	if(result->getType() == (1 << _SYDNEY::Inverted::FieldType::Rowid))
		retrieve(expungedDocumentId,maxDocumentId,(ModInvertedBooleanResult*)result);
	else
#endif
		retrieve(expungedDocumentId,maxDocumentId,result);

	// terminatorを削除する
	expungedDocumentId->popBack();

#else
	// MOD_INV_SIMPLE_DF_EVALUATIONが無効の場合、検索コードは動作しない
	// 上のコードのようにexpungedDocumentIdを参照しながら検索を行うようなコードにする必要がある

	ModInvertedDocumentID ID(1);
	ModInvertedRankingScoreCalculator* scoreCalculator = getScoreCalculator();
	Query::EvaluateMode mode =
		ModInvertedFileCapsuleRankingSearch::defaultEvaluateMode;

	if (((mode & Query::getTFbyMinEvaluationMode) != 0) &&
		(((mode & Query::getDFbyMinEvaluationMode) == 0) &&
		((mode & Query::getDFbyRoughEvaluationMode) == 0))) {
		// getTFbyMinEvaluationModeがセットされていて、
		// getDFbyMinEvaluationMode/getDFbyRoughEvaluationModeが
		// セットされていない場合は正確な評価が必要
		while (this->lowerBound(ID, ID, mode|Query::roughEvaluationFlag)
			== ModTrue) {

			if(ID > maxDocumentID){
				break;
			}

			if (evaluate(ID, mode) == ModTrue) {
				ModSize tf;
				if ((tf = this->getTermFrequency(ID, mode)) != 0) {
					// tf / ( X + tf ) 部分を計算する
					ModBoolean exist;
					DocumentScore firstStep
						= scoreCalculator->firstStep(tf, ID, exist);
					if (exist == ModTrue)
						result->pushBack(ID, firstStep,tf);
				}
			}
			++ID;
		}
	} else {
		while (this->lowerBound(ID, ID, mode|Query::roughEvaluationFlag)
			== ModTrue) {

			if(ID > maxDocumentID){
				break;
			}

			ModSize tf;
			if ((tf = this->getTermFrequency(ID, mode)) != 0) {
				// tf / ( X + tf ) 部分を計算する
				ModBoolean exist;
				DocumentScore firstStep
					= scoreCalculator->firstStep(tf, ID, exist);
				if (exist == ModTrue)
					result->pushBack(ID, firstStep,tf);
			}
			++ID;
		}
	}
#endif

	setFirstStepStatus(firstDone);


}

//
// FUNCTION
// ModInvertedAtomicNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
//
// NOTES
// ランキング検索で、スコア計算の第２ステップのみを実施する。
// 第１ステップの結果を用い、最終的な検索結果を生成する。
// 第１ステップを実施していない場合の結果は不定。
//
// ARGUMENTS
// ModInvertedRankingResult* result_
//      検索結果
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedAtomicNode::doSecondStepInRetrieveScore(ModInvertedSearchResult*& result_)
{
	doSecondStepInRetrieveScore();
	result_ = getRankingResult();
}

//
// FUNCTION
// ModInvertedAtomicNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
//
// NOTES
// ランキング検索で、スコア計算の第２ステップのみを実施する。
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
void
ModInvertedAtomicNode::doSecondStepInRetrieveScore()
{
	if(getFirstStepStatus() == firstDone) {
		ModInvertedRankingScoreCalculator* calculator = getScoreCalculator();
		if(calculator->isExtendedFirstStep() )
		{
			doSecondStepInRetrieveScore_Extended();
		}
		else
		{
			doSecondStepInRetrieveScore_Basic();
		}
		setFirstStepStatus(secondDone);
	}
}
//
// 基本スコア計算
//
void
ModInvertedAtomicNode::doSecondStepInRetrieveScore_Basic()
{
	ModInvertedSearchResult* result = getRankingResult();
	; ModAssert(result != 0);
	ModInvertedRankingScoreCalculator* calculator
				= getScoreCalculator();
	; ModAssert(calculator != 0);
	ModInvertedDocumentScore prepared
				= calculator->getPrepareResult();
	if(result->getType() == (
		(1 <<_SYDNEY::Inverted::FieldType::Score)|
		(1 << _SYDNEY::Inverted::FieldType::Rowid)
					)
		)
	{
		_lowerBoundScoreForSecondStep = &ModInvertedAtomicNode::lowerBoundScoreForSecondStep_highspeed;
		// 高速版のdoSecondStepInRetrieveScore
		ModInvertedSearchResultScore* r = (ModInvertedSearchResultScore*)result;
		ModInvertedSearchResultScore::Iterator i = r->begin();
		ModInvertedSearchResultScore::Iterator e = r->end();
		for (; i != e; ++i)
		{
			(*i).second *= prepared;
		}
	}
	else
	{
		_lowerBoundScoreForSecondStep = &ModInvertedAtomicNode::lowerBoundScoreForSecondStep_normal;
		for(ModSize i =  0 ; i < result->getSize(); i++)
		{
		// 汎用版のdoSecondStepInRetrieveScore
			result->setScore(i,result->getScore(i)*prepared);
		}
	}
}
//
// 拡張スコア計算( ModInvertedExternalScoreCalculator場合)
//
void
ModInvertedAtomicNode::doSecondStepInRetrieveScore_Extended()
{
	ModInvertedSearchResult* result = getRankingResult();
	; ModAssert(result != 0);
	ModInvertedRankingScoreCalculator* calculator
				= getScoreCalculator();
	; ModAssert(calculator != 0);

	calculator->setQueryNode(this);
	ModInvertedDocumentScore prepared
				= calculator->getPrepareResult();

	if(result->getType() == (
		(1 <<_SYDNEY::Inverted::FieldType::Score)|
		(1 << _SYDNEY::Inverted::FieldType::Rowid)
					)
		)
	{
		_lowerBoundScoreForSecondStep = &ModInvertedAtomicNode::lowerBoundScoreForSecondStep_highspeed;
	}
	else
	{
		_lowerBoundScoreForSecondStep = &ModInvertedAtomicNode::lowerBoundScoreForSecondStep_normal;
	}

	for(ModSize i =  0 ; i < result->getSize(); i++)
	{
		result->setScore(i,prepared*calculator->firstStepEx(i,result->getDocID(i)));
	}
}

//
// FUNCTION
// ModInvertedAtomicNode::lowerBoundScoreForSecondStep() -- ランキング検索の第２ステップで使用するlowerBound
//
// NOTES
// ランキング検索のスコア計算第２ステップで使用されるlowerBound。
//  lowerBoundScoreForSecondStep()は高速版と汎用版の２つある。
//  ModInvertedSearchResultの型に応じて使い分ける
// ARGUMENTS
// ModInvertedDocumentID givenID,
//      文書IDの下限
// ModInvertedDocumentID& foundID,
//      結果格納用の文書IDオブジェクト
// ModInvertedDocumentScore& score);
//      スコア(結果格納用)
//
// RETURN
// 求めたスコア
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//

//
// 汎用版lowerBoundScoreForSecondStep
// getRankingResult()の型がModInvertedSearchResultScore以外の場合にここにくる
// doSecondStepInRetrieveScore()で切り替えている
//
ModBoolean
ModInvertedAtomicNode::lowerBoundScoreForSecondStep_normal(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
	
	ModInvertedSearchResult* result = getRankingResult();
	ModUInt32 size = result->getSize();
	if ((ModInt32)_position < 0 ) {
		_position = 0;
		if (_position == size)
			return ModFalse;
	}
	if (_position == size ||
		(result->getDocID(_position) > givenID && givenID < _givenID))
		_position = 0;
	// 汎用版
	for(; _position  < size; ++_position) {
		if (result->getDocID(_position) >= givenID) {
			foundID = result->getDocID(_position);
			score = result->getScore(_position);
			_givenID = givenID;
			return ModTrue;
		}
	}
	return ModFalse;
}
//
// 高速版lowerBoundScoreForSecondStep
// getRankingResult()の型がModInvertedSearchResultScoreの場合にここにくる
// doSecondStepInRetrieveScore()で切り替えている
//

ModBoolean
ModInvertedAtomicNode::lowerBoundScoreForSecondStep_highspeed(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
#ifdef SYD_USE_LARGE_VECTOR
	if (_riterator.isValid() == false) {
#else
	if (&(*_riterator) == 0) {
#endif
	// resultの本当の型をresult->getType()で見て、TFを含まないので、あれば、
	// resultをModInvertedScoreResult型にcastし、高速化を計る
		ModInvertedSearchResultScore* result = (ModInvertedSearchResultScore*)getRankingResult();
		_riterator = result->begin();
		_rend = result->end();
		if (_riterator == _rend)
			return ModFalse;
	}
	if (_riterator == _rend ||
		((*_riterator).first > givenID && givenID < _givenID))
	{
	// resultの本当の型をresult->getType()で見て、TFを含まないので、あれば、
	// resultをModInvertedScoreResult型にcastし、高速化を計る
		ModInvertedSearchResultScore* result =  (ModInvertedSearchResultScore*)getRankingResult();
		_riterator = result->begin();
		_rend = result->end();
	}
	for(; _riterator != _rend; ++_riterator) {
		if ((*_riterator).first >= givenID) {
			foundID = (*_riterator).first;
			score = (*_riterator).second;
			_givenID = givenID;
			return ModTrue;
		}
	}
	return ModFalse;
}
//
// Copyright (c) 1999, 2000, 2001, 2002, 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

