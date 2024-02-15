// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedResultLeafNode.cpp -- 検索式ノードの実装
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2009, 2023 Ricoh Company, Ltd.
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

#include "ModInvertedSearchResult.h"
#include "ModInvertedBooleanResultLeafNode.h"
#include "ModOstrStream.h"
#include "ModInvertedFileCapsule.h"

#ifdef	SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedFile.h"
#include "Inverted/ModInvertedDocumentLengthFile.h"
#else
#include "ModInvertedFile.h"
#include "ModInvertedDocumentLengthFile.h"
#endif

//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::ModInvertedBooleanResultLeafNode -- コンストラクタ
//
// NOTES
// コンストラクタ
//
// ARGUMENTS
// const ModInvertedBooleanResultLeafNode& originalNode
//		コピー元
// const ModInvertedBooleanResult& result
//		もととなる検索結果
// ModInvertedBooleanResult::Iterator begin_
//		もととなる検索結果の先頭
// ModInvertedBooleanResult::Iterator end_
//		もととなる検索結果の末尾
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModInvertedBooleanResultLeafNode::ModInvertedBooleanResultLeafNode(const ModUInt32 resultType_)
	: searchResult(0), current(0),
		ModInvertedQueryLeafNode(ModInvertedQueryNode::booleanResultLeafNode,resultType_)
{}

ModInvertedBooleanResultLeafNode::ModInvertedBooleanResultLeafNode(
	const ModInvertedBooleanResultLeafNode* originalNode,const ModUInt32 resultType_)
	: ModInvertedQueryLeafNode(ModInvertedQueryNode::booleanResultLeafNode,resultType_)
{
	searchResult =	originalNode->searchResult->_create();
	searchResult->copy(originalNode->searchResult);

	current = begin = 0;
	end = estimatedDocumentFrequency = searchResult->getSize();
	// オリジナルにおいてソートされているはずなのでソートはしない
}

ModInvertedBooleanResultLeafNode::ModInvertedBooleanResultLeafNode(
	const ModInvertedSearchResult* result,const ModUInt32 resultType_)
	: ModInvertedQueryLeafNode(ModInvertedQueryNode::booleanResultLeafNode,resultType_)
{
	searchResult = result->_create();
	searchResult->copy((ModInvertedBooleanResult*)result);
	current = 0;

	begin = 0;
	end = estimatedDocumentFrequency = searchResult->getSize();
	// オリジナルのソースに従い、booleanの時だけ、sortする
	// 理由は分からない
	if(searchResult->getType() == (1 << _SYDNEY::Inverted::FieldType::Rowid))
		 searchResult->sort();
}

//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::~ModInvertedBooleanResultLeafNode -- デストラクタ
//
// NOTES
// デストラクタ
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
ModInvertedBooleanResultLeafNode::~ModInvertedBooleanResultLeafNode()
{
	delete searchResult;
	searchResult = 0;
}

#ifndef DEL_BOOL
//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::retrieve -- 一括検索
//
// NOTES
// 一括検索を実行する。
// 検索結果は引数queryResultにセットして返す。
//
// ARGUMENTS
// BooleanResult& queryResult
//		検索結果格納用
// Query::EvaluateMode mode
//		評価モード。未使用
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedBooleanResultLeafNode::retrieve(BooleanResult& queryResult,
											 Query::EvaluateMode mode)
{
	queryResult.clear();
	queryResult.reserve(getSize());	 // 不必要に大きな領域を確保しないよう
										// にする
	// 初期化
	if (this->upper != 0) {
		this->upper = 0;
		this->lower = 1;
	}

	// assignはコピーではない。オリジナルがなくなってしまうのでだめ
	for (ModSize i(begin); i < end; ++i) {
		queryResult.pushBack(searchResult->getDocID(i));
	}
}
#endif
//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::evaluate -- 与えられた文書が検索式を満たすかどうかの検査
//
// NOTES
//
// ARGUMENTS
// DocumentID documentID
//		文書ID
// Query::EvaluateMode mode
//		評価モード。未使用
//
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedBooleanResultLeafNode::evaluate(DocumentID documentID,
											 Query::EvaluateMode mode)
{
	ModSize	i(begin);

	if (documentID >= this->lower) {
		if (documentID == this->upper) {
			// 見つけてあった
			return ModTrue;
		} else if (documentID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		}
		// カレントの次から探してみる
		if (current >= 0 ) { // valid ?
			i = current, ++i;
		}
	}

	for (; i != end && searchResult->getDocID(i) <= documentID; ++i) {
		if (searchResult->getDocID(i) == documentID) {
			lower = upper = documentID;
			current = i;
			return ModTrue;
		}
	}

	return ModFalse;
}

