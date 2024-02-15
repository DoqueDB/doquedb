// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorAndNotNode.cpp -- 差集合ノードの実装
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#include "ModOstrStream.h"
#include "ModInvertedException.h"
#include "ModInvertedOperatorAndNotNode.h"
#include "ModInvertedTermLeafNode.h"
#include "ModInvertedQuery.h"
#include "ModInvertedOrderedDistanceNode.h"
#include "ModInvertedBooleanResultLeafNode.h"
#include "ModInvertedRankingScoreCombiner.h"
#include "ModInvertedAndNotScoreNegator.h"
#include "ModInvertedRankingResultLeafNode.h"

#define ROUGH

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::~ModInvertedOperatorAndNotNode -- And-Not ノードの破棄
//
// NOTES
// And-Not ノードの破棄
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
ModInvertedOperatorAndNotNode::~ModInvertedOperatorAndNotNode()
{
	delete scoreNegator, scoreNegator = 0;

	if(scoreCombiner != 0) {
		delete scoreCombiner;
		scoreCombiner = 0;
	}
}

// 
// FUNCTION protected
// ModInvertedOperatorAndNotNode::retrieveScoreWithoutNegator -- 否定器がない場合のランキング検索
// 
// NOTES
// スコア否定器がない場合のランキング検索。
//
// ARGUMENTS
// ModInvertedRankingResult* queryResult
//		ランキング検索結果オブジェクト
// evaluationMode mode
//		評価モード
// 
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModInvertedOperatorAndNotNode::retrieveScoreWithoutNegator(
	ModInvertedSearchResult* result,
	Query::EvaluateMode mode)
{
	ModInvertedDocumentID currentID(1);
	ModInvertedDocumentID nextID(0);
	DocumentScore score(0.0);

	// term-at-a-time モードは今のところ実装していない
	// 常に document-at-atime モード

	// children[1]がRegexを含むノード場合(calcSortFactor == MaxSortFaxtor)
	// lowerBoundを使うより、reevaluateを使ったほうが効率が良い
	// (regexの評価はコストが高いため、出来るだけ少なくしたほうが良い)
	if(children[1]->calcSortFactor() != MaxSortFactor) {

		int which = (children[0]->calcSortFactor()
				 	> children[1]->calcSortFactor()) ? 1 : 0;

		while (this->children[0]->lowerBound(currentID, currentID,
										 	mode|Query::roughEvaluationFlag)
		   			== ModTrue) {
		  retry:
			if (currentID < nextID) {
				// このケースはchildren[1]はアンマッチ
				if ((children[0])->evaluateScore(
					currentID, score, mode) == ModTrue) {
					result->pushBack(currentID, score);

				}

			} else if (currentID == nextID) {
				// 両方ともヒットの可能性がある
				if (which == 1) {
					if (children[1]->reevaluate(currentID) == ModFalse &&
						children[0]->evaluateScore(currentID, score, mode)
						== ModTrue) {
     					result->pushBack(currentID, score);
   					}
				} else {		
					if ((children[0])->evaluateScore(
						currentID, score, mode) == ModTrue
						&&
						children[1]->reevaluate(currentID) == ModFalse) {
      					result->pushBack(currentID, score);
					}
				}
			} else {
				; ModAssert(currentID > nextID);
				if (this->children[1]->lowerBound(currentID, nextID,
											  mode|Query::roughEvaluationFlag)
					== ModFalse) {
					nextID = ModInvertedUpperBoundDocumentID;
				}
				goto retry;
			}
			currentID++;
		}
	}
	// children[1]がRegexの場合lowerBoundを使うより、
	// reevaluateを使ったほうが効率が良い
	// (regexの評価はコストが高いため、出来るだけ少なくしたほうが良い)
	else {
		while (children[0]->lowerBound(currentID, currentID
										 , mode| Query::roughEvaluationFlag
				 ) == ModTrue) {
			if ((children[0])->evaluateScore(
				currentID, score, mode) == ModTrue
				&&
				children[1]->reevaluate(currentID) == ModFalse) {
     			result->pushBack(currentID, score);
			}
			currentID++;
		}
	}
}

// 
// FUNCTION protected
// ModInvertedOperatorAndNotNode::retrieveScoreWithoutNegator -- 否定器がある場合のランキング検索
// 
// NOTES
// スコア否定器がある場合のランキング検索。
//
// ARGUMENTS
// ModInvertedRankingResult* queryResult
//		ランキング検索結果オブジェクト
// evaluationMode mode
//		評価モード
// 
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModInvertedOperatorAndNotNode::retrieveScoreWithNegator(
	ModInvertedSearchResult* result,
	Query::EvaluateMode mode)
{
	ModInvertedDocumentID currentID(1);
	ModInvertedDocumentID nextID(0);
	DocumentScore score0(0.0);
	DocumentScore score1(0.0);

	// term-at-a-time モードは今のところ実装していない
	// 常に document-at-atime モード

	// children[1]がRegexの場合lowerBoundを使うより、
	// reevaluateを使ったほうが効率が良い
	// (regexの評価はコストが高いため、出来るだけ少なくしたほうが良い)
	if(children[1]->calcSortFactor() != MaxSortFactor) {

		while (this->children[0]->lowerBound(currentID, currentID,
										 mode|Query::roughEvaluationFlag)
		   		== ModTrue) {
		  retry:
			if (currentID < nextID) {
				// このケースはchildren[1]はアンマッチ
				if ((children[0])->evaluateScore(
					currentID, score0, mode) == ModTrue) {
					result->pushBack(currentID,
						(*scoreCombiner)(score0, (*scoreNegator)(0.0)));
				}

			} else if (currentID == nextID) {
				// 両方ともヒットの可能性がある
				if (children[0]->evaluateScore(currentID,
							score0, mode) == ModTrue) {
					// children[0]がヒット。これでこの文書はヒットした事になる
					if ((children[1])->evaluateScore(
						currentID, score1, mode) != ModTrue) {
						// children[1]はアンマッチ
						score1 = 0.0;
					}
					result->pushBack(currentID,
						(*scoreCombiner)(score0, (*scoreNegator)(score1)));
				}
			} else {
				; ModAssert(currentID > nextID);
				if (this->children[1]->lowerBound(currentID, nextID,
										  	mode|Query::roughEvaluationFlag)
					== ModFalse) {
					nextID = ModInvertedUpperBoundDocumentID;
				}
				goto retry;
			}
			currentID++;
		}
	}
	// children[1]がRegexの場合lowerBoundを使うより、
	// reevaluateを使ったほうが効率が良い
	// (regexの評価はコストが高いため、出来るだけ少なくしたほうが良い)
	else {
		while (children[0]->lowerBound(currentID, currentID
										 , mode| Query::roughEvaluationFlag
				 ) == ModTrue) {
			if (children[0]->evaluateScore(currentID,
						score0, mode) == ModTrue) {
				// children[0]がヒット。これでこの文書はヒットした事になる
				if ((children[1])->evaluateScore(
					currentID, score1, mode) != ModTrue) {
					// children[1]はアンマッチ
					score1 = 0.0;
				}
				result->pushBack(currentID,
					(*scoreCombiner)(score0, (*scoreNegator)(score1)));
			}
			currentID++;
		}
	}
}

