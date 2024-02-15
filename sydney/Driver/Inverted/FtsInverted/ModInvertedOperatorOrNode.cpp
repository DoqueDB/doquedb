// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorOrNode.cpp -- 和集合ノードの実装
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

#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedFile.h"
#else
#include "ModInvertedFile.h"
#endif
#include "ModInvertedQuery.h"
#include "ModInvertedOperatorOrNode.h"
#include "ModInvertedTermLeafNode.h"
#include "ModInvertedException.h"
#include "ModInvertedOrLocationListIterator.h"
#include "ModInvertedCompressedLocationListIterator.h"
#include "ModInvertedAtomicOrNode.h"
#include "ModInvertedBooleanResultLeafNode.h"
#include "ModInvertedRankingResultLeafNode.h"
#include "ModInvertedRankingScoreCombiner.h"

#include "ModInvertedSearchResult.h"

#define ALL_SIMPLE

//
// FUNCTION public
// ModInvertedOperatorOrNode::ModInvertedOperatorOrNode -- 和集合ノードのコンストラクタ
//
// NOTES
// OR ノードを生成する。引数 shortWordLength は short word の長さを表
// わす。このORノードがshort word用のORノードだった場合は0以外、short
// wordでない場合は0が渡される。
//
// short word の長さとはユーザが検索語として入力した部分の長さのことで
// ある。例えば、“光”と入力されshort wordとして展開された結果が次の
// ようになった場合、shortWordLengthは 1 である。
//
// #or(光,光線,光源,…)
//
ModInvertedOperatorOrNode::ModInvertedOperatorOrNode(const ModUInt32 resultType_)
	:
	shortWordLength(0),
	ModInvertedQueryInternalNode(QueryNode::operatorOrNode,resultType_)
{}

//
// FUNCTION public
// ModInvertedOperatorOrNode::~ModInvertedOperatorOrNode -- ORノードのデストラクタ
//
// NOTES
// 和集合(OR)ノードを破棄する
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
ModInvertedOperatorOrNode::~ModInvertedOperatorOrNode()
{
	if (scoreCombiner != 0) {
		delete scoreCombiner;
		scoreCombiner = 0;
	}
}

#ifndef DEL_BOOL
// 
// FUNCTION public
// ModInvertedOperatorOrNode::retrieve -- 検索の一括実行
// 
// NOTES
// 検索式を一括実行する。
// 
// ARGUMENTS
// ModInvertedBooleanResult& queryResult
//		Boolean検索結果オブジェクト
// Query::EvaluateMode mode
//		評価モード
// 
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//		子ノードがない
// 
void
ModInvertedOperatorOrNode::retrieve(
	ModInvertedBooleanResult& queryResult, Query::EvaluateMode mode)
{

#ifdef	MAKE_OR_ROUGH_NODE	// orノードには今のところラフノードがないので
	if ((mode & Query::roughEvaluationFlag) != 0
		&& this->queryNodeForRoughEvaluation != 0) {
		try {
			queryNodeForRoughEvaluation->retrieve(queryResult, mode);
		} catch (ModException& exception) {
			ModErrorMessage << "fail retrieve of rough pointer"
							<< ModEndl;
			ModRethrow(exception);
#ifndef SYD_INVERTED
		} catch (...) {
			/* purecov:begin deadcode */
			ModUnexpectedThrow(ModModuleInvertedFile);
			/* purecov:end */
#endif
		}
		return;
	}
#endif	// MAKE_OR_ROUGH_NODE

	// 検索結果をまず空にする。
	if (!queryResult.isEmpty()) {
		queryResult.clear();
	}

	estimateDocumentFrequency();

	// retrieve 処理
	ModVector<ModInvertedQueryNode*>::Iterator child = this->children.begin();
	if ((mode & Query::orDocumentAtATimeFlag) == 0) {
		// term-at-a-time (default)
        ModInvertedBooleanResult tmpResult[2]; 

		int tmpResultIndex = 0;
		ModBoolean newDocuments(ModTrue);

		if (estimatedDocumentFrequency != UndefinedFrequency) {
			// 出現頻度と同じ分だけ予約しておく(メモリの再割り当て
			// が発生しないように)
			tmpResult[0].reserve(estimatedDocumentFrequency);
		}
		try {
			(*child)->
				retrieve(tmpResult[tmpResultIndex], mode);
		} catch (ModException& exception) {
			ModErrorMessage << "fail retrieve of the first child"
							<< ModEndl;
			ModRethrow(exception);
#ifndef SYD_INVERTED
		} catch (...) {
			/* purecov:begin deadcode */
			ModUnexpectedThrow(ModModuleInvertedFile);
			/* purecov:end */
#endif
		}
		++child;
		for (; child != this->children.end(); ++child) {
			if (newDocuments == ModTrue) {
				tmpResultIndex = 1 - tmpResultIndex;	// 0 <--> 1
				newDocuments = ModFalse;
			}
			// tmpResult[tmpResultIndex] を空にする。
			// 新しいModVectorでは erase の方が clear + reserve より高速
			tmpResult[tmpResultIndex].erase(tmpResult[tmpResultIndex].begin(),
											tmpResult[tmpResultIndex].end());

			// nextID は条件を満たす最小のID (初期値は最小値)
			ModInvertedDocumentID nextID = 1;
			// tmpResult[1 - tmpResultIndex] は、途中までの OR の結果
			ModInvertedBooleanResult::Iterator p;
			p = tmpResult[1 - tmpResultIndex].begin();

			if (p == tmpResult[1 - tmpResultIndex].end()) {
				//検索結果が空の場合単純にretrieveしてしまう
				try {
					(*child)->retrieve(tmpResult[tmpResultIndex], mode);
				} catch (ModException& exception) {
					ModErrorMessage << "fail retrieve of other child"
									<< ModEndl;
					ModRethrow(exception);
#ifndef SYD_INVERTED
				} catch (...) {
					/* purecov:begin deadcode */
					ModUnexpectedThrow(ModModuleInvertedFile);
					/* purecov:end */
#endif
				}
				newDocuments = ModTrue;
				continue;				//次のchildへ
			}
			// 前回までの検索結果の最小IDよりも小さいIDの処理
			while (1) {
				if ((*child)->lowerBound(nextID, nextID,
										 mode|Query::roughEvaluationFlag) == ModTrue) {
					if (nextID < *p) {
						if ((*child)->reevaluate(nextID) == ModTrue) {
							newDocuments = ModTrue;
							tmpResult[tmpResultIndex].pushBack(nextID++);
							continue;
						} else {
							++nextID;
							continue;
						}
					} 
					goto nextLoop;
				} else {				// lowerBound() が失敗
					goto finishCurrentChildren;
				}
				break;
			}
			while (p != tmpResult[1 - tmpResultIndex].end()) {
				if ((*child)->lowerBound(nextID, nextID,
										 mode|Query::roughEvaluationFlag)) {
				nextLoop:
					while (p != tmpResult[1 - tmpResultIndex].end() &&
						   *p < nextID) {
						//前回までの結果をpush
						tmpResult[tmpResultIndex].pushBack(*p++);
					}

					if (p != tmpResult[1 - tmpResultIndex].end()) {
						if (*p > nextID) {
							if ((*child)->reevaluate(nextID) == ModFalse) {
								++nextID;
								continue;
							}
							//lowerBoundの結果をpush
							tmpResult[tmpResultIndex].pushBack(nextID++);
							newDocuments = ModTrue;

							while ((*child)->lowerBound(nextID, nextID,
											mode|Query::roughEvaluationFlag)
								   && nextID < *p) {
								if ((*child)->reevaluate(nextID) == ModFalse) {
									++nextID;
									continue;
								}
								tmpResult[tmpResultIndex].pushBack(nextID++);
							}
						}
						nextID = *p + 1;
					} else {
						// tmpResult[1-tmpResultIndex]が空になった場合
						if ((*child)->reevaluate(nextID) == ModTrue) {
							tmpResult[tmpResultIndex].pushBack(nextID++);
						} else {
							nextID++;
						}
						newDocuments = ModTrue;
						// カレントのchildについて最後まで検索する
						while ((*child)->lowerBound(nextID, nextID, mode)
							   == ModTrue) {
							tmpResult[tmpResultIndex].pushBack(nextID++);
						}
						break;
					}
				} else {
					// カレントのchildについては検索完了
				finishCurrentChildren:
					if (newDocuments == ModFalse) {
						break;
					}
					while (p != tmpResult[1 - tmpResultIndex].end()) {
						tmpResult[tmpResultIndex].pushBack(*p++);
					}
					break;
				}
			}
		}
		// 最終結果の代入
		if (newDocuments == ModFalse) {
			tmpResultIndex = 1 - tmpResultIndex;	// 0 <--> 1
		}
		// queryResult に割り当てられたメモリを最小限にするため 
		// reserve(getSize()) を実行
		queryResult.reserve(tmpResult[tmpResultIndex].getSize());
		queryResult = tmpResult[tmpResultIndex];
	} else {
		// document-at-atime
		// 結果の領域確保
		if (estimatedDocumentFrequency != UndefinedFrequency) {
			queryResult.reserve(estimatedDocumentFrequency);
		}
		// DocumentID (初期値は最小値)
		ModInvertedDocumentID currentID = 1;
		while (this->lowerBound(currentID, currentID, mode) == ModTrue) {
			queryResult.pushBack(currentID);
			currentID++;
		}
		// queryResult に割り当てられたメモリを最小限にするため 
		// reserve(getSize()) を実行
		queryResult.reserve(queryResult.getSize());
	}
	// retrieve 終了。estimatedDocumentFrequency に正確な文書頻度を設定する
	this->estimatedDocumentFrequency = queryResult.getSize();

	if ((mode & Query::roughEvaluationFlag) == 0) {
		// ラフモードでない場合
		retrieved = ModTrue;
	}
}
#endif

