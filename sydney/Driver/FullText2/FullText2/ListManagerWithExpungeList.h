// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListManagerWithExpungeList.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_LISTMANAGERWITHEXPUNGELIST_H
#define __SYDNEY_FULLTEXT2_LISTMANAGERWITHEXPUNGELIST_H

#include "FullText2/Module.h"
#include "FullText2/ListManager.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedSection;

//
//	CLASS
//	FullText2::ListManagerWithExpungeList -- 
//
//	NOTES
//	削除リストに該当する文書を取り除きなら走査するListManager
//
class ListManagerWithExpungeList : public ListManager
{
public:
	// コンストラクタ
	ListManagerWithExpungeList(FullTextFile& cFile_,
							   InvertedSection& cSection_,
							   ListManager* pListManager_);
	// デストラクタ
	virtual ~ListManagerWithExpungeList();
	// コピーコンストラクタ
	ListManagerWithExpungeList(const ListManagerWithExpungeList& src_);

	// コピーを得る
	ListManager* copy() const;

	//
	// 転置リストに関するメソッド
	//
	
	// キー文字列の取得
	const ModUnicodeString& getKey() const
		{ return m_pListManager->getKey(); }

	// 先頭に位置付けられた反復子の取得
	// (得たポインタは delete すること)
	ListIterator* getIterator();

	// リストを割り当てる
	bool reset(const ModUnicodeString& cstrKey_,
			   AccessMode::Value eAccessMode_)
		{ return m_pListManager->reset(cstrKey_, eAccessMode_); }
	// 次のリストに移動
	bool next()
		{ return m_pListManager->next(); }

protected:
	// 転置ファイルセクション
	InvertedSection& m_cSection;
	// リストマネージャー
	ListManager* m_pListManager;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_LISTMANAGERWITHEXPUNGELIST_H

//
//  Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
