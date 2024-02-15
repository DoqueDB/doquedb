// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorWordNode.h -- ModInvertedOperatorWordNode の宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedOperatorWordNode_H__
#define __ModInvertedOperatorWordNode_H__

#ifdef V1_4	// 単語単位検索

class ModInvertedFile;

//#include "ModInvertedQueryInternalNode.h"
//#include "ModInvertedAtomicNode.h"
//#include "ModInvertedQueryNode.h"
#include "ModInvertedTermLeafNode.h"
#include "ModInvertedSimpleTokenLeafNode.h"
#include "ModOstrStream.h"
#include "ModInvertedWordBaseNode.h"

//
// CLASS
// ModInvertedOperatorWordNode -- 単語単位検索演算子ノード
//
// NOTES
// 中間ノードの派生クラス。
// 単語単位検索のための中間ノードであり、TermLeafNodeの有効化で生成される。
//
class
ModInvertedOperatorWordNode
	: public ModInvertedWordBaseNode
{
public:
	// コンストラクタ
	ModInvertedOperatorWordNode(
					ModInvertedQueryNode*& node,
					ModSize _wordLenght,
					ModInvertedTermMatchMode _matchMode,
					ModInvertedQuery& query,
					ModInvertedFile* invertedFile,
					const ModInvertedQuery::ValidateMode vmode,
					const  ModUInt32 resultType_,
					ModInvertedSmartLocationList* emptyLoationList = 0);

	// デストラクタ
	~ModInvertedOperatorWordNode();

	// TermLeafNodeの消去
	// このノードはtermLeafNode::validateで生成されるので
	// eraseTermLeafNodeが呼ばれることはない
	virtual ModBoolean eraseTermLeafNode(QueryNode*& node, Query& query) {
	; ModAssert(0); return ModTrue; }

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
						const ModBoolean withCalOrCombName,
						const ModBoolean withCalOrCombParam) const;

#ifndef DEL_BOOL
	// 検索の一括実行
	virtual void retrieve(BooleanResult& queryResult,
						Query::EvaluateMode mode);
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

	virtual ModSize getTermFrequency(DocumentID documentID,
							 Query::EvaluateMode mode);

	void changeSimpleTypeNode(ModVector<ModInvertedQueryNode*>::Iterator child,
								  QueryNodePointerMap& nodePointerMap) {
	}

	void validate(InvertedFile* invertedFile,
							const ModInvertedQuery::ValidateMode mode,
							ModInvertedQuery* rQuery) {
		; ModAssert(0); 
	}

	virtual void checkQueryNode(ModInvertedQuery*, 
								const ModBoolean,
								const ModBoolean);

	void sortChildren(const ModInvertedQuery::ValidateMode mode) {
		ModInvertedQueryInternalNode::sortChildren(mode);
	}

    void flattenChildren(const QueryNodePointerMap& sharedNodeMap,
							const ModBoolean isChildOfWindowNode);

	ModSize calcSortFactor() {
		return ModInvertedQueryInternalNode::calcSortFactor();
	}

	// 自分のコピーを作成する
	// このノードはTermLeafNodeの有効化で生成されるので
	// duplicateが呼ばれる事はない
	virtual QueryNode* duplicate(const ModInvertedQuery& query)
					{ModAssert(0); return 0; }

	// originalTermStringのアクセサ関数
	void setOriginalString(const ModUnicodeString& termString,
#ifdef V1_6
						   const ModLanguageSet& langSet_,
#endif // V1_6
						   const ModInvertedTermMatchMode& mmode_);
	ModBoolean getOriginalString(ModUnicodeString& termString,
#ifdef V1_6
								 ModLanguageSet& langSet_,
#endif // V1_6
						   		 ModInvertedTermMatchMode& mmode_) const;
	
protected:
	// 粗い evaluate 満足を前提とした、正確な再評価
	virtual ModBoolean reevaluate(DocumentID documentID);
	// 位置情報が必要版
	// ただし、位置情報リストが取得できない場合は、TFを取得する
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* givenEndNode = 0);
	// 位置情報が必要版 (TF不要版)
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModInvertedQueryNode* givenEndNode = 0)
	{
		ModSize dummy = 0;
		return reevaluate(documentID, locations, dummy, givenEndNode);
	}

	virtual void getQueryString(ModUnicodeString& out,
		const ModBoolean asTermString = ModFalse,
		const ModBoolean withCalOrCombName = ModTrue,
		const ModBoolean withCalOrCombParam = ModTrue,
		const ModBoolean withRouh = ModFalse) const;

#ifdef V1_6
	ModLanguageSet langSet;
#endif // V1_6

private:
	// 検索語(Tokenize結果)の長さ
	ModSize wordLength;

	// マッチモード
	ModInvertedTermMatchMode matchMode;

	// 空文字列(単語区切)ノード
	ModInvertedQueryNode* emptyStringNode;

	// トークナイズ結果の空文字列(単語区切)位置情報
	ModInvertedSmartLocationList* boundaryOfTermString;

	// オリジナルのTermLeafNodeの文字列
	ModUnicodeString originalTermString;
};

//
// FUNCTION public
// ModInvertedOperatorWordNode::flattenChildren -- 子ノードリストの平坦化
//
// NOTES
// 子ノードリストの平坦化。
// ここの子ノードにはand/orは来ないため、
// OperatorAndNodeをオーバーライドして何もしない
//
// ARGUMENTS
// const QueryNodePointerMap& sharedNodeMap
//  OR標準形変換時に共有しているノードが登録されているマップ変数
//
// const ModBoolean isChildOfWindowNode
//      windowノードを親にもっている場合 ModTure となる
//
// RETURN
// なし
//
inline void
ModInvertedOperatorWordNode::flattenChildren(const QueryNodePointerMap& sharedNodeMap,
											const ModBoolean isChildOfWindowNode)
{
}

#endif // V1_4	 単語単位検索
#endif //__ModInvertedOperatorWordNode_H__

//
// Copyright (c) 2000, 2001, 2002, 2004, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
