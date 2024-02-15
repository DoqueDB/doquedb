// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOrderedDistanceNode.h -- 間隔演算ノードインタフェイスファイル
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedOrderedDistanceNode_H__
#define __ModInvertedOrderedDistanceNode_H__

#include "ModInvertedWindowBaseNode.h"
#include "ModInvertedAtomicNode.h"

class ModOstrStream;

class ModInvertedOrderedDistanceLocationListIterator;

//
// CLASS
// ModInvertedOrderedDistanceNode -- 間隔演算(正確な距離を指定した距離演算)を表すノード
//
// NOTES
// 検索式内部表現中間ノードクラスの派生クラスとして実装したが、意味的には、
// 積集合演算子ノードクラスの派生クラスとも言える。
// ２つの子ノードの条件を満足するだけでなく、両者の文書内位置のズレが指定
// された値と丁度一致する組合せが存在する文書のみを結果として返すような検索を
// 行なう。出現順序を考慮する。
// 単一の索引単位に対応しない検索語を表現するのにも用いられる。
//
class
ModInvertedOrderedDistanceNode 
	: public ModInvertedWindowBaseNode
{
public:
	typedef ModInvertedOrderedDistanceNode OrderedDistanceNode;
	typedef ModMap<QueryNode*,int,ModLess<QueryNode*> >	QueryNodePointerMap;
	typedef ModInvertedOrderedDistanceLocationListIterator OrderedDistanceLocationIterator;

	// コンストラクタ
	ModInvertedOrderedDistanceNode(ModSize freq,const  ModUInt32 resultType_);

#ifndef DEL_BOOL
	// 検索の一括実行
	virtual void retrieve(BooleanResult& queryResult,
						  Query::EvaluateMode mode);
#endif
	// 与えられた文書が検索条件を満たすかどうかの検査
	virtual ModBoolean evaluate(DocumentID documentID,
								Query::EvaluateMode mode);

	// 与えられた文書ID以降の、検索条件を満たす文書のIDの最小値を返す。
	virtual ModBoolean lowerBound(DocumentID givenDocumentID,
								  DocumentID& foundDocumentID,
								  Query::EvaluateMode mode);

	// 子ノードの追加
	void insertChild(ModSize position, ModInvertedQueryNode* child);
	// 使用できないようにオーバーライド
	void insertChild(ModInvertedQueryNode* child) { ModAssert(0); }

	// 中間ノードの共有化
	virtual ModSize sharedQueryNode(QueryNodeMap& globalNodeMap,
									QueryNodePointerMap& nodePointerMap);

	// queryNodeForRoughEvaluation の作成
	virtual void makeRoughPointer(const Query::ValidateMode,
								  QueryNodePointerMap&,
								  const ModInvertedQuery* Query);

	// 自分のコピーを作成する
	virtual QueryNode* duplicate(const ModInvertedQuery&)
			{ModAssert(0); return 0; }

	// 文書内頻度を得る
	virtual ModSize getTermFrequency(DocumentID documentID,
									 Query::EvaluateMode mode);

	// sortFactor の計算
	virtual ModSize calcSortFactor();

	// 演算子を表わす文字列を返す
	virtual void prefixString(ModUnicodeString& prefix,
							  const ModBoolean withCalOrCombName,
							  const ModBoolean withCalOrCombParam) const;

	// 有効化の最後に子ノードの数をチェックする
	virtual void checkQueryNode(ModInvertedQuery*, 
								const ModBoolean,
								const ModBoolean);

#ifdef DEBUG
	static int countLocCheck;
#endif

#if 1	// あいまい ------------------ FOR PUBLIC -----------------------------

	// RankingOr,RankingAndの場合は子ノード数の分だけsocresをリザーブ
	virtual void reserveScores();

	// TermLeafNode を消去して SimpleToken/OrderedDistance にする
	virtual ModBoolean eraseTermLeafNode(QueryNode*& node, Query& query);

#endif	// あいまい ------------------ FOR PUBLIC -----------------------------

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

	virtual void validate(InvertedFile* invertedFile,
						  const ModInvertedQuery::ValidateMode mode,
						  ModInvertedQuery* rQuery);

protected:
	// friend class ModInvertQueryParser;

	// 子ノードリストの並べ替え (実際には何もしない)
	void sortChildren(const ModInvertedQuery::ValidateMode mode);

	// 粗い evaluate 満足を前提とした、正確な再評価
	virtual ModBoolean reevaluate(DocumentID documentID);
	// 位置情報リストが必要版
	// ただし、位置情報リストが取得できない場合は、TFを取得する
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModSize& uiTF_,
								  ModInvertedQueryNode* givenEndNode = 0);

	//  000309
	// 
	// オリジナルのTermLeafNodeの文字列
	// 検索木の表示のさいに用いる。
	// TermLeafNodeから作成されたOrderedDistanceのうち、TOPのものだけが、
	// 保持する。Query::eraseTermLeafNodeQuery()でTermLeafNodeを削除する
	// 時にセットする
	ModUnicodeString originalTermString;

