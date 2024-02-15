// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/Page.h"
#include "FullText2/IndexFile.h"
#include "Buffer/Page.h"
#include "ModOsDriver.h"

_SYDNEY_USING

using namespace FullText2;

//
//	FUNCTION public
//	FullText2::Page::Page -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	FullText2::IndexFile& cFile_
//		ファイル
//	PhysicalFile::Page* pPhysicalPage_
//		物理ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Page::Page(IndexFile& cFile_, PhysicalFile::Page* pPhysicalPage_)
	: m_cFile(cFile_),
	  m_pPhysicalPage(pPhysicalPage_),
	  m_pNext(0),
	  m_bFree(false),
	  m_iReference(0),
	  _next(0), _prev(0)
{
}

//
//	FUNCTION public
//	FullText2::Page::~Page -- デストラクタ
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
//	FullText2::Page::reset -- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Page::reset(PhysicalFile::Page* pPhysicalPage_)
{
	m_pPhysicalPage = pPhysicalPage_;
	m_pNext = 0;
	m_bFree = false;
	_next = 0;
	_prev = 0;
}

//
//	FUNCTION public
//	FullText2::Page::isReadOnly -- 読み込み専用かどうか
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
//	FullText2::Page::isDirty -- 内容が変更されたかどうか
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
	return (m_pPhysicalPage->getUnfixMode()
			== PhysicalFile::Page::UnfixMode::Dirty);
}

//
//	FUNCTION public
//	FullText2::Page::getPageUnitSize -- ページユニットサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		実際に使用可能なページユニットサイズ
//
//	EXCEPTIONS
//	???
//
ModSize
Page::getPageUnitSize() const
{
	return getPageUnitSize(m_pPhysicalPage);
}

//
//	FUNCTION public static
//	FullText2::Page::getPageUnitSize -- ページユニットサイズを得る
//
//	NOTES
//
//	ARGUMENTS
// 	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//
//	RETURN
//	ModSize
//		実際に使用可能なページユニットサイズ
//
//	EXCEPTIONS
//	???
//
ModSize
Page::getPageUnitSize(PhysicalFile::Page* pPhysicalPage_)
{
	return pPhysicalPage_->getPageDataSize()/sizeof(ModUInt32);
}

//
//	FUNCTION public
//	Page::getFreeUnitSize -- 空きユニット数を得る
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
Page::getFreeUnitSize() const
{
	return getPageUnitSize() - getUsedUnitSize();
}

//
//	FUNCTION public
//	FullText2::Page::clear -- ページを0クリアする
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
	clear(m_pPhysicalPage, c_);
}

//
//	FUNCTION public static
//	FullText2::Page::clear -- ページをクリアする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//	unsigned char c_
//		初期値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Page::clear(PhysicalFile::Page* pPhysicalPage_, unsigned char c_)
{
	ModOsDriver::Memory::set(getBuffer(pPhysicalPage_), c_,
							 pPhysicalPage_->getPageDataSize());
	pPhysicalPage_->dirty();
}

//
//	FUNCTION public
//	FullText2::Page::incrementReference -- 参照カウンタを増やす
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
Page::incrementReference()
{
	//【注意】
	//
	// PageにCriticalSectionを持っていた方が同時実行性は向上するが、
	// リソース消費を考えて、Fileのものを利用する
	
	Os::AutoCriticalSection cAuto(m_cFile.getLatch());
	++m_iReference;
}

//
//	FUNCTION public
//	FullText2::Page::decrementReference -- 参照カウンタを減らす
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
Page::decrementReference()
{
	//【注意】
	//
	// PageにCriticalSectionを持っていた方が同時実行性は向上するが、
	// リソース消費を考えて、Fileのものを利用する
	
	Os::AutoCriticalSection cAuto(m_cFile.getLatch());
	if (--m_iReference == 0)
	{
		detach();
	}
}

//
//	FUNCTION private
//	FullText2::Page::detach -- ページをdetachする
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
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
