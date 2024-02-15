// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorAndNode.cpp -- 積集合ノードの実装
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "ModInvertedOrderedDistanceNode.h"
#include "ModInvertedOperatorAndNode.h"
#include "ModInvertedException.h"
#include "ModInvertedQuery.h"
#include "ModInvertedTermLeafNode.h"
#include "ModInvertedBooleanResultLeafNode.h"
#ifdef	SIMPLE
#include "ModInvertedUnorderedSimpleWindowLocationListIterator.h"
#else
#include "ModInvertedUnorderedOperatorWindowLocationListIterator.h"
#endif
#include "ModInvertedRankingResultLeafNode.h"
#include "ModInvertedRankingScoreCombiner.h"
#include "ModInvertedFileCapsule.h"

#include "ModInvertedUnorderedOperatorWindowLocationListIterator.h"

//
// FUNCTION public
// ModInvertedOperatorAndNode::~ModInvertedOperatorAndNode -- ANDノードを破棄する
//
// NOTES
// 積集合(AND)ノードを破棄する。
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
ModInvertedOperatorAndNode::~ModInvertedOperatorAndNode()
{
	if(scoreCombiner != 0) {
		delete scoreCombiner;
		scoreCombiner = 0;
	}
}

#ifndef DEL_BOOL
// 
// FUNCTION public
// ModInvertedOperatorAndNode::retrieve -- 検索の一括実行
// 
// NOTES
// 検索式を一括実行する。
//
// 引数 mode によって term-at-a-time で検索するか 
// document-at-a-time で検索するか決る。デフォルトでは 
// document-at-a-time。
//
// document-at-a-time の場合まず this->lowerBound() を roughEvaluation 
// mode で実行し、その結果を this->reevaluate() で評価し正確な検索結果
// を得る。
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
// 下位からの例外をそのまま返す
// 
void
ModInvertedOperatorAndNode::retrieve(ModInvertedBooleanResult& queryResult,
									 Query::EvaluateMode mode)
{
	if ((mode & Query::roughEvaluationFlag) != 0
		&& this->queryNodeForRoughEvaluation != 0
		&& this->queryNodeForRoughEvaluation != this) {
		// queryNodeForRoughEvaluation と this を比較しているのは
		// roughPointerを共有した場合に無限Loopに陥いるのを防ぐため
		queryNodeForRoughEvaluation->retrieve(queryResult, mode);
		return;
	}

	// 検索結果をまず空にする。
	if (!queryResult.isEmpty()) {
		queryResult.clear();
	}

	estimateDocumentFrequency();

	if (estimatedDocumentFrequency != QueryNode::UndefinedFrequency) {
		// 出現頻度と同じ分だけ予約しておく
		queryResult.reserve(estimatedDocumentFrequency);
	}

	// retrieve 処理
	ModVector<ModInvertedQueryNode*>::Iterator child = this->children.begin();
	if ((mode & Query::andTermAtATimeFlag) != 0) {
		// term-at-a-time
		ModInvertedBooleanResult tmpResult[2];
		int tmpResultIndex = 0;
		// 最初の子ノードで粗い評価による中間結果を得る。
		(*child)->retrieve(tmpResult[0], mode | Query::roughEvaluationFlag);

		// もう一方の tmpResult も同サイズにしておく。
		tmpResult[1].assign(tmpResult[0].getSize(), 0);
		// 残りの子ノードで粗い評価を行なう。
		for (++child; child != this->children.end(); ++child) {
			tmpResultIndex = 1 - tmpResultIndex;	// 0 <--> 1
			// これまでの中間結果の１つ１つについて、次の子ノードで検査する。
			ModSize i = 0;
			for (ModSize j = 0;j < tmpResult[1 - tmpResultIndex].getSize(); ++j) {
				if ((*child)->evaluate(tmpResult[1-tmpResultIndex].getDocID(j), mode | Query::roughEvaluationFlag)
					== ModTrue) {
					tmpResult[tmpResultIndex].setDocID(i++,tmpResult[1-tmpResultIndex].getDocID(j));
				}
			}
			// tmpResult の不要部分の消去
			tmpResult[tmpResultIndex].erase(i, tmpResult[tmpResultIndex].getSize());
		}
		// 最終結果の評価
		if ((mode & Query::roughEvaluationFlag) != 0) {
			queryResult = tmpResult[tmpResultIndex];
		} else {	// precise evaluation
			ModInvertedBooleanResult::Iterator p
				= tmpResult[tmpResultIndex].begin();
			ModInvertedBooleanResult::Iterator e
				= tmpResult[tmpResultIndex].end();
			for (; p != e; ++p) {
				if (this->evaluate(*p, mode & ~Query::roughEvaluationFlag)
					== ModTrue) {
					queryResult.pushBack(*p);
				}
			}
		}
	} else {	// document-at-atime (default)
		// DocumentID (初期値は最小値)
		ModInvertedDocumentID currentID = 1;

		// ラフがあるかはじめに判断できるのであれば、判断した方が効率的
		if (this->queryNodeForRoughEvaluation != 0 &&
			this->queryNodeForRoughEvaluation != this) {
			; ModAssert(queryNodeForRoughEvaluation
						->getQueryNodeForRoughEvaluation()
						== queryNodeForRoughEvaluation);
			while (queryNodeForRoughEvaluation->lowerBound(
				currentID, currentID,
#ifdef	ROUGH_MODE_FLAG
				mode|Query::roughEvaluationFlag
#else	// ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
				mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
				) == ModTrue) {
				if (this->reevaluate(currentID) == ModTrue) {
					// reevaluate() が True だったら結果を格納
					queryResult.pushBack(currentID);
				}			
				currentID++;
			}
		} else {
			// まず粗くlowerBound()を実行してからreevaluate()を実行する
			while (lowerBound(currentID, currentID ,
							  mode|Query::roughEvaluationFlag) == ModTrue) {

				if (this->reevaluate(currentID) == ModTrue) {
					// reevaluate() が True だったら結果を格納
					queryResult.pushBack(currentID);
				}			
				currentID++;
			}
		}
	}
	// retrieve 終了。estimatedDocumentFrequency に正確な文書頻度を設定する
	this->estimatedDocumentFrequency = queryResult.getSize();
	this->retrieved = ModTrue;

	// queryResultに割り当てられたメモリを最小限にするためreserve()を実行
	queryResult.reserve(estimatedDocumentFrequency);
}
#endif

