// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOrderedDistanceNode.cpp -- 索引語に対応する末端ノードの実装
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#include "ModAssert.h"
#include "ModOstrStream.h"
#include "ModInvertedException.h"
#include "ModInvertedOrderedDistanceNode.h"
#include "ModInvertedOrderedDistanceLocationListIterator.h"

#ifdef DEBUG
#include "ModInvertedSimpleTokenLeafNode.h"
#endif // DEBUG

#ifdef DEBUG
//
// VARIABLE
// ModInvertedOrderedDistanceNode::countLocCheck -- 位置検査の回数
//
/*static*/ int
ModInvertedOrderedDistanceNode::countLocCheck = 0;
#endif

#ifndef DEL_BOOL
// 
// FUNCTION public
// ModInvertedOrderedDistanceNode::retrieve -- 検索の一括実行
// 
// NOTES
// 検索式を一括実行する。
//
// document-at-atime (default)の場合
// roughEvaluationFlagをOnにしてthis->lowerBound()を実行し、結果がTrue
// の場合 this->reevaluate() を実行する。
//
// OrderedDistanceNodeではqueryNodeForRoughEvaluationはセットされてい
// ないので、roughEvaluationFlagがonでもoffでも処理内容は変らない。
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
ModInvertedOrderedDistanceNode::retrieve(ModInvertedBooleanResult& queryResult,
										 Query::EvaluateMode mode)
{
	// 検索結果をまず空にする。
	if (!queryResult.isEmpty()) {
		queryResult.clear();
	}

	estimateDocumentFrequency();

	if (estimatedDocumentFrequency != UndefinedFrequency) {
		// 出現頻度と同じ分だけ予約しておく
		queryResult.reserve(estimatedDocumentFrequency);
	}

	if ((mode & Query::roughEvaluationFlag) != 0 &&
		queryNodeForRoughEvaluation != 0 ) {
		// 粗い retrieve() を実行
		this->queryNodeForRoughEvaluation->retrieve(queryResult, mode);
		return;
	}

	// retrieve 処理
	if ((mode & Query::andTermAtATimeFlag) != 0) {
		ModVector<ModInvertedQueryNode*>::Iterator child = children.begin();
		// term-at-a-time
		ModInvertedBooleanResult tmpResult[2];
		int tmpResultIndex = 0;
		// 最初の子ノードで中間結果を得る。
		(*child)->retrieve(tmpResult[0], mode);
		// もう一方のtmpResultも同サイズにしておく
		tmpResult[1].assign(tmpResult[0].getSize(), 0);
		// 残りの子ノードで粗い評価を行う。
		ModInvertedBooleanResult::Iterator p0, p1;
		for (++child; child != children.end(); ++child)
		{
			tmpResultIndex = 1 - tmpResultIndex;	// 0 <--> 1
			p1 = tmpResult[tmpResultIndex].begin();
			// これまでの中間結果の１つ１つについて、次の子ノードで検査する。
			p0 = tmpResult[1 - tmpResultIndex].begin();
			ModInvertedBooleanResult::Iterator e
				= tmpResult[1 - tmpResultIndex].end();
			for (; p0 != e; ++p0) {
				if ((*child)->evaluate(*p0, mode) == ModTrue) {
					*p1 = *p0;
					++p1;
				}
			}
			// tmpResult の不要部分の消去
			tmpResult[tmpResultIndex].erase(p1,
											tmpResult[tmpResultIndex].end());
		}
		// 最終結果の評価
		ModInvertedBooleanResult::Iterator p
			= tmpResult[tmpResultIndex].begin();
		ModInvertedBooleanResult::Iterator e
			= tmpResult[tmpResultIndex].end();
		for (; p != e; ++p)
		{
			if (reevaluate(*p) == ModTrue)
				queryResult.pushBack(*p);
		}
		
		if (this->queryNodeForRoughEvaluation != 0) {
			// ラフノードを調査済みにしてしまうと、子ノードの位置が
			// 動いた場合にreeveluate処理がおかしくなるので、
			// ここは未調査に設定しておく。
			this->queryNodeForRoughEvaluation->resetUpperLower();
		}
	} else {	// document-at-atime (default)
		// DocumentID (初期値は最小値)
		ModInvertedDocumentID currentID = 1;
		if (this->queryNodeForRoughEvaluation != 0) {
			// ラフノードがあれば使う
			while (queryNodeForRoughEvaluation->lowerBound(
					   currentID, currentID,
#ifdef	ROUGH_MODE_FLAG
					   mode|Query::roughEvaluationFlag
#else	// ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
					   mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
					   ) == ModTrue) {
				// まずラフノードへlowerBound()を実行してから
				// reevaluate()を実行する
				if (reevaluate(currentID) == ModTrue) {
					// reevaluate() が True だったら結果を格納
					queryResult.pushBack(currentID);
				}
				currentID++;
			}
		} else {
			// ラフノードがない場合
			while (lowerBound(currentID, currentID,
							  mode|Query::roughEvaluationFlag) == ModTrue)
			{
				// まず粗くlowerBound()を実行してからreevaluate()を実行する
				// もう一方の子ノードを検査
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
	return;
}
#endif

// 
// FUNCTION public
// ModInvertedOrderedDistanceNode::evaluate -- 与えられた文書が検索式を満たすかどうかの検査
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
ModInvertedOrderedDistanceNode::evaluate(ModInvertedDocumentID documentID,
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

	// roughEvaluation mode check
	if ((mode & Query::roughEvaluationFlag) != 0
		&& this->queryNodeForRoughEvaluation != 0) {
		// 粗いlowerBoundを実行
		return this->queryNodeForRoughEvaluation->evaluate(
			documentID,
#ifdef	ROUGH_MODE_FLAG
			mode
#else	// ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
			mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
			);
	}

	if (this->queryNodeForRoughEvaluation != 0) {
		// ラフノードがあるなら使う
		if (queryNodeForRoughEvaluation->evaluate(
				documentID,
#ifdef	ROUGH_MODE_FLAG
				mode|Query::roughEvaluationFlag
#else	// ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
				mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
				) == ModFalse) {
			return ModFalse;		// 条件を不満足
		}
	} else {
		// ラフノードがない
		// 文書頻度が少なそうな方から始める。
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
		// roughEvaluation Mode
		return ModTrue;
	}

	// roughEvaluation Flag が OFF の場合 reevaluate() を実行する
	if (this->reevaluate(documentID) == ModTrue) {
		this->upper = this->lower = documentID;
		return ModTrue;			// 条件を満足
	}
	
	return ModFalse;					// 条件を満足しなかった
}

// 
// FUNCTION public
// ModInvertedOrderedDistanceNode::lowerBound -- 検索式を満たす文書のうち、文書IDが与えられた値以上で最小の文書の検索
// 
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する。
//
// 検索結果をupper/lower両変数に保存する。lowerには引数givenIDを保存し、
// upperには検索結果であるfoundIDを保存する。そうすることで 
// lowerBound() が次回呼ばれた時に givenID がlower <= givenID <= upper 
// であった場合検索結果として upper を返す。
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
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedOrderedDistanceNode::lowerBound(ModInvertedDocumentID givenID,
										   ModInvertedDocumentID& foundID,
										   Query::EvaluateMode mode)
{
	// 既に検索済みか調べる
	if (givenID >= this->lower) {
		if (this->upper == ModSizeMax) {
			return ModFalse;		// 既にEndに達っしている
		}
		if (givenID <= this->upper) {
			// 検索済みであったので、前回の結果を返す
			foundID = this->upper;
			return ModTrue;
		}
	}
	// roughEvaluation mode check
	if ((mode & Query::roughEvaluationFlag) != 0
		&& this->queryNodeForRoughEvaluation != 0) {
		// 粗いlowerBoundを実行しその結果を返す
		return this->queryNodeForRoughEvaluation->lowerBound(
			givenID, foundID,
#ifdef	ROUGH_MODE_FLAG
			mode
#else	// ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
			mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
			);
	}

	ModInvertedDocumentID currentID = givenID;
	if (this->queryNodeForRoughEvaluation != 0) {
		// ラフノードがあるなら使う
		while (1) {

			// まずラフノードに lowerBound
			if (queryNodeForRoughEvaluation->lowerBound(
					currentID, currentID,
#ifdef	ROUGH_MODE_FLAG
					mode|Query::roughEvaluationFlag
#else	// ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
					mode & ~Query::roughEvaluationFlag
#endif	// ROUGH_MODE_FLAG
					) == ModFalse) {
				lower = givenID;
				upper = ModSizeMax;				// ModSizeMax は最後まで検索したことを表わす
				return ModFalse;		// 条件を満足する文書はない
			}
			if ((mode & Query::roughEvaluationFlag) != 0
				|| this->reevaluate(currentID) == ModTrue) {
				foundID = currentID; // 条件を満足

				if ((mode & Query::roughEvaluationFlag) == 0) {
					this->upper = currentID;
					this->lower = givenID;
				}
				return ModTrue;
			}
			currentID++;
		}
	} else {
		// ラフノードがない場合 常に document-at-a-time
		ModVector<QueryNode*>::Iterator child = this->children.begin();
		ModInvertedDocumentID smallestID = givenID;
		while (1) {
			if (child == children.end()) {
				// 全ての子ノードの条件を満足
				if ((mode & Query::roughEvaluationFlag) != 0) {
					// 粗い検索の場合これで終り
					foundID = currentID;
					return ModTrue;
				}
				// 通常の検索では位置の付け合せをする
				if (this->reevaluate(currentID) == ModTrue) {
					// 検索結果が得られた
					this->upper = foundID = currentID;
					this->lower = givenID;
					return ModTrue;
				}
				++currentID;			// 次を探すために 1 増して、
				child = children.begin();
			} else if ((*child)->lowerBound(currentID, smallestID,
											mode|Query::roughEvaluationFlag)
					   == ModFalse) {
				// この子ノードの条件を満足する文書はもうない。
				lower = givenID;
				upper = ModSizeMax;
				return ModFalse;
			} else if (currentID < smallestID) {
				currentID = smallestID;	// 最小候補に改めて、
				if (child == children.begin())
					++child;					// 先頭なら次へ
				else
					child = children.begin();	// それ以外なら先頭へ
			} else { // currentID == smallestID
				++child;		// 次の子ノードへ
			}
		}
	}
}

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::insertChild -- 子ノードの追加
//
// NOTES
//
// ARGUMENTS
// ModSize position
//		追加する子ノードの位置
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
ModInvertedOrderedDistanceNode::insertChild(ModSize position,
											ModInvertedQueryNode* child)
{
	pathPosition.pushBack(position);
	ModInvertedOperatorAndNode::insertChild(child);
	if (maxPosition < position)
	{
		// 最大値が挿入されたら覚えておく
		maxPosition = position;
		maxElement = pathPosition.getSize() - 1;
	}
}

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::sharedQueryNode -- 中間ノードの共有化
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
// ている
//
// ARGUMENTS
// QueryNodeMap& globalNodeMap
//		全ノードの QueryNodeMap
// QueryNodePointerMap& nodePointerMap
//		OR標準形変換により共有されているノードが登録されているMap
//
// RETURN
// children の数
//		ただし、子ノードにEmptyeSetNodeを含む場合は自分自身もEmptySetNode
//		になるので0を返す
//
// EXCEPTIONS
// なし
//
ModSize
ModInvertedOrderedDistanceNode::sharedQueryNode(
	QueryNodeMap& globalNodeMap,
	QueryNodePointerMap& nodePointerMap)
{
	QueryNodeMap::Iterator p;
	ModVector<ModInvertedQueryNode*>::Iterator child;

	ModInvertedQueryNode* tmpEndNode=0;

	ModSize retGrandChildNum;			// 子ノードのsharedQueryNodeの戻値


	child = children.begin();
	for (; child != children.end(); ++child) {
		if (ModInvertedIsInternalNode((*child)->getType()) != ModTrue) {
			// リーフノードの場合なにもしない
			continue;					// 次の child へ
		}
		// internal node の場合

		// 子ノードに対して sharedQueryNode() を呼ぶ
		retGrandChildNum = static_cast<InternalNode*>(*child)->
			sharedQueryNode(globalNodeMap, nodePointerMap);

		if (retGrandChildNum == 1) {
			// ノードの共有化をした結果 children が 1つしかなくなった
			// 場合ノードを単純化する
			changeSimpleTypeNode(child, nodePointerMap);

			continue;					// 次の子ノードへ

		} else if (retGrandChildNum == 0) {
			// 子ノードのsharedQueryNodeの戻り値が0
			// この子ノードはEmptySetNode
			addQueryNodePointerMap(*child, nodePointerMap);
			*child = const_cast<ModInvertedQueryNode*>(emptySetNode);
			continue;
		}

		// 子ノードの QueryString を取得する
		ModUnicodeString key;
		(*child)->getQueryString(key);

		QueryNode* value = *child;      // QueryNode ポインタをセット

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
				// 既に同じQueryStringのノードがある
				// childのnodeをdeleteする

				// OrderedDistanceの共有の場合はendNodeを考慮する必要がある
				if (ModInvertedAtomicMask((*child)->getType())
					// NodeTypeは実際にはQueryNodeで持っているが、
					// windows版のあいまい除去のためAtomicNode
					// にしている
					== ModInvertedAtomicNode::orderedDistanceNode) {

					// tmpEndNodeが0以外なら、
					// 削除される側のOrderedDistanceがendNodeを持っている
					tmpEndNode = (*child)->getEndNode();

					// originalStringも共有する
					sharedOriginalString(*child, (*p).second);

#ifdef DEBUG
					ModDebugMessage << "OrderedDistanceNodeの共有 "
									<< "{" << key << "}"
									<< ModEndl;
#endif // DEBUG
				}

				addQueryNodePointerMap(*child, nodePointerMap);

				// childは先にglobalNodeMapにあったnodeへのポインタをセットする

				// ここでOrederedDistanceのendNodeとの比較が必要
				// 一致した場合はendNodeの変更が必要

				*child = (*p).second;
				// OrderedDistance かつ endNodeを持っていた
				if (tmpEndNode != 0) {
#ifdef DEBUG
					ModDebugMessage
						<< "OrderedDistanceNodeの共有[endNodeの修正]"
						<< ModEndl;
#endif // DEBUG
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

	// 子ノードのチェック
	child = children.begin();

	for (; child != children.end(); ++child) {
		if (*child == emptySetNode) {
#ifdef DEBUG
			ModDebugMessage << "子ノードがにEmptySetNodeを含む"
							<< "EmptySetNodeをセット"
							<< ModEndl;
#endif // DEBUG
			// 子ノードにEmptySetNodeを含む
			child = children.begin();
			while (child != children.end()) {
				// childrenにEmptySetNodeを含むため、自分自身もEmptySetNodeに
				// なる。このためchildrenを削除する。
				if (*child != emptySetNode) {
					addQueryNodePointerMap(*child, nodePointerMap);
				}
				child = children.erase(child);
			}
			return 0;
		}
	}

	return children.getSize();
}

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::prefixString -- 演算子を表わす文字列を返す
//
// NOTES
// QueryNodeで定義された内容をオーバライドする
//
// ARGUMENTS
// ModString& prefix
//		演算子を表わす文字列を返す(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void 
ModInvertedOrderedDistanceNode::prefixString(ModUnicodeString& prefix,
											 const ModBoolean withCalOrCombName,
											 const ModBoolean withCalOrCombParam) const
{
	ModOstrStream tmp;

	tmp << "#distance[";
	ModVector<ModSize>::ConstIterator p = pathPosition.begin();
	for (; p != pathPosition.end(); ++p)
	{
		if (p != pathPosition.begin())
			tmp << ",";
		tmp << *p;
	}
	tmp << "]";

	prefix += ModUnicodeString(tmp.getString());

	if (withCalOrCombName == ModTrue) {

		ModUnicodeString calculatorName;
		getCalculatorOrCombinerName(calculatorName, withCalOrCombParam);

		if(calculatorName.getLength() > 0) {
			prefix += "[";
			prefix += ModUnicodeString(calculatorName);
			prefix += "]";
		}
	}
}

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::checkQueryNode -- 子ノードの数をチェックする
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
ModInvertedOrderedDistanceNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_
	)
{
	if(needDF_ == ModTrue) {
		setNeedDF(ModTrue);
	}

	ModSize s = this->children.getSize();
	if (s == 0) {
		// 子ノードがないのは異常
		ModErrorMessage << "children is empty." << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}
	ModUnicodeString termString;
#ifdef V1_6
	ModLanguageSet tmpLangSet;
#endif // V1_6
	ModInvertedTermMatchMode tmpMmode;
	if (setStringInChildren_ == ModTrue && 
		getOriginalString(termString,
#ifdef V1_6
						  tmpLangSet,
#endif // V1_6
						  tmpMmode) == ModTrue) {
		termString.clear();
		getQueryString(termString, ModTrue, ModTrue, ModTrue, ModFalse);
		query_->insertTermNode(termString,
							   static_cast<ModInvertedOperatorAndNode*>(const_cast<ModInvertedOrderedDistanceNode*>(this)));

		ModSize avelen(query_->getAverageDocumentLength());
		if (avelen != 0) {
			ScoreCalculator* calculator = getScoreCalculator();
			; ModAssert(calculator != 0);
			calculator->setAverageDocumentLength(avelen);
		}
	}

	for (ModSize i = 0; i < s ; ++i)
	{
		children[i]->checkQueryNode(query_, ModFalse, ModFalse);
	}
}

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::getOriginalString() -- 自分が生成された元のTermLeafNodeの文字列を返す。
//
// NOTES
// 自分が生成された元のTermLeafNodeの文字列を返す。元の文字列はメンバ変数
// originalTermStringにeraseTermLeafNodeQuery()でセットされる。
// 但し、originalTermStringがセットされるのは生成されたorderedDistanceのうち
// 一番上のもののみである。
//
// originalTermStringがセットされていない場合はFalseを返す。
//
// ARGUMENTS
// ModUnicodeString& termString
//		結果格納用
//
// RETURN
// originalTermStringがある場合True/ない場合False
//
// EXCEPTIONS
// なし
//
ModBoolean
ModInvertedOrderedDistanceNode::getOriginalString(
	ModUnicodeString& termString,
#ifdef V1_6
	ModLanguageSet& langSet_,
#endif // V1_6
	ModInvertedTermMatchMode& mmode_) const
{
	if (originalTermString.getLength() != 0) {
		// originalTermStringがセットしてある
		termString = originalTermString;	
#ifdef V1_6
		langSet_ = langSet;
#endif // V1_6
		mmode_ = mmode;
		return ModTrue;
	}

	return ModFalse;
}

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::setOriginalString() -- 自分が生成された元のTermLeafNodeの文字列をメンバ変数originalTermStringにセットする。
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
ModInvertedOrderedDistanceNode::setOriginalString(
	const ModUnicodeString& termString,
#ifdef V1_6
	const ModLanguageSet& langSet_,
#endif // V1_6
	const ModInvertedTermMatchMode& mmode_)
{
#ifdef DEBUG
	ModDebugMessage << "OrderdDistanse:setOriginalString: " << termString << ModEndl;
#endif // DEBUG

	originalTermString = termString;
#ifdef V1_6
	langSet = langSet_;
#endif // V1_6
	mmode = mmode_;
}

//
//
// FUNCTION protected
// ModInvertedOrderedDistanceNode::reevaluate -- 正確な再 evaluate
//
// NOTES
// 粗い evaluate 後の、正確な再 evaluate。
// 出現位置の検査のみを行なう。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
//
// RETURN
// 出現位置検査を満足すれば ModTrue、しなければ ModFalse
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
ModBoolean
ModInvertedOrderedDistanceNode::reevaluate(ModInvertedDocumentID documentID)
{
	// 三引数のreevaluateと動作は基本的に同じ
	
	ModSize uiChildrenCount = children.getSize();
	ModInvertedOrderedDistanceLocationListIterator* iterator
		= static_cast<ModInvertedOrderedDistanceLocationListIterator*>(getFreeList());
	if (iterator == 0)
	{
		iterator = new ModInvertedOrderedDistanceLocationListIterator(this);
		iterator->reserve(uiChildrenCount);
	}
	LocationIterator::AutoPointer p = iterator;

#ifdef DEBUG
	++countLocCheck;
#endif

	// 常に document-at-a-time
	// ２つのノードそれぞれについて正確に再 evaluate すると共に、
	// 両者の出現位置も得る。
	ModVector<ModInvertedQueryNode*>::Iterator child = children.begin();
	ModVector<ModSize>::Iterator pos = pathPosition.begin();
	for (; child != children.end(); ++child, ++pos)
	{
		ModInvertedLocationListIterator* p = 0;
		if ((*child)->reevaluate(documentID, p) == ModFalse)
		{
			return ModFalse;
		}
		
		if (p != 0)
		{
			iterator->pushIterator(*pos, p);
		}
	}

	// 位置を比較
	ModSize uiIteratorCount = iterator->getSize();
	if (uiIteratorCount == uiChildrenCount)
	{
		iterator->initialize();
		if (iterator->isEnd() == ModTrue)
		{
			return ModFalse;
		}
	}
	else if (uiChildrenCount == 0 || uiIteratorCount > 0)
	{
		// ノードが0件だった、
		// または、一部のノードで位置情報リストを取得できなかった
		return ModFalse;
	}
	
	return ModTrue;
}

//
// FUNCTION protected
// ModInvertedOrderedDistanceNode::reevaluate -- 正確な再 evaluate と位置情報の獲得
//
// NOTES
// 粗い evaluate を前提として、正確な再 evaluate を行ない、満足の場合、
// 与えられた出現位置情報オブジェクトに出現位置情報を格納する。
//
// ARGUMENTS
// ModInvertedLocationListIterator*& locations
//		出現位置反復子へのポインタ (結果格納用)
// ModSize& uiTF_
//		(locationsが取得できない場合) TFの推定値
// ModInvertedQueryNode* givenEndNode
//		OrderedDistanceが作成される前のTermLeafNodeの最終端simpleTokenLeafNode
//		へのポインタ。自分の子ノードがこのgivenEndNodeと一致した場合は、子ノード
//		から作成したlocationListIteratorを自分のlocation->endにセットする。
//		(現在は使用しない)
//
// RETURN
// 出現位置検査を満足すれば ModTrue、しなければ ModFalse
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
ModBoolean
ModInvertedOrderedDistanceNode::reevaluate(DocumentID documentID,
										   LocationIterator*& locations,
										   ModSize& uiTF_,
										   ModInvertedQueryNode* givenEndNode)
{
	// 各子ノードの位置リストを再評価する。
	
	// 入力は、例えば索引語長が4で、検索語が'ABCDEFGHI'の場合、
	// SimpleQuecyの各子ノードは('FGHI',6), ('BCDE',2), ('ABCE',1) となる。
	// 数字は、ModVector<ModSize> pathPositionの各値を表し、
	// 検索語における索引語の位置が保持されている。

	ModInvertedOrderedDistanceLocationListIterator* iterator
		= static_cast<ModInvertedOrderedDistanceLocationListIterator*>(getFreeList());
	if (iterator == 0)
	{
		iterator = new ModInvertedOrderedDistanceLocationListIterator(this);
		iterator->reserve(children.getSize());
	}
	LocationIterator::AutoPointer p = iterator;

#ifdef DEBUG
	++countLocCheck;
#endif

	// 最後方に位置する索引語の位置リスト
	ModInvertedLocationListIterator* max = 0;
	
	// 常に document-at-a-time
	// ２つのノードそれぞれについて正確に再 evaluate すると共に、
	// 両者の出現位置も得る。
	ModVector<ModInvertedQueryNode*>::Iterator child = children.begin();
	ModVector<ModSize>::Iterator pos = pathPosition.begin();
	ModSize i = 0;
	ModSize uiTF;
	ModSize uiMinTF = ModSizeMax;
	for (; child != children.end(); ++child, ++pos, ++i)
	{
		// 子ノードを順番に確認する。
		
		// 子ノードが指す索引語は、documentIDが示す文書に含まれるか？
		ModInvertedLocationListIterator* p = 0;
		uiTF = 0;
		if ((*child)->reevaluate(documentID, p, uiTF) == ModFalse)
		{
			// 各ノードの条件はANDの関係にあるので、
			// 一つのノードが条件を満たさなければ、全体も条件を満たさない。
			return ModFalse;
		}
		
		// documentIDが示す文書に含まれることが確認できた。
		
		if (p != 0)
		{
			// 位置情報リストを取得できた
			
			// 検索語における索引語の位置と、
			// 検索語のdocumentIDにおける位置情報リストとを保持する。
			iterator->pushIterator(*pos, p);
			if (i == maxElement) max = p;
		}
		else
		{
			// 位置情報リストを取得できなかった

			// 各索引語の文書内での位置がわからないので、
			// 呼び出し先で正確なTFは計算できない。
			// そこで、各ノードから得られるTFの中で最小のTFを返す。
			// ちなみに、各ノードの条件はANDの関係にあるので、
			// 最小TFはTFの上限を意味する。(本当のTFを超えている可能性もある)
			; ModAssert(uiTF > 0);
			if (uiTF < uiMinTF)
			{
				uiMinTF = uiTF;
			}
		}
	}

	//
	// 位置を比較
	//
	ModSize uiIteratorCount = iterator->getSize();
	if (uiIteratorCount == i)
	{
		// 全てのノードの位置情報リストを取得できた
		
		// 各子ノードの位置リストが条件を満たすかどうか
		iterator->initialize();
		if (iterator->isEnd() == ModFalse) {
			// OrderedDistanceLocationListIteratorのendのセット
			iterator->setEnd(max);
			locations = p.release();
		}
		else
		{
			// 満たさなかった
			return ModFalse;
		}
	}
	else if (i > 0 && uiIteratorCount == 0)
	{
		// ノードの位置情報リストが一つも取得できなかった
		
		// 位置の比較はできないが、条件は満たされたとみなし、TFを返す
		uiTF_ = uiMinTF;
	}
	else
	{
		// ノードが0件だった、
		// または、一部のノードだけ位置情報リストを取得できなかった
		return ModFalse;
	}
	
	return ModTrue;
}

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::makeRoughPointer -- queryNodeForRoughEvaluation の作成
//
// NOTES
// ModInvertedQueryNode の makeRoughPointer() をオーバライドしている。
//
// 通常のノードではラフノードの生成を行うが、ordredDistanceの場合は、
// TermLeafNodeのvalidateでラフノードが既に作られているので、
// 自分の親ノードのラフノードを生成するために、自分のラフノードの要素を
// 引数parentMapにセットして返す。
//
// 	※ラフノードの要素
//		ラフノードがandならandの子ノード
//		ラフノードがsimpleTokenならsimpleTokenそのもの
//
// 自分のrough pointerがセットされていない場合なにもっせずreturnする。
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
// ModInvertedErrorRetrieveFail
//
void
ModInvertedOrderedDistanceNode::makeRoughPointer(
	const Query::ValidateMode mode,
	QueryNodePointerMap& parentMap,
	const ModInvertedQuery* Query) 
{
	if (this->queryNodeForRoughEvaluation == 0) {
		return;
	}

	// ラフノードの種類をチェック
	NodeType nodeType
		= ModInvertedAtomicMask(queryNodeForRoughEvaluation->getType());

	// ラフノードにはandとsimpleTokenのケースがある

	if (nodeType == ModInvertedAtomicNode::operatorAndNode) {
		// NodeTypeは実際にはQueryNodeで持っているが、
		// windows版のあいまい除去のためAtomicNode
		// にしている

		// ラフノードは AndNode だった

		ModVector<QueryNode*>* roughChildren
			= static_cast<AndNode*>(queryNodeForRoughEvaluation)->getChildren();
		ModVector<QueryNode*>::Iterator p = roughChildren->begin();
		ModVector<QueryNode*>::ConstIterator end = roughChildren->end();

		// And ノードの子ノードを parentMap へ挿入
		for (;p != end; ++p) {
			parentMap[*p] = 1;
		}
	} else if (nodeType == ModInvertedAtomicNode::simpleTokenLeafNode) {
		// NodeTypeは実際にはQueryNodeで持っているが、
		// windows版のあいまい除去のためAtomicNode
		// にしている

		// ラフノードの内容が SimpleTokenLeafNode だったら
		// simpleTokenをマップに登録
		parentMap[queryNodeForRoughEvaluation] = 2;
	} else {
		// ラフノードはsimpleTokenまたはandなのでここに来るのは異常
		ModErrorMessage << "makeRoughPointer() Invalid Node Type" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}
}

// 
// FUNCTION public
// ModInvertedOrderedDistanceNode::calcSortFactor -- sortFactor の計算
// 
// NOTES
// sortChildren() 関数で使用する sortFactor メンバ変数を計算する。
// 
// ModInvertedQueryOperatorAndNode::calcSortFactor() をオーバライドし
// ている。子ノードの sortFactor の和に定数（値は適当）をかけて 
// sortFactor とする。
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
ModSize
ModInvertedOrderedDistanceNode::calcSortFactor()
{
	if (this->sortFactor == 0) {
		// まだ計算していないのなら、計算する
		AndNode::calcSortFactor();

		// 位置検査のための係数（1.1は適当）
		this->sortFactor = ModSize(1.1*this->sortFactor);
	}
	return this->sortFactor;
}

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