// 
// FUNCTION public
// ModInvertedOperatorAndNotNode::evaluateScore -- 与えられた文書のスコアを計算する
// 
// NOTES
// 与えられた文書のスコアを計算する。第2オペランドを表わすスコア（今の
// ところ常に 0.0 )を否定器にかけその結果と第1オペランドを表わすスコア
// を合成器にかけることでスコアをもとめる。
//
// ARGUMENTS
// const DocumentID documentID
//		文書ID
// DocumentScore& score
//		スコア（結果格納用）
// evaluationMode mode
//		評価モード
// 
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedOperatorAndNotNode::evaluateScore(
	const DocumentID documentID,
	DocumentScore& score,
	Query::EvaluateMode mode)
{
	// 既に調査済み？
	if (documentID >= this->lower) {
		if (documentID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		}
	}

	DocumentScore score1(0.0);

	// 通常のノードはまずはじめにラフノードを使った評価を行う。
	// and-notのほかのノード同様にラフノードを持っているが、内容は
	// children[0]のラフノードと同じである。

	// children[0]->evaluateScore()でchildren[0]のラフノードが最初に
	// 評価されるのでここではラフノードの評価は行っていない。

	// 子ノードのスコア計算
	// 第1オペランドの条件を検索
	if ((children[0])->evaluateScore(
		documentID, score, mode) == ModFalse) {
		return ModFalse;			// 条件を満足しなかった
	}

	if (scoreNegator == 0) {
		if (children[1]->evaluate(documentID, mode) == ModTrue) {
			// アンマッチ
			score = 0.0;
			return ModFalse;
		}

	} else {
		// 第2オペランドの条件を検索
		if ((children[1])->evaluateScore(
			documentID, score1, mode) != ModTrue) {
			// アンマッチ
			score1 = 0.0;
		}

		// 第1/第2 オペランドとも条件を満足した スコアを計算する。
		// childScore1 を否定器にかけた結果を合成器へかける
		score = (*scoreCombiner)(score, (*scoreNegator)(score1));
	}

	// 条件を満足したのでlower/upperをセット
	this->lower = documentID;
	this->upper = documentID;

	return ModTrue;
}

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
// 
// FUNCTION public
// ModInvertedOperatorAndNotNode::evaluateScore -- 与えられた文書のスコアを計算する（位置も計算する）
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
//		評価モード
// 
// ModInvertedQueryNode* givenEndNode
// 		ここでは未使用。orderedDistanceでのみ使用。
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedOperatorAndNotNode::evaluateScore(
	const DocumentID documentID,
	DocumentScore& score,
	LocationIterator*& locations,
	Query::EvaluateMode mode,
	ModInvertedQueryNode* givenEndNode)
{
	locations = 0;

	// 既に調査済み？
	if (documentID >= this->lower) {
		if (documentID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		}
	}

	DocumentScore score1(0.0);

	try {
		// 子ノードのスコア計算
		// 第1オペランドの条件を検索
		if ((children[0])->evaluateScore(
			documentID, score, locations, mode) == ModFalse) {
			return ModFalse;			// 条件を満足しなかった
		}

		if (scoreNegator == 0) {
			if (children[1]->evaluate(documentID, mode) == ModTrue) {
				// アンマッチ
				score = 0.0;
				locations->release(), locations = 0;
				return ModFalse;
			}

		} else {
			// 第2オペランドの条件を検索
			if ((children[1])->evaluateScore(
				documentID, score1, mode) != ModTrue) {
				// アンマッチ
				score1 = 0.0;
			}

			// 第1/第2 オペランドとも条件を満足した スコアを計算する。
			// childScore1 を否定器にかけた結果を合成器へかける
			score = (*scoreCombiner)(score, (*scoreNegator)(score1));
		}

		// 条件を満足したのでlower/upperをセット
		this->lower = documentID;
		this->upper = documentID;

		return ModTrue;

	} catch (ModException& exception) {
		ModErrorMessage << "evaluateScore failed: " << documentID << ": "
						<< exception << ModEndl;
		locations->release();
		ModRethrow(exception);
	}
}
#endif