// 
// FUNCTION public
// ModInvertedOperatorOrNode::evaluate -- 与えられた文書が検索式を満たすかどうかの検査
// 
// NOTES
// 与えられた文書が検索式を満たすかどうか検査する。条件を満たした場合
// は検索履歴として lower/upper に文書 ID をセットする。ただしラフモー
// ドのときは、この lower/up をセットしないが、この or ノードがショー
// トワードの場合（子ノードが全て simpleTokenNode なのでラフモードの 
// on/off によって検索結果に変化がない。）は例外的に lower/upper をセッ
// トする。
// 
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// Query::EvaluateMode mode
//		評価モード
// 
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedOperatorOrNode::evaluate(DocumentID documentID,
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

#ifdef	MAKE_OR_ROUGH_NODE
	if ((mode & Query::roughEvaluationFlag) != 0
		&& this->queryNodeForRoughEvaluation != 0) {
		return this->queryNodeForRoughEvaluation->evaluate(documentID, mode);
	}
#endif	// MAKE_OR_ROUGH_NODE
	// 子ノードの検査
	ModBoolean returnValue = ModFalse;
	// 常に document-at-a-time
	ModVector<ModInvertedQueryNode*>::Iterator child(this->children.begin());
	ModVector<ModInvertedQueryNode*>::Iterator end(this->children.end());
	for (; child != end; ++child) {
		// 各ノードの evaluate() を改良したので明示的にラフモードで検
		// 索する必要はない
		if ((*child)->evaluate(documentID, mode) == ModTrue) {
			// evaluate成功
			if ((mode & Query::roughEvaluationFlag) == 0
				|| (this->shortWordLength != 0)) {
				// ラフモードでないか、ショートワードの時は 
				// lower/upper を設定する
				this->upper = this->lower = documentID;
			}
			return ModTrue;
		}
	}
	return ModFalse;
}

// 
// FUNCTION public
// ModInvertedOperatorOrNode::lowerBound -- 検索式を満たす文書のうち、文書IDが与えられた値以上で最小の文書の検索
// 
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する。
//
// 検索結果は検索履歴として lower/upper にセットしておく。ただしラフモー
// ドのときは、この lower/up をセットしないが、この or ノードがショー
// トワードの場合（子ノードが全て simpleTokenNode なのでラフモードの 
// on/off によって検索結果に変化がない。）は例外的に lower/upper をセッ
// トする。
// 
// ARGUMENTS
// ModInvertedDocumentID givenDocumentID
//		文書ID
// ModInvertedDocumentID& foundDocumentID
//		結果格納用の文書IDオブジェクト
// Query::EvaluateMode mode
//		評価モード
// 
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedOperatorOrNode::lowerBound(ModInvertedDocumentID givenDocumentID,
									  ModInvertedDocumentID& foundDocumentID,
									  Query::EvaluateMode mode)
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
#ifdef	MAKE_OR_ROUGH_NODE
	if ((mode & Query::roughEvaluationFlag) != 0
		&& this->queryNodeForRoughEvaluation != 0) {
		return this->queryNodeForRoughEvaluation->lowerBound(givenDocumentID,
															 foundDocumentID,
															 mode);
	}
