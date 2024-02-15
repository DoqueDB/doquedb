// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedSimpleTokenLeafNode.h -- 単一の索引語に対応する末端ノード
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedSimpleTokenLeafNode_H__
#define __ModInvertedSimpleTokenLeafNode_H__

#include "ModInvertedQueryLeafNode.h"
#include "ModInvertedIterator.h"
#include "ModInvertedCompressedLocationListIterator.h"

class ModInvertedList;
//
// CLASS
// ModInvertedSimpleTokenLeafNode -- 単一の索引語に対応する末端ノード
//
// NOTES
// 検索式の内部表現には、オペランドをオペレータで結んだ木構造を用いる。
// その末端ノードで、単一の索引語に対応するもののクラス。
//
class
ModInvertedSimpleTokenLeafNode
	: public ModInvertedQueryLeafNode
{
public:
	typedef ModInvertedRankingScoreCalculator	ScoreCalculator;
	typedef ModInvertedIterator Iterator;	// ModInvertedList::Iterator ==
											// ModInvertedIterator であること

	// コンストラクタ
	ModInvertedSimpleTokenLeafNode(const ModUnicodeString& token_,
								   const ModSize tokenLength_,
								   ModInvertedList* invertedList_ = 0,
							   const ModUInt32 resultType_= 0,
								   ModSize documentFrequency = UndefinedFrequency
								   );

	// デストラクタ
	virtual ~ModInvertedSimpleTokenLeafNode();

#ifndef DEL_BOOL
	// 検索の一括実行
	virtual void retrieve(BooleanResult& queryResult, Query::EvaluateMode mode);
#endif
	// 与えられた文書が検索条件を満たすかどうかの検査
	// 二番目の形式は、位置情報が必要な場合に用いられる。
	virtual ModBoolean evaluate(DocumentID documentID, 
								Query::EvaluateMode mode);

	// 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
	// 二番目の形式は、位置情報が必要な場合に用いられる。
	virtual ModBoolean lowerBound(DocumentID givenDocumentID,
								  DocumentID& foundDocumentID,
								  Query::EvaluateMode mode);

	// 与えられた文書のスコアを計算する
	virtual ModBoolean evaluateScore(const DocumentID documentID,
									 DocumentScore& score,
									 Query::EvaluateMode mode);

#if (!defined(MOD_DIST)) && (!defined(SYD_INVERTED)) // EVALUATESCORE
	// 与えられた文書のスコアを計算する（位置も計算する）
	virtual ModBoolean evaluateScore(const DocumentID documentID,
									 DocumentScore& score,
									 LocationIterator*& locations,
									 Query::EvaluateMode mode,
									 ModInvertedQueryNode* givenEndNode = 0);
#endif

	// lowerBoundのランキング版（スコアも計算する）
	virtual ModBoolean lowerBoundScore(const DocumentID givenDocumentID,
									   DocumentID& foundDocumentID,
									   DocumentScore& score,
									   Query::EvaluateMode mode);

	virtual ModSize getDocumentFrequency(Query::EvaluateMode mode);
	virtual ModSize getTermFrequency(DocumentID documentID, 
									 Query::EvaluateMode mode);

	// token の長さを得る
	void getTermLength(ModSize& length) const;

	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery& query);

	virtual void contentString(ModUnicodeString& content) const;

	// 演算子を表わす文字列を返す
	// とりあえずsimpleTokenLeafNodeは何もしない
	virtual void prefixString(ModUnicodeString& prefix,
		const ModBoolean withCalOrCombName, 
		const ModBoolean withCalOrCombParam) const;

	// 転置リスト反復子のセット
	void setIterator();
	// 転置リスト反復子を得る
	Iterator* getIterator() const { return this->iterator; }

	ModInvertedList* getInvertedList();

#ifdef DEBUG
	static int countLowerBound;
	static int countLocCheck;
