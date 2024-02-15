// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HeaderPage.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Array/HeaderPage.h"

#include "Common/Assert.h"
#include "Os/Memory.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

//
//	FUNCTION public
//	Array::HeaderPage::HeaderPage -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
// 	Array::ArrayFile& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
HeaderPage::HeaderPage(File& cFile_)
	: Page(cFile_), m_uiTupleCount(0)
{
}

//
//	FUNCTION public
//	Array::HeaderPage::~HeaderPage -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
HeaderPage::~HeaderPage()
{
}

//
//	FUNCTION public
//	Array::HeaderPage::setPhysicalPage -- 物理ページを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::setPhysicalPage(PhysicalFile::Page* pPage_)
{
	Page::setPhysicalPage(pPage_);

	// Not care whether all Header is used or not.
	const ModUInt32* p = getBuffer();
	m_cDataHeader.restore(p);
	m_cNullDataHeader.restore(p);
	m_cNullArrayHeader.restore(p);
	Os::Memory::copy(&m_uiTupleCount, p, static_cast<ModSize>(sizeof(ModSize)));
}

//
//	FUNCTION public
//	Array::HeaderPage::initialize -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
HeaderPage::initialize()
{
	// Initialize each TreeHeader.
	m_cDataHeader.initialize();
	m_cNullDataHeader.initialize();
	m_cNullArrayHeader.initialize();
}

//
//	FUNCTION public
//	Array::HeaderPage::preFlush -- 確定前処理を行う
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::preFlush()
{
	// Not care whether all Header is used or not.
	ModUInt32* p = getBuffer();
	m_cDataHeader.dump(p);
	m_cNullDataHeader.dump(p);
	m_cNullArrayHeader.dump(p);
	Os::Memory::copy(p, &m_uiTupleCount, static_cast<ModSize>(sizeof(ModSize)));
}

//
//	FUNCTION public
//	Array::HeaderPage::decrementTupleCount --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
HeaderPage::decrementTupleCount()
{
	; _SYDNEY_ASSERT(m_uiTupleCount > 0);
	--m_uiTupleCount;
}

//
//	FUNCTION public
//	Array::HeaderPage::getOneEntryTupleCount --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
HeaderPage::getOneEntryTupleCount() const
{
	// In NullArrayTree, the tuple always consists of one entry.
	return m_cNullArrayHeader.getCount();

	// m_uiTupleCount had better to be returnd,
	// when the number of array's elements is 1.
	// But, such case is rare, so NOT need to implement it.
}

//
//	Copyright (c) 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