#endif	// MAKE_OR_ROUGH_NODE

	// 子ノードの lowerBound 呼び出し
	// 常に document-at-a-time
	ModVector<ModInvertedQueryNode*>::Iterator child(this->children.begin());
	ModVector<ModInvertedQueryNode*>::Iterator end(this->children.end());
	ModInvertedDocumentID currentID(givenDocumentID);
	ModInvertedDocumentID smallestID(ModInvertedUpperBoundDocumentID);

	for (; child != end; ++child) {
		if ((*child)->lowerBound(givenDocumentID, currentID, mode)
			== ModTrue) {
			if (givenDocumentID == currentID) {
				smallestID = currentID;
				break;
			} else if (smallestID > currentID) {
				smallestID = currentID;
			}
		}
	}

	if (smallestID == ModInvertedUpperBoundDocumentID) {
		// 条件を満足する文書はなかった
		this->lower = givenDocumentID;
		this->upper = ModSizeMax;
		return ModFalse;
	}

	if ((shortWordLength != 0) || (mode & Query::roughEvaluationFlag) == 0) {
		// ラフモードでないか、ショートワードの時は lower/upper を設定する
		this->upper = smallestID;
		this->lower = givenDocumentID;
	}
	foundDocumentID = smallestID;
	return ModTrue;
}

// 
// FUNCTION public
// ModInvertedOperatorOrNode::evaluateScore -- 与えられた文書のスコアを計算する
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
// 下位からの例外をそのまま返す
// 
ModBoolean
ModInvertedOperatorOrNode::evaluateScore(const DocumentID ID,
										 DocumentScore& score,
										 Query::EvaluateMode mode)
{
	score = 0.0;						// 0 クリアしておく
	// ModInvertedOperatorOrNode::evaluateScore()に rough
	// modeは実装していない

	// 既に調査済み？
	if (ID >= this->lower) {
		if (ID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		}
	}

	// 子ノードのスコア計算
	// 常に document-at-a-time
	DocumentScore childScore;


	// 子ノードどれかのスコアが０以外
	ModBoolean childScoreExist = ModFalse;

	ModVector<ModInvertedQueryNode*>::Iterator child(this->children.begin());
	ModVector<ModInvertedQueryNode*>::Iterator end(this->children.end());

	ModVector<DocumentScore>::Iterator i = scores.begin();
	for (; child != end; ++child, ++i) {
		if ((*child)->evaluateScore(ID, childScore, mode) == ModFalse) {
			childScore = 0.0L;
		}
		else if (childScoreExist == ModFalse) {
			childScoreExist = ModTrue;
		}

		*i = childScore;
	}

	// スコアを計算する。
	if (childScoreExist == ModFalse) {
		return ModFalse;
	} else {
		// 子ノードのスコアから自分のスコアを合成する
		score = this->scoreCombiner->apply(scores);
	}
	this->upper = this->lower = ID;
	return ModTrue;

}

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
// 
// FUNCTION public
// ModInvertedOperatorOrNode::evaluateScore -- 与えられた文書のスコアを計算する（位置も計算する）
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
// ModInvertedQueryNode* givenEndNode 
// 		ここでは未使用。orderedDistanceでのみ使用。
// 
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
ModBoolean
ModInvertedOperatorOrNode::evaluateScore(const DocumentID ID,
										 DocumentScore& score,
										 LocationIterator*& locations,
										 Query::EvaluateMode mode,
										 ModInvertedQueryNode* givenEndNode)
{
	// 既に調査済み？
	if (ID >= this->lower) {
		if (ID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		}
	}

	OrLocationIterator* tmpLocation
		= static_cast<OrLocationIterator*>(getFreeList());
	if (tmpLocation == 0)
	{
		tmpLocation = new OrLocationIterator(this);
		tmpLocation->reserve(children.getSize());
	}
	LocationIterator::AutoPointer p = tmpLocation;

	// 子ノードどれかのスコアが０以外
	ModBoolean childScoreExist = ModFalse;

	score = 0.0;						// 0 クリアしておく

	// ModInvertedOperatorOrNode::evaluateScore()に rough
	// modeは実装していない

	ModSize numberOfChildren = children.getSize();

	// 子ノードそれぞれについて正確に再 evaluate すると共に、両者
	// の出現位置も得る。
	LocationIterator* loc = 0;

	for (ModSize i = 0; i < numberOfChildren; ++i) {
		if (children[i]->evaluateScore(ID, score, loc, mode) == ModTrue) {
			if (this->shortWordLength > 0
				&& this->shortWordLength != ModSizeMax) {
				// short word の長さを LocationListIterator にセットする
				// short word の場合、ユーザが与えた長さを使う
				(static_cast<ModInvertedCompressedLocationListIterator*>(
					loc))->setLength(this->shortWordLength);
#ifdef DEBUG
				ModDebugMessage << "shortWordLength " << shortWordLength
								<< " i " << i << " docID "
								<< ID << ModEndl;
#endif
			}
			if (childScoreExist == ModFalse) {
				childScoreExist = ModTrue;
			}
			// 位置情報が得られたら tmpLocation へ push
			tmpLocation->pushBack(loc);
		}
		else {
			score = 0.0L;
		}
		scores[i] = score;
	}

	if (childScoreExist == ModFalse) {
		return ModFalse;			// ひとつも条件に合うものがなかった
	}

	// OR 用の位置情報反復子を生成
	tmpLocation->initalize();

	// Endになるのは異常なケース
	ModAssert(tmpLocation->isEnd() == ModFalse);

	// 位置条件に合ったのでlocationsをセットしてTrueでリターン
	locations = p.release();

	// 子ノードのスコアから自分のスコアを合成する
	score = this->scoreCombiner->apply(scores);

	this->upper = this->lower = ID;
	return ModTrue;
}
#endif

// 
// FUNCTION public
// ModInvertedOperatorOrNode::lowerBoundScore -- 検索式を満たす文書のうち、文書IDが与えられた値以上で最小の文書の検索しスコアを計算する
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
ModInvertedOperatorOrNode::lowerBoundScore(const DocumentID givenID,
										   DocumentID& foundID,
										   DocumentScore& score,
										   Query::EvaluateMode mode)
{
	if (givenID >= this->lower) {
		if (givenID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		} else if (givenID <= this->upper) {
			// 既にヒットする文章がわかっている

			// evaluateScoreによりlowerが書き換えられるのでlowerを保存
			DocumentID tmpLower = this->lower;
			if (this->evaluateScore(this->upper, score, mode) == ModTrue) {
				foundID = this->upper;
				this->lower = tmpLower;
				return ModTrue;
			}
		}
	}
	
	DocumentID tmpID = givenID;
	while (1) {
		if (lowerBound(tmpID, foundID,
					   mode|Query::roughEvaluationFlag) != ModTrue) {
			// マッチする文書はない
			this->upper = ModSizeMax;
			this->lower = givenID;
			return ModFalse;
		}
		if (this->evaluateScore(foundID, score, mode) == ModTrue) {
			break;
		}
		tmpID = foundID + 1;		// lowerBound をやりなおし
	}
	this->upper = foundID;
	this->lower = givenID;

	return ModTrue;
}