// 
// FUNCTION public
// ModInvertedOperatorAndNotNode::lowerBoundScore -- 検索式を満たす文書のうち、文書IDが与えられた値以上で最小の文書の検索しスコアを計算する
// 
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する。またスコアを計算して返す。
// 
// ARGUMENTS
// ModInvertedDocumentID givenDocumentID
//		文書ID
// ModInvertedDocumentID& foundDocumentID
//		結果格納用の文書IDオブジェクト
// DocumentScore& score
//		スコア（結果格納用）
// evaluationMode mode
//		評価モード
// 
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedOperatorAndNotNode::lowerBoundScore(const DocumentID givenID,
											   DocumentID& foundID,
											   DocumentScore& score,
											   Query::EvaluateMode mode)
{
	score = 0.0;

	// スコア
	DocumentScore score1(0.0);

	if (givenID >= this->lower) {	
		if (givenID <= this->upper) {
			// スコア計算をする
			// 第1オペランドについてスコアを求める

			// evaluateScoreによりlowerが書き換えられてしまうので保存
			if ((children[0])->evaluateScore(
				this->upper, score, mode) == ModFalse) {
				// 条件を不満足（これはありえないはず！）
				ModAssert(0);
			}
			foundID = this->upper;

			if (scoreNegator == 0) {
				// reevaluate しなくても結果であることはわかっている
				return ModTrue;
			}
			if ((children[1])->evaluateScore(
				this->upper, score1, mode) == ModFalse) {
				score1 = 0.0;
			}

			// 否定器にかけた結果を合成器へかける
			score = (*scoreCombiner)(score, (*scoreNegator)(score1));

			return ModTrue;

		} else if (givenID <= this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		}
	}

	// まずブーリアン検索をする
	DocumentID currentID(givenID);

	while (lowerBound(currentID, foundID, mode) == ModTrue) {
		// スコア計算をする
		// 第1オペランドについてスコアを求める
		if ((children[0])->evaluateScore(
			foundID, score, mode) == ModTrue) {

			if (scoreNegator == 0) {
				// lowerBound を呼ぶときにラフフラグを立てていないので、
				// reevaluate しなくても結果であることはわかっている
				return ModTrue;
			}

			// 第2オペランドについてスコアを求める
			if ((children[1])->evaluateScore(
				foundID, score1, mode) == ModFalse) {
				score1 = 0.0;
			}

			// childScore1 を否定器にかけた結果を合成器へかける
			this->lower = givenID;
			this->upper = currentID;
			score = (*scoreCombiner)(score,
									 (*scoreNegator)(score1));

			return ModTrue;
		}
		currentID = foundID + 1;
	}
	return ModFalse;

}
#if 1
// 
// FUNCTION public
// ModInvertedOperatorAndNotNode::booleanRetrieve -- 検索の一括実行
// 
// NOTES
// 検索式を一括実行する。children[0] が True で children[1] が False 
// な document ID を結果としてかえす。
//
// まず children[0]->lowerBound() を roughEvaluation mode で実行し、そ
// の結果を children[0]->reevaluate() で評価し正確な検索結果を得る。次
// に得られた currentDocumentID を使って children[1]->evaluate() を行
// ない、False なものを結果として格納しておく。
//
// ARGUMENTS
// ModInvertedBooleanResult& queryResult
//		Boolean検索結果オブジェクト
// ModInvertedQuery::EvaluateMode mode
//		評価モード
// 
// RETURN
// 正常終了の場合 ModOk、異常終了の場合 ModError
// 
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
void
ModInvertedOperatorAndNotNode::booleanRetrieve(
	ModInvertedBooleanResult& queryResult,
	Query::EvaluateMode mode)
{
	if ((mode & Query::roughEvaluationFlag) != 0
		&& this->queryNodeForRoughEvaluation != 0) {
		queryNodeForRoughEvaluation->retrieve(queryResult, mode);
		return;
	}

	// 検索結果をまず空にする。
	if (!queryResult.isEmpty()) {
		queryResult.clear();
	}

	estimateDocumentFrequency();

	if (estimatedDocumentFrequency != UndefinedFrequency) {
		// 出現頻度と同じ分だけ予約しておく
		queryResult.reserve(estimatedDocumentFrequency);
	}

	// retrieve 処理
	// 必ず document-at-atime

	ModInvertedDocumentID currentDocumentID(1);

	ModInvertedDocumentID nextDocumentID = 0;
	int which =
		(children[0]->calcSortFactor() > children[1]->calcSortFactor()) ? 1 : 0;


	// children[1]がRegexを含むノードの場合lowerBoundを使うより、
	// reevaluateを使ったほうが効率が良い
	// (regexの評価はコストが高いため、出来るだけ少なくしたほうが良い)
	if(children[1]->calcSortFactor() != MaxSortFactor) {

		while(this->children[0]->lowerBound(currentDocumentID, currentDocumentID
						 , mode| Query::roughEvaluationFlag) == ModTrue) {
		   retry:
			if (currentDocumentID < nextDocumentID) {

				if (children[0]->reevaluate(currentDocumentID) == ModTrue) {
					queryResult.pushBack(currentDocumentID);
				}
			}
			else if (currentDocumentID == nextDocumentID) {
				if (which) {
					if (children[1]->reevaluate(currentDocumentID)==ModFalse &&
						children[0]->reevaluate(currentDocumentID) == ModTrue) {
						queryResult.pushBack(currentDocumentID);
					}
				} else {		
					if (children[0]->reevaluate(currentDocumentID) == ModTrue &&
						children[1]->reevaluate(currentDocumentID) == ModFalse){
						queryResult.pushBack(currentDocumentID);
					}
				}
			}
			else {
				; ModAssert(currentDocumentID > nextDocumentID);
				if (this->children[1]->lowerBound(currentDocumentID,
											nextDocumentID
											, mode|Query::roughEvaluationFlag
											 )== ModFalse) {
					nextDocumentID = ModInvertedUpperBoundDocumentID;
				}
				goto retry;
			}
			currentDocumentID++;
		}

	// children[1]がRegexの場合lowerBoundを使うより、
	// reevaluateを使ったほうが効率が良い
	// (regexの評価はコストが高いため、出来るだけ少なくしたほうが良い)
	} else {
		while (this->lowerBound(currentDocumentID, currentDocumentID
										 , mode| Query::roughEvaluationFlag
										 ) == ModTrue) {
			if(reevaluate(currentDocumentID) == ModTrue) {
				queryResult.pushBack(currentDocumentID);
			}
			currentDocumentID++;
		}
	}

	// retrieve 終了。estimatedDocumentFrequency に正確な文書頻度を設定する
	this->estimatedDocumentFrequency = queryResult.getSize();

	if ((mode & Query::roughEvaluationFlag) == 0) {
		// ラフモードでない場合
		this->retrieved = ModTrue;
	}

	// queryResultに割り当てられたメモリを最小限にするためreserve()を実行
	queryResult.reserve(estimatedDocumentFrequency);
}
#endif
// 
// FUNCTION public
// ModInvertedOperatorAndNotNode::booleanEvaluate -- 与えられた文書が検索式を満たすかどうかの検査
// 
// NOTES
// 与えられた文書が検索式を満たすかどうか検査する。
// 
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedQuery::EvaluateMode mode
//		評価モード
// 
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
// 
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
ModBoolean
ModInvertedOperatorAndNotNode::booleanEvaluate(DocumentID documentID,
											   Query::EvaluateMode mode)
{
	// 既に調査済み？
	if (documentID >= this->lower) {
		if (documentID == this->upper) {
			return ModTrue;
		} else if (documentID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		}
	}
	if ((mode & Query::roughEvaluationFlag) != 0
		&& this->queryNodeForRoughEvaluation != 0) {
		return this->queryNodeForRoughEvaluation->evaluate(
			documentID,
#ifdef	ROUGH_MODE_FLAG
			mode
#else	// ROUGH_MODE_FLAG  ラフノードへはラフフラグをoffにする
			mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
			);
	}

	// 子ノードの検査 常に document-at-a-time
	// まず children[0] へラフに evaluate
	if (this->children[0]->evaluate(documentID,
									mode | Query::roughEvaluationFlag)
		== ModFalse) {
		return ModFalse;

	} else if ((mode & Query::roughEvaluationFlag) != 0) {
		// ラフモードでは children[1] の条件は考慮しない
		return ModTrue;

	} else if (this->children[0]->reevaluate(documentID) == ModTrue) {
		if (this->children[1]->evaluate(documentID, mode) == ModTrue) {
			return ModFalse;
		} else {
			this->upper = this->lower = documentID;
			return ModTrue;
		}
	}
	return ModFalse;
}

