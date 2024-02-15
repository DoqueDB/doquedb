// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedSimpleTokenLeafNode.cpp -- 索引語に対応する末端ノードの実装
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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
#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedList.h"
#else
#include "ModInvertedList.h"
#endif


#include "ModInvertedSearchResult.h"
#include "ModInvertedSimpleTokenLeafNode.h"
#include "ModInvertedFileCapsule.h"
#include "ModInvertedQueryParser.h"

#ifdef DEBUG

//
// VARIABLE
// ModInvertedSimpleTokenLeafNode::countLowerBound -- 下限検索の回数
//
/*static*/ int
ModInvertedSimpleTokenLeafNode::countLowerBound = 0;

//
// VARIABLE
// ModInvertedSimpleTokenLeafNode::countLocCheck -- 位置検査の回数
//
/*static*/ int
ModInvertedSimpleTokenLeafNode::countLocCheck = 0;
#endif

//
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::~ModInvertedSimpleTokenLeafNode -- 単一の索引語に対応する末端ノードの破棄
//
// NOTES
// 単一の索引語に対応する末端ノードの破棄
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
ModInvertedSimpleTokenLeafNode::~ModInvertedSimpleTokenLeafNode()
{
	delete this->iterator, this->iterator = 0;
	delete this->invertedList, this->invertedList = 0;
}

#ifndef DEL_BOOL
//
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::retrieve -- 索引語検索の一括実行
//
// NOTES
// 検索式を一括実行し、引数で指定されたBoolean検索結果オブジェクトに
// 結果を格納する。
//
// ARGUMENTS
// ModInvertedBooleanResult& queryResult
//		Boolean検索結果オブジェクト (結果格納用)
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
ModInvertedSimpleTokenLeafNode::retrieve(ModInvertedBooleanResult& queryResult,
										 Query::EvaluateMode mode)
{
	// 結果を空にしておく
	if (queryResult.isEmpty() == ModFalse) {
		queryResult.clear();
	}
#ifdef ITERATOR_CHECK
	if (!iterator) {
		// iteratorが設定されていないのは異常である
		ModErrorMessage << "iterator = 0" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorRetrieveFail);
	}
#endif

	try {
		// 初期化
		if (this->upper != 0) {
#ifdef DEBUG
			++countLowerBound;
#endif
			this->iterator->reset();
			this->upper = 0;
			this->lower = 1;
		}

		// 場所の確保
		queryResult.reserve(invertedList->getDocumentFrequency());

		// 文書IDの格納
		for (; this->iterator->isEnd() == ModFalse; ++(*iterator)) {
#ifdef DEBUG
			++countLocCheck;
#endif
			queryResult.pushBack(this->iterator->getDocumentId());

#ifdef DEBUG
			++countLowerBound;
#endif
		}
		this->upper = ModSizeMax;
		this->lower = this->iterator->getDocumentId();

	} catch (ModException& exception) {
		ModErrorMessage << "retrieve failed: token=" << token
						<< ": " << exception << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorRetrieveFail);
#ifndef SYD_INVERTED
	} catch (...) {
		/* purecov:begin deadcode */
		ModUnexpectedThrow(ModModuleInvertedFile);
		/* purecov:end */
#endif
	}
}
#endif

// 
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::evaluate -- 与えられた文書が検索式を満たすかどうかの検査
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
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedSimpleTokenLeafNode::evaluate(ModInvertedDocumentID documentID,
										 Query::EvaluateMode mode)
{
#ifdef ITERATOR_CHECK
	if (!iterator) {
		// iteratorが設定されていないのは異常である
		ModErrorMessage << "iterator = 0" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorRetrieveFail);
	}