//
// FUNCTION public
// ModInvertedOperatorOrNode::sharedQueryNode -- 中間ノードの共有化
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
// また、ブーリアン検索の場合は、カレントの中間ノードの子ノードに同じ key
// のノードがないかをlocalNodeMap 変数を使って調査する。もし同じ key のノード
// が発見できたらそのノードを delete し、children からも削除し、その子ノード
// を完全に消去する。このようにchildren が削除され child の数が 1 になった
// 場合さらにそのノードを単純化できるので、 changeSimpleNodeType() 関
// 数を呼び出す。
// ランキング検索の場合はlocalNodeMapを用いchildを削除するとスコアが変わってし
// まうため、カレントの中間ノードの検査は行わない。
//
// sharedQueryNode() は再帰的に呼び出す形式をなっていて、globalNodeMap 
// には下位のノードから順番にデーターを挿入していくことになる。
//
// この関数は OperatorAndNodeでオーバーライド(OpearorAndNodeのsharedQeuryNodeを
// OperatorAndNot, OrderedDistanceNode や WindowNode でさらにオーバ
// ライドされる。
//
// ARGUMENTS
// QueryNodeMap& globalNodeMap
//		全ノードの QueryNodeMap
// QueryNodePointerMap& nodePointerMap
//		OR標準形変換により共有されているノードが登録されているMap
//
// RETURN
// children の数。
// ただし子ノードがすべて削除された場合はEmptySetNodeになるため0を返す。
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize
ModInvertedOperatorOrNode::sharedQueryNode(
	QueryNodeMap& globalNodeMap,
	QueryNodePointerMap& nodePointerMap)
{
	QueryNodeMap::Iterator p;
	ModVector<ModInvertedQueryNode*>::Iterator child;
	ModInvertedQueryNode* tmpEndNode = 0;

	ModSize retGrandChildNum;		// 子ノードのsharedQueryNodeの戻値
	
	QueryNodeMap localNodeMap;		// この internal node 内のマップ

	for (child = children.begin(); child != children.end(); ++child) {
		if (ModInvertedIsInternalNode((*child)->getType()) != ModTrue) {
			// リーフノードの場合なにもしない
			continue;					// 次の child へ
		}
		// internal node の場合

		// 子ノードに対して sharedQueryNode() を呼ぶ
		retGrandChildNum = (static_cast<InternalNode*>(*child))
			-> sharedQueryNode(globalNodeMap, nodePointerMap);

		if (retGrandChildNum == 1) {

			// ノードの共有化をした結果 children が 1つしかなくなった
			// 場合ノードを単純化する
			changeSimpleTypeNode(child, nodePointerMap);

			if (*child == QueryNode::emptySetNode) {

				// 通常のノードは子ノードにemptySetNodeが含まれる場合
				// 自分自身が空集合ノードになるが、Orの場合は他に子ノードが
				// ある場合空集合ノードにはならないので、とりあえず空集合ノード
				// になった子ノードをchildrenからeraseする
				// 最後に子ノードの数を調べその結果を返す

				// 通常の中間ノードではchildがemptySetNodeでもchildrenから
				// 削除しないでそのまま残しておき、最後に子ノードを順番に
				// 見ていきemptySetNodeがあった場合には子ノードの数ではなく
				// 自分自身を空集合ノードにするため0を返す


				// changeSimpleTypeNode()の実行により child のノードが
				// 削除されてしまったので child も Vector から erase する
				child = children.erase(child);
				// eraseしたことによって child の内容が変化するので、
				// child を1つ戻して loop の頭からやり直す
				--child;
				continue;				// 次の子ノードへ
			}
		} else if (retGrandChildNum == 0) {
			// 子ノードのsharedQueryNodeの戻り値が0
			// この子ノードはEmptySetNode
			addQueryNodePointerMap(*child, nodePointerMap);
			child = children.erase(child);
			// eraseしたことによって child の内容が変化するので、
			// child を1つ戻して loop の頭からやり直す
			child--;
			continue;				// 次の子ノードへ
		}

		// childがまだInternalNodeであるか検査する
		if (ModInvertedIsInternalNode((*child)->getType()) != ModTrue) {
			// リーフノードの場合はなにもせず次の子ノードに行く
			continue;
		}

		// 子ノードの QueryString を取得する
		ModUnicodeString key;
		(*child)->getQueryString(key);

		QueryNode* value = *child;      // QueryNode ポインタをセット

		if (getScoreCombiner() == 0) {
			// getScoreCombiner()が0かどうかのみチェックしていたが、
			// Atomicな中間ノードの場合はOKなのか？
			//
			// Atomicな中間ノードの場合自分自身の出現頻度をもとにスコアを
			// 計算しているので子ノードの中の同じノードは削除してもOKである
			//
			// 自分自身の出現頻度には影響はない
			//

			// ブーリアンの場合、Atomicの中間ノードの場合はlocalNodeMapの
			// 調査も行う。 ランキング検索の場合(Atomicな中間ノードを除く)は
			// localNodeMapを用いて子ノードの削除を行うと
			// スコアが変わってしまうため行わない。

			// localNodeMap に同じkeyがあるか調査する
			p = localNodeMap.find(key);

			if (p != localNodeMap.end()) {
#ifdef DEBUG
				ModDebugMessage << "delete node local: " << key << ModEndl;
#endif
				// 既に同じQueryStringのノードがあるこの子ノードを削除する
				addQueryNodePointerMap(*child, nodePointerMap);

				child = children.erase(child);      // childrenからの削除する
				--child;

				continue;                   // 次の child へ

			} else {
				// 同じkeyはないのでlocalNodeMap へ挿入する
				localNodeMap[key] = value;
			}
		}

		// globalNodeMapに同じQueryStringのノードがあるか調査する
		p = globalNodeMap.find(key);
		if (p != globalNodeMap.end()) {
#ifdef DEBUG
			ModDebugMessage << "delete node global: " << key << ModEndl;
#endif
			if (*child == (*p).second) {
				// すでに共有しているので特に処理しない
#ifdef DEBUG
				ModDebugMessage << "Already shared" << ModEndl;
#endif
			} else {
				// 既に同じQueryStringのノードがある childのnodeを破棄する

				// OrderedDistanceの共有の場合はendNodeを考慮する必要がある
				if (ModInvertedAtomicMask((*child)->getType())
						== ModInvertedQueryNode::orderedDistanceNode) {
					// tmpEndNodeが0以外なら、
					// 削除される側のOrderedDistanceがendNodeを持っている
					tmpEndNode = (*child)->getEndNode();

					// すでに共有されたorderdDistanceNodeがある場合、
					// そのにoriginalStringがセットされていない場合があるので
					// 削除されるノードのoriginalStringをセットする
					ModUnicodeString tmpOriginalString;
#ifdef V1_6
					ModLanguageSet tmpLangSet;
#endif // V1_6
					ModInvertedTermMatchMode tmpMmode;
					((*p).second)->getOriginalString(tmpOriginalString,
#ifdef V1_6
													 tmpLangSet,
#endif // V1_6
													 tmpMmode);
					if(tmpOriginalString.getLength() == 0) {
						(*child)->getOriginalString(tmpOriginalString,
#ifdef V1_6
													tmpLangSet,
#endif // V1_6
													tmpMmode);
						((*p).second)->setOriginalString(tmpOriginalString,
#ifdef V1_6
														 tmpLangSet,
#endif // V1_6
														 tmpMmode);
					}
#ifdef DEBUG
					ModDebugMessage << "share OrderedDistanceNode "
									<< '{' << key << '}' << ModEndl;
#endif // DEBUG
				}

				addQueryNodePointerMap(*child, nodePointerMap);

				// childは先にglobalNodeMapにあったnodeへのポインタをセットする
				*child = (*p).second;

				// OrderedDistance かつ endNodeを持っていた
				if (tmpEndNode != 0) {
					(*child)->setEndNode(tmpEndNode);
					// クリア
					tmpEndNode = 0;
				}
			}
		} else {
			// globalNodeMap へ挿入する
			globalNodeMap[key] = value;
		}
	}

	return children.getSize();
}