// 
// FUNCTION public
// ModInvertedOperatorAndNode::evaluate -- 与えられた文書が検索式を満たすかどうかの検査
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
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedOperatorAndNode::evaluate(DocumentID documentID,
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
	if ((mode & Query::roughEvaluationFlag) != 0 &&
		this->queryNodeForRoughEvaluation != 0 &&
		this->queryNodeForRoughEvaluation != this) {
		// queryNodeForRoughEvaluation と this を比較しているのは
		// roughPointerを共有した場合に無限Loopに陥いるのを防ぐため
		return this->queryNodeForRoughEvaluation->evaluate(
			documentID,
#ifdef	ROUGH_MODE_FLAG
			mode
#else	// ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
			mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
			);
	}

	// 子ノードの検査
	// まずラフに evaluate
	if (this->queryNodeForRoughEvaluation != 0 &&
		this->queryNodeForRoughEvaluation != this) {
		// ラフノードがあり自分と違うなら使う
		if (queryNodeForRoughEvaluation->evaluate(
			documentID,
#ifdef	ROUGH_MODE_FLAG
			mode|Query::roughEvaluationFlag
#else	// ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
			mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
			) == ModFalse) {
			return ModFalse;			// 条件を不満足
		}
	} else {
		// ラフノードがないか自分と同じ場合
		// 常に document-at-a-time
		for (ModVector<QueryNode*>::Iterator child = this->children.begin();
			 child != this->children.end();
			 ++child) {
			if ((*child)->evaluate(documentID, mode|Query::roughEvaluationFlag)
				== ModFalse) {
				return ModFalse;		// この子ノードの条件を不満足
			}
		}
	}

	if ((mode & Query::roughEvaluationFlag) != 0) {
		if (this == this->queryNodeForRoughEvaluation) {
			// ラフノードと自分が同じなら、ラフモードでも lower/upper 
			// を設定する (つまり全て SimpleTokenNode で構成されている
			// ということ
			this->upper = this->lower = documentID;
		}
		return ModTrue;					// ラフモードはここでおわり
	}
	// 再評価を行なう
	if (this->reevaluate(documentID) == ModTrue) {
		this->upper = this->lower = documentID;
		return ModTrue;					// 条件を満足
	}
	return ModFalse;
}

