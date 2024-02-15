// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedWindowBaseNode.cpp -- 近傍演算基底ノードの実装
// 
// Copyright (c) 2002, 2005, 2009, 2023 Ricoh Company, Ltd.
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
#include "ModAutoPointer.h"
#include "ModInvertedException.h"
#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedFile.h"
#else
#include "ModInvertedFile.h"
#endif
#include "ModInvertedWindowBaseNode.h"

#include "ModInvertedSearchResult.h"
#include "ModInvertedRankingScoreCalculator.h"

//
// VARIABLE
// ModInvertedWindowBaseNode::getTermFrequencyMethod -- 文書内頻度取得方法の指定
//
// NOTES
// 文書内頻度取得方法を指定する。
//	0: すべての組み合わせを異なりとしてカウントする（デフォルト）
//		  ModInvertedQueryNode::getTermFrequency()で実現する
//	1: 先頭位置の異なるものを異なりとしてカウントする
//	2: 照合範囲に重なりのないものを異なりとしてカウントする
//
/*static*/ int
ModInvertedWindowBaseNode::getTermFrequencyMethod = 0;

#ifndef DEL_BOOL
//
// FUNCTION public
// ModInvertedWindowBaseNode::retrieve -- 検索の一括実行
//
// NOTES
// 検索式を一括実行する。
// roughEvaluationFlagをOnにしてthis->lowerBound()を実行し、結果がTrue
// の場合 this->reevaluate() を実行する。
//
// ARGUMENTS
// ModInvertedBooleanResult& queryResult
//		Boolean検索結果オブジェクト
// Query::EvaluateMode mode
//		評価モード
//
// RETURN
// 正常終了の場合 ModOk、異常終了の場合 ModError
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
void
ModInvertedWindowBaseNode::retrieve(ModInvertedBooleanResult& queryResult,
									Query::EvaluateMode mode)
{
	// 評価モードのチェック
	if ((mode & Query::roughEvaluationFlag) != 0 &&
			this->queryNodeForRoughEvaluation != 0) {
		// 粗い検索を行なう
		queryNodeForRoughEvaluation->retrieve(queryResult, mode);
		return;
	}

	// 検索結果をまず空にする。
	if (!queryResult.isEmpty()) {
		queryResult.clear();
	}

	this->estimateDocumentFrequency();
	if (estimatedDocumentFrequency != UndefinedFrequency) {
		// 出現頻度と同じ分だけ予約しておく
		queryResult.reserve(estimatedDocumentFrequency);
	}

	// retrieve 処理 document-at-atime
	ModVector<ModInvertedQueryNode*>::Iterator child = this->children.begin();
	// DocumentID (初期値は最小値)
	ModInvertedDocumentID currentDocumentID = 1;

	while (lowerBound(currentDocumentID, currentDocumentID,
			mode|Query::roughEvaluationFlag)
			== ModTrue) {

		if( (mode & Query::roughEvaluationFlag) != 0 ) {
			// ラフモードの場合はreevaluateせずに結果を格納
			queryResult.pushBack(currentDocumentID);
		} else {
			// まず粗くlowerBound()を実行してからreevaluate()を実行する
			if (this->reevaluate(currentDocumentID) == ModTrue) {
				// reevaluate() が True だったら結果を格納
				queryResult.pushBack(currentDocumentID);
			}
		}
		currentDocumentID++;
	}

	// retrieve 終了。estimatedDocumentFrequency に正確な文書頻度を設定する
	this->estimatedDocumentFrequency = queryResult.getSize();

	// ラフノードでない場合のみretrievedをTrueにする
	if ((mode & Query::roughEvaluationFlag) == 0) {
		this->retrieved = ModTrue;
	}

	// queryResultに割り当てられたメモリを最小限にするためreserve()を実行
	queryResult.reserve(estimatedDocumentFrequency);
}
#endif
//
// FUNCTION public
// ModInvertedWindowBaseNode::getTermFrequency -- 文書内頻度の取得
//
// NOTES
// 条件を満たす語句の出現頻度を求める。QueryNode::getTermFrequency()
// を呼び出すだけ
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// Query::EvaluateMode mode
//		評価モード
//
// RETURN
// 求めた文書内頻度
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize
ModInvertedWindowBaseNode::getTermFrequency(
	DocumentID documentID,
	ModInvertedQuery::EvaluateMode mode)
{
#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // TF_WINDOW
	switch (getTermFrequencyMethod) {
	case 0:
		break;

	case 1:
	{
		ModInvertedLocationListIterator* locations = 0;
		if (this->evaluate(documentID, mode | Query::roughEvaluationFlag)
			== ModTrue) {

			ModSize tf = 0;
			ModSize prevLocation(0);
			if (reevaluate(documentID, locations) == ModTrue) {
				for (; locations->isEnd() == ModFalse; locations->next()) {
					// 必ずしも先頭位置でソートされているわけではない
					if (locations->getLocation() > prevLocation) {
						++tf;
						prevLocation = locations->getLocation();
					}
				}
				locations->release();
				return tf;
			}
		}
	}
	return 0;

	case 2:
	{
		ModInvertedLocationListIterator* locations = 0;
		if (this->evaluate(documentID, mode | Query::roughEvaluationFlag
						) == ModTrue) {
			ModSize tf = 0;
			ModSize prevLocation(0);
			if (reevaluate(documentID, locations) == ModTrue) {
				for (; locations->isEnd() == ModFalse; locations->next()) {
					// 必ずしも先頭位置でソートされているわけではない
					if (locations->getLocation() >= prevLocation) {
						++tf;
						prevLocation = locations->getEndLocation();
					}
				}
			}
			locations->release();
			return tf;
		}
	}
	return 0;

	default:
		break;
	}
#endif

	return ModInvertedQueryBaseNode::getTermFrequency(documentID, mode);
}