#endif

	// 既に調査済み？

	// lower: 前回調査したID
	// upper: 前回調査で得られたID
	// つまり、lower以上upper未満のIDは条件を満たさず、upperは条件を満たす。
	
	if (documentID >= this->lower) {
		// 前回調査したIDより、大きいIDで再調査する場合
		if (documentID == this->upper) {
			return ModTrue;
		} else if (documentID < this->upper || this->upper == ModSizeMax) {
			return ModFalse;
		}
		// else
		// 今回調査するIDの存在が、まだ分からない場合
	}

	//
	// 初期化
	//
	if (this->iterator->isEnd() || this->lower > documentID) {
		// 前回調査したIDより、小さいIDで再調査する場合
		
		// [YET] iterator->isEnd()になるのはどんな時？
		
		this->iterator->reset();
		this->upper = 0;
		this->lower = 1;
	}
#ifdef DEBUG
	++countLowerBound;
#endif

	//
	// 調査
	//
	
	// 転置リストの lowerBound
	this->iterator->lowerBound(documentID);

	// [YET] 初期化で実行した方がわかりやすくないか？
	if (documentID > this->upper) {
		this->lower = documentID;
	}

#ifdef DEBUG
	++countLocCheck;
#endif

	//
	// 調査結果を取得
	//

	// lowerBound()を実行した結果、iterator->isEnd() になった場合、
	// getDocumentId()は、ModInvertedUpperBoundDocumentID を返す。
	this->upper = this->iterator->getDocumentId();

	//
	// 調査結果の判定
	//
	if (documentID == this->upper) {
		// 条件を満たした場合
		return ModTrue;
	}
	// 条件を満たさない場合
	if (this->iterator->isEnd() == ModTrue) {
		// [NOTE] ModInvertedUpperBoundDocumentIDとModSizeMaxは異なる。
		this->upper = ModSizeMax;
	}
	return ModFalse;
}

// 
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::lowerBound -- 検索式を満たす文書のうち、文書IDが与えられた値以上で最小の文書の検索
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
ModInvertedSimpleTokenLeafNode::lowerBound(ModInvertedDocumentID givenID,
										   ModInvertedDocumentID& foundID,
										   Query::EvaluateMode mode)
{
#ifdef ITERATOR_CHECK
	if (!iterator) {
		// iteratorが設定されていないのは異常である
		ModErrorMessage << "iterator = 0" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorRetrieveFail);
	}
#endif

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

	// 初期化
	if (this->lower > givenID || this->iterator->isEnd()) {
		this->iterator->reset();
		this->upper = 0;
		this->lower = 1;
	}

#ifdef DEBUG
	++countLowerBound;
#endif
	// 転置リストの lowerBound 呼び出し
	this->iterator->lowerBound(givenID);
	if (givenID > this->upper) {
		this->lower = givenID;
	}

#ifdef DEBUG
	++countLocCheck;
#endif
	if (this->iterator->isEnd() == ModFalse) {
		this->upper = this->iterator->getDocumentId();
		foundID = this->upper;
		return ModTrue;
	}
	this->upper = ModSizeMax;
	return ModFalse;
}

// 
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::evaluateScore -- 与えられた文書のスコアを計算する
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
ModInvertedSimpleTokenLeafNode::evaluateScore(const DocumentID ID,
													 DocumentScore& score,
													 Query::EvaluateMode mode)
{
	if (SimpleTokenLeafNode::evaluate(ID, mode) == ModTrue) {
		// evaluateに成功したのでスコアを計算する
		ModBoolean exist;
		score = (*scoreCalculator)(iterator->getInDocumentFrequency(),
								  ID, exist);
		return exist;
	}

	// evaluateに失敗したのでスコアを0.0にする
	score = 0.0;
	return ModFalse;
}

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
// 
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::evaluateScore -- 与えられた文書のスコアを計算する（位置も計算する）
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
ModInvertedSimpleTokenLeafNode::evaluateScore(
	const DocumentID documentID,
	DocumentScore& score,
	LocationIterator*& locations,
	Query::EvaluateMode mode,
	ModInvertedQueryNode* givenEndNode)
{
	// 処理をわざわざ書かなくてもevaluate()を行いTrueの場合はスコア計算
	// を行えばよい
	if(ModInvertedQueryNode::evaluate(documentID, locations, mode, givenEndNode)
		== ModTrue) {
		// evaluateに成功したのでスコアを計算する
		// iteratorはevaluateにより現在のIDの文書の位置移動している
		ModBoolean exist;
		score = (*scoreCalculator)(
					iterator->getInDocumentFrequency(), documentID, exist);
		return exist;
	}

	// evaluateに失敗したのでスコアを0.0にする
	score = 0.0;
	return ModFalse;
}
#endif

