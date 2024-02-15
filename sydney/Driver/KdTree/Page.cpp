// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.cpp --
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "KdTree/Page.h"
#include "KdTree/BtreeFile.h"

#include "Buffer/Page.h"

#include "Version/Page.h"

#include "Os/Memory.h"
#include "Os/AutoCriticalSection.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

//
//	FUNCTION public
//	KdTree::Page::Page -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	KdTree::BtreeFile& cFile_
//		ファイル
//	PhysicalFile::Page* pPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Page::Page(BtreeFile& cFile_, PhysicalFile::Page* pPage_)
	: m_cFile(cFile_), m_pPhysicalPage(pPage_),
	  m_iReference(0), m_bFree(false)
{
}

//
//	FUNCTION public
//	KdTree::Page::~Page -- デストラクタ
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

//	FUNCTION public
//	KdTree::Page::isReadOnly -- 読み込み専用かどうか
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
//	KdTree::Page::isDirty -- 内容が変更されたかどうか
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
//	KdTree::Page::getPageDataSize -- 利用可能なページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		使用可能なページサイズ
//
//	EXCEPTIONS
//
int
Page::getPageDataSize() const
{
	return static_cast<int>(m_pPhysicalPage->getPageDataSize());
}

//
//	FUNCTION public
//	KdTree::Page::dirty -- ページをdirtyにする
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
//	KdTree::Page::clear -- ページを0クリアする
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
	Os::Memory::set(getBuffer(), c_, getPageDataSize());
	dirty();
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
//	FUNCTION public
//	KdTree::Page::detach -- ページをdetachする
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
	// フリーされた場合、すでにフリーリストにのっているので、
	// detachPage してはいけない
	
	if (m_bFree == false)
		m_cFile.detachPage(this);
}

//
//	FUNCTION public
//	KdTree::BtreePage::updateMode -- 更新モードに変更する
//
//	NOTES
//	ReadOnlyでattachしている物理ページをWriteでattachしなおす。
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
Page::updateMode()
{
	if (!isReadOnly()) return;

	m_pPhysicalPage = m_cFile.changeFixMode(m_pPhysicalPage);
	load();
}

//
//	FUNCTION public static
//	KdTree::Page::getPageDataSize -- 利用可能なサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	int iPageSize_
//		ページサイズ
//
//	RETURN
//	int
//		利用可能サイズ
//
//	EXCEPTIONS
//
int
Page::getPageDataSize(int iPageSize_)
{
	// PhysicalFile::PageManageFile::getPageDataSize と同じ実装
	return static_cast<int>(Version::Page::getContentSize(iPageSize_));
}

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