// 
// FUNCTION public
// ModInvertedOperatorAndNotNode::booleanLowerBound -- 検索式を満たす文書のうち、文書IDが与えられた値以上で最小の文書の検索
// 
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する。
// 
// 子ノードである children[0] と children[1] についてラフモードで 
// lowerBound()を実行しその後必要に応じて reevaluate() を読び出す。ま
// た lowerBound() がくり返し呼び出されることに考慮して、children[0],
// children[1] の lowerBound() の結果を保持しておく。このために以下の
// メンバ変数を用意した。
//	  ModInvertedDocumentID child0Upper children[0] の lowerBound() 結果
//	  ModInvertedDocumentID child0Lower children[0] の lowerBound() 入力値
//	  ModInvertedDocumentID child1Upper children[1] の lowerBound() 結果
//	  ModInvertedDocumentID child1Lower children[1] の lowerBound() 入力値
// 1999/01/21 改良
//	
// ARGUMENTS
// ModInvertedDocumentID givenID
//		文書ID
// ModInvertedDocumentID& foundID
//		結果格納用の文書IDオブジェクト
// ModInvertedQuery::EvaluateMode mode
//		評価モード
// 
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
// 
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
ModBoolean
ModInvertedOperatorAndNotNode::booleanLowerBound(ModInvertedDocumentID givenID,
												 ModInvertedDocumentID& foundID,
												 Query::EvaluateMode mode)
{
	// 既に調査済み？
	if (givenID >= this->lower) {
		if (this->upper == ModSizeMax) {
			return ModFalse;		// 既にEndに達っしている
		}
		if (givenID <= this->upper) {
			foundID = this->upper;
			return ModTrue;
		}
	}
	if ((mode & Query::roughEvaluationFlag) != 0
		&& this->queryNodeForRoughEvaluation != 0) {
		return this->queryNodeForRoughEvaluation->lowerBound(
			givenID, foundID,
#ifdef	ROUGH_MODE_FLAG
			mode
#else	// ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
			mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
			);
	}

	if ((mode & Query::roughEvaluationFlag) != 0) {
		// ラフモード
		if (givenID >= child0Lower) {
			if (child0Upper == ModSizeMax) {
				return ModFalse;		// 以前の結果を使う
			}
			if (givenID <= child0Upper) {
				foundID = child0Upper;	// 以前の結果を使う
				return ModTrue;
			}
		}
		child0Lower = givenID;
		if (this->children[0]->lowerBound(givenID, foundID,
										  mode|Query::roughEvaluationFlag)
			== ModTrue) {
			child0Upper = foundID;
			return ModTrue;
		}
		child0Upper = ModSizeMax;
		return ModFalse;
	}
	// children[0]とchildren[1]へlowerBound()を実行してその結果を変
	// 数へ記憶しておく方式

	foundID = 0;						// 0 クリアしておく
	ModInvertedDocumentID child1Current = 0;
	ModInvertedDocumentID currentID = 0;
	int which =
		(children[0]->calcSortFactor()>children[1]->calcSortFactor()) ? 1 : 0;

	// child0Lower/child0Upper と givenID を比較
	if (givenID >= child0Lower) {
		if (child0Upper == ModSizeMax) {
			this->lower = givenID;
			this->upper = ModSizeMax;
			return ModFalse;		// 以前の結果を使う
		}
		if (givenID <= child0Upper) {
			// 以前に実行した lowerBound() の結果を使う
			currentID = child0Upper;
			goto retry;
		}
	}
	currentID = child0Lower = givenID;

	while (this->children[0]->lowerBound(currentID, currentID,
										 mode|Query::roughEvaluationFlag)
		   == ModTrue) {
		child0Upper = currentID;		// 結果をとっておく

		   retry:
		if (currentID < child1Current) {
			if (children[0]->reevaluate(currentID) == ModTrue) {
				foundID = currentID;
				break;					// 条件を満足
			}

		} else if (currentID == child1Current) {
			if (which) {
				if (children[1]->reevaluate(currentID) == ModFalse &&
					children[0]->reevaluate(currentID) == ModTrue) {
					foundID = currentID;
					break;				// 条件を満足
				}

			} else {		
				if (children[0]->reevaluate(currentID) == ModTrue &&
					children[1]->reevaluate(currentID) == ModFalse) {
					foundID = currentID;
					break;				// 条件を満足
				}
			}

		} else {
			; ModAssert(currentID > child1Current);
			// child1Lower/child1Upper と currentID を比較する
			if (child1Lower <= currentID && currentID <= child1Upper) {
				// 以前実行した lowerBound() の結果を使う
				child1Current = child1Upper;
			} else {
				child1Lower = currentID;
				if (this->children[1]->lowerBound(
					currentID, child1Current, mode|Query::roughEvaluationFlag)
					== ModFalse) {
					// 文書IDの最大値をセット
					child1Current = ModInvertedUpperBoundDocumentID;
					child1Upper = ModSizeMax;
				} else {
					child1Upper = child1Current; // 結果をとっておく
				}
			}
			goto retry;
		}
		currentID++;					// children[0]->lowerBound() へ
	}
	this->lower = givenID;
	if (foundID == 0) {
		// 条件を満足する文書はみつからなかった
		this->upper = ModSizeMax;
		return ModFalse;
	} else {
		this->upper = foundID;
		return ModTrue;
	}
}
#ifndef DEL_BOOL
//
// FUNCTION public
// ModInvertedOperatorAndNotNode::retrieve -- 検索の一括実行
//
// NOTES
// 検索式を一括実行する。
// ブーリアン検索で使用されるだけであれば、scoreNegator の有無によって
// 場合分けする必要はないが、ランキング検索のために場合分けしている。
// すなわち、ランキング検索においても AtomicNode から呼ばれることがあり、
// その際は scoreNegator があるか否かによって結果集合が異なるからである。
//
// ARGUMENTS
// ModInvertedBooleanResult& queryResult
//		Boolean検索結果
// ModInvertedQuery::EvaluateMode mode
//		評価モード
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
void
ModInvertedOperatorAndNotNode::retrieve(ModInvertedBooleanResult& result,
										Query::EvaluateMode mode)
{
	if (scoreNegator == 0) {
		booleanRetrieve(result, mode);
		return;
	}
	children[0]->retrieve(result, mode);
}
#endif
//
// FUNCTION public
// ModInvertedRankingOperatorAndNotNode::evaluate -- 与えられた文書が検索式を満~たすかどうかの検査
//
// NOTES
// 与えられた文書が検索式を満たすかどうか検査する。
// AndNotとRankingAndNotでは動作がことなるため、AndNotNode::evaluateを
// オーバーライド。
//	  AndNotはchildre[0]がマッチかつchildren[1]がアンマッチの場合のみヒット
//	  RankingAndNotはchildre[0]がマッチの場合にヒットした事になる
// children[0]のevaluateをコール
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedQuery::EvaluateMode mode
//		評価モード
//
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
inline ModBoolean
ModInvertedOperatorAndNotNode::evaluate(DocumentID documentID,
										Query::EvaluateMode mode)
{
	if (scoreNegator == 0) {
		return booleanEvaluate(documentID, mode);
	}
	return children[0]->evaluate(documentID, mode);
}

//
// FUNCTION public
// ModInvertedRankingOperatorAndNotNode::lowerBound -- 検索式を満たす文書のうち~、文書IDが与えられた値以上で最小の文書の検索
//
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する。
// AndNotとRankingAndNotでは動作がことなるため、AndNotNode::lowerBoundを
// オーバーライド。
//	  AndNotはchildre[0]がマッチかつchildren[1]がアンマッチの場合のみヒット
//	  RankingAndNotはchildre[0]がマッチの場合にヒットした事になる
// children[0]のlowerBoundをコール
//
// ARGUMENTS
// ModInvertedDocumentID givenID
//		文書ID
// ModInvertedDocumentID& foundID
//		結果格納用の文書ID
// ModInvertedQuery::EvaluateMode mode
//		評価モード
//
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
inline ModBoolean
ModInvertedOperatorAndNotNode::lowerBound(ModInvertedDocumentID givenID,
										  ModInvertedDocumentID& foundID,
										  Query::EvaluateMode mode)
{
	if (scoreNegator == 0) {
		return booleanLowerBound(givenID, foundID, mode);
	}
	return children[0]->lowerBound(givenID, foundID, mode);
}