// 
// FUNCTION public
// ModInvertedOperatorAndNode::lowerBound -- 検索式を満たす文書のうち、文書IDが与えられた値以上で最小の文書の検索
// 
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する。
// 
// ARGUMENTS
// ModInvertedDocumentID givenDocumentID
//		文書ID
// ModInvertedDocumentID& foundDocumentID
//		結果格納用の文書IDオブジェクト
// ModInvertedQuery::EvaluateMode mode
//		評価モード
// 
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedOperatorAndNode::lowerBound(ModInvertedDocumentID givenDocumentID,
									   ModInvertedDocumentID& foundDocumentID,
									   ModInvertedQuery::EvaluateMode mode)
{
	// 既に調査済み？
	if (givenDocumentID >= this->lower) {
		if (this->upper == ModSizeMax) {
			return ModFalse;		// 既にEndに達っしている
		}
		if (givenDocumentID <= this->upper) {
			foundDocumentID = this->upper;
			return ModTrue;
		}
	}
	if ((mode & Query::roughEvaluationFlag) != 0 &&
		this->queryNodeForRoughEvaluation != 0 &&
		this->queryNodeForRoughEvaluation != this) {
		// queryNodeForRoughEvaluation と this を比較しているのは
		// roughPointerを共有した場合に無限Loopに陥いるのを防ぐため
		return this->queryNodeForRoughEvaluation->lowerBound(
			givenDocumentID, foundDocumentID,
#ifdef	ROUGH_MODE_FLAG
			mode
#else	// ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
			mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
			);
	}

	ModInvertedDocumentID currentID = givenDocumentID;
	if (this->queryNodeForRoughEvaluation != 0 &&
		this->queryNodeForRoughEvaluation != this) {
		// ラフノードがあり自分と違うなら使う
		while (1) {

			// まずラフに lowerBound
			if (queryNodeForRoughEvaluation->lowerBound(
				currentID, currentID,
#ifdef	ROUGH_MODE_FLAG
				mode|Query::roughEvaluationFlag
#else	// ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
				mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
				) == ModFalse) {
				return ModFalse;			// 条件を満足する文書はない
			}
			; ModAssert((mode & Query::roughEvaluationFlag) == 0);
			if (this->reevaluate(currentID) == ModTrue) {
				foundDocumentID = currentID; // 条件を満足
				if ((mode & Query::roughEvaluationFlag) == 0) {
					this->upper = currentID;
					this->lower = givenDocumentID;
				}
				return ModTrue;
			}
			currentID++;
		}

	} else {
		// 子ノードの lowerBound 呼び出し - 常に document-at-a-time
		ModVector<QueryNode*>::Iterator child = this->children.begin();
		ModInvertedDocumentID smallestID = givenDocumentID;
		while (1) {
			if (child == this->children.end()) { // 全ての子ノードの条件を満足
				if ((mode & Query::roughEvaluationFlag) == 0 &&
					this != this->queryNodeForRoughEvaluation) {
					// ラフフラグがオフでラフノードが自分と違う場合は
					// reevaluate() を呼ぶ
					if (this->reevaluate(currentID) == ModTrue) {
						// 条件を満足した。
						this->upper = foundDocumentID = currentID;
						this->lower = givenDocumentID;
						return ModTrue;
					}
				} else {
					// ラフノードなので reevaluate() の呼び出しは必要ない
					foundDocumentID = currentID;
					if (this == queryNodeForRoughEvaluation) {
						// ラフモードであっても、ラフノードが自分と同じなら
						// lower/upper をセット 
						this->upper = currentID;
						this->lower = givenDocumentID;
					}
					return ModTrue;
				}

				++currentID;
				child = this->children.begin();

			} else if ((*child)->lowerBound(currentID, smallestID,
											mode|Query::roughEvaluationFlag)
					   == ModFalse) {
				this->lower = givenDocumentID;
				this->upper = ModSizeMax;
				return ModFalse;	// この子ノードの条件を満足する文書はない

			} else if (currentID < smallestID) {
				currentID = smallestID;	 // 最小候補に改めて、
				if (child == this->children.begin()) {		// 先頭なら次へ
					++child;
				} else {									// 他なら先頭へ
					child = this->children.begin();
				}
			} else { // currentID == smallestID
				// currentID で満足
				++child;					// 次の子ノードへ
			}
		}
	}
}