//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::lowerBound -- 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す
//
// NOTES
// 文書IDが与えられた値以上で、検索式を満たす文書の内、文書ID最小のものを
// 検索し、そのような文書が存在する場合は、与えられた文書IDオブジェクトに
// 結果を格納する。
//
// ARGUMENTS
// ModInvertedDocumentID givenID
//		入力文書ID
// ModInvertedDocumentID& foundID
//		検索結果格納用文書ID
// Query::EvaluateMode mode
//		評価モード。未使用
//
// RETURN
// そのような文書が存在する場合 ModTrue、存在しない場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedBooleanResultLeafNode::lowerBound(ModInvertedDocumentID givenID,
											 ModInvertedDocumentID& foundID,
											 Query::EvaluateMode mode)
{
	ModSize i(begin);

	// 既に調査済み？
	if (givenID >= lower) {
		if (this->upper == ModSizeMax) {
			return ModFalse;		// 既にEndに達っしている
		}
		if (givenID <= this->upper) {
			foundID = this->upper;
			return ModTrue;
		}
		// カレントの次から探してみる
		if (current >= 0) {
			i = current, ++i;
		}
	}

	lower = givenID;

	for (; i < end; ++i) {
		if (searchResult->getDocID(i) >= givenID) {
			foundID = searchResult->getDocID(i);
			upper = foundID;
			current = i;
			return ModTrue;
		}
	}

	upper = ModSizeMax;
	return ModFalse;
}


//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::evaluateScore -- 与えられた文書のスコアを計算する
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
ModInvertedBooleanResultLeafNode::evaluateScore(const DocumentID documentID,
												DocumentScore& score,
												Query::EvaluateMode mode)
{
	if (evaluate(documentID, mode) != ModTrue) {
		return ModFalse;
	}
	score = searchResult->getScore(current);						// スコアは 1.0 で固定
	return ModTrue;
}

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::evaluateScore -- 与えられた文書のスコアを計算する（位置も計算する）
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
//		ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// ブーリアン検索条件にマッチしていれば True, アンマッチであれば False
//
// EXCEPTIONS
// ModInvertedErrorInternal -- 内部的な不整合
//
ModBoolean
ModInvertedBooleanResultLeafNode::evaluateScore(const DocumentID ID,
												DocumentScore& score,
												LocationIterator*& locations,
												Query::EvaluateMode mode,
												ModInvertedQueryNode* givenEndNode)
{
	// この関数を実行するのは RankingResultLeafNode が #window[,,]( )
	// の中に指定された場合であるが、現在の所この機能はサポートしていない。

	// ここに到達するのはどこかおかしい
	ModThrowInvertedFileError(ModInvertedErrorInternal);

	return ModTrue;
}
#endif

//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::lowerBoundScore -- lowerBoundのランキング版（スコアも計算する）
//
// NOTES
// BooleanResultLeafNode::lowerBound()を実行し、Tureだったらスコア計算をする。
//
// ARGUMENTS
// ModInvertedDocumentID givenID
//		文書ID
// ModInvertedDocumentID& foundID
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
ModInvertedBooleanResultLeafNode::lowerBoundScore(const DocumentID givenID,
													DocumentID& foundID,
													DocumentScore& score,
													Query::EvaluateMode mode)
{
	if (lowerBound(givenID, foundID, mode) != ModTrue) {
		return ModFalse;
	}
	score = searchResult->getScore(current);					 // スコアは 1.0 で固定
	return ModTrue;
}

//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::duplicate -- 自分のコピーを作成する
//
// NOTES
// 自分のコピーを作成する。Query::から呼ばれ、検索木を上からたどり、
// 検索木のコピーを作成する際に使用される。
// QueryNode::dupulicateをオーバーライド。BooleafResultLeafNodeのコピーを作る。
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
ModInvertedBooleanResultLeafNode::duplicate(const ModInvertedQuery& rQuery)
{
	return new ModInvertedBooleanResultLeafNode(this,firstStepResult->getType());
}

//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::isEmptyResultLeafNode -- 空かどうかのチェック
//
// NOTES
// 空集合かどうかのチェック。空集合(begin = end)の場合はModTrueを返す。
//
// ARGUMENTS
// なし
//
// RETURN
// 空集合の場合はModTrueを返す。それ以外の場合はModFalseを返す。
//
// EXCEPTIONS
// なし
//
ModBoolean
ModInvertedBooleanResultLeafNode::isEmptyResultLeafNode() const
{
	return (this->begin == this->end) ? ModTrue : ModFalse;
}