#if	1 // あいまい ------------------ FOR PROTECTED --------------------------

	// 検索条件ノードを出力 ラフノード表示 on/off 可
	virtual void getQueryString(ModUnicodeString& out, 
				const ModBoolean asTermString = ModFalse,
				const ModBoolean withCalOrCombName = ModTrue, 
				const ModBoolean withCalOrCombParam = ModTrue,
				const ModBoolean withRouh = ModFalse) const;

#endif	// あいまい ------------------ FOR PROTECTED --------------------------

#ifdef V1_6
	ModLanguageSet langSet;
#endif // V1_6
	ModInvertedTermMatchMode mmode;

	// 位置情報
	ModVector<ModSize> pathPosition;

	// 最大の位置
	ModSize maxPosition;
	// 最大位置の要素番号
	ModSize maxElement;

private:
	// 使用禁止
	ModInvertedOrderedDistanceNode(const OrderedDistanceNode& original);
	OrderedDistanceNode& operator=(const OrderedDistanceNode& original);
};

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::ModInvertedOrderedDistanceNode -- 間隔演算ノードの生成
//
// NOTES
// 間隔演算(正確な距離を指定した距離演算)ノードを生成する。
//
// ARGUMENTS
// ModSize distance_
//	間隔
//
inline
ModInvertedOrderedDistanceNode::ModInvertedOrderedDistanceNode(ModSize freq,const  ModUInt32 resultType_)
	: ModInvertedWindowBaseNode(AtomicNode::orderedDistanceNode,resultType_),
	  maxPosition(0), maxElement(0)
{
	ModInvertedOperatorAndNode::setTotalDocumentFrequency(freq);
}

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::getTermFrequency -- 文書内頻度を得る
//
// NOTES
// 文書内頻度を得る。
//
// ARGUMENTS
// ModInvertedDocumentID DocumentID
//      文書ID
// Query::EvaluateMode mode
//      評価モード
//
// RETURN
// 文書内頻度
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModSize
ModInvertedOrderedDistanceNode::getTermFrequency(DocumentID documentID,
												 Query::EvaluateMode mode)
{
	return ModInvertedOperatorAndNode::getTermFrequency(documentID, mode);
}

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::sortChildren -- 子ノードリストの並べ替え
//
// NOTES
// 形式上、子ノードリストを並べ替える。実際には、距離演算の子ノードを
// 並べ替えてはいけないので、何もしない。AndNodeのsortChildren()をオー
// バライドしている。
//
// ARGUMENTS
// const ModInvertedQuery::ValidateMode mode
//
// RETURN
// なし
//
inline void
ModInvertedOrderedDistanceNode::sortChildren(
	const ModInvertedQuery::ValidateMode mode)
{}

#if 1	// あいまい ------------------ FOR PUBLIC -----------------------------

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::reserveScores -- scoresをリザーブ
//
inline void
ModInvertedOrderedDistanceNode::reserveScores()
{
	// orderedDistanceではscoresはない
	// なにもしない
}

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::eraseTermLeafNode -- TermLeafNodeの消去
//
inline ModBoolean
ModInvertedOrderedDistanceNode::eraseTermLeafNode(QueryNode*& node,Query& query)
{
	// これまでandnode::eraseTermLeafNodeを行っていたが、
	// orderedDistanceにeraseTermLeafNodeが呼ばれるのは異常なケース
	; ModAssert(0);
	return ModTrue;
}

//
// FUNCTION public
// ModInvertedOrderedDistanceNode::validate
//
inline void
ModInvertedOrderedDistanceNode::validate(InvertedFile* invertedFile,
									const Query::ValidateMode mode,
									ModInvertedQuery* rQuery)
{
	// ordredDistanceはTermLeafNodeのvalidateで作成されるので、
	// validateが呼ばれることはない

	// とりあえずAssertにする
	//
	// ただし、orderedDistanceを検索条件として与えられるようにしたいという話
	// もあるのでその場合はここを修正する必要がある
	; ModAssert(0);
}

#endif	// あいまい ------------------ FOR PUBLIC -----------------------------

#if 1	// あいまい ------------------ FOR PROTECTED --------------------------

//
// FUNCTION protected
// ModInvertedOrderedDistanceNode::getQueryString()
//
inline void 
ModInvertedOrderedDistanceNode::getQueryString(ModUnicodeString& out,
	const ModBoolean asTermString,
	const ModBoolean withCalOrCombName,
	const ModBoolean withCalOrCombParam,
	const ModBoolean withRouh) const
{
	ModInvertedQueryInternalNode::getQueryString(out, asTermString, withCalOrCombName,
												 withCalOrCombParam, withRouh);
}

#endif	// あいまい ------------------ FOR PROTECTED --------------------------

#endif //__ModInvertedOrderedDistanceNode_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2004, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