// 
// FUNCTION public
// ModInvertedOperatorAndNotNode::estimateDocumentFrequency -- 文書頻度の見積もり
// 
// NOTES
// 条件を満たす文書数を見積もる。
//
// ARGUMENTS
// なし
// 
// RETURN
// 見積もった文書頻度
// 
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
ModSize
ModInvertedOperatorAndNotNode::estimateDocumentFrequency()
{
	if (this->estimatedDocumentFrequency == UndefinedFrequency) {
		estimatedDocumentFrequency = children[0]->estimateDocumentFrequency();
	}
	return estimatedDocumentFrequency;
}

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::insertChild -- 子ノードの追加
//
// NOTES
// オペランドを表す子ノードを追加する。InternalNodeのinsertChild()をオー
// バライドしている。
//
// ARGUMENTS
// ModInvertedQueryNode* child
//		追加する子ノード
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorQueryValidateFail
//
void
ModInvertedOperatorAndNotNode::insertChild(ModInvertedQueryNode* child)
{
	if (children.getSize() >= 2) {
		// childrenの数が2以上ではinsertできない
		ModErrorMessage << "Invalid children size. size = "
						<< children.getSize() << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}
	ModInvertedQueryInternalNode::insertChild(child);
}

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::makeRoughPointer -- queryNodeForRoughEvaluation の作成
//
// NOTES
// queryNodeForRoughEvaluation を作成する。
//
// ModInvertedQueryNode の makeRoughPointer() をオーバライドしている。
//
// children[0] の queryNodeForRoughEvaluation を自分の 
// queryNodeForRoughEvaluation とする。
//
// children が InternalNode で OrderedDistanceNode 以外の node であった場
// 合は makeRoughPointer() を再帰的に呼び出す。また makeRoughPointer()の返
// り値はその node 以下の queryNodeForRoughEvaluation を表わしているので、
// 返り値を自分の queryNodeForRoughEvaluation に加える。
//
// このように makeRoughPointer() を呼び出しその返り値を利用することにより 
// queryNodeForRoughEvaluation を下位の node から組み上げていく。下に降り
// て行くときに OrderedDistanceNode に出会った時点でそれ以上下に降りるのを
// やめる。
//
// 引数 Mode の makeRoughOfSimpleNode ビットがオンであった場合は 
// SimpleTokenNode が children に含まれていた時には SimpleTokenNode も 
// queryNodoForRoughEvaluation に加えるが、makeRoughOfSimpleNode ビットが
// オフであった場合には SimpleTokenNode は queryNodeForRoughEvaluation に
// 加えない。
//
// ARGUMENTS
// なし
//
// RETURN
// queryNodeForRoughEvaluationが作成できたときその内容を返す
// 作成できなかった場合は 0 を返す
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModInvertedOperatorAndNotNode::makeRoughPointer(const Query::ValidateMode mode,
												QueryNodePointerMap& parentMap,
												const ModInvertedQuery* Query)
{
	QueryNodePointerMap tmpMap;

	// childlen[0] について調べる
	if (this->makeRoughMap(mode, children[0], tmpMap, Query) != ModTrue) {
		// 子ノードは空集合だった
		// roughPointer の内容も空集合にしておく
		this->queryNodeForRoughEvaluation = children[0];
		// 親ノード用の map の内容を削除して、空集合を挿入する
		parentMap.erase(parentMap.begin(), parentMap.end());
		parentMap[children[0]] = ModSizeMax;

		return;
	}

	// tmpMap の内容をラフノードとする
	this->makeRoughNode(tmpMap, Query, mode);

	// tmpMap を親ノード用の map に挿入する
	parentMap.insert(tmpMap.begin(), tmpMap.end());

	// 自分の rough用ポインタが完成

	// 第2オペランドである children[1] に対してmakeRoughPointerを再帰
	// 的に呼び出し、子ノードにラフノード作成を行なわせる
	QueryNodePointerMap dummy;
	this->children[1]->makeRoughPointer(mode, dummy, Query);
}

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::sharedQueryNode -- 中間ノードの共有化
// 
// NOTES
// 中間ノードの共有化。
//
// 子ノードのgetQueryString() の結果（検索式）とそのノードへのポインタ
// を 引数 globalNodeMap に格納していく。検索式が key でノードへのポイ
// ンタが value。
//
// globalNodeMap への登録時に同じ key が登録されていたら共有化できると
// 考え、子ノー ドの実体を delete し children に設定されているポインタ
// を張り変える。
//
//
// また、カレントノード(And-Not)のふたつの子ノードが同じ key であるか
// 調査する。もし同じ keyであった場合、第1パラメータである子ノード
// (children[0])を破棄しchildren[0]に0を代入するとことで子ノードが破棄
// されたことを記録し、それをカレントノードの親ノードへ伝える。
// (通常ランキング検索ではスコアが変わってしまうためカレントノード内のチェック
// は行わないが、And-Notノードの場合はスコアは変わらないため行っている)
//
// sharedQueryNode() は再帰的に呼び出すことでカレントノード以下全ノー
// ドに対してノードの共有化を実行できる。子ノードに対して 
// sharedQueryNode() を実行した結果子ノードの子ノード（孫ノード）が1つ
// になった場合は、さらにそのノードを単純化できるので、 
// changeSimpleNodeType() 関数を呼び出す。
//
// And-not は以下のような変換を sharedQueryNode() により実現している。
// （例)
//	 1. #and-not(A,A) → 空集合
//	 2. #and-not(#and-not(A,A),B) → 空集合
//	 3. #and-not(A,#and-not(B,B)) → A
//
// 本関数は ModInvertedQueryInternalNode::sharedQueryNode() をオーバラ
// イドしている。
//
// ARGUMENTS
// QueryNodeMap& globalNodeMap
//		全ノードの QueryNodeMap
// QueryNodePointerMap& nodePointerMap
//		OR標準形変換により共有されているノードが登録されているMap
//
// RETURN
// children の数
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModSize
ModInvertedOperatorAndNotNode::sharedQueryNode(
	QueryNodeMap& globalNodeMap,
	QueryNodePointerMap& nodePointerMap)
{
	QueryNodeMap::Iterator p;
	ModSize retGrandChildNum;		// 子ノードのsharedQueryNodeの戻値
	int i;

	for (i = 0; i < 2; ++i) {
		if (ModInvertedIsInternalNode(children[i]->getType()) != ModTrue) {
			// リーフノードの場合なにもしない
			continue;					// 次の child へ
		}
		// internal node の場合

		// 子ノードに対して sharedQueryNode() を呼ぶ
#if defined(CC_SUN4_2)
		retGrandChildNum = static_cast<InternalNode*&>(children[i])->
			sharedQueryNode(globalNodeMap, nodePointerMap) ;
#else //  CC_VC
		retGrandChildNum = static_cast<InternalNode*>(children[i])->
			sharedQueryNode(globalNodeMap, nodePointerMap) ;
#endif //  CC_VC

		if (retGrandChildNum == 1) {
			// ノードの共有化をした結果 children が 1つしかなくなった
			// 場合ノードを単純化する
			changeSimpleTypeNode(&children[i], nodePointerMap);
			// 0 ではなくEmptySetNodeをセットするように
			// AndNotNode::sharedQueryNodeを変更
			if (children[i] == emptySetNode) {
				if (i == 0) {
					// children[0]が空集合になったときは呼び出し側で自身を
					// 空集合にする
					return 0;
				} else {
					// children[1]が空集合になったときはchildrenの数(=1)を返す
					return 1;
				}
			}

		} else if (retGrandChildNum == 0) {
			// 子ノードのsharedQueryNodeの戻り値が0
			// この子ノードはEmptySetNode
			addQueryNodePointerMap(children[i], nodePointerMap);
			children[i] = const_cast<ModInvertedQueryNode*>(emptySetNode);
			if (i == 0) {
				// children[0]が空集合になったときは呼び出し側で自身を
				// 空集合にする
				return 0;
			} else {
				// children[1]が空集合になったときはchildrenの数(=1)を返す
				return 1;
			}
		}
	}
	//
	// ここにくるのはchildが2個ある場合だけ
	//
	ModUnicodeString key[2];
	children[0]->getQueryString(key[0]);
	children[1]->getQueryString(key[1]);

	if (key[0] == key[1]) {
		// childrenの2つが同じ内容だった場合
		addQueryNodePointerMap(children[0], nodePointerMap);
		children[0] = const_cast<ModInvertedQueryNode*>(emptySetNode);

		return 1;                       // 1でリターン
	}

	ModInvertedQueryNode* tmpEndNode=0;

	// globalNodeMapへ登録する
	for (i = 0; i < 2; ++i) {
		if (ModInvertedIsInternalNode(children[i]->getType()) != ModTrue) {
			// リーフノードの場合globalNodeMapへ登録してはいけない
			continue;                   // 次の child へ
		}
		// globalNodeMapに同じQueryStringのノードがあるか調査する
		p = globalNodeMap.find(key[i]);
		if (p != globalNodeMap.end()) {
			if (children[i] == (*p).second) {
				// すでに共有しているので特に処理しない
			} else {
				// 既に同じQueryStringのノードがある

				// OrderedDistanceの共有の場合はendNodeを考慮する必要がある
				if (ModInvertedAtomicMask(children[i]->getType())
					== ModInvertedQueryNode::orderedDistanceNode) {
					// tmpEndNodeが0以外なら、
					// 削除される側のOrderedDistanceがendNodeを持っている
					tmpEndNode = children[i]->getEndNode();

					// originalStringも共有する
					sharedOriginalString(children[1], (*p).second);
				}
				addQueryNodePointerMap(children[i], nodePointerMap);

				// childは先にglobalNodeMapにあったnodeへのポインタをセットする
				children[i] = (*p).second;

				// OrderedDistance かつ endNodeを持っていた
				if (tmpEndNode != 0) {
					children[i]->setEndNode(tmpEndNode);
					tmpEndNode = 0;
				}
			}
		} else {
			// globalNodeMap へ挿入する
			globalNodeMap[key[i]] = children[i];
		}
	}

	return children.getSize();
}

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::eraseTermLeafNode -- TermLeafNodeの消去
//
// NOTES
// TermLeafNode を消去して SimpleToken/OrderedDistance にする
// QueryInternaNode の eraseTermLeafNode をオーバライドしている
//
// InternalNodeと異なる点
//  - returnの条件が異なる
//		一つ目の子ノードが空集合ノードの場合ModFalseを返し、呼び出し側で
//		自分自信を空集合ノードに置き換えてもらう
//
//		二つ目の子ノードが空集合ノードの場合ModFalseを返す。この時、
//		引数ノードに一つ目の子ノードをセットしてかえし呼び出し側で
//		children[0]に置き換えてもらう(子ノードの昇格)
//
//
// ARGUMENTS
// QueryNode*& node
//      昇格させる子ノードのポインタをセットしてかえす。
//		(2つめの子ノードが空集合のケース)
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
ModBoolean
ModInvertedOperatorAndNotNode::eraseTermLeafNode(QueryNode*& node,
												 Query& query)
{
	ModBoolean noEmpty = ModTrue;		// 空集合なしフラグ
	node = 0;							// 常に 0
	ModInvertedQueryNode::NodeType childType;

	// まず children[0] の処理
	ModVector<ModInvertedQueryNode*>::Iterator child(&children[0]);

	childType = ModInvertedAtomicMask((*child)->getType());

	if (childType == ModInvertedQueryNode::termLeafNode) {
		// child が TermLeafNode の場合
		TermLeafNode* termNode = static_cast<TermLeafNode*>(*child);
		if (termNode->getQueryNodeForPreciseEvaluation() == emptySetNode) {
			// 空集合ノード発見
			noEmpty = ModFalse;			// 空集合ありにセット
		}
		// ポインタを付け換える

		// 明示的にAtomicを指定された場合(#term(...))はeraseTermLefNodeが
		// 指定されてもTermLeafNodeの削除は行わない。ただしpresiceNodeが
		// EmptySetNodeの場合は削除する

		*child = termNode->getQueryNodeForPreciseEvaluation();
		(*child)->setQueryNodeForRoughEvaluation(
			termNode->getQueryNodeForRoughEvaluation());
		(*child)->setOriginalString(termNode->termString,
#ifdef V1_6
									termNode->getLangSet(),
#endif // V1_6
									termNode->getMatchMode());

		termNode->setQueryNodeForPreciseEvaluation(0);
		termNode->setQueryNodeForRoughEvaluation(0);

		delete termNode;

	} else if (childType == ModInvertedQueryNode::booleanResultLeafNode) {
		// BooleanResultLeafNodeの場合
		query.addOrStanderdSharedNode(*child);
		if (static_cast<ModInvertedBooleanResultLeafNode*>(*child)
			->isEmptyResultLeafNode() == ModTrue) {
			// 空集合の場合
			noEmpty = ModFalse;
		}

	} else if (childType == ModInvertedQueryNode::rankingResultLeafNode) {
		// RankingResultLeafNodeの場合
		query.addOrStanderdSharedNode(*child);
		if (static_cast<ModInvertedRankingResultLeafNode*>(*child)
			->isEmptyResultLeafNode() == ModTrue) {
			// 空集合の場合
			noEmpty = ModFalse;
		}
	} else {
		// child が TermLeafNode 以外の場合再帰呼び出し
		QueryNode* tmpNode = 0;
		if ((*child)->eraseTermLeafNode(tmpNode, query) != ModTrue) {
			// 後処理が必要
			query.addOrStanderdSharedNode(*child);
			if (tmpNode == 0) {
				// 子ノードを削除
				noEmpty = ModFalse;
			} else {
				// 子ノードを昇格
				*child = tmpNode;
			}
		}
	}

	// 次に children[1] の処理
	child = &children[1];

	childType = ModInvertedAtomicMask((*child)->getType());

	if (childType == ModInvertedQueryNode::termLeafNode) {
		// child が TermLeafNode の場合
		TermLeafNode* termNode = static_cast<TermLeafNode*>(*child);
		if (termNode->getQueryNodeForPreciseEvaluation() == emptySetNode) {
			// 空集合ノード発見
			if(noEmpty != ModFalse) {
				// noEmptyがModFalseになっているのは一つ目の子ノードが
				// 空集合ノードのケース
				// 空集合ノードを昇格させても意味が無いのでnodeのセットは
				// 行わない
				node = this->children[0];	// children[0]を昇格させる
				noEmpty = ModFalse;			// 空集合ありにセット
			}
		}
		// 明示的にAtomicを指定された場合(#term(...))はeraseTermLefNodeが
		// 指定されてもTermLeafNodeの削除は行わない。ただしpresiceNodeが
		// EmptySetNodeの場合は削除する

		*child = termNode->getQueryNodeForPreciseEvaluation();
		(*child)->setQueryNodeForRoughEvaluation(
			termNode->getQueryNodeForRoughEvaluation());
		(*child)->setOriginalString(termNode->termString,
#ifdef V1_6
									termNode->getLangSet(),
#endif // V1_6
									termNode->getMatchMode());

		termNode->setQueryNodeForPreciseEvaluation(0);
		termNode->setQueryNodeForRoughEvaluation(0);

		delete termNode;
	
	} else if (childType == ModInvertedQueryNode::booleanResultLeafNode) {
		// BooleanResultLeafNodeの場合
		if (static_cast<ModInvertedBooleanResultLeafNode*>(*child)
			->isEmptyResultLeafNode() == ModTrue) {
			// 空集合の場合
			query.addOrStanderdSharedNode(*child);

			// children[1]がなくなったので、children[0]を昇格
			if(noEmpty != ModFalse) {
				// noEmptyがModFalseになっているのは一つ目の子ノードが
				// 空集合ノードのケース
				// 空集合ノードを昇格させても意味が無いのでnodeの
				// セットは行わない
				node = children[0];
				noEmpty = ModFalse;
			}
		}

	} else if (childType == ModInvertedQueryNode::rankingResultLeafNode) {
		// RankingResultLeafNodeの場合
		if (static_cast<ModInvertedRankingResultLeafNode*>(*child)
			->isEmptyResultLeafNode() == ModTrue) {
			// 空集合の場合
			query.addOrStanderdSharedNode(*child);

			// children[1]がなくなったので、children[0]を昇格
			if(noEmpty != ModFalse) {
				// noEmptyがModFalseになっているのは一つ目の子ノードが
				// 空集合ノードのケース
				// 空集合ノードを昇格させても意味が無いのでnodeの
				// セットは行わない
				node = children[0];
				noEmpty = ModFalse;
			}
		}

	} else {
		// child が TermLeafNode 以外の場合再帰呼び出し
		QueryNode* tmpNode = 0;
		if ((*child)->eraseTermLeafNode(tmpNode, query) != ModTrue) {
			// 後処理が必要
			query.addOrStanderdSharedNode(*child);
			if (tmpNode == 0) {
				// children[1]がなくなったので、children[0]を昇格
				if(noEmpty != ModFalse) {
					// noEmptyがModFalseになっているのは一つ目の子ノードが
					// 空集合ノードのケース
					// 空集合ノードを昇格させても意味が無いので
					// nodeのセットは行わない
					node = children[0];
					noEmpty = ModFalse;
				}
			} else {
				// 子ノードを昇格
				*child = tmpNode;
			}
		}
	}
	return noEmpty;
}

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::prefixString -- 演算子を表わす文字列を返す
//
// NOTES
// QueryNodeで定義された内容をオーバライドする
// 演算子を表わす文字列(#and-not)を返す
//
// ARGUMENTS
// ModString& prefix
//		演算子を表わす文字列を返す(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void 
ModInvertedOperatorAndNotNode::prefixString(ModUnicodeString& prefix,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam) const
{
	prefix += "#and-not";

	if (withCalOrCombName == ModTrue) {
		ModUnicodeString combinerName;
		getCalculatorOrCombinerName(combinerName, withCalOrCombParam);
		if(combinerName.getLength() > 0) {
			prefix += "[";
			prefix += combinerName;
			prefix += "]";
		}
	}
}

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::setGivneNegatorAsScoreNegator -- スコア否定器をセット
//
// NOTES
// スコア否定器をセット
//
// ARGUMENTS
// ScoreNegator* negator
//		スコア否定器へのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedOperatorAndNotNode::setGivneNegatorAsScoreNegator(
	ScoreNegator* negator)
{
	this->scoreNegator = negator;
}

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::setScoreNegator -- スコア否定器をセット
//
// NOTES
// スコア否定器をセット
//
// ARGUMENTS
// const ModUnicodeString& negatorName
//		スコア否定器名
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedOperatorAndNotNode::setScoreNegator(
	const ModUnicodeString& negatorName)
{
	scoreNegator = ModInvertedRankingScoreNegator::create(negatorName);
}

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::duplicate -- 自分のコピーを作成する
//
// NOTES
// 自分のコピーを作成する。Query::から呼ばれ、検索木を上からたどり、
// 検索木のコピーを作成する際に使用される。
// QueryNode::dupulicateをオーバーライド。AndNotNodeのコピーを作る。
//
// ARGUMENTS
// const ModInvertedQuery&
//		クエリ。
//
// RETURN
// 生成したコピーのノードのポインタ
//
// EXCEPTIONS
// ModInvertedErrorQueryValidateFail
//
ModInvertedQueryNode*
ModInvertedOperatorAndNotNode::duplicate(const ModInvertedQuery& rQuery)
{
	ModInvertedOperatorAndNotNode* node
		= new ModInvertedOperatorAndNotNode(firstStepResult->getType());

	// 自分にスコア計算器がセットされている場合のみ
	// 新しいノードに計算器をセットする
	if (this->scoreCombiner != 0) {
		node->setScoreCombiner(this->scoreCombiner->duplicate());
	}

	// 自分にスコア計算器がセットされている場合のみ
	// 新しいノードに計算器をセットする
	if (this->scoreNegator != 0) {
		node->setGivneNegatorAsScoreNegator(this->scoreNegator->duplicate());
	}

	ModVector<ModInvertedQueryNode*>::Iterator child = this->children.begin();
	ModVector<ModInvertedQueryNode*>::Iterator end = this->children.end();

	// 子ノードのduplicate
	while (child != end) {

		// 子ノードのduplicateの結果
		ModInvertedQueryNode* newChild;

		newChild = (*child)->duplicate(rQuery);

		// 子ノードを追加
		node->insertChild(newChild);
		++child;
	}

	return node;
}

