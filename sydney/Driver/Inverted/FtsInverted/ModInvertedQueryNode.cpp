// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedQueryNode.cpp -- 検索式ノードの実装
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "ModInvertedQueryNode.h"
#include "ModInvertedOperatorAndNode.h"
#include "ModInvertedSearchResult.h"
#include "ModInvertedBooleanResultLeafNode.h"
#include "ModInvertedLocationListIterator.h"
#include "ModInvertedRankingScoreCombiner.h"


/*static*/ const ModSize
ModInvertedQueryNode::UndefinedFrequency = (unsigned)-1;

/*static*/ const ModInvertedQuery::EvaluateMode
ModInvertedQueryNode::defaultEMode = 0x00;

// VALIABLE
// ModInvertedQueryNode::EmptySetNode -- 空集合ノード
//
// NOTES
// 空集合を示すノード。beginとendを0に初期化
//
/*static*/ const ModInvertedBooleanResultLeafNode ModInvertedQueryNode::EmptySetNode;

/*static*/ const ModInvertedQueryNode*
ModInvertedQueryNode::emptySetNode = &ModInvertedQueryNode::EmptySetNode;

// VALIABLE
// ModInvertedQueryNode::MaxSortFactor -- sortFactorの最大値
//
// NOTES
// sortFactorの最大値。ModSizeの最大値をセットする。
//
/*static*/ const ModSize 
ModInvertedQueryNode::MaxSortFactor = ModSizeMax;

//
// FUNCTION public
// ModInvertedQueryNode::~ModInvertedQueryNode -- 検索式内部表現ノードの廃棄
//
// NOTES
// 検索式内部表現ノードオブジェクトを廃棄する。
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
ModInvertedQueryNode::~ModInvertedQueryNode()
{}

//
// FUNCTION public
// ModInvertedQueryNode::evaluate -- 検索式内部表現ノードの廃棄
//
// NOTES
// 検索式内部表現ノードオブジェクトを廃棄する。
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
ModBoolean
ModInvertedQueryNode::evaluate(ModInvertedDocumentID documentID,
							   LocationIterator*& locations,
							   ModSize& uiTF_,
							   ModInvertedQuery::EvaluateMode mode,
							   ModInvertedQueryNode* givenEndNode)
{
	// 位置情報が必要なので、mode の指定によらず precise evaluation
	if (evaluate(documentID, mode | Query::roughEvaluationFlag)) {
		return this->reevaluate(documentID, locations, uiTF_, givenEndNode);
	}
	return ModFalse;
}

//
// FUNCTION public
// ModInvertedQueryNode::isConvertedFromTermLeafNode
//		-- 検索語ノードから変換されたノードかの検査
//
// NOTES
// 検索語ノードから変換されたノードかの検査を行う。
//
// ARGUMENTS
// なし
//
// RETURN
// 変換されたノードであれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
// 
ModBoolean
ModInvertedQueryNode::isConvertedFromTermLeafNode() const
{
	ModUnicodeString termString;
#ifdef V1_6
	ModLanguageSet tmpLangSet;
#endif // V1_6
	ModInvertedTermMatchMode mmode;
	return getOriginalString(termString,
#ifdef V1_6
							 tmpLangSet,
#endif // V1_6
							 mmode);
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
ModBoolean
ModInvertedQueryNode::lessSortFactor(QueryNode* x, QueryNode* y)
{
	return x->calcSortFactor() < y->calcSortFactor() ? ModTrue : ModFalse;
}
#if 0
//
// FUNCTION public
// ModInvertedQueryNode::removeFromFirstStepResult -- 第１ステップ結果からの削除
//
// NOTES
// ランキング検索の第１ステップを実施した後で、ノードの結果から bresult を
// 削除する。
//
// ARGUMENTS
// const ModInvertedSearchResult* bresult
//              削除する文書IDの集合
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedQueryNode::removeFromFirstStepResult(
	const ModInvertedSearchResult* bresult)
{
	ModInvertedSearchResult* result = getRankingResult();
	if(result != 0) {
		ModSize f1 = 0,f2 = 0;
		ModSize bf = 0;
		while (f1 < result->getSize() && bf < bresult->getSize()) {
			ModInvertedDocumentID docid1 = result->getDocID(f1);
			ModInvertedDocumentID docid2 = bresult->getDocID(bf);
			if (docid1 < docid2) {
				if (f1 != f2) {
					result->setDocID(f2,docid1);
					result->setScore(f2,result->getScore(f1));
					result->_setTF(f2,result->_getTF(f1));
					// TF は、論理演算をサポートしないので考慮しない->嘘、ここには単純単語検索でも来る
				}
				++f1;
				++f2;
			} else if (docid1 == docid2) {
				++f1;
				++bf;
			} else {
				++bf;
			}
		}

    // 残りがある場合、それまでに削除されている文書がある時のみ
    // 移動を行なう
		if (f1 != f2) {
			while (f1 < result->getSize()) {
				result->setDocID(f2,result->getDocID(f1));
				result->setScore(f2,result->getScore(f1));
				result->_setTF(f2,result->_getTF(f1));
				++f1;
				++f2;
			}
      // あまりを削除する
			result->erase(f2, result->getSize());
		}
	}
}
#endif

//
// FUNCTION public
// ModInvertedQueryNode::getSearchTermList --
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
ModInvertedQueryNode::getSearchTermList(
	ModInvertedQuery::SearchTermList& vecSearchTerm_,
	ModSize uiSynonymID_) const
{
	;ModAssert(false);
}

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2005, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
