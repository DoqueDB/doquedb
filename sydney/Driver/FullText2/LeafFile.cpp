// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LeafFile.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "FullText2/LeafFile.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/Parameter.h"
#include "FullText2/FakeError.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Path.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//  VARIABLE
	//
	ParameterInteger _cLeafCachePageSize("FullText2_LeafCachePageSize", 6);
}

//
//  FUNCTION public
//  FullText2::LeafFile::LeafFile -- コンストラクタ
//
//  NOTES
//  コンストラクタ。
//
//  ARGUMENTS
//  const FullText2::FileID& cFileID_
//		ファイルID
//	const Os::Path& cPath_
//		パス
//	bool bBatch_
//		バッチモードかどうか
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
LeafFile::LeafFile(const FullText2::FileID& cFileID_,
				   const Os::Path& cPath_,
				   bool bBatch_)
	: IndexFile(Type::Leaf, _cLeafCachePageSize.get())
{
	// 物理ファイルをアタッチする
	attach(cFileID_, cFileID_.getLeafPageSize(), cPath_, bBatch_);
}

//
//  FUNCTION public
//  FullText2::LeafFile::~LeafFile -- デストラクタ
//
//  NOTES
//  デストラクタ。
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
LeafFile::~LeafFile()
{
	// 物理ファイルをデタッチする
	detach();
}

