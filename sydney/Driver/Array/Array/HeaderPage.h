// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HeaderPage.h -- ヘッダーページ
// 
// Copyright (c) 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ARRAY_HEADERPAGE_H
#define __SYDNEY_ARRAY_HEADERPAGE_H

#include "Array/Module.h"

#include "Array/Page.h"
#include "Array/PagePointer.h"		//ObjectPointer
#include "Array/TreeHeader.h"

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_ARRAY_BEGIN

//
//	CLASS
//	Array::HeaderPage -- ヘッダーページ
//
//	NOTES
//
class HeaderPage : public Page
{
public:

	//
	//	TYPEDEF
	//	Array::HeaderPage::Pointer
	//
	typedef PageObjectPointer<HeaderPage> PagePointer;
	
	// コンストラクタ
	HeaderPage(File& cFile_);
	// デストラクタ
	virtual ~HeaderPage();

	// 物理ページを設定する
	void setPhysicalPage(PhysicalFile::Page* pPhysicalPage_);

	// 使用サイズを得る [unit size]
	ModSize getUsedSize() { return getUsedByteSize() / sizeof(ModUInt32); }

	// 初期化する
	void initialize();

	// flush前処理
	void preFlush();

	// Get the total of the entries in all trees.
	ModSize getTupleCount() const { return m_uiTupleCount; }
	// Increment the count of the tuples.
	void incrementTupleCount() { ++m_uiTupleCount; }
	// Decrement the count of the tuples.
	void decrementTupleCount();

	// Get the total of tuples which always consist of only one entry. 
	ModSize getOneEntryTupleCount() const;

	// Get the TreeHeader.
	// [YET!] constではないポインタを返してしまっている。
	TreeHeader* getDataHeader() { return &m_cDataHeader; }
	TreeHeader* getNullDataHeader() { return &m_cNullDataHeader; }
	TreeHeader* getNullArrayHeader() { return &m_cNullArrayHeader; }

private:
	// Get Used size [byte].
	ModSize getUsedByteSize() const
	{
		return m_cDataHeader.getDumpedByteSize() +
			m_cNullDataHeader.getDumpedByteSize() +
			m_cNullArrayHeader.getDumpedByteSize() +
			sizeof(ModSize);
	}

	TreeHeader m_cDataHeader;		// DataTree Header
	TreeHeader m_cNullDataHeader;	// NullDataTree Header
	TreeHeader m_cNullArrayHeader;	// NullArrayTree Header

	// The number of the tuples
	ModSize m_uiTupleCount;
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_HEADERPAGE_H

//
//	Copyright (c) 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
