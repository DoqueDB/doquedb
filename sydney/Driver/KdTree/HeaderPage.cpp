// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HeaderPage.cpp --
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
#include "KdTree/HeaderPage.h"

#include "KdTree/BtreeFile.h"

#include "Os/Math.h"
#include "Os/Memory.h"

#include "Common/Message.h"

#include "Exception/BadArgument.h"

#include "ModAlgorithm.h"
#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	//
	//	FUNCTION local
	//	_$$::_calcSelectCount -- 検索に利用される次元数を得る
	//
	int _calcSelectCount(int iDimension_)
	{
		// 次元数の平方根を四捨五入した数が検索に利用される次元数となる
		
		return static_cast<int>(
			Os::Math::sqrt(static_cast<double>(iDimension_)) + 0.5);
	}
	
	//
	//	FUNCTION local
	//	_$$::_calcHeaderSize -- ヘッダーのサイズを求める
	//
	int _calcHeaderSize(int iSelectCount_)
	{
		if (iSelectCount_ < 1) iSelectCount_ = 1;
		
		return static_cast<int>(
			(sizeof(HeaderPage::Header)
			 + (sizeof(ModInt32) * (iSelectCount_ - 1)) + 7) / 8 * 8);
	}
}

//
//	FUNCTION public
//	KdTree::HeaderPage::HeaderPage -- コンストラクタ
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
HeaderPage::HeaderPage(BtreeFile& cFile_, PhysicalFile::Page* pPage_)
	: Page(cFile_, pPage_)
{
	m_pHeader = syd_reinterpret_cast<Header*>(getBuffer());
}

//
//	FUNCTION public
//	KdTree::HeaderPage::~HeaderPage -- デストラクタ
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
HeaderPage::~HeaderPage()
{
}

//
//	FUNCTION public
//	KdTree::HeaderPage::initialize -- 初期化する
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::initialize()
{
	dirty();
	clear(0);

	m_pHeader = syd_reinterpret_cast<Header*>(getBuffer());

	m_pHeader->m_iCount = 0;
	m_pHeader->m_uiRootPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_pHeader->m_uiLeftPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_pHeader->m_uiRightPageID = PhysicalFile::ConstValue::UndefinedPageID;
}

//
//	FUNCTION public
//	KdTree::HeaderPage::addCount
//		-- エントリ数を増やす
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::addCount()
{
	dirty();
	++m_pHeader->m_iCount;
}

//
//	FUNCTION public
//	KdTree::HeaderPage::delCount
//		-- エントリ数を減らす
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::delCount()
{
	dirty();
	--m_pHeader->m_iCount;
}

//
//	FUNCTION public
//	KdTree::HeaderPage::setRootPageID
//		-- その次元のルートページのページIDを設定する
//
//	NOTES
//
//	ARGUMETNS
//	PhysicalFile::PageID id_
//		ルートページのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::setRootPageID(PhysicalFile::PageID id_)
{
	dirty();
	m_pHeader->m_uiRootPageID = id_;
	if (id_ == PhysicalFile::ConstValue::UndefinedPageID)
	{
		// ルートページが存在しないので、左右のリーフも存在しない
		
		m_pHeader->m_uiLeftPageID = id_;
		m_pHeader->m_uiRightPageID = id_;
	}
}

//
//	FUNCTION public
//	KdTree::HeaderPage::setLeftPageID
//		-- その次元の左端のリーフページのページIDを設定する
//
//	NOTES
//
//	ARGUMETNS
//	PhysicalFile::PageID id_
//		リーフページのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::setLeftPageID(PhysicalFile::PageID id_)
{
	dirty();
	m_pHeader->m_uiLeftPageID = id_;
}

//
//	FUNCTION public
//	KdTree::HeaderPage::setRightPageID
//		-- その次元の右端のリーフページのページIDを設定する
//
//	NOTES
//
//	ARGUMETNS
//	PhysicalFile::PageID id_
//		リーフページのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::setRightPageID(PhysicalFile::PageID id_)
{
	dirty();
	m_pHeader->m_uiRightPageID = id_;
}

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
