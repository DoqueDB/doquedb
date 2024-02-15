// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.cpp --
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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
#include "Array/Page.h"
#include "Array/File.h"

#include "Buffer/Page.h"

#include "Os/Memory.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

//
//	FUNCTION public
//	Array::Page::Page -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Array::File& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Page::Page(File& cFile_)
: m_cFile(cFile_), m_pPhysicalPage(0),
  m_bFree(false), m_iAttachCounter(0), m_iReference(0)
{
}

//
//	FUNCTION public
//	Array::Page::~Page -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Page::~Page()
{
}

//
//	FUNCTION public
//	Array::Page::setPhysicalPage -- 物理ページを設定する
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
Page::setPhysicalPage(PhysicalFile::Page* pPage_)
{
	m_pPhysicalPage = pPage_;
}

//
//	FUNCTION public
//	Array::Page::isReadOnly -- 読み込み専用かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		読み込み専用モードの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
Page::isReadOnly() const
{
	return m_pPhysicalPage->getFixMode() & Buffer::Page::FixMode::ReadOnly;
}

//
//	FUNCTION public
//	Array::Page::isDirty -- 内容が変更されたかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		変更されている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
Page::isDirty() const
{
	return m_pPhysicalPage->getUnfixMode()
				== PhysicalFile::Page::UnfixMode::Dirty;
}

//
//	FUNCTION public
//	Array::Page::getPageSize -- ページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		実際に使用可能なページサイズ(UNIT数)
//
//	EXCEPTIONS
//	???
//
ModSize
Page::getPageSize()
{
	return m_pPhysicalPage->getPageDataSize() / sizeof(ModUInt32);
}

//
//	FUNCTION public
//	Page::getFreeSize -- 空きサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		空きユニット数
//
//	EXCEPTIONS
//	なし
//
ModSize
Page::getFreeSize()
{
	return getPageSize() - getUsedSize();
}

//
//	FUNCTION public
//	Array::Page::dirty -- ページをdirtyにする
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
Page::dirty()
{
	;_SYDNEY_ASSERT(!isReadOnly());
	m_pPhysicalPage->dirty();
}

//
//	FUNCTION public
//	Array::Page::clear -- ページを0クリアする
//
//	NOTES
//
//	ARGUMENTS
//	unsigned char c_
//		初期値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Page::clear(unsigned char c_)
{
	Os::Memory::set(getBuffer(), c_, getPageSize() * sizeof(ModUInt32));
	dirty();
}

//
//	FUNCTION public
//	Array::Page::detach -- ページをdetachする
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
Page::detach()
{
	if (m_pPhysicalPage == 0)
		// attachPhysicalPageで例外が発生するとここにくる
		m_cFile.pushPage(this);
	
	else if (m_bFree == false)
		m_cFile.detachPage(this);
}

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