// 
// FUNCTION public
// ModInvertedOperatorAndNode::evaluateScore -- 与えられた文書のスコアを計算する
// 
// NOTES
// 与えられた文書のスコアを計算する
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
// 下位からの例外はそのままスロー
// 
ModBoolean
ModInvertedOperatorAndNode::evaluateScore(const DocumentID ID,
										  DocumentScore& score,
										  Query::EvaluateMode mode)
{
	score = 0.0;						// 0 クリアしておく
	// rough mode は実装しない

	// 既に調査済み？
	if (ID >= this->lower) {
		if (ID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		} else if (ID != this->upper) {
			// まずラフに検査
			if (evaluate(ID, mode|Query::roughEvaluationFlag) == ModFalse) {
				return ModFalse;
			}
		}
	} else {
		// まずラフに検査
		if (evaluate(ID, mode|Query::roughEvaluationFlag) != ModTrue) {
			return ModFalse;
		}
	}

	// 子ノードのスコア計算 - 常に document-at-a-time
	DocumentScore childScore;
	ModVector<QueryNode*>::Iterator child = this->children.begin();

	ModVector<DocumentScore>::Iterator i = scores.begin();
	for (; child != this->children.end(); ++child, ++i) {
		if ((*child)->evaluateScore(ID, childScore, mode) == ModFalse) {
			return ModFalse;	// この子ノードの条件を不満足
		}

		*i = childScore;
	}

	// スコアを計算する。
	// 子ノードのスコアから自分のスコアを合成する
	score = this->scoreCombiner->apply(scores);

	// ヒットしたのでupper/lowerのセット
	this->upper = this->lower = ID;

	return ModTrue;
}

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
// 
// FUNCTION public
// ModInvertedOperatorAndNode::evaluateScore -- 与えられた文書のスコアを計算する（位置も計算する）
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
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
ModBoolean
ModInvertedOperatorAndNode::evaluateScore(const DocumentID ID,
										  DocumentScore& score,
										  LocationIterator*& locations,
										  Query::EvaluateMode mode,
										  ModInvertedQueryNode* givenEndNode)
{
	score = 0.0;						// 0 クリアしておく
	DocumentScore childScore = 0.0;		// 子ノードのスコア

	// rough mode は実装しない

	// 既に調査済み？
	if (ID >= this->lower) {
		if (ID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		} else if (ID != this->upper) {
			// まずラフに検査
			if (evaluate(ID, mode|Query::roughEvaluationFlag) == ModFalse) {
				return ModFalse;
			}
		}
	} else {
		// まずラフに検査
		if (evaluate(ID, mode|Query::roughEvaluationFlag) == ModFalse) {
			return ModFalse;
		}
	}

	// 子ノードのスコアと位置情報を計算
	// 常に document-at-a-time
	ModSize numberOfChildren = children.getSize();
	LocationIterator* childLocation = 0;

	iterator = static_cast<UnorderedOperatorLocationIterator*>(getFreeList());
	if (iterator == 0)
	{
		iterator = new UnorderedOperatorLocationIterator(this);
		itertor->reserve(numberOfChildren);
	}
	LocationIterator::AutoPointer p = iterator;

	for (ModSize i = 0; i < numberOfChildren; ++i) {

		if ((children[i])->evaluateScore(ID,
										 scores[i], childLocation, mode)
			== ModFalse) {
			return ModFalse;	// この子ノードの条件を不満足
		}
		iterator->pushIterator(childLocation);
	}

	// 順序無視位置検査用反復子を有効化 (min=1, max=ModSizeMax)
	iterator->initialize(1, ModSizeMax);

	// iteratorがendなのは異常
	ModAssert(iterator->isEnd() == ModFalse);

	// スコアを計算する。
	// 子ノードのスコアから自分のスコアを合成する
	score = this->scoreCombiner->apply(scores);
	this->upper = this->lower = ID;
	locations = p.release();
	return ModTrue;
}
#endif

// 
// FUNCTION public
// ModInvertedOperatorAndNode::lowerBoundScore -- 検索式を満たす文書のうち、文書IDが与えられた値以上で最小の文書の検索しスコアを計算する
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
// ModInvertedErrorRetrieveFail
//
ModBoolean
ModInvertedOperatorAndNode::lowerBoundScore(const DocumentID givenID,
											DocumentID& foundID,
											DocumentScore& score,
											Query::EvaluateMode mode)
{
	DocumentID tmpID(givenID);
	score = 0.0;					// 0クリアしておく
	ModBoolean boolval;

	// 既に調査済み？
	if (givenID >= this->lower) {
		if (givenID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		} else if (givenID <= this->upper) {
			// evaluateScoreによりlowerが書き換えられないようlowerを保存
			DocumentID tmpLower = this->lower;

			// evaluateScore() で正確な検索とスコアを計算する
			boolval = evaluateScore(this->upper, score, mode);
			if (boolval == ModTrue) {
				// マッチした(必ずここを通るはず)
				// evaluateScoreにより書き換えられたlowerを元に戻す
				this->lower = tmpLower;
				foundID = this->upper;
				return ModTrue;
			} else {
				// evaluateScoreがfalseを返すのは異常なケース
				; ModAssert(0);
			}
		} 
	}

	while (1) {
		// まず粗く lowerBound() してみる
		boolval = lowerBound(tmpID, foundID, mode | Query::roughEvaluationFlag);

		if (boolval == ModFalse) {
			// マッチするものはない
			this->lower = givenID;
			this->upper = ModSizeMax;
			break;
		}
		// evaluateScore() で正確な検索とスコアを計算する
		boolval = evaluateScore(foundID, score, mode);
		if (boolval == ModTrue) {
			this->lower = givenID;
			this->upper = foundID;
			// マッチした
			break;
		}
		// マッチしなかったので、lowerBound() からやりなおし
		tmpID = foundID + 1;
	}
	return boolval;

}