//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::prefixString -- 演算子を表わす文字列を返す
//
// NOTES
// ノードの演算子等を表わす文字列 "#list" を返す。
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
ModInvertedBooleanResultLeafNode::prefixString(ModUnicodeString& prefix,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam) const
{
	prefix = "#list";
}

//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::contentString -- ノードの内容を表わす文字列を返す
//
// NOTES
// ノードの内容を表わす文字列を返す
//
// ARGUMENTS
// ModString& prefix
//		ノードの内容を表わす文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedBooleanResultLeafNode::contentString(ModUnicodeString& content) const
{
	ModOstrStream out;
	ModSize i = this->begin;

	if (i != this->end) {
		out << searchResult->getDocID(i);
		for (++i; i != this->end; ++i) {
			out << ',' << searchResult->getDocID(i);
		}
	}
	content = ModUnicodeString(out.getString());
}

//
// FUNCTION protected
// ModInvertedBooleanResultLeafNode::validate -- ノードの有効化
//
// NOTES
// ノードの有効化。BooleanResultLeafNodeは有効化といってもorStanderedSharedNode
// へ登録しているだけ。
//
// ARGUMENTS
// ModInvertedQuery& query
//		検索式内部表現オブジェクト
// ModInvertedFile* invertedFile
//		転置ファイルへのポインタ
// validateMode mode
//		有効化のモード
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedBooleanResultLeafNode::validate(ModInvertedFile* invertedFile,
											 const Query::ValidateMode mode,
											 Query* rQuery)
{
	if (this == QueryNode::emptySetNode) {
		// 空集合は登録しない
		return;
	}

	// この小転置に入っていないdocIDが検索対象に含まれていた場合は削除する

#if 1 // このコードだと、文書が削除されたかどうか関係なくhit判定される。
		// else側にすればこれは回避できるが、現状このままにする。
	ModBoolean isExpunge(ModFalse);
	ModSize i = this->begin;
	while(i != this->end) {

		if(searchResult->getDocID(i) < invertedFile->getMinDocumentID()) {
			isExpunge = ModTrue;
		} else {
			break;
		}
		++i;
	}
	if(isExpunge == ModTrue) {
		searchResult->erase(0, i);
		begin = 0;
		end = searchResult->getSize();
		isExpunge = ModFalse;
	}

	if(this->getSize() == 0) {
		return;
	}

	i = this->end;
	while(1) {
		--i;
		if(searchResult->getDocID(i) > invertedFile->getLastDocumentID()) {
			isExpunge = ModTrue;
		} else {
			break;
		}
		if(i == this->begin) {
			--i;
			break;
		}
	}
	if (isExpunge == ModTrue) {
		searchResult->erase(i + 1, this->end);
		begin = 0;
		estimatedDocumentFrequency = end = searchResult->getSize();
	}
#else
	ModInvertedBooleanResult::Iterator p = this->begin;
	ModAutoPointer<ModInvertedDocumentIterator> p2(invertedFile->begin());
	while(p != this->end) {
		if(*p < invertedFile->getMinDocumentID() ||
			 *p > invertedFile->getLastDocumentID() ||
			 p2->find(*p) != ModTrue) {
			searchResult->erase(p);
			begin = searchResult->begin();
			end = searchResult->end();
			estimatedDocumentFrequency = searchResult->getSize();
		} else {
			++p;
		}
	}
#endif

	// orStanderedSharedNode へ登録する。
	rQuery->addOrStanderdSharedNode(this);
}

//
// FUNCTION protected
// ModInvertedBooleanResultLeafNode::reevaluate -- 与えられた文書が検索式を満たすかどうかの正確な再評価
//
// NOTES
// 与えられた文書が検索式を満たすかどうか検査する。
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
inline ModBoolean
ModInvertedBooleanResultLeafNode::reevaluate(ModInvertedDocumentID documentID)
{
	return evaluate(documentID, 0);
}

//
// FUNCTION protected
// ModInvertedBooleanResultLeafNode::reevaluate -- 与えられた文書が検索式を満たすかどうかの正確な再評価
//
// NOTES
// 与えられた文書が検索式を満たすかどうか検査する。
// 検索式を満たす場合、文書中の出現位置の情報も求める。(未実装)
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedLocationListIterator*& locations
//		出現位置反復子へのポインタ (結果格納用)
// ModInvertedQueryNode* givenEndNode
//		ここでは未使用。orderedDistanceでのみ使用。
//
// RETURN
// 与えられた文書が検索式を満たす場合 ModTrue、満たさない場合 ModFalse
//
// EXCEPTIONS
// ModInvertedErrorInternal -- 内部的な不整合
//
ModBoolean
ModInvertedBooleanResultLeafNode::reevaluate(ModInvertedDocumentID documentID,
											 LocationIterator*& locations,
											 ModSize& uiTF_,
											 ModInvertedQueryNode* givenEndNode)
{
	// この関数を実行するのは RankingResultLeafNode が #window[,,]( )
	// の中に指定された場合であるが、現在の所この機能はサポートしていない。

	// ここに到達するのはどこかおかしい
	ModThrowInvertedFileError(ModInvertedErrorInternal);

	return ModTrue;
}

