// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorWordNode.cpp -- ModInvertedOperatorWordNodeの実装
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifdef V1_4	// 単語単位検索

#include "ModInvertedQueryNode.h"
#include "ModInvertedOperatorWordNode.h"
#include "ModInvertedBooleanResultLeafNode.h"
#include "ModInvertedRankingResultLeafNode.h"
#include "ModAutoPointer.h"
#include "ModInvertedOperatorWordNodeLocationListIterator.h"
#include "ModInvertedQueryParser.h"
#ifdef SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedFile.h"
#include "Inverted/ModInvertedList.h"
#else
#include "ModInvertedFile.h"
#include "ModInvertedList.h"
#endif

//
// FUNCTION public
// ModInvertedOperatorWordNode::ModInvertedOperatorWordNode -- 単語単位検索のためのノードを生成する
//
// NOTES
// 単語単位検索のためのノードを生成する。ただし、このノードを使うのは
// simpleWordMode/exactWordModeの場合のみである。
// その他のモードの場合はOrderedDistanceを作り検索する。
//
// ARGUMENTS
// ModInvertedQueryNode*& node
//	TermLeafNodeから生成されたノード。WordNodeはこのノードを子ノードにセットする
//
// ModSize _wordLenght
//	もととなった検索語の長さ
//
// ModInvertedTermMatchMode _matchMode
//	マッチモード
//
// ModInvertedQuery& query
//
// ModInvertedFile* invertedFile,
//
// const ModInvertedQuery::ValidateMode vmode,
//	有効化モード
//
// ModInvertedSmartLocationList* _emptyLocationList
//	検索語内区切り文字位置情報。exactWordModeの時に、WordNodeLocationList
//	で使用する
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedOperatorWordNode::ModInvertedOperatorWordNode(
	ModInvertedQueryNode*& node,
	ModSize _wordLenght,
	ModInvertedTermMatchMode _matchMode,
	ModInvertedQuery& query,
	ModInvertedFile* invertedFile,
	const ModInvertedQuery::ValidateMode vmode,
	const ModUInt32 resultType_,
	ModInvertedSmartLocationList* _emptyLocationList
	)
	: wordLength(_wordLenght),
	  emptyStringNode(0),
	  boundaryOfTermString(0),
	  matchMode(_matchMode),
	  ModInvertedWordBaseNode(ModInvertedAtomicNode::operatorWordNode,resultType_)
{
	// 単語単位検索は空文字列を用いて行う
	ModUnicodeString emptyString;

#ifdef SYD_INVERTED
	ModInvertedList* invertedList = invertedFile->getInvertedList();
#else
	ModInvertedList* invertedList = new ModInvertedList();
#endif
	if (invertedFile->getInvertedList(emptyString,
										*invertedList,
										ModInvertedListSearchMode)
		== ModFalse) {
		// 空文字列が転置ファイルになかった
		// 単語単位検索は出来ない。
		delete invertedList;
		; ModAssert(0);
	}

	if (query.getSimpleTokenNode(emptyString, 0, emptyString, invertedList,
								 emptyStringNode, vmode) == ModTrue) {
		delete invertedList;
	}
	static_cast<SimpleTokenLeafNode*>(emptyStringNode)->setIterator();

	// TermLeafNodeから生成されたノードを子ノードにセット
	this->insertChild(node);

	// コピーを作る
	if (matchMode == ModInvertedTermExactWordMode
#ifndef MOD_DIST // APPMODE
		|| matchMode == ModInvertedTermApproximateMode
#endif
		) {
		// _emptyLocationList が 0 であってもそのままセットする
		// simpleWordModeへの変換は行わない
		//
		// WordNodeLocationListIteratorの中で動作を変更する
		//
	
		// 検索語内区切文字位置情報をセットする
		boundaryOfTermString = _emptyLocationList;
	}

	setTotalDocumentFrequency(query.getTotalDocumentFrequency());
}

//
// FUNCTION public
// ModInvertedOperatorWordNode::~ModInvertedOperatorWordNode() -- WordNodeの廃棄
//
// NOTES
// WordNodeの破棄。
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
ModInvertedOperatorWordNode::~ModInvertedOperatorWordNode()
{
	// 検索語内区切り文字位置情報の破棄
	delete boundaryOfTermString;
	boundaryOfTermString = 0;

	delete scoreCalculator;
	scoreCalculator = 0;
}