//
// FUNCTION public
// ModInvertedOperatorAndNode::validate -- 正規表現ノードの有効化
//
// NOTES
// 初期化としてベクターファイルのオープン，ヒープファイルのオープン，
// 正規表現のコンパイルを行う。
//
// ARGUMENTS
// ModInvertedFile* invertedFile
//		転置ファイルへのポインタ
// const Query::ValidateMode mode
//		有効化モード
// ModInvertedQuery* rQuery
//		クエリ
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedOperatorAndNotNode::validate(
	InvertedFile* invertedFile,
	const Query::ValidateMode mode,
	ModInvertedQuery* rQuery)
{
	// ランキング検索の場合はスコア合成器をセットする
	if ((mode & Query::rankingMode) != 0) {
		if (this->scoreCombiner == 0) {
			// QueryNodeには必ずデフォルトの合成器をセットするように
			// なったので、ここではduplicateだけ
			ScoreCombiner* combiner = rQuery->getDefaultAndScoreCombiner();
			;ModAssert(combiner != 0);
			setScoreCombiner(combiner->duplicate());
		}
		if (this->scoreNegator == 0) {
			// スコア否定器をRankingOperatorAndNotNodeへセット
			const ScoreNegator* sn = rQuery->getDefaultScoreNegator();
			if (sn != 0) {
				// 否定器は陽にセットされている場合にのみセットする
				// 否定器がある場合、１/２番目の条件を満たす文書も正解に含まれ
				// てしまい、ブーリアンと検索結果が異なってしまう。
				// そこで、デフォルトでは否定器はセットされないようにする。
				setGivneNegatorAsScoreNegator(sn->duplicate());
			}
		}
	}

	// 子ノードの有効化
	ModInvertedQueryInternalNode::validate(invertedFile,mode,rQuery);
}

