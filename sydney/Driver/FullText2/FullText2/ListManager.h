// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListManager.h --
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

#ifndef __SYDNEY_FULLTEXT2_LISTMANAGER_H
#define __SYDNEY_FULLTEXT2_LISTMANAGER_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "FullText2/ListIterator.h"
#include "FullText2/Tokenizer.h"

#include "ModUnicodeString.h"

class ModInvertedCoder;

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class FullTextFile;

//
//	CLASS
//	FullText2::ListManager -- 
//
//	NOTES
//	本クラスは検索処理が必要とするインターフェース
//	を規定しているクラスである。
//
class ListManager
{
public:
	//
	//	STRUCT
	//	FullText2::ListManager::AccessMode -- 転置リストアクセスモード
	//
	struct AccessMode
	{
		enum Value
		{
			Create,		// なければ新規作成
			Search,		// 完全一致検索
			LowerBound	// 下限検索
		};
	};

	// コンストラクタ
	ListManager(FullTextFile& cFile_);
	// デストラクタ
	virtual ~ListManager();
	// コピーコンストラクタ
	ListManager(const ListManager& cSrc_);

	// コピーを得る
	virtual ListManager* copy() const = 0;

	//
	// 転置リストに関するメソッド
	//
	
	// キー文字列の取得
	virtual const ModUnicodeString& getKey() const = 0;

	// 先頭に位置付けられた反復子の取得
	// (得たポインタは delete すること)
	virtual ListIterator* getIterator() = 0;

	// リストを割り当てる
	virtual bool reset(const ModUnicodeString& cstrKey_,
					   AccessMode::Value eAccessMode_) = 0;
	// 次のリストに移動
	virtual bool next() = 0;

	//
	// 検索時に利用するメソッド
	//

	// 位置情報が格納されているかどうか
	bool isNoLocation() const { return m_bNoLocation; }

	// トークナイザーを得る
	Tokenizer::AutoPointer getTokenizer();

protected:
	// 全文ファイル
	FullTextFile& m_cFile;

	// 位置情報が格納されているかどうか
	bool m_bNoLocation;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_LISTMANAGER_H

//
//  Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