// 
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::lowerBoundScore -- lowerBoundのランキング版（スコアも計算する）
// 
// NOTES
// SimpleTokenLeafNode::lowerBound()を実行し、Tureだったらスコア計算をする。
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
ModInvertedSimpleTokenLeafNode::lowerBoundScore(
	const DocumentID givenID,
	DocumentID& foundID,
	DocumentScore& score,
	Query::EvaluateMode mode)
{
	if (SimpleTokenLeafNode::lowerBound(givenID, foundID, mode) == ModTrue) {
		// lowerBound が成功したのでスコアを計算する
		ModBoolean exist;
		score = (*scoreCalculator)(iterator->getInDocumentFrequency(),
								   foundID, exist);
		return exist;
	}

	// lowerBound が失敗
	score = 0.0;
	return ModFalse;
}

// 
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::getTermFrequency -- 文書内頻度を得る
// 
// NOTES
// 文書内頻度を得る。
// 
// ARGUMENTS
// ModInvertedDocumentID DocumentID
//		文書ID
// Query::EvaluateMode mode
// 		評価モード
// 
// RETURN
// 文書内頻度
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize
ModInvertedSimpleTokenLeafNode::getTermFrequency(DocumentID documentID,
												 Query::EvaluateMode mode)
{
#ifdef ITERATOR_CHECK
	if (!iterator) {
		// iteratorが設定されていないのは異常である
		ModErrorMessage << "iterator = 0" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorRetrieveFail);
	}
#endif

	if (documentID == this->iterator->getDocumentId()) {
		return this->iterator->getInDocumentFrequency();
	}

	// 処理をわざわざ書かなくてもevaluate()を利用できる
	if(evaluate(documentID, mode) == ModTrue) {
		// evaluateに成功したのでTFを求める
		return this->iterator->getInDocumentFrequency();
	}
	// マッチしないのでTFは0
	return 0;
}

//
// FUNCTION public
// ModInvertedQueryNode::duplicate -- 自分のコピーを作成する
//
// NOTES
// 自分のコピーを作成する。Query::から呼ばれ、検索木を上からたどり、
// 検索木のコピーを作成する際に使用される。
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
ModInvertedSimpleTokenLeafNode::duplicate(const ModInvertedQuery& rQuery)
{
	// 新しいTermLeafNodeを作成
	ModInvertedSimpleTokenLeafNode* node
		= new ModInvertedSimpleTokenLeafNode(this->token,
											 this->tokenLength);

	if (this->scoreCalculator != 0) {
		node->scoreCalculator = this->scoreCalculator->duplicate();
	}

	node->setTotalDocumentFrequency(totalDocumentFrequency);

	return node;
}

//
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::contentString -- ノードの内容を表わす文字列を返す
//
// NOTES
// ノードの内容を表わす文字列を返す。QueryNodeで定義された内容をオーバ
// ライドする。
//
// ARGUMENTS
// ModString& prefix
//		ノードの内容を表わす文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedSimpleTokenLeafNode::contentString(ModUnicodeString& content) const
{
	if (this->token.getLength() == 0) {
		// 単語単位検索で空文字列を用いて検索する。
		// 検索条件に空文字列が入った場合、検索条件式が見にくいので、
		// 空文字列の場合は "EMPTY" と表示する
		content += "\"EMPTY\"";
	} else {
		content += this->token;
	}
}

//
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::setIterator -- 転置リスト反復子のセット
//
// NOTES
// 転置リスト反復子のセット
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
ModInvertedSimpleTokenLeafNode::setIterator()
{
	if (this->iterator == 0) {
		this->iterator = this->invertedList->begin();
	}
}