//
// FUNCTION public
// ModInvertedOperatorAndNotNode::checkQueryNode -- 子ノードの数をチェックする
//
// NOTES
// 有効化の最後に呼び出されて、子ノードの数をチェックする。もし異常で
// あれば例外を投げる。
//
// ここでは OrderedDistanceNode 用の定義がされている。子ノードの数が 2 
// 以外の場合異常と判断する。
//
// また、子ノードに対しても再帰的に本関数を呼び出す。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorQueryValidateFail
//		子ノード数が2以外であった
//
void
ModInvertedOperatorAndNotNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_
	, const ModBoolean needDF_
	)
{
	if (this->children.getSize() != 2) {
		// 子ノードが 2 以外なのは異常
		ModErrorMessage << "children size is not 2." << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}

	this->children[0]->checkQueryNode(query_, setStringInChildren_, needDF_);
	this->children[1]->checkQueryNode(query_, setStringInChildren_, ModTrue);
}

//
// FUNCTION protected
// ModInvertedOperatorAndNotNode::reevaluate -- 正確な再評価
//
// NOTES
// 粗い evaluate 満足を前提とした、正確な再評価
// AndNotとRankingAndNotでは動作がことなるため、AndNotNode::reevaluateを
// オーバーライド。
//	  AndNotはchildre[0]がマッチかつchildren[1]がアンマッチの場合のみヒット
//	  RankingAndNotはchildre[0]がマッチの場合にヒットした事になる
// children[0]のreevaluateをコール
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
//
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
//
ModBoolean
ModInvertedOperatorAndNotNode::reevaluate(
			ModInvertedDocumentID documentID)
{
	if (scoreNegator == 0) {
		return booleanReevaluate(documentID);
	}
	return this->children[0]->reevaluate(documentID);
}

