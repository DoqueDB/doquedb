// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LeafNode.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_LEAFNODE_H
#define __SYDNEY_FULLTEXT2_LEAFNODE_H

#include "FullText2/Module.h"
#include "FullText2/LocationListManager.h"
#include "FullText2/LocationListIterator.h"

#include "ModMap.h"
#include "ModPair.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ListManager;
class SearchInformation;

//
//	CLASS
//	FullText2::LeafNode -- 検索を実行するクラスの基底クラス
//
//	NOTES
//
class LeafNode : public LocationListManager
{
public:
	// 位置情報と索引単位文字列のペア
	typedef ModPair<ModSize, ModUnicodeString>		LocationPair;
	// 位置情報を配列
	typedef ModVector<LocationPair>					LocationVector;

	// コンストラクタ
	LeafNode();
	// デストラクタ
	virtual ~LeafNode();
	// コピーコンストラクタ
	LeafNode(const LeafNode& src_);

	// おおよその文書頻度を求める
	virtual ModSize getEstimateCount(SearchInformation& cSearchInfo_) = 0;

	// リセットする
	virtual void reset() = 0;
	
	// 文書IDを検索する
	//
	//【注意】
	// 現在の文書ID以下の文書IDが渡された場合には、現在の文書IDを返すこと
	// この動作は ListIterator とは異なる
	// 
	virtual DocumentID lowerBound(SearchInformation& cSearchInfo_,
								  DocumentID id_, bool isRough_) = 0;

	// 文書内頻度を得る
	virtual ModSize getTermFrequency() = 0;
	// 位置情報へのイテレータを得る
	virtual LocationListIterator::AutoPointer getLocationListIterator() = 0;

	// コピー
	virtual LeafNode* copy() const = 0;

	// リーフノードを作成する(NormalLeafNode or SimpleLeafNode)
	static LeafNode* createNormalLeafNode(ListManager& cListManager_,
										  const LocationVector& cToken_);
	
	// リーフノードを作成する(ShortLeafNode or ShortLeafNodeCompatible)
	static LeafNode* createShortLeafNode(ListManager& cListManager_,
										 const ModUnicodeString& cTerm_,
										 int iLength_,
										 ModSize uiMinLength_,
										 bool isCompatible_);

	// 空のリーフノードを作成する
	static LeafNode* createEmptyLeafNode();
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_LEAFNODE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
