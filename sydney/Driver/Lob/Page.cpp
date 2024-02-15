// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.cpp --
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Lob";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Lob/Page.h"
#include "Lob/File.h"

#include "PhysicalFile/File.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

//
//	FUNCTION public
//	Lob::Page::Page -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Lob::File& cFile_
//		ファイル
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Page::Page(File& cFile_, PhysicalFile::Page* pPhysicalPage_)
	: m_cFile(cFile_), m_pPhysicalPage(pPhysicalPage_),
	  m_bFree(false), m_iAttachCounter(0), m_iReference(0)
{
}

//
//	FUNCTION public
//	Lob::Page::~Page -- デストラクタ
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
Page::~Page()
{
}

//
//	FUNCTION public
//	Lob::Page::isReadOnly -- 読み込み専用かどうか
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
//	Lob::Page::isDirty -- 内容が変更されたかどうか
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
//	Lob::Page::getPageSize -- ページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		実際に使用可能なページサイズ
//
//	EXCEPTIONS
//
ModSize
Page::getPageSize()
{
	return m_pPhysicalPage->getPageDataSize() - sizeof(int);
}

//
//	FUNCTION public
//	Lob::Page::getFreeSize -- 空きページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		空きページサイズ
//
//	EXCEPTIONS
//
ModSize
Page::getFreeSize()
{
	return getPageSize() - getUsedSize();
}

//
//	FUNCTION public
//	Lob::Page::clear -- ページを0クリアする
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
	ModOsDriver::Memory::set(getBuffer(), c_, getPageSize());
	dirty();
}

//
//	FUNCTION public
//	Lob::Page::detach -- ページをdetachする
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
	if (m_bFree == false)
		m_cFile.detachPage(this);
}

//
//	FUNCTION public static
//	Lob::Page::getPageSize -- ページサイズを得る
//
//	NOTES
//	利用可能なページサイズを得る
//
//	ARGUMENTS
//	Os::Memory::Size uiPageSize_
//		元のページサイズ
//
//	RETURN
//	ModSize
//		利用可能なページサイズ
//
//	EXCEPTIONS
//
ModSize
Page::getPageSize(Os::Memory::Size uiPageSize_)
{
	return PhysicalFile::File::getPageDataSize(
		PhysicalFile::PageManageType, uiPageSize_) - sizeof(int);
}

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