// 
// FUNCTION public
// ModInvertedOperatorOrNode::estimateDocumentFrequency -- 文書頻度の見積もり
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
ModInvertedOperatorOrNode::estimateDocumentFrequency()
{
	if (this->estimatedDocumentFrequency == QueryNode::UndefinedFrequency
		&& this->children.getSize() > 0) {
		// 上目に見積もるために、和を用いる。
		estimatedDocumentFrequency = 0;
		for (ModVector<ModInvertedQueryNode*>::Iterator child
			 = this->children.begin();
			 child != this->children.end();
			 ++child) {
			estimatedDocumentFrequency += (*child)->estimateDocumentFrequency();

			if (estimatedDocumentFrequency > totalDocumentFrequency) {
				estimatedDocumentFrequency = totalDocumentFrequency;
				break;
			}
		}
	}
	return this->estimatedDocumentFrequency;
}

//
//
// FUNCTION public
// ModInvertedOperatorOrNode::flattenChildren -- 子ノードリストの平坦化
// 
// NOTES
// 子ノードリストの平坦化 (例 #or(#or(X,Y),Z) → #or(X,Y,Z))
// 
// ただしRankingの場合は、scoreCombinerの種類が一致しかつ、
// Associative(結合律がなりたつ)場合のみ可
// 
// orFlattenThreshold より子ノード数が多い ORノードは平坦化しない。
//
// またショートワード用に生成された ORノードで、windowノードを親ノード
// にもっている場合は平坦化しない。ショートワード用の ORノードかどうか
// は shortWordLength が 0 以外であることで判断する。windowノードを親
// にもっているかは引数 isChildOfWindowNode で判断する。
//
// 平坦化後不要になったORノードを削除する際に sharedNodeMap を参照し、
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
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModInvertedOperatorOrNode::flattenChildren(
	const QueryNodePointerMap& sharedNodeMap,
	const ModBoolean isChildOfWindowNode)
{
	ModVector<ModInvertedQueryNode*>::Iterator child = children.begin();

	// すべての子供が SimpleTokenLeafNode になるかを調べる
	ModBoolean allSimple;
	if (isChildOfWindowNode == ModTrue) {
		// Window の子ノードの場合、すべての子供が Simple でも shorWordLength
		// を書き換えてはいけないので、書き換えが起こらないように false にする
		allSimple = ModFalse;
	} else {
		allSimple = ModTrue;
		while (child != children.end()) {
			switch ((*child)->getType()) {
			case simpleTokenLeafNode:
				break;
			case operatorOrNode:
				if (((OrNode*)*child)->getShortWordLength() == 0) {
					allSimple = ModFalse;
					goto fini;
				}
				break;
			default:
				allSimple = ModFalse;
				goto fini;
				break;
			}
			++child;
		}
	fini:;
	}

	ModUnicodeString combinerName;
	ModUnicodeString childCombinerName;

	child = children.begin();
	// パラメータまで比較する
	this->getCalculatorOrCombinerName(combinerName, ModTrue);

	while (child != children.end()) {
		if((*child)->getType()
				!= ModInvertedQueryNode::operatorOrNode) {
			// OR ノード以外の場合(atomicOrは不可)
			(*child)->flattenChildren(sharedNodeMap, isChildOfWindowNode);
			++child;
			continue;
		}

		//
		// OR ノードの場合
		//
		ModVector<ModInvertedQueryNode*>* grandChildren
					= (*child)->getChildren();

		// ブーリアンのときは常に平坦化可
		if (combinerName.getLength() != 0) {
			(*child)->getCalculatorOrCombinerName(childCombinerName, ModTrue);

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

		if (isChildOfWindowNode == ModTrue) {
			// 親ノードにwindowノードがある
			if (this->shortWordLength
				!= (static_cast<OrNode*>(*child))->getShortWordLength()) {
				// 自分のshortWordLengthと子ノードのshortWordLengthが不一致
				// なので平坦化しない
				++child;
				continue;
			}
		}
		if (ModInvertedQuery::orFlattenThreshold < grandChildren->getSize()) {
			//子ノードの数が多いので平坦化しない
			++child;
			continue;
		}
		//
		// 平坦化処理
		//
		ModVector<ModInvertedQueryNode*>::Iterator
			grandChild(grandChildren->begin());
		ModInvertedQueryNode* reducedChild = *child;

		// OR のあった位置に孫の1番目のノードをセット
		*child++ = *grandChild++;

		// childのindexを計算
		ModSize n = child - children.begin();

		// 孫の2番目以降のノードをセット
		children.insert(child, grandChild, grandChildren->end());

		// child を孫を挿入した位置まで戻す
		child = (children.begin() + n - 1);

		if (sharedNodeMap.find(reducedChild) == sharedNodeMap.end()) {
			// sharedNodeMapには登録されていない
			delete reducedChild;		// OR ノードを削除
		}
	}

	if (shortWordLength == 0 && allSimple == ModTrue) {
		shortWordLength = ModSizeMax;
	}
}

//
// FUNCTION public
// ModInvertedOperatorOrNode::getOriginalString() -- 検索語文字列を返す
//
// NOTES
// 自分が生成された元のTermLeafNodeの文字列を返す。元の文字列はメンバ変数
// originalTermStringにeraseTermLeafNodeQuery()でセットされる。
//
// originalTermStringがセットされていない場合はFalseを返す。
//
// ARGUMENTS
// ModUnicodeString& termString
//		結果格納用
//
// RETURN
// originalTermStringがある場合 ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
ModBoolean
ModInvertedOperatorOrNode::getOriginalString(ModUnicodeString& termString,
#ifdef V1_6
											 ModLanguageSet& langSet_,
#endif // V1_6
											 ModInvertedTermMatchMode& mmode_) const
{
	if (originalTermString.getLength() != 0) {
		// originalTermStringがセットしてある
		// TemrLeafNodeから生成されたOrである
		termString = originalTermString;
#ifdef V1_6
		langSet_ = langSet;
#endif // V1_6
		mmode_ = mmode;
		return ModTrue;
	}
	// originalTermStringがセットされていないのでFalseを返す
	// termLeafNodeから生成されたAtomicOrではない
	return ModFalse;
}

//
// FUNCTION public
// ModInvertedOperatorOrNode::setOriginalString() -- 自分が生成された元のTermLeafNodeの文字列をメンバ変数originalTermStringにセットする。
//
// NOTES
// 自分が生成された元のTermLeafNodeの文字列をメンバ変数originalTermStringに
// セットする。QueryBaseNode::setOriginalString()をオーバーライド。
//
// ARGUMENTS
// ModUnicodeString termString
//		セットする文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedOperatorOrNode::setOriginalString(const ModUnicodeString& termString,
#ifdef V1_6
											 const ModLanguageSet& langSet_,
#endif // V1_6
											 const ModInvertedTermMatchMode& mmode_)
{
	originalTermString = termString;
#ifdef V1_6
	langSet = langSet_;
#endif // V1_6
	mmode = mmode_;
}