//
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::prefixString -- 演算子を表わす文字列を返す
//
// NOTES
//  演算子をあらわす文字列を返す。simpleTokenの場合は#token
// 
// ARGUMENTS
// ModString& prefix
//  演算子を表わす文字列を返す(結果格納用)
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedSimpleTokenLeafNode::prefixString(
	ModUnicodeString& prefix,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam) const
{
	prefix += "#token";
	if (withCalOrCombName == ModTrue) {
		ModUnicodeString calculatorName;
		getCalculatorOrCombinerName(calculatorName, withCalOrCombParam);
		if(calculatorName.getLength() > 0) {
			prefix += '[';
			prefix += calculatorName;
			prefix += ']';
		}
	}
}

//
// FUNCTION public
// ModInvertedSimpleTokenNode::getDocumentFrequency -- 文書頻度の取得
//
// NOTES
// 文書頻度を取得する。
//
// ARGUMENTS
// なし
//
// RETURN
// 文書頻度
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize
ModInvertedSimpleTokenLeafNode::getDocumentFrequency(Query::EvaluateMode mode)
{
	return invertedList->getDocumentFrequency();
}

//
// FUNCTION protected
// ModInvertedSimpleTokenLeafNode::reevaluate -- 正確な再評価
//
// NOTES
// デフォルト処理では evaluate と同じ。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
//
// RETURN
// 条件を満す場合 ModTrue 満さない場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedSimpleTokenLeafNode::reevaluate(ModInvertedDocumentID documentID)
{
#ifdef ITERATOR_CHECK
	if (!iterator) {
		// iteratorが設定されていないのは異常である
		ModErrorMessage << "iterator = 0" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorRetrieveFail);
	}
#endif	// ITERATOR_CHECK

	if (documentID == this->iterator->getDocumentId()) {
		return ModTrue;
	}

	// evalauteを使う事が出来る
	return evaluate(documentID, ModInvertedQueryNode::defaultEMode);
}

//
// FUNCTION protected
// ModInvertedSimpleTokenLeafNode::reevaluate -- 正確な再評価
//
// NOTES
// 引数 documentID で与えられた文書が条件を満すかどうか評価する。また
// 条件を満した場合 LocationListIterator を引数 locations にセットして
// 返す
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
// ModInvertedLocationListIterator*& locations
//		出現位置反復子 (結果格納用)
// ModSize& uiTF_
//		(locationsが取得できない場合) TFの推定値
// ModInvertedQueryNode* givenEndNode
//		ここでは未使用。orderedDistancdでのみ使用。
//
// RETURN
// 条件を満す場合 ModTrue 満さない場合 ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedSimpleTokenLeafNode::reevaluate(
	ModInvertedDocumentID documentID,
	ModInvertedLocationListIterator*& locations,
	ModSize& uiTF_,
	ModInvertedQueryNode* givenEndNode)
{
	// iteratorの位置のチェック
	if (documentID == this->iterator->getDocumentId()) {
		if (invertedList->isNolocation() == false)
		{
			// 位置リストが格納されている
			
			locations = this->iterator->getLocationListIterator();
			// 出現位置反復子に長さを設定する
#ifdef CC_SUN4_2
			static_cast<ModInvertedCompressedLocationListIterator*&>(locations)
				->setLength(this->tokenLength);
#else
			static_cast<ModInvertedCompressedLocationListIterator*>(locations)
				->setLength(this->tokenLength);
#endif
		}
		else
		{
			// 位置リストが格納されていない

			// TFだけでも取得する
			// (TFも格納されていない場合、1が返るはず)
			uiTF_ = iterator->getInDocumentFrequency();
			; ModAssert(uiTF_ > 0);
		}
		return ModTrue;
	}

	// evalauteを使う事が出来る
	return ModInvertedQueryNode::evaluate(documentID, locations, uiTF_,
										  ModInvertedQueryNode::defaultEMode,
										  givenEndNode);
}