#endif

	// originalTermStringのアクセサ関数
	void setOriginalString(const ModUnicodeString& termString_,
#ifdef V1_6
						   const ModLanguageSet& langSet_,
#endif // V1_6
						   const ModInvertedTermMatchMode& mmode_);
	ModBoolean getOriginalString(ModUnicodeString& termString_,
#ifdef V1_6
								 ModLanguageSet& langSet_,
#endif // V1_6
						   		 ModInvertedTermMatchMode& mmode_) const;

	virtual void checkQueryNode(ModInvertedQuery*, 
								const ModBoolean,
								const ModBoolean);

	// ランキング検索のスコア計算第１ステップ
	virtual void doFirstStepInRetrieveScore(
		ModInvertedBooleanResult *expungedDocumentId,
		const ModInvertedDocumentID maxDocumentId);

	// ランキング検索のスコア計算第２ステップ

	virtual void doSecondStepInRetrieveScore(
		ModInvertedSearchResult*& result);

	virtual void doSecondStepInRetrieveScore();

	void doSecondStepInRetrieveScore_Extended();

	void doSecondStepInRetrieveScore_Basic()
	{
		(this->*_doSecondStepInRetrieveScore)();
	}

	// ランキング検索のスコア計算第２ステップで使用するlowerBound
	virtual ModBoolean lowerBoundScoreForSecondStep(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score)
	{
		return (this->*_lowerBoundScoreForSecondStep)(givenID,foundID,score);
	}
	
protected:
	friend class ModInvertedQuery;
	friend class ModInvertedQueryInternalNode;

	// 粗い evaluate 満足を前提とした、正確な再評価
	virtual ModBoolean reevaluate(DocumentID documentID);
	// 位置情報リストが必要版
	// ただし、位置情報リストが取得できない場合は、TFを取得する
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* givenEndNode = 0);

	virtual void getQueryString(ModUnicodeString& out,
								const ModBoolean asTermString = ModFalse,
								const ModBoolean withCalOrCombName = ModTrue,
								const ModBoolean withCalOrCombParam = ModTrue,
								const ModBoolean withRouh = ModFalse) const;

	Iterator* iterator;						// 転置リスト反復子
	ModUnicodeString token;					// トークン文字列
	ModSize tokenLength;
	ModInvertedList* invertedList;			// 転置リスト
#ifdef V1_6
	ModLanguageSet langSet;
#endif // V1_6
	ModInvertedTermMatchMode mmode;

private:
	// ランキング検索のスコア計算第２ステップで使用するdoSecondStepInRetrieveScore
	// iteratorアクセスによる高速版
	// 検索結果型がModInvertedSearchResultScore時の呼ばれるメンバー関数
	void doSecondStepInRetrieveScore_highspeed();
	// メンバー関数アクセスによる通常版
	// 検索結果型がModInvertedSearchResultScore以外の時に呼ばれるメンバー関数
	void doSecondStepInRetrieveScore_normal();
	void (ModInvertedSimpleTokenLeafNode::*_doSecondStepInRetrieveScore)();

	///////////////////////////////////////////////////////////////////
	// ランキング検索のスコア計算第２ステップで使用するlowerBound
	//
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

	// iteratorアクセスによる高速版
	// 検索結果型がModInvertedSearchResultScore時の呼ばれるメンバー関数
	ModBoolean (ModInvertedSimpleTokenLeafNode::*_lowerBoundScoreForSecondStep)(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score);
	// オリジナルのTermLeafNodeの文字列
	ModUnicodeString originalTermString;

	// lowerBoundScoreForSecondStepの高速化用
	ModSize _position;
	ModInvertedSearchResultScore::Iterator _riterator;
	ModInvertedSearchResultScore::Iterator _rend;
	ModInvertedDocumentID _givenID;

	// 使用禁止
	ModInvertedSimpleTokenLeafNode(
		const ModInvertedSimpleTokenLeafNode& original);
	ModInvertedSimpleTokenLeafNode& operator=(
		const ModInvertedSimpleTokenLeafNode& original);
#ifdef DEL_BOOL
void retrieve(ModInvertedBooleanResult *expungedDocumentId,
			 const ModInvertedDocumentID maxDocumentId,
			 ModInvertedBooleanResult *result);