//
// FUNCTION protected
// ModInvertedOperatorAndNotNode::reevaluate -- 正確な再評価
//
// NOTES
// 粗い evaluate 満足を前提とした、正確な再評価
// AndNotとRankingAndNotでは動作がことなるため、AndNotNode::reevaluateを
// オーバーライド。
//	  AndNotはchildre[0]がマッチかつchildren[1]がアンマッチの場合のみヒット
//	  RankingAndNotはchildre[0]がマッチの場合にヒットした事になる
// children[0]のreevaluateをコール
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedLocationListIterator*& locations
//		位置情報（結果格納用）
// ModSize& uiTF_
//		(位置情報リストを取得できない場合) TF
// ModInvertedQueryNode* givenQueryNode
//		ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
//
ModBoolean
ModInvertedOperatorAndNotNode::reevaluate(
	ModInvertedDocumentID documentID,
	ModInvertedLocationListIterator*& locations,
	ModSize& uiTF_,
	ModInvertedQueryNode* givenNode)
{
	if (scoreNegator == 0) {
		return booleanReevaluate(documentID, locations, uiTF_, givenNode);
	}
	return this->children[0]->reevaluate(
		documentID, locations, uiTF_, givenNode);
}

//
// FUNCTION protected
// ModInvertedOperatorAndNotNode::booleanReevaluate -- 正確な再評価
//
// NOTES
// 粗い evaluate 満足を前提とした、正確な再評価
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
//
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedOperatorAndNotNode::booleanReevaluate(
	ModInvertedDocumentID documentID)
{
	if(children[1]->calcSortFactor() != MaxSortFactor) {

		// 否定条件を調べる
		if (children[1]->evaluate(documentID, QueryNode::defaultEMode)
			== ModTrue) {
			return ModFalse;
		}

		return children[0]->reevaluate(documentID);

	} else {

		if(children[0]->reevaluate(documentID) == ModFalse) {
			return ModFalse;
		}
		// 否定条件を調べる
		if (children[1]->evaluate(documentID, QueryNode::defaultEMode)
			== ModTrue) {
			return ModFalse;
		}
		return ModTrue;
	}
}

//
// FUNCTION protected
// ModInvertedOperatorAndNotNode::booleanReevaluate -- 正確な再評価
//
// NOTES
// 粗い evaluate 満足を前提とした、正確な再評価
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedLocationListIterator*& locations
//		位置情報（結果格納用）
// ModSize& uiTF_
//		(位置情報リストを取得できない場合) TF
// ModInvertedQueryNode* givenQueryNode
//		ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedOperatorAndNotNode::booleanReevaluate(
	ModInvertedDocumentID documentID,
	ModInvertedLocationListIterator*& locations,
	ModSize& uiTF_,
	ModInvertedQueryNode* givenQueryNode)
{
	// ANDNOTなので子ノードは二つだけ
	
	// ANDNOTなので否定の方の条件の位置情報リストは不要
	// 否定の方の索引語を含む文書IDは省かれるため
	// ModInvertedOperatorAndNotNode::reevaluateを参照のこと
	
	// SortFactorは、子ノードの処理にかかるコストをあらわす。
	// ModInvertedQueryNode.hを参照のこと。

	if(children[1]->calcSortFactor() != MaxSortFactor) {
		// 否定条件を調べる
		if (children[1]->reevaluate(documentID) == ModTrue) {
			return ModFalse;
		}
		return this->children[0]->reevaluate(documentID, locations, uiTF_);
	} else {
		// 否定条件を調べる
		if(this->children[0]->reevaluate(documentID, locations, uiTF_) == ModFalse) {
			return ModFalse;
		}
		if (children[1]->reevaluate(documentID) == ModTrue) {
			locations->release();
			return ModFalse;
		}
		return ModTrue;
	}
}

//
// FUNCTION
// ModInvertedOperatorAndNotNode::lowerBoundScoreForSecondStep() -- ランキング検索の第２ステップで使用するlowerBound
//
// NOTES
// ランキング検索のスコア計算第２ステップで使用されるlowerBound。
//
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
ModBoolean
ModInvertedOperatorAndNotNode::lowerBoundScoreForSecondStep(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
	// スコアの初期化
	DocumentScore score0(0.0);
	DocumentScore score1(0.0);

	DocumentID id0 = givenID;
	DocumentID id1(0);
	while(children[0]->lowerBoundScoreForSecondStep(
			id0, id0, score0) == ModTrue) {
		if(children[1]->lowerBoundScoreForSecondStep(
				id0, id1, score1) == ModFalse ||
		   id0 < id1) {
				foundID = id0;
				if(scoreNegator != 0){
					score = (*scoreCombiner)(score0,(*scoreNegator)(0));
				} else {
					score = score0;
				}
				return ModTrue;
		}
		if(scoreNegator != 0) {
			foundID = id0;
			score = (*scoreCombiner)(score0,(*scoreNegator)(score1));
			return ModTrue;
		}
		++id0;
	}
	return ModFalse;
}

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