//
// FUNCTION protected
// ModInvertedSimpleTokenLeafNode::getQueryString -- 検索条件ノードを出力
//
// NOTES
// 検索条件ノードを出力。
//
// ARGUMENTS
// ModOstrStream& out
//      結果格納用オブジェクト
// ModBoolean withCalOrCombName,
//      スコア計算器・合成器の表示ON/OFF
// ModBoolean withCalOrCombParam,
//      スコア計算器・合成器のパラメータ表示ON/OFF
// ModBoolean withRough
//      ラフノードを表示するかどうかを示すフラグ（trueで表示）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedSimpleTokenLeafNode::getQueryString(ModUnicodeString& out,
									const ModBoolean asTermString,
									const ModBoolean withCalOrCombName,
									const ModBoolean withCalOrCombParam,
									const ModBoolean withRough) const
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
													 tmpMmode) != 0) {
#ifdef V1_6
		if(tmpMmode == ModInvertedTermExactWordMode) {
			out = "#term[e,"; 	
		} else {
			out = "#term[n,"; 	
		}
#else
		out = "#term[n"; 	
#endif // V1_6

		if (withCalOrCombName == ModTrue) {
			ModUnicodeString calculatorName;
			getCalculatorOrCombinerName(calculatorName, withCalOrCombParam);
			if(calculatorName.getLength() > 0) {
#ifndef V1_6
				out += ',';
#endif // NOT V1_6
				out += calculatorName;
			}
		}
#ifdef V1_6
		out += ',';
		out += tmpLangSet.getName();
#endif // V1_6
		out += ']';

		ModUnicodeString tmp;
		ModInvertedQueryParser::convertTermString(termString , tmp);
		out += '(';
		out += tmp;
		out += ')';
		return;
	}

	ModInvertedQueryBaseNode::getQueryString(out,
		asTermString, withCalOrCombName, withCalOrCombParam, withRough);
}

//
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::getOriginalString() -- 自分が生成された元のTermLeafNodeの文字列を返す。
//
// NOTES
// 自分が生成された元のTermLeafNodeの文字列を返す。
// originalTermStringがセットされていない場合はFalseを返す。
//
// ARGUMENTS
// ModUnicodeString& termString
//      結果格納用
//
// RETURN
// originalTermStringがある場合True/ない場合False
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedSimpleTokenLeafNode::getOriginalString(
	ModUnicodeString& termString_,
#ifdef V1_6
	ModLanguageSet& langSet_,
#endif // V1_6
	ModInvertedTermMatchMode& mmode_) const
{
	if (originalTermString.getLength() != 0) {
		// originalTermStringを返す
		termString_ = originalTermString;
#ifdef V1_6
		langSet_ = langSet;
#endif // V1_6
		mmode_ = mmode;
#ifdef DEBUG
	ModDebugMessage << "SimpleToken:getOriginalString : " << termString_ << ModEndl;
#endif // DEBUG
		return ModTrue;
	}

	return ModFalse;
}

//
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::setOriginalString() -- 自分が生成された元のTermLeafNodeの文字列をメンバ変数originalTermStringにセットする。
//
// NOTES
// 自分が生成された元のTermLeafNodeの文字列をメンバ変数originalTermStringに
// セットする。QueryBaseNode::setOriginalString()をオーバーライド。
//
// ARGUMENTS
// ModUnicodeString termString
//      セットする文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedSimpleTokenLeafNode::setOriginalString(
	const ModUnicodeString& termString_,
#ifdef V1_6
	const ModLanguageSet& langSet_,
#endif // V1_6
	const ModInvertedTermMatchMode& mmode_)
{
#ifdef DEBUG
	ModDebugMessage << "SimpleToken:setOriginalString:" << termString_ << ModEndl;
#endif // DEBUG

	originalTermString = termString_;
#ifdef V1_6
	langSet = langSet_;
#endif // V1_6
	mmode = mmode_;
}

#ifdef DEL_BOOL
void
ModInvertedSimpleTokenLeafNode::retrieve(ModInvertedBooleanResult *expungedDocumentId,
										 const ModInvertedDocumentID maxDocumentId,
										 ModInvertedBooleanResult *result)
{
	ModInvertedVector<ModInvertedDocumentID>::Iterator iter = expungedDocumentId->begin();

	// boolean!
	for (; this->iterator->isEnd() == ModFalse; ++(*iterator)) {
		ModInvertedDocumentID ID = iterator->getDocumentId();
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

		result->pushBack(ID);
	}
}
#endif

