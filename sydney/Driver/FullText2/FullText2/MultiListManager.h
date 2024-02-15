// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiListManager.h --
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

#ifndef __SYDNEY_FULLTEXT2_MULTILISTMANAGER_H
#define __SYDNEY_FULLTEXT2_MULTILISTMANAGER_H

#include "FullText2/Module.h"
#include "FullText2/ListManager.h"
#include "FullText2/LeafPage.h"
#include "FullText2/InvertedList.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedSection;

//
//	CLASS
//	FullText2::MultiListManager --
//
//	NOTES
//	distribute版のListManager
//
class MultiListManager : public ListManager
{
public:
	// コンストラクタ
	MultiListManager(FullTextFile& cFile_,
					 InvertedSection& cSection_);
	// デストラクタ
	virtual ~MultiListManager();
	// コピーコンストラクタ
	MultiListManager(const MultiListManager& cSrc_);

	// コピーする
	ListManager* copy() const;

	// 配列数を予約する
	void reserve(ModSize size_);
	// ListManagerを追加する
	void pushBack(ListManager* pListManager_);

	// キー文字列の取得
	const ModUnicodeString& getKey() const;

	// 先頭に位置付けられた反復子の取得
	// (得たポインタは delete すること)
	ListIterator* getIterator();

	// リストを割り当てる
	bool reset(const ModUnicodeString& cstrKey_,
			   AccessMode::Value eAccessMode_);
	// 次のリストに移動
	bool next();

protected:
	// ListManagerを破棄する
	void destruct();

	// 転置ファイルセクション
	InvertedSection& m_cSection;
	
	// 各ユニットごとのListManager
	ModVector<ModPair<ListManager*, bool> > m_vecpListManager;

	// 一番小さい索引単位
	ModUnicodeString m_cstrKey;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_MULTILISTMANAGER_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