//
// FUNCTION public
// ModInvertedRankingOperatorOrNode::reserveScores -- scoresをリザーブ
//
// NOTES
// 子ノード辿りrankingOr,rankingAndの場合はメンバ変数であるscoresを
// リザーブする。本関数がコールされるのはrankingAndの場合であるため、
// scoresをリザーブしさらに自分の子ノードをたどる。
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
void 
ModInvertedOperatorAndNode::reserveScores()
{
	ModSize size(children.getSize());

	scores.reserve(size);
	scores.insert(scores.begin(), size, 0.0);

	ModInvertedQueryInternalNode::reserveScores();
}


// 
// FUNCTION public
// ModInvertedOperatorAndNode::estimateDocumentFrequency -- 文書頻度の見積もり
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
// 下位からの例外をそのまま返す
// 
ModSize
ModInvertedOperatorAndNode::estimateDocumentFrequency()
{
	if (this->estimatedDocumentFrequency == QueryNode::UndefinedFrequency
		&& this->children.getSize() > 0) {
		// 上目に見積もるために、最小値を用いる。
		ModVector<QueryNode*>::Iterator child = this->children.begin();
		this->estimatedDocumentFrequency
		  = (*child)->estimateDocumentFrequency();
		for (++child; child != this->children.end(); ++child) {
			if (this->estimatedDocumentFrequency 
				> (*child)->estimateDocumentFrequency()) {
				this->estimatedDocumentFrequency
				  = (*child)->estimateDocumentFrequency();
			}
		}
	}
	return this->estimatedDocumentFrequency;
}

//
// FUNCTION public
// ModInvertedOperatorAndNode::flattenChildren -- 子ノードリストの平坦化
//
// NOTES
// 子ノードリストの平坦化 (例 #and(#and(X,Y),Z) → #and(X,Y,Z))
//
// ただしRankingの場合はandノードのscoreCombinerの種類が同じ、かつ
// Associative(結合律が成り立つ)の場合のみ可。
//
// 平坦化後不要になったANDノードを削除する際に sharedNodeMap を参照し、
// このMapに登録されているノードは削除しない。Queryのデストラクト時に
// sharedNodeMapを参照しながら削除する。
//
// ARGUMENTS
// const QueryNodePointerMap& sharedNodeMap
//		OR標準形変換時に共有しているノードが登録されているマップ変数
// const ModBoolean isChildOfWindowNode
//		windowノードを親にもっている場合 ModTure となる
//
// RETURN
// なし
//
void
ModInvertedOperatorAndNode::flattenChildren(
	const QueryNodePointerMap& sharedNodeMap,
	const ModBoolean isChildOfWindowNode)
{
	ModVector<ModInvertedQueryNode*>::Iterator child = children.begin();

	ModString combinerName;
	if (this->scoreCombiner != 0) {
		this->scoreCombiner->getDescription(combinerName);
	}

	while (child != children.end()) {
		if(ModInvertedAtomicMask((*child)->getType())
					!= ModInvertedQueryNode::operatorAndNode) {
			// And ノード以外の場合
			(*child)->flattenChildren(sharedNodeMap, isChildOfWindowNode);
			++child;
			continue;
		}


		// ANDノードの場合
		ModVector<ModInvertedQueryNode*>* grandChildren;

		grandChildren = (*child)->getChildren();

		// 平坦化できるかチェック
		if (this->scoreCombiner != 0) {
			// 自分にCombinerがセットされている

			ModInvertedRankingScoreCombiner* childSc 
				= (*child)->getScoreCombiner();
			ModAssert(childSc != 0);
			
			ModString childCombinerName;
			childSc->getDescription(childCombinerName);

			if (combinerName != childCombinerName) {
				// combinerの種類が異なるため平坦化できない
				++child;
				continue;
			} else {
				// combinerの種類は同じ
				if (this->scoreCombiner->isAssociative() != ModTrue) {
					// combinerがAssociativeでない
					++child;
					continue;
				}
			}
		}

		ModVector<ModInvertedQueryNode*>::Iterator grandChild = grandChildren->begin();
		ModInvertedQueryNode* reducedChild = *child;

		// AND のあった位置に孫の1番目のノードをセット
		*child++ = *grandChild++;

		// childのindexを計算
		ModSize n = child - children.begin();

		// 孫の2番目以降のノードをセット
		children.insert(child, grandChild, grandChildren->end());

		// child を孫を挿入した位置まで戻す
		child = (children.begin() + n - 1);

		if (sharedNodeMap.find(reducedChild) == sharedNodeMap.end()) {
			// sharedNodeMapには登録されていない
			delete reducedChild;		// AND ノードを削除
		}
	}
}