#ifndef DEL_BOOL
//
// FUNCTION public
// ModInvertedOperatorWordNode::retrieve -- 検索の一括実行
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
ModInvertedOperatorWordNode::retrieve(ModInvertedBooleanResult& queryResult,
									Query::EvaluateMode mode)
{
	// 検索結果をまず空にする。
	if (!queryResult.isEmpty()) {
		queryResult.clear();
	}

	estimateDocumentFrequency();

	// UndefinedFrequency は実際にはQueryNodeで持っているが、
	// あいまい除去のためAtomicNodeにしている
	if (estimatedDocumentFrequency != AtomicNode::UndefinedFrequency) {
		// 出現頻度と同じ分だけ予約しておく
		queryResult.reserve(estimatedDocumentFrequency);
	}

	ModInvertedDocumentID currentID = 1;
	// はじめに粗い評価
	while (lowerBound(currentID, currentID, mode|Query::roughEvaluationFlag)
		   == ModTrue) {
		// 正確な評価
		if (this->reevaluate(currentID) == ModTrue) {
			// reevaluate() が True だったら結果を格納
			queryResult.pushBack(currentID);
		}
		currentID++;
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
// ModInvertedOperatorWordNode::evaluate -- 与えられた文書が検索式を満たすかどうかの検査
//
// NOTES
// 与えられた文書が検索式を満たすかどうか検査する。
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
//
ModBoolean
ModInvertedOperatorWordNode::evaluate(ModInvertedDocumentID documentID,
									ModInvertedQuery::EvaluateMode mode)
{
	// 既に調査済みかチェック
	if (documentID >= this->lower) {
		if (documentID == this->upper) {
			return ModTrue;			 // 調査済みだった
		} else if (documentID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		}
	}

	// 最初にラフモードで評価
	if (this->queryNodeForRoughEvaluation != 0 &&
		this->queryNodeForRoughEvaluation 
		!= static_cast<ModInvertedQueryInternalNode*>(this)) {
		if (this->queryNodeForRoughEvaluation->evaluate(
						documentID,
#ifdef ROUGH_MODE_FLAG
						mode
#else  // ROUGH_MODE_FLAG ラフノードへはラフフラグをoffにする
						mode & ~Query::roughEvaluationFlag
#endif // ROUGH_MODE_FLAG
			) == ModFalse) {
			return ModFalse;
		}
	} else {
		if (children[0]->evaluate(documentID, mode|Query::roughEvaluationFlag)
			== ModFalse) {
			return ModFalse;
		}
	}

	if ((mode & Query::roughEvaluationFlag) != 0) {
		return ModTrue;				 // 粗い検索の場合ここまで
	}

	// 正確なを行なう
	if (this->reevaluate(documentID) == ModTrue) {
		this->upper = this->lower = documentID;
		return ModTrue;
	}
	return ModFalse;
}

//
// FUNCTION public
// ModInvertedOperatorWordNode::lowerBound -- 検索式を満たす文書のうち、文書IDが与えられた値以上で最小の文書の検索
//
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する。
//
// 検索結果をupper/lower両変数に保存する。lowerには引数givenDocumentID
// を保存し、upperには検索結果であるfoundDocumentIDを保存する。
// そうすることで lowerBound() が次回呼ばれた時に givenDocumentID が
// lower <= givenDocumentID <= upper であった場合検索結果として upper
// を返す。
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
// ModInvertedErrorRetrieveFail
//
ModBoolean
ModInvertedOperatorWordNode::lowerBound(ModInvertedDocumentID givenID,
								ModInvertedDocumentID& foundID,
								ModInvertedQuery::EvaluateMode mode)
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

	ModSize currentID(givenID);

	// 最初にラフモードで評価
	if (this->queryNodeForRoughEvaluation != 0 &&
		this->queryNodeForRoughEvaluation
		!= static_cast<ModInvertedQueryInternalNode*>(this)) {
		while (this->queryNodeForRoughEvaluation->lowerBound(
			currentID, foundID, mode&~Query::roughEvaluationFlag) == ModTrue) {
			if ((mode & Query::roughEvaluationFlag) != 0) {
				return ModTrue;
			}
			if (this->reevaluate(foundID) == ModTrue) {
				this->upper = this->lower =foundID;
				this->lower = givenID;
				return ModTrue;
			}
			currentID = foundID+1;
		}
	} else {
		while (children[0]->lowerBound(
			currentID, foundID, mode|Query::roughEvaluationFlag) == ModTrue) {
			if ((mode & Query::roughEvaluationFlag) != 0) {
				return ModTrue;
			}
			if (this->reevaluate(foundID) == ModTrue) {
				this->upper = this->lower =foundID;
				this->lower = givenID;
				return ModTrue;
			}
			currentID = foundID+1;
		}
	}
	this->lower = givenID;
	this->upper = ModSizeMax;
	return ModFalse;
}

//
// FUNCTION public
// ModInvertedOperatorWordNode::prefixString -- 演算子を表わす文字列(#word)を返す
//
// NOTES
// QueryNodeで定義された内容をオーバライドする
// 演算子を表わす文字列(#word)を返す
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
ModInvertedOperatorWordNode::prefixString(ModUnicodeString& prefix,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam) const
{
	prefix += "#word";
	if (withCalOrCombName == ModTrue) {
		// calculator/combinerどちらを使うか未定

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
// ModInvertedQueryBaseNode::getTermFrequency -- 文書内頻度の取得
//
// NOTES
// QueryBaseNode::getTermFrequency()をオーバーライド。
// ただし、Approximate Mode以外の場合はBaseNode::getTermFrequency()を
// 行う。
//
// Approximate Modeの場合は各照合位置で照合モードを取得し、
// 照合モードに応じて以下のようなweightを付けたTFを返す。
//	厳格一致で照合					weight = 10
//	先頭一致または終端一致で照合	weigth = 5
//	文字列として一致				weight = 1
//
// 本当は上記のweigthの1/10の値をセットしたかったが、スコア計算機に渡す
// TFはModSizeであるため、上記のようにした。
//
// ただしデフォルトのスコア計算機の場合TFを使うのは以下の部分の式である
//		tf/(tf+k)
// このためTFを10倍にしてもそれほど大きな影響はないと思われる。
//
// ※正確なsocreを求めたい場合はスコア計算機の指定でKの値も10にすると良い
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
// なし
//
ModSize
ModInvertedOperatorWordNode::getTermFrequency(
	DocumentID documentID,
	Query::EvaluateMode mode)
{
#ifdef MOD_DIST // APPMODE
	return ModInvertedQueryInternalNode::getTermFrequency(documentID, mode);
#else
	if (matchMode != ModInvertedTermApproximateMode) {
		// appMode以外のケース
		return ModInvertedQueryInternalNode::getTermFrequency(
			documentID, mode);
	}

	// appModeの場合（appMode は関連文書検索の実験用）

	ModInvertedLocationListIterator* locations = 0;
	ModInvertedTermMatchMode currentMatchMode;

	ModSize tf = 0;
	if (reevaluate(documentID, locations, tf) == ModTrue) {
		if (locations != 0)
		{
			// 位置情報リストを取得できた
			
			ModSize weight ;

			// WordNodeLocationListIterator::getCurrentMatchType()を使うため
			ModInvertedOperatorWordNodeLocationListIterator* wloc =
				static_cast<ModInvertedOperatorWordNodeLocationListIterator*>(
					locations);

			for (; locations->isEnd() == ModFalse; ++*locations) {

				currentMatchMode = wloc->getCurrentMatchType();

				// weigthを実際に使いたい値の10倍にしているが大きな影響が無い
				//	tf/(tf+k)
				// もし正しい値を求める場合はスコア計算機の指定でkを10倍にする
				//
				if (currentMatchMode == ModInvertedTermExactWordMode) {
					// 厳格一致
					// 本当は1.0にしたいがTFはModSizeなので10倍の値を渡す
					weight = 10;
				} else if (currentMatchMode == ModInvertedTermWordTail) {
					// Tail
					// 本当は0.5にしたいがTFはModSizeなので10倍の値を渡す
					weight = 5;
				} else if (currentMatchMode == ModInvertedTermWordHead) {
					// Head
					// 本当は0.5にしたいがTFはModSizeなので10倍の値を渡す
					weight = 5;
				} else {
					// wordMode
					// 本当は0.1にしたいがTFはModSizeなので10倍の値を渡す
					weight = 1;
				}
#ifdef DEBUG
				ModDebugMessage << "WordNode[approximateMode]::weight("
								<< "ID=" << documentID
								<< ",LOC=" << wloc->getLocation()
								<< ")=" << weight
								<< "[mode =" << currentMatchMode << "]"
								<< ModEndl;
#endif
				tf += weight;
			}
			locations->release();
		}
		else
		{
			// 位置情報リストは取得できなかったが、TFを取得できた
			; ModAssert(tf > 0);
			
			// 位置情報リストが使えないので、wordModeとみなす。
			// したがって、weightは付けずにそのままtfを返す。
		}
#ifdef DEBUG
		ModDebugMessage << "WordNode[approximateMode]::getTermFrequency("
						<< documentID << ") TF =" << tf << ModEndl;
#endif
		return tf;
	}
	return 0;
#endif
}

//
// FUNCTION protected
// ModInvertedOperatorWordNode::reevaluate -- 正確な再 evaluate
//
// NOTES
// 粗い evaluate 後の、正確な再 evaluate。出現位置の検査のみを行なう。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
//
// RETURN
// 出現位置検査を満足すれば ModTrue、しなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModInvertedOperatorWordNode::reevaluate(ModInvertedDocumentID documentID)
{
	// 子ノードの出現位置
	ModInvertedLocationListIterator* childLocation;

	// 空文字列ノードの出現位置
	ModInvertedLocationListIterator* emptyStringNodeLocation;

	if (children[0]->reevaluate(documentID, childLocation) != ModTrue) {
		return ModFalse;
	}

	if (emptyStringNode->evaluate(documentID, emptyStringNodeLocation,
								  ModInvertedQueryInternalNode::defaultEMode, 0)
		!= ModTrue) {
		// 削除のバグがあったのでここに来ることがある
		childLocation->release();
		return ModFalse;
	}

	ModInvertedOperatorWordNodeLocationListIterator* iterator
		= static_cast<ModInvertedOperatorWordNodeLocationListIterator*>(getFreeList());
	if (iterator == 0)
	{
		iterator = new ModInvertedOperatorWordNodeLocationListIterator(this,
						 matchMode,
						 wordLength,
						 boundaryOfTermString);
	}
	LocationIterator::AutoPointer p = iterator;
	
	iterator->initialize(childLocation,
						 emptyStringNodeLocation);
	if (iterator->isEnd() != ModTrue) {
		return ModTrue;
	}
	return ModFalse;
}

//
// FUNCTION protected
// ModInvertedOperatorWordNode::reevaluate -- 正確な再 evaluate と位置情報の獲得
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
//		(位置情報リストを取得できなかった場合) TF
// ModInvertedQueryNode* givenEndNode
//		ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// 出現位置検査を満足すれば ModTrue、しなければ ModFalse
//
// EXCEPTIONS
ModBoolean
ModInvertedOperatorWordNode::reevaluate(ModInvertedDocumentID documentID,
										LocationIterator*& locations,
										ModSize& uiTF_,
										ModInvertedQueryNode* givenEndNode)
{
	// 子ノードの出現位置
	ModInvertedLocationListIterator* childLocation;

	// 空文字列ノードの出現位置
	ModInvertedLocationListIterator* emptyStringNodeLocation;

	if (children[0]->reevaluate(documentID, childLocation, uiTF_) != ModTrue) {
		return ModFalse;
	}
	if (childLocation == 0 && uiTF_ > 0)
	{
		// 位置情報リストを取得できなかったが、TFを取得できたので、
		// 位置を確認することはできないが、条件を満たしたとみなす。
		return ModTrue;
	}
	// ここに来るなら、位置情報リストは取得できているはず。
	; ModAssert(childLocation != 0);

	if (emptyStringNode->evaluate(documentID, emptyStringNodeLocation,
								  ModInvertedQueryInternalNode::defaultEMode, 0)
		!= ModTrue) {
		// 削除のバグがあったのでここに来ることがある
		childLocation->release();
		return ModFalse;
	}
	// ここに来るなら、位置情報リストは取得できているはず。
	; ModAssert(emptyStringNodeLocation != 0);

	ModInvertedOperatorWordNodeLocationListIterator* iterator
		= static_cast<ModInvertedOperatorWordNodeLocationListIterator*>(getFreeList());
	if (iterator == 0)
	{
		iterator = new ModInvertedOperatorWordNodeLocationListIterator(this,
						 matchMode, 
						 wordLength,
						 boundaryOfTermString);
	}
	LocationIterator::AutoPointer p = iterator;
	
	iterator->initialize(childLocation,
						 emptyStringNodeLocation);

	if (iterator->isEnd() != ModTrue) {
		locations = p.release();
		return ModTrue;			 // 条件を満足
	}
	return ModFalse;		// 条件を不満足
}


//
// FUNCTION public
// ModInvertedOperatorWordNode::setOriginalString() -- 自分が生成された元のTermLeafNodeの文字列をメンバ変数originalTermStringにセットする
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
ModInvertedOperatorWordNode::setOriginalString(const ModUnicodeString& termString,
#ifdef V1_6
											   const ModLanguageSet& langSet_,
#endif // V1_6
											   const ModInvertedTermMatchMode& mmode_)
{
	originalTermString = termString;
#ifdef V1_6
	langSet = langSet_;
#endif // V1_6
	matchMode = mmode_;
}

//
// FUNCTION public
// ModInvertedOperatorWordNode::getOriginalString() -- 自分が生成された元のTermLeafNodeの文字列を返す。
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
// originalTermStringがある場合True/ない場合False
//
// EXCEPTIONS
// なし
//
ModBoolean
ModInvertedOperatorWordNode::getOriginalString(ModUnicodeString& termString,
#ifdef V1_6
											   ModLanguageSet& langSet_,
#endif // V1_6
											   ModInvertedTermMatchMode& mmode_) const
{
	if (originalTermString.getLength() != 0) {
		termString = originalTermString;
#ifdef V1_6
		langSet_ = langSet;
#endif // V1_6
		mmode_ = matchMode;
		return ModTrue;
	}

	// originalTermStringがセットされていないのでFalseを返す
	return ModFalse;
}

//
// FUNCTION public
// ModInvertedOperatorWordNode::checkQueryNode -- 子ノードの数をチェックする
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
// ModInvertedErrorQueryValidateFail
//子ノード数が2未満であった
//
void
ModInvertedOperatorWordNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_
	)
{
	if (needDF_ == ModTrue) {
		setNeedDF(ModTrue);
	}

	ModUnicodeString termString;
	if (setStringInChildren_ == ModTrue) {
		getQueryString(termString, ModTrue, ModTrue, ModTrue, ModFalse);
		query_->insertTermNode(termString,
						static_cast<ModInvertedQueryInternalNode*>(const_cast<ModInvertedOperatorWordNode*>(this)));

		ModSize avelen(query_->getAverageDocumentLength());
		if (avelen != 0) {
			ScoreCalculator* calculator = getScoreCalculator();
			; ModAssert(calculator != 0);
			calculator->setAverageDocumentLength(avelen);
		}
	}

	//子ノードに対してcheckQueryNodeを呼ぶ
	this->children[0]->checkQueryNode(query_, ModFalse, ModFalse);
}

//
// FUNCTION protected
// ModInvertedOperatorWordNode::getQueryString -- 検索条件ノードを出力
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
ModInvertedOperatorWordNode::getQueryString(ModUnicodeString& out,
	const ModBoolean asTermString,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam,
	const ModBoolean withRouh) const
{
	ModUnicodeString termString;
#ifdef V1_6
	ModLanguageSet tmpLangSet;
#endif // V1_6
	ModInvertedTermMatchMode tmpMmode;
	if (asTermString == ModTrue && getOriginalString(termString,
#ifdef V1_6
													 tmpLangSet,
#endif // V1_6
													 tmpMmode) != ModFalse) {
		// マッチモードを出力
		out += "#term[";
		if (matchMode == ModInvertedTermSimpleWordMode) {
			out += 's';
		} else if (matchMode == ModInvertedTermExactWordMode) {
			out += 'e';
		} else if (matchMode == ModInvertedTermWordHead) {
			out += 'h';
		} else if (matchMode == ModInvertedTermWordTail) {
			out += 't';
#ifndef MOD_DIST // APPMODE
		} else if (matchMode == ModInvertedTermApproximateMode) {
			out += 'a';
#endif
		}

		// 計算器名を出力
#ifdef V1_6
		out += ',';
#endif // V1_6
		if (withCalOrCombName == ModTrue) {
			ModUnicodeString calculatorName;
			getCalculatorOrCombinerName(calculatorName, withCalOrCombParam);
	
			if (calculatorName.getLength() > 0) {
#ifndef V1_6
				out += ',';
#endif // NOT V1_6
				out += calculatorName;
			}
		}

#ifdef V1_6
		// 言語セットを出力
		ModUnicodeString langSetName;
		langSetName = tmpLangSet.getName();
		if(langSetName.getLength() > 0) {
			out += ',';
			out += langSetName;
		}
#endif // V1_6

		out += ']';
		out += '(';
		ModUnicodeString tmp;
		ModInvertedQueryParser::convertTermString(termString , tmp);
		out += tmp;
		out += ')';
	} else {
		ModInvertedQueryInternalNode::getQueryString(out,
													 asTermString,
													 withCalOrCombName,
													 withCalOrCombParam,
													 withRouh);
	}
}

#endif // V1_4 単語単位検索

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