//
// FUNCTION protected
// ModInvertedRankingOperatorOrNode::sortChildren -- 子ノードリストの並べ替え
//
// NOTES
// 子ノードリストを並べ替える。
// 検索処理コスト順に並べ替える。
// ただし、Commutative(交換律が成り立つ)場合のみ可。
//
// ARGUMENTS
// const ModInvertedQuery::ValidateMode mode
// 	ランキング・ブーリアン検索の判定に用いる
//
// RETURN
// なし
//
void
ModInvertedOperatorOrNode::sortChildren(
	const ModInvertedQuery::ValidateMode mode)
{
	InternalNode::sortChildren(mode);
	if ((mode & Query::rankingMode) == 0 ||
		this->scoreCombiner->isCommutative() == ModTrue) {
		// ブーリアン検索か、ランキング検索で交換律が成立する場合ソート可
		ModSort(this->children.begin(), this->children.end(),
				lessSortFactor);
	}
}

// 
// FUNCTION public
// ModInvertedQueryNode::lessSortFactor -- sortChildren()用比較関数
// 
// NOTES
// sortChildren()関数で sortFactor をキーにして降順にソートする場合の
// 比較関数
// 
// ARGUMENTS
// QueryNode* x
//		比較対象のノード
// QueryNode* y
//		比較対象のノード
// 
// RETURN
//	x が y より小さいとき ModTrue、それ以外は ModFalse
// 
// EXCEPTIONS
// なし
// 
/*static*/ ModBoolean
ModInvertedOperatorOrNode::lessSortFactor(QueryNode* x, QueryNode* y)
{
	// regex （を含む）ノードを後ろに回す
	if (y->calcSortFactor() == ModInvertedQueryNode::MaxSortFactor) {
		return ModTrue;
	}
	else if (x->calcSortFactor() == ModInvertedQueryNode::MaxSortFactor) {
		return ModFalse;
	}

	// そうでなければ、文書頻度の大きい順にソートする
	return ModInvertedQueryNode::moreFrequent(x, y);
}

//
// FUNCTION public
// ModInvertedOperatorOrNode::prefixString -- 演算子を表わす文字列を返す
//
// NOTES
// QueryNodeで定義された内容をオーバライドする
// 演算子を表わす文字列(#or)を返す
//
// ARGUMENTS
// ModString& prefix
//	演算子を表わす文字列を返す(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void 
ModInvertedOperatorOrNode::prefixString(ModUnicodeString& prefix,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam) const
{
	prefix += "#or";

	if (withCalOrCombName == ModTrue) {
		ModUnicodeString combinerName;
		getCalculatorOrCombinerName(combinerName, withCalOrCombParam);

		if (combinerName.getLength() > 0) {
			prefix += '[';
			prefix += combinerName;
			prefix += ']';
		}
	}
}

//
// FUNCTION public
// ModInvertedRankingOperatorOrNode::reserveScores -- scoresをリザーブ
//
// NOTES
// 子ノードを辿りrankingOr,rankingAndの場合はメンバ変数であるscoresを
// リザーブする。本関数がコールされるのはrankingOrの場合であるため、
// scoresをリザーブしさらに自分の子ノードをたどる。
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
ModInvertedOperatorOrNode::reserveScores()
{
	ModSize size(children.getSize());

	// scoresを子ノードの数だけリザーブする
	scores.reserve(size);
	scores.insert(scores.begin(), size, 0.0);

	ModInvertedQueryInternalNode::reserveScores();
}

//
// FUNCTION public
// ModInvertedOpeartorOrNode::duplicate -- 自分のコピーを作成する
//
// NOTES
// 自分のコピーを作成する。Query::から呼ばれ、検索木を上からたどり、
// 検索木のコピーを作成する際に使用される。
// QueryNode::dupulicateをオーバーライド。OperatorOrNodeのコピーを作る。
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
ModInvertedOperatorOrNode::duplicate(const ModInvertedQuery& rQuery)
{
	ModInvertedOperatorOrNode* node = new ModInvertedOperatorOrNode(firstStepResult->getType());

	// 自分にスコア計算器がセットされている場合のみ
	// 新しいノードに計算器をセットする
	if (this->scoreCombiner != 0) {
		node->setScoreCombiner((this->scoreCombiner)->duplicate());
	}

	// retrieveScore において評価モードが retrieveTF モードの場合、
	// 全文書数が必要となる
	// 自分が持っていたものをセットする
	node->setTotalDocumentFrequency(totalDocumentFrequency);

	ModVector<ModInvertedQueryNode*>::Iterator child(this->children.begin());
	ModVector<ModInvertedQueryNode*>::Iterator end(this->children.end());

	while (child != end) {
		// 子ノードのduplicateを追加
		node->insertChild((*child)->duplicate(rQuery));
		++child;
	}

	return node;
}

//
// FUNCTION public
// ModInvertedOperatorOrNode::validate -- 正規表現ノードの有効化
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
ModInvertedOperatorOrNode::validate(
	InvertedFile* invertedFile,
	const Query::ValidateMode mode,
	ModInvertedQuery* rQuery)
{
	// ランキング検索の場合はスコア合成器をセットする
	if ((mode & Query::rankingMode) != 0) {
		if (this->scoreCombiner == 0) {
			// QueryNodeには必ずデフォルトの計算器をセットするように
			// なったので、ここではduplicateだけ
			ScoreCombiner* combiner = rQuery->getDefaultOrScoreCombiner();
			;ModAssert(combiner != 0);
			setScoreCombiner(combiner->duplicate());
		}
	}

	// totalDocumentFrequencyのセット
	setTotalDocumentFrequency(rQuery->getTotalDocumentFrequency());

	// 子ノードの有効化
	ModInvertedQueryInternalNode::validate(invertedFile,mode,rQuery);
}