#endif
void retrieve(ModInvertedBooleanResult *expungedDocumentId,
			 const ModInvertedDocumentID maxDocumentId,
			 ModInvertedSearchResult *result);
};

//
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::ModInvertedSimpleTokenLeafNode -- 単一の索引語に対応する末端ノードの生成
//
// NOTES
// 単一の索引語に対応する末端ノードの生成
//
// ARGUMENTS
// const ModUnicodeString& token_
//		索引語
// const ModSize tokenLength_,
//		索引語の長さ
// ModInvertedList* invertedList_
//		転置リスト
// ModSize documentFrequency
//		出現頻度
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedSimpleTokenLeafNode::ModInvertedSimpleTokenLeafNode(
	const ModUnicodeString& token_,
	const ModSize tokenLength_,
	ModInvertedList* invertedList_,
	const  ModUInt32 resultType_,
	ModSize documentFrequency
	)
	: ModInvertedQueryLeafNode(simpleTokenLeafNode,resultType_, documentFrequency),
	  iterator(0), token(token_), tokenLength(tokenLength_),
	  invertedList(invertedList_), _givenID(0),_position(-1)
#ifndef SYD_USE_LARGE_VECTOR
	  , _riterator(0)
#endif
{
	if(resultType_ == ((1 <<_SYDNEY::Inverted::FieldType::Score)|
		(1 << _SYDNEY::Inverted::FieldType::Rowid)))
	{
		_lowerBoundScoreForSecondStep =  &ModInvertedSimpleTokenLeafNode::lowerBoundScoreForSecondStep_highspeed;
		_doSecondStepInRetrieveScore =  &ModInvertedSimpleTokenLeafNode::doSecondStepInRetrieveScore_highspeed;
	}
	else
	{
		_lowerBoundScoreForSecondStep =  &ModInvertedSimpleTokenLeafNode::lowerBoundScoreForSecondStep_normal;
		_doSecondStepInRetrieveScore =  &ModInvertedSimpleTokenLeafNode::doSecondStepInRetrieveScore_normal;
	}
}

// 
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::getTermLength -- 索引語の長さを得る
//
// NOTES
// 	索引語の長さを得る
//
// ARGUMENTS
// ModSize& termLength
//		索引語の長さ（結果格納用）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedSimpleTokenLeafNode::getTermLength(ModSize& length) const
{
	length = tokenLength;
}

//
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::checkQueryNode -- 子ノードの数をチェックする
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
inline void
ModInvertedSimpleTokenLeafNode::checkQueryNode(
	ModInvertedQuery* query_,
	const ModBoolean setStringInChildren_,
	const ModBoolean needDF_
	)
{
	if(needDF_ == ModTrue) {
		setNeedDF(needDF_);
	}

	if (setStringInChildren_ == ModTrue &&
		originalTermString.getLength() != 0) {
		// 検索条件を取得
		ModUnicodeString termString;
		getQueryString(termString, ModTrue, ModTrue, ModTrue, ModFalse);

		// 検索条件をキーにQueryNodeMapに登録
		query_->insertTermNode(termString,
							   const_cast<ModInvertedSimpleTokenLeafNode*>(this));

		// 平均文書長を設定
		ModSize avelen(query_->getAverageDocumentLength());
		if (avelen != 0) {
			ScoreCalculator* calculator = getScoreCalculator();
			; ModAssert(calculator != 0);
			calculator->setAverageDocumentLength(avelen);
		}
	}
}

//
// FUNCTION public
// ModInvertedSimpleTokenLeafNode::getInvertedList()
//
// NOTES
// メンバ変数invertedListを取得する
//
// ARGUMENTS
// なし
//
// RETURN
// ModInvertedList* invertedList
//
// EXCEPTIONS
// なし
//
inline ModInvertedList*
ModInvertedSimpleTokenLeafNode::getInvertedList()
{
	return this->invertedList;
}

#endif //__ModInvertedSimpleTokenLeafNode_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