//
// FUNCTION public
// ModInvertedOperatorAndNode::makeRoughPointer -- queryNodeForRoughEvaluation の作成
//
// NOTES
// queryNodeForRoughEvaluation を作成する。
//
// ModInvertedQueryNode の makeRoughPointer() をオーバライドしている。
//
// children をひとつずつみていき、children の queryNodeForRoughEvaluation 
// をそれぞれ And で継げたものを自分の queryNodeForRoughEvaluation とする。
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
// const Query::ValidateMode mode
//		有効化モード
// QueryNodePointerMap& parentMap,
//		ノードマップ
// const ModInvertedQuery* Query
//		クエリ
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModInvertedOperatorAndNode::makeRoughPointer(const Query::ValidateMode mode,
											 QueryNodePointerMap& parentMap,
											 const ModInvertedQuery* Query) 
{
	QueryNodePointerMap tmpMap;
	ModVector<ModInvertedQueryNode*>::Iterator p = this->children.begin();
	for (; p != this->children.end(); ++p) {
		// makeRoughPointer の下請け関数を呼び出す
		if (this->makeRoughMap(mode, *p, tmpMap, Query) != ModTrue) {
			// 子ノードのラフノードに空集合が含まれていた
			// roughPointer の内容も空集合にしておく
			this->queryNodeForRoughEvaluation = *p;

			return;
		}
	}

	// tmpMap の内容をラフノードとする
	this->makeRoughNode(tmpMap, Query, mode);

	// tmpMap を親ノード用の map に挿入する
	parentMap.insert(tmpMap.begin(), tmpMap.end());
}

// 
// FUNCTION public
// ModInvertedOperatorAndNode::calcSortFactor -- sortFactor の計算
// 
// NOTES
// sortChildren() 関数で使用する sortFactor メンバ変数を計算する。
// 
// ModInvertedQueryInternalNode::calcSortFactor() をオーバライドしてい
// る。AndNode の sortFactor は 子ノードの sortFactor の最小値と子ノー
// ド数を掛け合せた値になる。
//
// ARGUMENTS
// なし
// 
// RETURN
// 計算した sortFactor 値。
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModSize
ModInvertedOperatorAndNode::calcSortFactor()
{
	if (this->sortFactor == 0) {
		// まだ計算していないのなら、計算する
		ModSize cmin(ModSizeMax);
		ModSize childSortFactor;
		ModVector<ModInvertedQueryNode*>::Iterator child = children.begin();
		for (;child != children.end(); ++child) {
			childSortFactor = (*child)->calcSortFactor();

			if (childSortFactor == ModInvertedQueryNode::MaxSortFactor) {
				// MaxSortFactorが返されるのは自分以下のノードにRegex
				// が含まれるケース
				this->sortFactor = childSortFactor;
				return this->sortFactor;
			}
			if (cmin > childSortFactor) {
				cmin = childSortFactor;
			}
		}
		this->sortFactor = children.getSize() * cmin;
	}
	return this->sortFactor;
}