void 
ModInvertedSimpleTokenLeafNode::retrieve(ModInvertedBooleanResult *expungedDocumentId,
										 const ModInvertedDocumentID maxDocumentId,
										 ModInvertedSearchResult *result)
{
	ModInvertedVector<ModInvertedDocumentID>::Iterator iter = expungedDocumentId->begin();

	for (; this->iterator->isEnd() == ModFalse; ++(*iterator)) {
		ModInvertedDocumentID ID = iterator->getDocumentId();
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

		ModBoolean exist;

		ModUInt32 tf = iterator->getInDocumentFrequency();
		// tf / ( X + tf ) 部分を計算する
		ModInvertedDocumentScore score	= scoreCalculator->firstStep(tf, ID, exist);
		if(exist == ModTrue)
			result->pushBack(ID, score,tf);
	}
}

//
// FUNCTION public
// ModInvertedSimpleTokenLeafNode:::doFirstStepInRetrieveScore -- スコア計算の第１ステップを実行する
//
// NOTES
//
// ARGUMENTS
// const ModInvertedDocumentID maxDocumentId
//      ドキュメントID
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedSimpleTokenLeafNode::doFirstStepInRetrieveScore(
	ModInvertedBooleanResult *expungedDocumentId,
	const ModInvertedDocumentID maxDocumentId)
{
	if(getFirstStepStatus() == firstDone) {
		// すでに呼ばれている
		return;
	}

	ModInvertedSearchResult* result = getRankingResult();

	this->iterator->reset();

	// terminatorとして、(maxDocumentId + 1)をセットする
	expungedDocumentId->pushBack(maxDocumentId + 1);
	//
#ifdef DEL_BOOL
	if(result->getType() == (1 << _SYDNEY::Inverted::FieldType::Rowid))
		retrieve(expungedDocumentId,maxDocumentId,(ModInvertedBooleanResult*)result);
	else
#endif
		retrieve(expungedDocumentId,maxDocumentId,result);

	setFirstStepStatus(firstDone);

	// terminatorを削除する
	expungedDocumentId->popBack();

	// iteraorがendに達しているので、upperlowerも更新
	this->upper = this->lower = iterator->getDocumentId();
}

//
// FUNCTION
// ModInvertedSimpleTokenLeafNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
//
// NOTES
// ランキング検索で、スコア計算の第２ステップのみを実施する。
// 第１ステップの結果を用い、最終的な検索結果を生成する。
// 第１ステップを実施していない場合の結果は不定。
//
// ARGUMENTS
// ModInvertedSearchResult* result_
//      検索結果
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedSimpleTokenLeafNode::doSecondStepInRetrieveScore(ModInvertedSearchResult*& result_)
{
	doSecondStepInRetrieveScore();
	result_ = getRankingResult();
}

//
// FUNCTION
// ModInvertedSimpleTokenLeafNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
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
ModInvertedSimpleTokenLeafNode::doSecondStepInRetrieveScore()
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
// 拡張スコア計算
//
void
ModInvertedSimpleTokenLeafNode::doSecondStepInRetrieveScore_Extended()
{
 	ModInvertedSearchResult* result = getRankingResult();
 	; ModAssert(result != 0);
	ModInvertedRankingScoreCalculator* calculator
				= getScoreCalculator();
	; ModAssert(calculator != 0);
	calculator->setQueryNode(this);

	ModInvertedDocumentScore prepared
				= calculator->getPrepareResult();

	for(ModSize i =  0 ; i < result->getSize(); i++)
	{
		result->setScore(i,prepared*calculator->
					firstStepEx(i,result->getDocID(i)));
	}
}

//
// FUNCTION
// ModInvertedSimpleTokenLeafNode::doSecondStepInRetrieveScore -- ランキング検索の第２ステップ
//
//
// 基本スコア計算
//
//  doSecondStepInRetrieveScore()は高速版と汎用版の２つある。
//  ModInvertedSearchResultの型に応じて使い分ける
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