//
//  FUNCTION public
//  FullText2::LeafFile::create -- ファイルを作成する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
LeafFile::create(InvertedUnit& cInvertedUnit_)
{
	// まず下位を実行する
	IndexFile::create();

	try
	{
		// ページをattachする
		PagePointer pPage
			= allocatePage(PhysicalFile::ConstValue::UndefinedPageID,
						   PhysicalFile::ConstValue::UndefinedPageID);

		// 空文字列のエリアを作成する
		ModUnicodeString cstrKey;
		pPage->insert(cInvertedUnit_, cstrKey, 0);
	}
	catch (...)
	{
		recoverAllPages();
		IndexFile::destroy();
		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION public
//  FullText2::LeafFile::move -- ファイルを移動する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//		トランザクション
//  const Os::Path& cPath_
//		パス
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
LeafFile::move(const Trans::Transaction& cTransaction_,
			   const Os::Path& cPath_)
{
	IndexFile::move(cTransaction_, cPath_);
}

//
//  FUNCTION public
//  FullText2::LeafFile::clear -- クリアする
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//  bool bForce_
//		強制モードかどうか
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
LeafFile::clear(InvertedUnit& cInvertedUnit_,
				bool bForce_)
{
	// まず下位を実行する
	IndexFile::clear(bForce_);

	// ページをattachする
	PagePointer pPage = allocatePage(PhysicalFile::ConstValue::UndefinedPageID,
									 PhysicalFile::ConstValue::UndefinedPageID);

	// 空文字列のエリアを作成する
	ModUnicodeString cstrKey;
	pPage->insert(cInvertedUnit_, cstrKey, 0);
}

//
//  FUNCTION public
//  FullText2::LeafFile::allocatePage -- ページをallocateする
//
//  NOTES
//
//  ARGUMENTS
//  PhysicalFile::PageID uiPrevPageID_
//		前ページリンクするページID
//  PhysicalFile::PageID uiNextPageID_
//		後ページリンクするページID
//
//  RETURN
//  FullText2::LeafFile::PagePointer
//		allocateしたページ
//
//  EXCEPTIONS
//
LeafFile::PagePointer
LeafFile::allocatePage(PhysicalFile::PageID uiPrevPageID_,
					   PhysicalFile::PageID uiNextPageID_)
{
	Os::AutoCriticalSection cAuto(getLatch());
	
	PagePointer pLeafPage
		= _SYDNEY_DYNAMIC_CAST(LeafPage*, IndexFile::getFreePage());
	if (pLeafPage)
	{
		pLeafPage->reset(uiPrevPageID_, uiNextPageID_);
	}
	else
	{
		; _FULLTEXT2_FAKE_ERROR(LeafFile::allocatePage);
		// 新しいページを確保する
		PhysicalFile::Page* pPage = IndexFile::allocatePage();
		 pLeafPage
			 = _SYDNEY_DYNAMIC_CAST(LeafPage*, IndexFile::popInstanceList());
		if (pLeafPage)
		{
			pLeafPage->reset(pPage, uiPrevPageID_, uiNextPageID_);
		}
		else
		{
			pLeafPage = new LeafPage(*this, pPage,
									 uiPrevPageID_, uiNextPageID_);
		}
	}

	IndexFile::attachPage(pLeafPage);

	return pLeafPage;
}

//
//  FUNCTION public
//  FullText2::LeafFile::attachPage -- ページをアタッチする
//
//  NOTES
//
//  ARGUMENTS
//  PhysicalFile::PageID uiPageID_
//		ページID
//
//  RETURN
//  FullText2::LeafFile::PagePointer
//		アタッチしたページ
//
//  EXCEPTIONS
//
LeafFile::PagePointer
LeafFile::attachPage(PhysicalFile::PageID uiPageID_)
{
	Os::AutoCriticalSection cAuto(getLatch());
	
	// キャッシュを検索する
	PagePointer pLeafPage = _SYDNEY_DYNAMIC_CAST(LeafPage*, findMap(uiPageID_));

	if (pLeafPage == 0)
	{
		; _FULLTEXT2_FAKE_ERROR(LeafFile::attachPage);
		// 物理ページをアタッチする
		PhysicalFile::Page* pPage = attachPhysicalPage(uiPageID_);
		pLeafPage
			= _SYDNEY_DYNAMIC_CAST(LeafPage*, IndexFile::popInstanceList());
		if (pLeafPage == 0)
		{
			pLeafPage = new LeafPage(*this, pPage);
		}
		else
		{
			pLeafPage->reset(pPage);
		}
		IndexFile::attachPage(pLeafPage);
	}

	return pLeafPage;
}

#ifndef SYD_COVERAGE
//
//  FUNCTION public
//  FullText2::LeafFile::reportFile -- ファイル状態を出力する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//		トランザクション
//  ModOstream& stream_
//		出力ストリーム
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
LeafFile::reportFile(const Trans::Transaction& cTransaction_,
					 ModOstream& stream_)
{
	ModSize uiShortList = 0;
	ModSize uiMiddleList = 0;
	ModSize uiTotalUnit = 0;
	ModSize uiUsedUnit = 0;

	// 先頭の物理ページを得る
	PhysicalFile::PageID uiPageID = getTopPageID(cTransaction_);
	while (uiPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		LeafPage::PagePointer pPage = attachPage(uiPageID);

		// トータルユニット数と使用ユニット数
		uiTotalUnit += pPage->getPageUnitSize();
		uiUsedUnit += pPage->getUsedUnitSize();

		LeafPage::Iterator i = pPage->begin();
		for (; i != pPage->end(); ++i)
		{
			if ((*i)->getListType() == ListType::Short)
			{
				uiShortList++;
			}
			else
			{
				// まだLongListはない
				uiMiddleList++;
			}
		}

		// 次へ
		uiPageID = getNextPageID(cTransaction_, uiPageID);
	}

	stream_ << "LeafFile:" << ModEndl;
	stream_ << "\tPageCount=" << getUsedPageNum(cTransaction_) << ModEndl;
	stream_ << "\tUsedFileSize=" << getUsedSize(cTransaction_) << ModEndl;
	stream_ << "\tPageDataSize=" << getPageDataSize() << ModEndl;
	stream_ << "\tShortListCount=" << uiShortList << ModEndl;
	stream_ << "\tMiddleListCount=" << uiMiddleList << ModEndl;
	stream_ << "\tAverageUsedPageRatio="
			<< static_cast<float>(uiUsedUnit)/static_cast<float>(uiTotalUnit)
			<< ModEndl;
}
#endif

//
//  Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
