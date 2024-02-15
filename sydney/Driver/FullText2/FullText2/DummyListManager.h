// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DummyListManager.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_DUMMYLISTMANAGER_H
#define __SYDNEY_FULLTEXT2_DUMMYLISTMANAGER_H

#include "FullText2/Module.h"
#include "FullText2/ListManager.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedSection;

//
//	CLASS
//	FullText2::DummyListManager --
//
//	NOTES
//	空のListManager
//
class DummyListManager : public ListManager
{
public:
	// コンストラクタ
	DummyListManager(FullTextFile& cFile_);
	// デストラクタ
	virtual ~DummyListManager();
	// コピーコンストラクタ
	DummyListManager(const DummyListManager& cSrc_);

	// コピーする
	ListManager* copy() const;

	// キー文字列の取得
	const ModUnicodeString& getKey() const;

	// 先頭に位置付けられた反復子の取得
	// (得たポインタは delete すること)
	ListIterator* getIterator() { return 0; }

	// リストを割り当てる
	bool reset(const ModUnicodeString& cstrKey_,
			   AccessMode::Value eAccessMode_)
		{ return false; }
	// 次のリストに移動
	bool next() { return false; }

protected:
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_DUMMYLISTMANAGER_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