//
// 汎用版doSecondStepInRetrieveScore
// getRankingResult()の型がModInvertedSearchResultScore以外の場合にここにくる
// ModInvertedSimpleTokenLeafNodeのコンストラクタで切り替えている
void
ModInvertedSimpleTokenLeafNode::doSecondStepInRetrieveScore_normal()
{
	ModInvertedSearchResult* result = getRankingResult();
	; ModAssert(result != 0);
	ModInvertedRankingScoreCalculator* calculator
		= getScoreCalculator();
	; ModAssert(calculator != 0);
	ModInvertedDocumentScore prepared
		= calculator->getPrepareResult();
	for(ModSize i = 0; i < result->getSize(); i++){
		result->setScore(i , result->getScore(i)*prepared);
	}
	setFirstStepStatus(secondDone);
}

//
// 高速版doSecondStepInRetrieveScore
// getRankingResult()の型がModInvertedSearchResultScore以外の場合にここにくる
// ModInvertedSimpleTokenLeafNodeのコンストラクタで切り替えている
void
ModInvertedSimpleTokenLeafNode::doSecondStepInRetrieveScore_highspeed()
{
	ModInvertedSearchResultScore* result = (ModInvertedSearchResultScore*)getRankingResult();
	; ModAssert(result != 0);
	ModInvertedRankingScoreCalculator* calculator
		= getScoreCalculator();
	; ModAssert(calculator != 0);
	ModInvertedDocumentScore prepared
		= calculator->getPrepareResult();
	ModInvertedSearchResultScore::Iterator r = result->begin();
	ModInvertedSearchResultScore::Iterator e = result->end();
	for (; r != e; ++r) {
		(*r).second *= prepared;
	}
	setFirstStepStatus(secondDone);
}

//
// FUNCTION
// ModInvertedAtomicNode::lowerBoundScoreForSecondStep() -- ランキング検索の第２ステップで使用するlowerBound
//
// NOTES
// ランキング検索のスコア計算第２ステップで使用されるlowerBound。
//  lowerBoundScoreForSecondStep()は高速版と汎用版の２つある。
//  ModInvertedSearchResultの型に応じて使い分ける
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

//
// 汎用版lowerBoundScoreForSecondStep
// getRankingResult()の型がModInvertedSearchResultScore以外の場合にここにくる
// doSecondStepInRetrieveScore()で切り替えている
ModBoolean
ModInvertedSimpleTokenLeafNode::lowerBoundScoreForSecondStep_normal(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
	
	ModInvertedSearchResult* result = getRankingResult();
	// resultの本当の型をresult->getType()で見て、TFを含まないので、あれば、
	// resultをModInvertedScoreResult型にcastし、高速化を計る
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
ModInvertedSimpleTokenLeafNode::lowerBoundScoreForSecondStep_highspeed(
	ModInvertedDocumentID givenID,
	ModInvertedDocumentID& foundID,
	ModInvertedDocumentScore& score)
{
#ifdef SYD_USE_LARGE_VECTOR
	if (_riterator.isValid() == false) {
#else
	if (&(*_riterator) == 0) {
#endif
		ModInvertedSearchResultScore* result = (ModInvertedSearchResultScore*)getRankingResult();
		_riterator = result->begin();
		_rend = result->end();
		if (_riterator == _rend)
			return ModFalse;
	}
	if (_riterator == _rend ||
		((*_riterator).first > givenID && givenID < _givenID))
	{
		// SearchResultScoreを読み込む
		
		// [YET] isValid == false以外の条件でここに来るのは、
		// * 検索途中にrewindが発生した時？
		// * 前回の検索で条件を満たすIDが存在しなかった時
		// 後者については、再読み込みせずにfalseを返せば良いが、
		// 前者との区別がつかないので、全て再読み込みしている？
		
		ModInvertedSearchResultScore* result = (ModInvertedSearchResultScore*)getRankingResult();
		_riterator = result->begin();
		_rend = result->end();
		if (_riterator == _rend || givenID > (*(_rend - 1)).first)
		{
			// 最後のIDがgivenIDより小さければ、
			// lowerBoundを満たすIDがないのは明らか
			return ModFalse;
		}
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
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