//
// FUNCTION public
// ModInvertedBooleanResultLeafNode:::doFirstStepInRetrieveScore -- スコア計算の第１ステップを実行する
//
// NOTES
//
// ARGUMENTS
// const ModInvertedDocumentID maxDocumentId
//			ドキュメントID
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedBooleanResultLeafNode::doFirstStepInRetrieveScore(
	ModInvertedBooleanResult *expungedDocumentId,
	const ModInvertedDocumentID maxDocumentId)
{
	if(getFirstStepStatus() == firstDone) {
		// すでに呼ばれている
		return;
	}
	setFirstStepStatus(firstDone);
}

//
// FUNCTION
// ModInvertedRankingResultLeafNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
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
ModInvertedBooleanResultLeafNode::doSecondStepInRetrieveScore()
{
	setFirstStepStatus(secondDone);
}

//
// FUNCTION
// ModInvertedNodeBooleanResultLeafNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
//
// NOTES
// ランキング検索で、スコア計算の第２ステップのみを実施する。
// 第１ステップの結果を用い、最終的な検索結果を生成する。
// 第１ステップを実施していない場合の結果は不定。
//
// ARGUMENTS
// ModInvertedRankingResult* result_
//		検索結果
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedBooleanResultLeafNode::doSecondStepInRetrieveScore(
	ModInvertedSearchResult* &result_)
{
	result_ = searchResult;
}

//
// FUNCTION
// ModInvertedBooleanResultLeafNode::lowerBoundScoreForSecondStep() -- ランキング検索の第２ステップで使用するlowerBound
//
// NOTES
// ランキング検索のスコア計算第２ステップで使用されるlowerBound。
//
// ARGUMENTS
// ModInvertedDocumentID givenID,
//		文書IDの下限
// ModInvertedDocumentID& foundID,
//		結果格納用の文書IDオブジェクト
// ModInvertedDocumentScore& score);
//		スコア(結果格納用)
//
// RETURN
// 求めたスコア
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedBooleanResultLeafNode::lowerBoundScoreForSecondStep(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
	Query::EvaluateMode mode =
		ModInvertedFileCapsuleRankingSearch::defaultEvaluateMode;
	if (this->lowerBoundScore(givenID, foundID,score, mode) != ModTrue) {
		return ModFalse;
	}
	return ModTrue;
}

#if 0
//
// FUNCTION public
// ModInvertedBooleanResultLeafNode::removeFromFirstStepResult -- 第１ステップ結果からの削除
//
// NOTES
// ランキング検索の第１ステップを実施した後で、ノードの結果から bresult を
// 削除する。
//
// ARGUMENTS
// const ModInvertedSearchResult* bresult
//							削除する文書IDの集合
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedBooleanResultLeafNode::removeFromFirstStepResult(
	const ModInvertedSearchResult* bresult)
{
	ModSize i(begin);
	for( ModSize j = 0 ; j < bresult->getSize() && i < end ; j++){
		if(bresult->getDocID(i) == bresult->getDocID(j)) {
			searchResult->erase(i);
			begin = 0;
			end	 = estimatedDocumentFrequency = searchResult->getSize();
			++j;
	} else {
			if(bresult->getDocID(i) < bresult->getDocID(j)) {
				 ++i;
		} else {
				 ++j;
		}
	}
 }
}
#endif
//
// FUNCTION public
// ModInvertedBoleanResultLeafNode::checkQueryNode -- 子ノードの数をチェックする
//
// NOTES
// 有効化の最後に呼び出されて、子ノードの数をチェックする。もし異常で
// あれば例外を投げる。
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
ModInvertedBooleanResultLeafNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_
)
{
	if(needDF_ == ModTrue) {
		setNeedDF(needDF_);
	}

	ModUnicodeString token,tmp;
	this->prefixString(token, ModFalse, ModFalse);
	this->contentString(tmp);
	token += '(';
	token += tmp;
	token += ')';
	query_->insertTermNode(token,
							 const_cast<ModInvertedBooleanResultLeafNode*>(this));
}


//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