//
// FUNCTION public
// ModInvertedOperatorOrNode::makeRoughPointer -- queryNodeForRoughEvaluation の作成
//
// NOTES
// 自分にはラフを設定せず、子ノードに対し再帰的に makeRoughPointer を呼び出す。
//
// ARGUMENTS
// const validateMode mode
//		有効化モード
// ModVector<ModInvertedQueryNode*>& parentRoughPointer
// 		親ノード用のroughPointerのchildrenになるVector (結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedOperatorOrNode::makeRoughPointer(const Query::ValidateMode mode,
											 QueryNodePointerMap& parentMap,
											 const ModInvertedQuery* Query) 
{
	ModVector<ModInvertedQueryNode*>::Iterator p = this->children.begin();
	for (; p != this->children.end(); ++p) {
		// 再帰的に呼び出す
		QueryNodePointerMap dummy;
		(*p)->makeRoughPointer(mode, dummy, Query);
	}
}

// 
// FUNCTION public
// ModInvertedOperatorOrNode::eraseTermLeafNode -- TermLeafNode消去
// 
// NOTES
// TermLeafNode を消去して SimpleToken/OrderedDistance にする
// QueryInternaNode の eraseTermLeafNode をオーバライドしている
//
// InternalNodeと異なる点
//  - 子ノードに空集合ノードがあった場合はchildrenからeraseする
//
//	- returnの条件が異なる
//		最終的に子ノードの数がeraseにより0になった場合はModFalse
//		を返す(自分自身を空集合にするため)。それ以外の場合はModTrueを返す
//		(呼び出し側で後処理が不要)
//
// - nodeを使用
//		子ノードが一つしかなくなった場合は、このーどを昇格させるため
//		nodeに残ったこのーどをセットしてかえす
//
// ARGUMENTS
// QueryNode*& node
// 		昇格させる子ノードのポインタをセットしてかえす。
//		(子ノードが一つしかなくなった場合)
//
// Query& query
//		Query
// 
// RETURN
// この関数の呼び出し側で後処理が必要な場合 ModFalse を返す、特に必要
// ない場合 ModTrue を返す
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedOperatorOrNode::eraseTermLeafNode(QueryNode*& node, Query& query)
{
	ModVector<ModInvertedQueryNode*>::Iterator child(children.begin());
	ModInvertedQueryNode::NodeType childType;

	while (child != children.end()) {
		// children の最初から最後まで
		childType = ModInvertedAtomicMask((*child)->getType());
		if (childType == ModInvertedQueryNode::termLeafNode) {
			// child が TermLeafNode の場合 - static_cast は使えない
			TermLeafNode* termNode = static_cast<TermLeafNode*>(*child);

			if (termNode->getQueryNodeForPreciseEvaluation() == emptySetNode) {
				// 空集合ノード発見
				child = this->children.erase(child);
				delete termNode;		// TermLeafNode破棄
				continue;				// 次の子ノードへ
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
			query.addOrStanderdSharedNode(*child);
			if ((static_cast<ModInvertedBooleanResultLeafNode*>(*child))
				->isEmptyResultLeafNode() == ModTrue) {
				// 空集合の場合
				child = children.erase(child);
				continue;
			}

		} else if (childType == ModInvertedQueryNode::rankingResultLeafNode) {
			// RankingResultLeafNode の場合
			query.addOrStanderdSharedNode(*child);

			if (static_cast<ModInvertedRankingResultLeafNode*>(*child)
							->isEmptyResultLeafNode() == ModTrue) {
				// 空集合
				child = children.erase(child);
				continue;
			}

		} else {
			// child が TermLeafNode 以外の場合
			QueryNode* tmpNode = 0;
			if ((*child)->eraseTermLeafNode(tmpNode, query) != ModTrue) {
				// 後処理が必要
				query.addOrStanderdSharedNode(*child);
				if (tmpNode == 0) {
					// 子ノードを削除
					child = children.erase(child);
					continue;
				} else {
					// 子ノードを昇格
					*child = tmpNode;
				}
			}
		}
		++child;
	}
	// 全ての子ノードの処理が終了。子ノード数を確認する

	ModBoolean boolval = ModTrue;
	node = 0;
#ifdef DEBUG
	ModDebugMessage << "Or::eraseTermLeafNode child num = " << children.getSize()
					<< ModEndl;
#endif // DEBUG

	switch(children.getSize()) {
	case 0:								// 子ノード数0
		boolval = ModFalse;
		break;
	case 1:								// 子ノード数1 子ノードの昇格が必要
		boolval = ModFalse;
		node = children[0];
		break;
	}
	return boolval;
}

//
// FUNCTION protected
// ModInvertedOperatorOrNode::reevaluate -- 正確な再 evaluate
//
// NOTES
// 正確な再 evaluate を行なう
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
//
// RETURN
// 出現位置検査を満足すれば ModTrue、しなければ ModFalse
//
// EXCEPTIONS
// 下位の例外をそのまま投げる
//
ModBoolean
ModInvertedOperatorOrNode::reevaluate(ModInvertedDocumentID documentID)
{
	for (ModVector<QueryNode*>::Iterator child(this->children.begin()),
			 end(this->children.end());
		child != end; ++child) {
		if ((*child)->evaluate(documentID, QueryNode::defaultEMode)
			== ModTrue) {
			return ModTrue;
		}
	}
	return ModFalse;
}

//
// FUNCTION protected
// ModInvertedOperatorOrNode::reevaluate -- 正確な再 evaluate と位置情報の獲得
//
// NOTES
// 粗い evaluate を前提として、正確な再 evaluate を行ない、満足の場合、
// 与えられた出現位置情報オブジェクトに出現位置情報を格納する。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedLocationListIterator*& locations
//		出現位置反復子へのポインタ (結果格納用)
// ModSize& uiTF_
//		(位置情報リストを取得できない場合) TF
// ModInvertedQueryNode* givenEndNode
// 		ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// 出現位置検査を満足すれば ModTrue、しなければ ModFalse
//
// EXCEPTIONS
// 下位の例外をそのまま投げる
//
ModBoolean
ModInvertedOperatorOrNode::reevaluate(ModInvertedDocumentID documentID,
									  LocationIterator*& locations,
									  ModSize& uiTF_,
									  ModInvertedQueryNode* givenEndNode)
{
	// TFの扱いはModInvertedAtomicOrNodeと同様
	
	// 子ノードそれぞれについて正確に再 evaluate すると共に、
	// 両者の出現位置も得る。

	OrLocationIterator* tmpLocation
		= static_cast<OrLocationIterator*>(getFreeList());
	if (tmpLocation == 0)
	{
		tmpLocation = new OrLocationIterator(this);
		tmpLocation->reserve(children.getSize());
	}
	LocationIterator::AutoPointer p = tmpLocation;

	LocationIterator* childLocation = 0;
	ModSize uiTF = 0;
	ModSize uiMaxTF = 0;
	for (ModVector<QueryNode*>::Iterator child(this->children.begin()),
			 end(this->children.end());
		 child != end; ++child) {
///	高速化のために個別に呼び分ける
///			if ((*child)->evaluate(documentID, tmpLocation, 
///								   defaultEMode ,0) == ModTrue) {
		if ((*child)->evaluate(documentID,
							   defaultEMode | Query::roughEvaluationFlag)
			== ModTrue &&
			(*child)->reevaluate(documentID, childLocation, uiTF, 0) == ModTrue) {
			if (childLocation != 0)
			{
				if (this->shortWordLength > 0 &&
					this->shortWordLength != ModSizeMax) {
					// short word の長さを LocationListIterator にセットする
					// short word の場合、ユーザが与えた長さを使う
					static_cast<ModInvertedCompressedLocationListIterator*>(
						childLocation)->setLength(this->shortWordLength);
				}
				// 位置情報が得られたら tmpLocation へ push
				tmpLocation->pushIterator(childLocation);
				childLocation = 0;
			}
			else
			{
				; ModAssert(uiTF > 0);
				if (uiTF > uiMaxTF)
				{
					uiMaxTF = uiTF;
				}
				uiTF = 0;
			}
		}
	}

	if (tmpLocation->getSize() > 0)
	{
		tmpLocation->initialize();

		if (tmpLocation->isEnd() == ModTrue)
		{
			return ModFalse;			// ひとつも条件に合うものがなかった
		}
	
		locations = p.release();
	}
	else if (uiMaxTF > 0)
	{
		// 一つも位置情報リストを取得できなかったが、TFは取得できた。
		uiTF_ = uiMaxTF;
	}
	else
	{
		// 子ノードが一件も存在しなかった
		return ModFalse;
	}
	
	return ModTrue;
}

//
// FUNCTION public
// ModInvertedOperatorOrNode::setUnion -- 和集合の生成
//
// NOTES
// ２つの集合の和集合を生成する
//
// ARGUMENTS
// const RankingResult& result1
//		集合１
// const RankingResult& result2
//		集合２
// RankingResult& result
//		和集合をセットする場所
// ScoreCombiner* combiner
//		スコア合成器
// const ModInvertedDocumentScore secondStep
//		第２段階のスコア
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedSearchResultScore::Iterator
ModInvertedOperatorOrNode::setUnion(
	const RankingResult& result1,
	ModInvertedSearchResultScore::ConstIterator riter2,
	ModInvertedSearchResultScore::ConstIterator rend2,
	ModInvertedSearchResultScore::Iterator riter,
	ModInvertedSearchResultScore::Iterator rend ,
	ModInvertedRankingScoreCombiner& combiner,
	const ModInvertedDocumentScore secondStep)
{
	ModInvertedSearchResultScore::ConstIterator riter1(result1.begin());
	ModInvertedSearchResultScore::ConstIterator rend1(result1.end());

	while (riter1 != rend1 && riter2 != rend2) {
		; ModAssert(riter != rend);
		// どちらもendに達してない
		if ((*riter2).first> (*riter1).first) {
			(*riter).first = (*riter1).first;
			(*riter).second = combiner.combine((*riter1).second*secondStep, 0);
			++riter1;

		} else if ((*riter2).first < (*riter1).first) {
			(*riter).first = (*riter2).first;
			(*riter).second = combiner.combine((*riter2).second, 0);
			++riter2;

		} else {
			(*riter).first = (*riter2).first;
			(*riter).second = combiner.combine((*riter1).second*secondStep,
											  (*riter2).second);
			++riter1;
			++riter2;
		}
		++riter;
	}

	// 残っているデータをコピーする
	while (riter1 != rend1) {
		; ModAssert(riter != rend);
		(*riter).first = (*riter1).first;
		(*riter).second = combiner.combine((*riter1).second*secondStep, 0);
		++riter;
		++riter1;
	}
	while (riter2 != rend2) {
		; ModAssert(riter != rend);
		(*riter).first = (*riter2).first;
		(*riter).second = combiner.combine((*riter2).second, 0);
		++riter;
		++riter2;
	}

	return riter;
}

//
// FUNCTION
// ModInvertedOperatorOrNode::lowerBoundScoreForSecondStep() -- ランキング検索の第２ステップで使用するlowerBound
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
ModInvertedOperatorOrNode::lowerBoundScoreForSecondStep(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
	score = 0.0;
	DocumentScore childScore;
	ModVector<QueryNode*>::Iterator child = this->children.begin();
	ModVector<QueryNode*>::Iterator end = this->children.end();

	DocumentID lowerID(ModInvertedUpperBoundDocumentID);
	ModVector<DocumentScore>::Iterator i = scores.begin();
	// scoresのリセット開始位置
	ModVector<DocumentScore>::Iterator c = i;
	for (; child != end; ++child, ++i) {
		if((*child)->lowerBoundScoreForSecondStep(givenID, foundID, childScore)
			== ModTrue) {
			// ヒットした子ノードの中で最小の文書IDを求める
			
			// lowerIDを更新するのは foundID < lowerIDの時
			
			// lowerIDの初期値はUpperBoundDocumentIDなので、
			// 初めてfoundIDが得られた時は、更新扱いになる。
			
			// 得られるfoundIDは、UpperBoundDocumentID未満のはず。
			; ModAssert(foundID < ModInvertedUpperBoundDocumentID);
			
			// scoresをリセットするのもlowerIDが更新された時。
			
			// リセットの開始位置は、前回lowerIDを更新した位置から。
			// それ以前のscoresは0にリセットされている。
			
			if (foundID == lowerID) {
			} else if(foundID < lowerID) {
				lowerID = foundID;
				for (; c < i; ++c) {
					*c = 0.0L;
				}
			} else {
				childScore = 0.0L;
			}
		} else {
			childScore = 0.0L;
		}
		*i = childScore;
	}

	if(lowerID == ModInvertedUpperBoundDocumentID) {
		// １件もヒットしない場合はFALSE
		return ModFalse;
	}

	// 子ノードのスコアから自分のスコアを合成する
	// 全てのノードのスコアから新しいスコアを合成する必要がない時もあるが、
	// 実際に計測してみると、beginからendまで使って計算した方が速かった。
	score = this->scoreCombiner->apply(scores);
	// 最小の文書IDを返す
	foundID = lowerID;

	return ModTrue;
}

//
// FUNCTION public
// ModInvertedOperatorOrNode::getSearchTermList --
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
ModInvertedOperatorOrNode::getSearchTermList(
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