//
// FUNCTION public
// ModInvertedOperatorAndNode::prefixString -- 演算子を表わす文字列(#and)を返す
//
// NOTES
// QueryNodeで定義された内容をオーバライドする
// 演算子を表わす文字列(#and)を返す
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
ModInvertedOperatorAndNode::prefixString(ModUnicodeString& prefix,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam) const
{
	prefix += "#and";
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
// ModInvertedQueryNode::duplicate -- 自分のコピーを作成する
//
// NOTES
// 自分のコピーを作成する。Query::から呼ばれ、検索木を上からたどり、
// 検索木のコピーを作成する際に使用される。
// QueryNode::dupulicateをオーバーライド。AndNodeのコピーを作る。
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
ModInvertedOperatorAndNode::duplicate(const ModInvertedQuery& rQuery)
{
	ModInvertedOperatorAndNode* node = new ModInvertedOperatorAndNode(firstStepResult->getType());

	ModString scoreCombinerName;

	// 自分にスコア計算器がセットされている場合のみ
	// 新しいノードに計算器をセットする
	if (this->scoreCombiner != 0) {
		node->setScoreCombiner(this->scoreCombiner->duplicate());
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
// ModInvertedOperatorAndNode::validate -- 有効化
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
ModInvertedOperatorAndNode::validate(
	InvertedFile* invertedFile,
	const Query::ValidateMode mode,
	ModInvertedQuery* rQuery)
{
	// ランキング検索の場合はスコア計算器をセットする
	if ((mode & Query::rankingMode) != 0) {
		if (this->scoreCombiner == 0) {
			// QueryNodeには必ずデフォルトの合成器をセットするように
			// なったので、ここではduplicateだけ
			ScoreCombiner* combiner = rQuery->getDefaultAndScoreCombiner();
			;ModAssert(combiner != 0);
			setScoreCombiner(combiner->duplicate());
		}
	}
	// 子ノードの有効化
	ModInvertedQueryInternalNode::validate(invertedFile,mode,rQuery);
}

// FUNCTION protected
// ModInvertedRankingOperatorAndNode::sortChildren -- 子ノードリストの並べ替え
//
// NOTES
// 子ノードリストを並べ替える。
// 検索処理コスト順に並べ替える。
// ただし、Commutative(交換律が成り立つ)場合のみ可。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
void
ModInvertedOperatorAndNode::sortChildren(
	const ModInvertedQuery::ValidateMode mode)
{
	InternalNode::sortChildren(mode);
	if ((mode & Query::rankingMode) == 0 ||
		this->scoreCombiner->isCommutative() == ModTrue) {
		// ブーリアン検索か、ランキング検索で交換律が成立する場合ソート可
		ModSort(this->children.begin(), this->children.end(),
				ModInvertedQueryNode::lessSortFactor);
	}
}

//
// FUNCTION protected
// ModInvertedOperatorAndNode::reevaluate -- 正確な再評価
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
ModInvertedOperatorAndNode::reevaluate(ModInvertedDocumentID documentID)
{
	ModVector<ModInvertedQueryNode*>::Iterator child = this->children.begin();

	// 従来、lowerBound() を呼び出していた。しかし、子ノードに正規表現ノード
	// がある場合の処理を考慮すると reevaluate() がよい。(2000/09/14)
	while (child != this->children.end()) {
		if ((*child)->evaluate(documentID, QueryNode::defaultEMode)
			== ModFalse) {
			return ModFalse;
		}
		++child;
	}
	return ModTrue;
}

//
// FUNCTION protected
// ModInvertedOperatorAndNode::reevaluate -- 正確な再評価
//
// NOTES
// 粗い evaluate 満足を前提とした、正確な再評価
//
// 引数 locations には 
// ModInvertedUnorderedOperatorWindowLocationListIterator をセットする。
// min = 1。max = ModSizeMax。max = ModSizeMax とは無制限を意味する。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedLocationListIterator*& locations
//		位置情報（結果格納用）
// ModSize& uiTF_
//	   	(位置情報リストを取得できない場合) TF
// ModInvertedQueryNode* giveEndNode
//		ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedOperatorAndNode::reevaluate(ModInvertedDocumentID documentID,
									   LocationIterator*& locations,
									   ModSize& uiTF_,
									   ModInvertedQueryNode* giveEndNode)
{
	// TFの処理はModInvertedOrderedDistanceNode::reevaluateと同様
	
#ifdef	SIMPLE
	UnorderedSimpleLocationIterator* iterator =
		static_cast<UnorderedSimpleLocationIterator*>(getFreeList());
	if (iterator == 0)
	{
		iterator = new UnorderedSimpleLocationIterator(this);
		iterator->reserve(children.getSize());
	}
#else
	ModInvertedUnorderedOperatorWindowLocationListIterator* iterator =
		static_cast<ModInvertedUnorderedOperatorWindowLocationListIterator*>(getFreeList());
	if (iterator == 0)
	{
		iterator = 	new ModInvertedUnorderedOperatorWindowLocationListIterator(this);
		iterator->reserve(children.getSize());
	}
#endif
	LocationIterator::AutoPointer p = iterator;

	Query::EvaluateMode mode = 0x00;

	// 子ノードそれぞれについて正確に再 evaluate すると共に、
	// 両者の出現位置も得る。
	ModSize numberOfChildren = children.getSize();
	LocationIterator* childLocation = 0;
	ModSize uiTF = 0;
	ModSize uiMinTF = ModSizeMax;
	for (ModSize i = 0; i < numberOfChildren; ++i) {
		if (children[i]->evaluate(documentID, childLocation, uiTF,
								  QueryNode::defaultEMode, giveEndNode)
			!= ModTrue) {
			// 評価結果が False 今まで割り当てられたメモリを開放する
			return ModFalse;
		}

		if (childLocation != 0)
		{
			iterator->pushIterator(childLocation);
			childLocation = 0;
		}
		else
		{
			; ModAssert(uiTF > 0);
			if (uiTF < uiMinTF)
			{
				uiMinTF = uiTF;
			}
			uiTF = 0;
		}
	}

	ModSize uiIteratorCount = iterator->getSize();
	if (uiIteratorCount == numberOfChildren)
	{
		// 順序無視位置検査用反復子を有効化 (min=1, max=ModSizeMax)
		iterator->initialize(1, ModSizeMax);

		if (iterator->isEnd() == ModTrue) {
			// 位置条件に合わなかったのでiteratorを破棄してFalseでリターン
			return ModFalse;
		}
		
		// 位置条件に合ったのでlocationsをセットしてTrueでリターン
		locations = p.release();
	}
	else if (numberOfChildren > 0 && uiIteratorCount == 0)
	{
		uiTF_ = uiMinTF;
	}
	else
	{
		return ModFalse;
	}
	
	return ModTrue;
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
//              ドキュメントID
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedOperatorAndNode::doFirstStepInRetrieveScore(
	ModInvertedBooleanResult *expungedDocumentId,
	const ModInvertedDocumentID maxDocumentId)
{
	ModInvertedQueryInternalNode::doFirstStepInRetrieveScore(expungedDocumentId,maxDocumentId);
}


//
// FUNCTION
// ModInvertedOperatorAndNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
//
// NOTES
// ランキング検索で、スコア計算の第２ステップのみを実施する。
// 第１ステップが実施済みの場合のみ、QueryInternalNode::doSecondStepInRetrieveScore
// を呼ぶ。
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
ModInvertedOperatorAndNode::doSecondStepInRetrieveScore(ModInvertedSearchResult*& result_)
{
	ModInvertedQueryInternalNode::doSecondStepInRetrieveScore(result_);
}

//
// FUNCTION
// ModInvertedOperatorAndNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
//
// NOTES
// ランキング検索で、スコア計算の第２ステップのみを実施する。
// 第１ステップが実施済みの場合のみ、QueryInternalNode::doSecondStepInRetrieveScore
// を呼ぶ。
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
ModInvertedOperatorAndNode::doSecondStepInRetrieveScore()
{
	ModInvertedQueryInternalNode::doSecondStepInRetrieveScore();
}

//
// FUNCTION
// ModInvertedOperatorAndNode::lowerBoundScoreForSecondStep() -- ランキング検索の第２ステップで使用するlowerBound
//
// NOTES
// ランキング検索のスコア計算第２ステップで使用されるlowerBound。
// すべての子ノードがヒットする最小の文書IDを求め、それらのスコアを
// 合成して求める。
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
ModInvertedOperatorAndNode::lowerBoundScoreForSecondStep(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
	score = 0.0;
	DocumentScore childScore;
	ModVector<QueryNode*>::Iterator child = this->children.begin();
	ModVector<QueryNode*>::Iterator end = this->children.end();

	DocumentID lowerID(0);
	ModVector<DocumentScore>::Iterator i = scores.begin();
	while (child != end) {
		if((*child)->lowerBoundScoreForSecondStep(givenID, foundID, childScore)
		   == ModTrue) {
			// ヒットした子ノードの中で最小の文書IDを求める
			if(lowerID == 0) {
				lowerID = foundID;
				givenID = foundID;
			} else if(foundID == lowerID) {
			} else if(foundID > lowerID) {
				givenID = foundID;
				lowerID = 0;
				child = this->children.begin();
				i = scores.begin();
				continue;
			}
			*i = childScore;
			++i;
			++child;
		} else {
			return ModFalse;
		}
	}		

    if(lowerID == 0) {
		// １件もヒットしない場合はFALSE
		return ModFalse;
	}

	// 子ノードのスコアから自分のスコアを合成する
	score = this->scoreCombiner->apply(scores);
	// 最小の文書IDを返す
	foundID = lowerID;

	return ModTrue;
}

//
// FUNCTION public
// ModInvertedOperatorAndNode::checkQueryNode -- 子ノードの数をチェックする
//
// NOTES
// 有効化の最後に呼び出されて、子ノードの数をチェックする。もし異常で
// あれば例外を投げる。
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
// 下位からの例外をそのまま返す
//
void
ModInvertedOperatorAndNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_
	)
{ 
	ModSize s = this->children.getSize();
	if (s == 0) {
		// 子ノードがないのは異常
		ModErrorMessage << "children is empty." << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}
	
	for (ModSize i(0); i < s ; ++i) {
		this->children[i]->checkQueryNode(query_, setStringInChildren_, ModTrue);		
	}
}

//
// FUNCTION public
// ModInvertedOperatorAndNode::getSearchTermList --
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
void 
ModInvertedOperatorAndNode::getSearchTermList(
	ModInvertedQuery::SearchTermList& vecSearchTerm_,
	ModSize uiSynonymID_) const
{
	ModVector<ModInvertedQueryNode*>::ConstIterator i = children.begin();
	const ModVector<ModInvertedQueryNode*>::ConstIterator e = children.end();
	for (; i != e; ++i)
	{
		(*i)->getSearchTermList(vecSearchTerm_, uiSynonymID_);
	}
}

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
