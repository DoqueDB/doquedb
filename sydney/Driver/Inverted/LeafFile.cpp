// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LeafFile.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Inverted/LeafFile.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/Parameter.h"
#include "Inverted/FakeError.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//  VARIABLE
	//
	const Os::Ucs2 _pszPath[] = {'L','e','a','f',0};

	//
	//  VARIABLE
	//
	ParameterInteger _cLeafCachePageSize("Inverted_LeafCachePageSize", 6);
}

//
//  FUNCTION public
//  Inverted::LeafFile::LeafFile -- コンストラクタ
//
//  NOTES
//  コンストラクタ。
//
//  ARGUMENTS
//  const Inverted::FileID& cFileID_
//	  転置ファイルパラメータ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
LeafFile::LeafFile(const Inverted::FileID& cFileID_,
					 const Os::Path& cFilePath_,
					 bool batch_)
	: File(Type::Leaf, _cLeafCachePageSize.get())
{
	// 物理ファイルをアタッチする
	attach(cFileID_,
			 cFileID_.getLeafPageSize(),
			 cFilePath_,
			 Os::Path(_pszPath),
			 batch_);
}

//
//  FUNCTION public
//  Inverted::LeafFile::~LeafFile -- デストラクタ
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
//  Inverted::LeafFile::create -- ファイルを作成する
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::InvertedUnit& cInvertedUnit_
//	  転置ファイル
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
	File::create();

	try
	{
		// ページをattachする
		PagePointer pPage
			= allocatePage(PhysicalFile::ConstValue::UndefinedPageID,
							 PhysicalFile::ConstValue::UndefinedPageID);

		// 空文字列のエリアを作成する
		ModUnicodeString cstrKey;
		pPage->insert(cInvertedUnit_, cstrKey, 0);

		// 転置リスト数を1つ増やす
		cInvertedUnit_.incrementListCount();
	}
	catch (...)
	{
		recoverAllPages();
		File::destroy();
		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION public
//  Inverted::LeafFile::move -- ファイルを移動する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  const Os::Path& cFilePath_
//	  パス
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
LeafFile::move(const Trans::Transaction& cTransaction_,
						const Os::Path& cFilePath_)
{
	File::move(cTransaction_, cFilePath_, Os::Path(_pszPath));
}

//
//  FUNCTION public
//  Inverted::LeafFile::clear -- クリアする
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::InvertedUnit& cInvertedUnit_
//	  転置ファイル
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  bool bForce_
//	  強制モードかどうか
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
LeafFile::clear(InvertedUnit& cInvertedUnit_, const Trans::Transaction& cTransaction_, bool bForce_)
{
	// まず下位を実行する
	File::clear(cTransaction_, bForce_);

	// ページをattachする
	PagePointer pPage = allocatePage(PhysicalFile::ConstValue::UndefinedPageID,
									 PhysicalFile::ConstValue::UndefinedPageID);

	// 空文字列のエリアを作成する
	ModUnicodeString cstrKey;
	pPage->insert(cInvertedUnit_, cstrKey, 0);

	// 転置リスト数を1つ増やす
	cInvertedUnit_.incrementListCount();
}

//
//  FUNCTION public
//  Inverted::LeafFile::allocatePage -- ページをallocateする
//
//  NOTES
//
//  ARGUMENTS
//  PhysicalFile::PageID uiPrevPageID_
//	  前ページリンクするページID
//  PhysicalFile::PageID uiNextPageID_
//	  後ページリンクするページID
//
//  RETURN
//  Inverted::LeafFile::PagePointer
//	  allocateしたページ
//
//  EXCEPTIONS
//
LeafFile::PagePointer
LeafFile::allocatePage(PhysicalFile::PageID uiPrevPageID_,
						 PhysicalFile::PageID uiNextPageID_)
{
	PagePointer pLeafPage
		= _SYDNEY_DYNAMIC_CAST(LeafPage*, File::getFreePage());
	if (pLeafPage)
	{
		pLeafPage->reset(uiPrevPageID_, uiNextPageID_);
	}
	else
	{
		; _INVERTED_FAKE_ERROR(LeafFile::allocatePage);
		// 新しいページを確保する
		PhysicalFile::Page* pPage = File::allocatePage();
		 pLeafPage = _SYDNEY_DYNAMIC_CAST(LeafPage*, File::popInstanceList());
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

	File::attachPage(pLeafPage);

	return pLeafPage;
}

//
//  FUNCTION public
//  Inverted::LeafFile::attachPage -- ページをアタッチする
//
//  NOTES
//
//  ARGUMENTS
//  PhysicalFile::PageID uiPageID_
//	  ページID
//
//  RETURN
//  Inverted::LeafFile::PagePointer
//	  アタッチしたページ
//
//  EXCEPTIONS
//
LeafFile::PagePointer
LeafFile::attachPage(PhysicalFile::PageID uiPageID_)
{
	// キャッシュを検索する
	PagePointer pLeafPage = _SYDNEY_DYNAMIC_CAST(LeafPage*, findMap(uiPageID_));

	if (pLeafPage == 0)
	{
		; _INVERTED_FAKE_ERROR(LeafFile::attachPage);
		// 物理ページをアタッチする
		PhysicalFile::Page* pPage = attachPhysicalPage(uiPageID_);
		pLeafPage = _SYDNEY_DYNAMIC_CAST(LeafPage*, File::popInstanceList());
		if (pLeafPage == 0)
		{
			pLeafPage = new LeafPage(*this, pPage);
		}
		else
		{
			pLeafPage->reset(pPage);
		}
		File::attachPage(pLeafPage);
	}

	return pLeafPage;
}

#ifndef SYD_COVERAGE
//
//  FUNCTION public
//  Inverted::LeafFile::reportFile -- ファイル状態を出力する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  ModOstream& stream_
//	  出力ストリーム
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
//  Copyright (c) 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
