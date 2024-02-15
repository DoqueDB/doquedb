// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedBooleanResultLeafNode.h -- Boolean検索結果を表す末端ノードのインタフェイス
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

#ifndef __ModInvertedBooleanResultLeafNode_H__
#define __ModInvertedBooleanResultLeafNode_H__

#include "ModInvertedQueryLeafNode.h"
#include "ModInvertedSearchResult.h"

#ifdef SYD_INVERTED
#include "ModInvertedQuery.h"
#endif


//
// CLASS
// ModInvertedBooleanResultLeafNode -- 検索式の内部表現 (木構造) の末端ノードで、Boolean検索結果を表すもの
//
// NOTES
// 検索式の内部表現には、オペランドをオペレータで結んだ木構造を用いる。
// その末端ノードで、検索中間結果を表すもののクラス。
//
class
ModInvertedBooleanResultLeafNode : public ModInvertedQueryLeafNode {
public:
	// コンストラクタ
	ModInvertedBooleanResultLeafNode(const ModInvertedSearchResult* result,const  ModUInt32 resultType_);
	ModInvertedBooleanResultLeafNode(const ModUInt32 resultType_=0);
	ModInvertedBooleanResultLeafNode(
		const ModInvertedBooleanResultLeafNode* originalNode,const  ModUInt32 resultType_);

	// デストラクタ
	virtual ~ModInvertedBooleanResultLeafNode();

#ifndef DEL_BOOL
	// 検索の一括実行
	virtual void retrieve(BooleanResult& queryResult, Query::EvaluateMode mode);
#endif
	// 与えられた文書が検索条件を満たすかどうかの検査
	// 二番目の形式は、位置情報が必要な場合に用いられる。
	virtual ModBoolean evaluate(DocumentID documentID,
								Query::EvaluateMode mode);

	// 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
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

	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery& query);

	// 空集合かどうかのチェック
	ModBoolean isEmptyResultLeafNode() const;

	// 自分の件数を返す
	ModSize getSize(){ return this->end - this->begin; }

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
								const ModBoolean withCalOrCombName,
								const ModBoolean withCalOrCombParam) const;

	// ノードの内容を表わす文字列を返す
	virtual void contentString(ModUnicodeString& content) const;

	virtual void validate(InvertedFile* invertedFile,
							const ModInvertedQuery::ValidateMode mode,
							ModInvertedQuery* rQuery);

	// ランキング検索のスコア計算第１ステップ
	void doFirstStepInRetrieveScore(
			ModInvertedBooleanResult *expungedDocumentId,
			const ModInvertedDocumentID maxDocumentId);
	// スコア計算の第２ステップ
	void doSecondStepInRetrieveScore(
			ModInvertedSearchResult*& result);
	void doSecondStepInRetrieveScore();

	// ランキング検索のスコア計算第２ステップで使用するlowerBound
	virtual ModBoolean lowerBoundScoreForSecondStep(
		ModInvertedDocumentID givenID,
		ModInvertedDocumentID& foundID,
		ModInvertedDocumentScore& score);

	virtual void checkQueryNode(
				ModInvertedQuery* query_,
				const ModBoolean setStringInChildren_,
				const ModBoolean needDF_);

#if 0
	virtual void removeFromFirstStepResult(
			const ModInvertedSearchResult* bresult);
#endif
	// prepareを呼ぶ関数だが、計算器を持っていないので何もしない
	void prepareScoreCalculator(const ModSize totalFrequency_,
								const ModSize documentFrequency_) {}
	// prepareを呼ぶ関数だが、計算器を持っていないので何もしない
	void prepareScoreCalculatorEx(const ModUInt64 totalTermFrequency_,
								const ModUInt64 totalDocumentLength_,
								const ModUInt32 queryTermFrequency_){}

	ModUInt64 getTotalTermFrequency()
	{
		return (ModUInt64)0;
	}
protected:
	// 粗い evaluate 満足を前提とした、正確な再評価
	virtual ModBoolean reevaluate(DocumentID documentID);
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModSize& uiTF,
								  ModInvertedQueryNode* givenEndNode = 0);

	// 評価結果
	ModInvertedSearchResult* searchResult;

	// 検索結果反復子
	ModSize begin,end,current;
};





#endif //__ModInvertedBooleanResultLeafNode_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