//
// FUNCTION public
// ModInvertedSimpleBaseNode::sharedQueryNode -- 中間ノードの共有化
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
// この関数は QueryInternalNode の sharedQueryNode() をオーバライドし
// ている。QueryInternalNode::sharedQueryNode()との違いは子ノード同士
// にgetQueryString()の結果が同じであるものがあるかどうかのチェックを
// しない点である。(localNodeMapは使わない)
//
// ARGUMENTS
// QueryNodeMap& globalNodeMap
//		全ノードの QueryNodeMap
// QueryNodePointerMap& nodePointerMap
//		OR標準形変換により共有されているノードが登録されているMap
//
// RETURN
// children の数。
//		ただし子ノードにEmptySetNodeを含む場合は、自分自身もEmptySetNodeに
//		なるので0を返す。
//
ModSize
ModInvertedWindowBaseNode::sharedQueryNode(
	QueryNodeMap& globalNodeMap,
	QueryNodePointerMap& nodePointerMap)
{
	QueryNodeMap::Iterator p;
	ModVector<ModInvertedQueryNode*>::Iterator child;
	ModInvertedQueryNode* tmpEndNode = 0;
	ModSize retGrandChildNum; // 子ノードのsharedQueryNodeの戻値

	for (child = children.begin(); child != children.end(); ++child) {
		if (ModInvertedIsInternalNode((*child)->getType()) != ModTrue) {
			// リーフノードの場合なにもせず、次の child へ
			continue;
		}
		// internal node の場合

		retGrandChildNum = static_cast<InternalNode*>(*child)->
		sharedQueryNode(globalNodeMap, nodePointerMap);

		// 子ノードに対して sharedQueryNode() を呼ぶ
		if (retGrandChildNum == 1) {
			// ノードの共有化をした結果 children が 1つしかなくなった
			// 場合ノードを単純化する。しかし、
			changeSimpleTypeNode(child, nodePointerMap);
			// continue;				  // 次の子ノードへ
		} else if (retGrandChildNum == 0) {
			// 子ノードのsharedQueryNodeの戻り値が0
			// この子ノードはEmptySetNodeなので、
			addQueryNodePointerMap(*child, nodePointerMap);
			*child = const_cast<ModInvertedQueryNode*>(emptySetNode);
			continue;
		}

		// childがまだInternalNodeであるか検査する
		if (ModInvertedIsInternalNode((*child)->getType()) != ModTrue) {
			// リーフノードの場合はなにもせず次の子ノードに行く
			continue;
		}

		// 子ノードの QueryString を取得する
		ModUnicodeString key;
		(*child)->getQueryString(key);

		QueryNode* value = *child;

		// globalNodeMapに同じQueryStringのノードがあるか調査する
		p = globalNodeMap.find(key);
		if (p != globalNodeMap.end()) {
			if (*child == (*p).second) {
				// すでに共有しているので特に処理しない
			} else {
				// 既に同じQueryStringのノードがある
				// childのnodeをdeleteする

				// OrderedDistanceの共有の場合はendNodeを考慮する必要がある
				if (ModInvertedAtomicMask((*child)->getType())
					== ModInvertedAtomicNode::orderedDistanceNode) {
					// tmpEndNodeが0以外なら、
					// 削除される側のOrderedDistanceがendNodeを持っている
					tmpEndNode = (*child)->getEndNode();

					// originalStringも共有する
					sharedOriginalString(*child, (*p).second);
				}

				addQueryNodePointerMap(*child, nodePointerMap);

				// childは先にglobalNodeMapにあったnodeへのポインタをセットする
				*child = (*p).second;

				// OrderedDistance かつ endNodeを持っていた
				if (tmpEndNode != 0) {
					(*child)->setEndNode(tmpEndNode);
					tmpEndNode = 0;
				}
			}
		} else {
			// globalNodeMap へ挿入する
			globalNodeMap[key] = value;
		}
	}
	
	// 子ノードのチェック
	for (child = children.begin(); child != children.end(); ++child) {
		if (*child == emptySetNode) {
			// 子ノードにEmptySetNodeを含む
			child = children.begin();
			while (child != children.end()) {
				// childrenにEmptySetNodeを含むため、自分自身もEmptySetNodeに
				// なる。このためchildrenを削除する。
				addQueryNodePointerMap(*child, nodePointerMap);
				child = children.erase(child);
			}
			return 0;
		}
	}
	return children.getSize();
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::validate -- 正規表現ノードの有効化
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
ModInvertedWindowBaseNode::validate(InvertedFile* invertedFile,
									const Query::ValidateMode mode,
									ModInvertedQuery* rQuery)
{
	if (children.getSize() < 2) {
		ModErrorMessage << "children size is not 2." << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}

	// ランキング検索の場合はスコア計算器をセットする
	if ((mode & Query::rankingMode) != 0) {
		if (this->scoreCalculator == 0) {
			// QueryNodeには必ずデフォルトの計算器をセットするように
			// なったので、ここではduplicateだけ
			ScoreCalculator* calculator = rQuery->getDefaultScoreCalculator();
			;ModAssert(calculator != 0);
			this->scoreCalculator = calculator->duplicate();
		}
		scoreCalculator->setDocumentLengthFile(
		invertedFile->getDocumentLengthFile());
	}
	// totalDocumnetFrequencyのセット
	setTotalDocumentFrequency(rQuery->getTotalDocumentFrequency());

	ModInvertedQueryInternalNode::validate(invertedFile,mode,rQuery);
}

//
// FUNCTION protected
// ModInvertedWIndowBaseNode::getQueryString -- 検索条件ノードを出力
//
// NOTES
// 検索条件ノードを出力。ラフノードの内容を表示するかどうかを選択できる
//
// ARGUMENTS
// ModOstrStream& out
//		結果格納用オブジェクト
// ModBoolean withRouh
//		ラフノードを表示するかどうかを示すフラグ（trueで表示）
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedWindowBaseNode::getQueryString(
	ModUnicodeString& out,
	const ModBoolean asTermString,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam,
	const ModBoolean withRouh) const
{
	ModUnicodeString prefix;
	ModUnicodeString termString;

	// calculator/combinerの表示ON/OFFが出来るようにする
	this->prefixString(prefix, withCalOrCombName, withCalOrCombParam);

	out += prefix;

	if (children.isEmpty() == ModFalse) {
		//
		// 子ノードの内容を表示
		//
		ModVector<ModInvertedQueryNode*>::ConstIterator p;
		ModVector<ModUnicodeString> childString;
		childString.clear();
		p = children.begin();
		(*p)->getQueryString(termString, asTermString, withCalOrCombName,
							 withCalOrCombParam, withRouh);
		childString.pushBack(termString);
		for (++p; p != this->children.end(); ++p) {
			termString.clear();
			(*p)->getQueryString(termString, asTermString, withCalOrCombName,
								 withCalOrCombParam, withRouh);
			childString.pushBack(termString);
		}
		if (asTermString == ModTrue) {
			ModSort(childString.begin(), childString.end());
		}
		ModVector<ModUnicodeString>::Iterator iter(childString.begin());
		out += '(';
		out += *iter;
		for(++iter ; iter != childString.end() ; ++iter) {
			out += ',';
			out += *iter;
		}
		out += ')';
	}

	ModUnicodeString roughString;
	// rough node 表示
	if (withRouh == ModTrue && queryNodeForRoughEvaluation != 0) {
		out += '<';
		this->queryNodeForRoughEvaluation->getQueryString(roughString,
				asTermString, withCalOrCombName, withCalOrCombParam, withRouh);
		out += roughString;
		out += '>';
	}
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::setChildForWindowNode -- WindowNode用の子ノードセット関数
//
// NOTES
// WindowNodeに子ノードをセットする
//
// ARGUMENTS
// ModInvertedQueryInternalNode*& node_
//		セットするWindowNode
// const ModInvertedQuery& rQuery_
//		クエリ	
// RETURN
//		なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void 
ModInvertedWindowBaseNode::setChildForWindowNode(ModInvertedQueryInternalNode*& node_,
												 const ModInvertedQuery& rQuery_)
{
	// 子ノードのduplicate
	ModSize i(0), s(this->children.getSize());
	while (i < s) {
		node_->insertChild(this->children[i]->duplicate(rQuery_));
		++i;
	}
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::checkQueryNode -- 子ノードの数をチェックする
//
// NOTES
// 有効化の最後に呼び出されて、子ノードの数をチェックする。
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
//
void
ModInvertedWindowBaseNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_)
{
	// 子ノードの数が2つ以上でなければ異常
	ModSize s(children.getSize());
	if (s < 2) {
		ModErrorMessage << "children size is not 2." << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}

	if (needDF_ == ModTrue) {
		setNeedDF(ModTrue);
	}
	
	ModUnicodeString termString;
	if (setStringInChildren_ == ModTrue) {
		getQueryString(termString, ModTrue, ModTrue, ModTrue, ModFalse);
		query_->insertTermNode(
			termString,
			static_cast<ModInvertedOperatorAndNode*>(this));
		ModSize avelen(query_->getAverageDocumentLength());
		if (avelen != 0) {
			ScoreCalculator* calculator = getScoreCalculator();
			; ModAssert(calculator != 0);
			calculator->setAverageDocumentLength(avelen);
		}
	}

	for (ModSize i(0); i < s ; ++i) {
		// すべての子ノードに対してcheckQueryNodeを呼ぶ
		if (children[i]->getType() ==
			ModInvertedAtomicNode::booleanResultLeafNode ||
			children[i]->getType() ==
			ModInvertedAtomicNode::rankingResultLeafNode) {
			ModErrorMessage << "cannot have boolean/ranking result leaf nodes"
							<< ModEndl;
			ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
		}
		children[i]->checkQueryNode(query_, ModFalse, ModFalse);
	}
}

//
// FUNCTION public
// ModInvertedWindowBaseNode::sortChildren -- 子ノードリストの並べ替え
//
// NOTES
// unordered の時だけ子ノードリストを並べ替える。子ノードに対しても
// sortChildren() を呼び出す。AndNodeのsortChildren()をオーバライドし
// ている。
//
// ARGUMENTS
// const ModInvertedQuery::ValidateMode mode
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedWindowBaseNode::sortChildren(
	const ModInvertedQuery::ValidateMode mode)
{
	InternalNode::sortChildren(mode);

	if (BaseNode::getType() &
		ModInvertedAtomicNode::unorderedNode) {
		// unordered の場合、AND と同様にソートする
		// ただし、ランキングの動作が異なるので、AndNode::sortChildren() を
		// 呼ぶことはできない
		ModSort(this->children.begin(), this->children.end(),
				AndNode::lessSortFactor);
	}
}

//
// Copyright (c) 2002, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
