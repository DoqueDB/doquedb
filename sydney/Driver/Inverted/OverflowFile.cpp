// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OverflowFile.cpp --
// 
// Copyright (c) 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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
#include "Inverted/OverflowFile.h"
#include "Inverted/Parameter.h"
#include "Inverted/FakeError.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//	VARIABLE
	//
	const Os::Ucs2 _pszPath[] = {'O','v','e','r','f','l','o','w',0};

	//
	//	VARIABLE
	//
	ParameterInteger _cOverflowCachePageSize("Inverted_OverflowCachePageSize", 3);
}

//
//	FUNCTION public
//	Inverted::OverflowFile::OverflowFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Inverted::FileID& cFileID_
//		転置ファイルパラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
OverflowFile::OverflowFile(const FileID& cFileID_,
						   const Os::Path& cFilePath_,
						   bool batch_)
	: File(Type::Overflow, _cOverflowCachePageSize.get())
{
	// 物理ファイルをアタッチする
	attach(cFileID_,
		   cFileID_.getOverflowPageSize(),
		   cFilePath_,
		   Os::Path(_pszPath),
		   batch_);
}

//
//	FUNCTION public
//	Inverted::OverflowFile::~OverflowFile -- デストラクタ
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
//	なし
//
OverflowFile::~OverflowFile()
{
	// 物理ファイルをデタッチする
	detach();
}

//
//	FUNCTION public
//	Inverted::OverflowFile::allocatePage -- ページを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiBlockSize_
//		IDブロックのユニット数
//
//	RETURN
//	Inverted::OverflowFile::PagePointer
//		新しく確保したページ
//
//	EXCEPTIONS
//
OverflowFile::PagePointer
OverflowFile::allocatePage(ModSize uiBlockSize_)
{
	PagePointer pOverflowPage
		= _SYDNEY_DYNAMIC_CAST(OverflowPage*, File::getFreePage());
	if (pOverflowPage)
	{
		pOverflowPage->reset(uiBlockSize_);
	}
	else
	{
		; _INVERTED_FAKE_ERROR(OverflowFile::allocatePage1);
		// 新しいページを確保する
		PhysicalFile::Page* pPage = File::allocatePage();
		pOverflowPage
			= _SYDNEY_DYNAMIC_CAST(OverflowPage*, File::popInstanceList());
		if (pOverflowPage)
		{
			pOverflowPage->reset2(pPage, uiBlockSize_);
		}
		else
		{
			pOverflowPage = new OverflowPage(*this, pPage, uiBlockSize_);
		}
	}

	File::attachPage(pOverflowPage);

	return pOverflowPage;
}

//
//	FUNCTION public
//	Inverted::OverflowFile::allocatePage -- ページを得る
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	Inverted::OverflowFile::PagePointer
//		新しく確保したページ
//
//	EXCEPTIONS
//
OverflowFile::PagePointer
OverflowFile::allocatePage(PhysicalFile::PageID uiPrevPageID_,
						   PhysicalFile::PageID uiNextPageID_)
{
	PagePointer pOverflowPage
		= _SYDNEY_DYNAMIC_CAST(OverflowPage*, File::getFreePage());
	if (pOverflowPage)
	{
		pOverflowPage->reset(uiPrevPageID_, uiNextPageID_);
	}
	else
	{
		; _INVERTED_FAKE_ERROR(OverflowFile::allocatePage2);
		// 新しいページを確保する
		PhysicalFile::Page* pPage = File::allocatePage();
		pOverflowPage
			= _SYDNEY_DYNAMIC_CAST(OverflowPage*, File::popInstanceList());
		if (pOverflowPage)
		{
			pOverflowPage->reset2(pPage, uiPrevPageID_, uiNextPageID_);
		}
		else
		{
			pOverflowPage
				= new OverflowPage(*this, pPage, uiPrevPageID_, uiNextPageID_);
		}
	}

	File::attachPage(pOverflowPage);

	return pOverflowPage;
}

//
//	FUNCTION public
//	Inverted::OverflowFile::allocatePage -- ページを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiBlockSize_
//		IDブロックのユニット数
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	Inverted::OverflowFile::PagePointer
//		新しく確保したページ
//
//	EXCEPTIONS
//
OverflowFile::PagePointer
OverflowFile::allocatePage(ModSize uiBlockSize_,
						   PhysicalFile::PageID uiPrevPageID_,
						   PhysicalFile::PageID uiNextPageID_)
{
	PagePointer pOverflowPage
		= _SYDNEY_DYNAMIC_CAST(OverflowPage*, File::getFreePage());
	if (pOverflowPage)
	{
		pOverflowPage->reset(uiBlockSize_, uiPrevPageID_, uiNextPageID_);
	}
	else
	{
		; _INVERTED_FAKE_ERROR(OverflowFile::allocatePage3);
		// 新しいページを確保する
		PhysicalFile::Page* pPage = File::allocatePage();
		pOverflowPage
			= _SYDNEY_DYNAMIC_CAST(OverflowPage*, File::popInstanceList());
		if (pOverflowPage)
		{
			pOverflowPage->reset2(pPage, uiBlockSize_,
								  uiPrevPageID_, uiNextPageID_);
		}
		else
		{
			pOverflowPage = new OverflowPage(*this, pPage, uiBlockSize_,
											 uiPrevPageID_, uiNextPageID_);
		}
	}

	File::attachPage(pOverflowPage);

	return pOverflowPage;
}

//
//	FUNCTION public
//	Inverted::OverflowFile::attachPage -- ページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		ページID
//
//	RETURN
//	Inverted::OverflowFile::PagePointer
//		アタッチしたページ
//
//	EXCEPTIONS
//
OverflowFile::PagePointer
OverflowFile::attachPage(PhysicalFile::PageID uiPageID_)
{
	// キャッシュを検索する
	PagePointer pOverflowPage
		= _SYDNEY_DYNAMIC_CAST(OverflowPage*, findMap(uiPageID_));
	if (pOverflowPage == 0)
	{
		; _INVERTED_FAKE_ERROR(OverflowFile::attachPage);
		// 物理ページをアタッチする
		PhysicalFile::Page* pPage = attachPhysicalPage(uiPageID_);
		pOverflowPage
			= _SYDNEY_DYNAMIC_CAST(OverflowPage*, popInstanceList());
		if (pOverflowPage)
		{
			pOverflowPage->reset2(pPage);
		}
		else
		{
			pOverflowPage = new OverflowPage(*this, pPage);
		}
		File::attachPage(pOverflowPage);
	}

	return pOverflowPage;
}

//
//	FUNCTION public
//	Inverted::OverflowFile::freePage -- ページを開放する
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::OverflowPage* pPage_
//		開放するページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowFile::freePage(OverflowPage* pPage_)
{
	if (pPage_->getType() == OverflowPage::Type::LOC ||
		pPage_->getType() == OverflowPage::Type::IDLOC)
	{
		// 前後のリンク情報をつなぎかえる
		if (pPage_->getNextPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			PagePointer pNextPage = attachPage(pPage_->getNextPageID());
			pNextPage->setPrevPageID(pPage_->getPrevPageID());
		}
		if (pPage_->getPrevPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			PagePointer pPrevPage = attachPage(pPage_->getPrevPageID());
			pPrevPage->setNextPageID(pPage_->getNextPageID());
		}
	}
	// 開放する
	File::freePage(pPage_);
}

//
//	FUNCTION public
//	Inverted::OverflowFile::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cFilePath_
//		パス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowFile::move(const Trans::Transaction& cTransaction_,
					  const Os::Path& cFilePath_)
{
	File::move(cTransaction_, cFilePath_, Os::Path(_pszPath));
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//	Inverted::OverflowFile::reportFile -- ファイル状態を出力する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	ModOstream& stream_
//		出力ストリーム
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowFile::reportFile(const Trans::Transaction& cTransaction_,
					 ModOstream& stream_)
{
	ModSize uiIdPageCount = 0;
	ModSize uiLocPageCount = 0;
	ModSize uiIdLocPageCount = 0;

	ModSize uiIdPageTotalUnit = 0;
	ModSize uiIdPageUsedUnit = 0;

	ModSize uiLocPageTotalUnit = 0;
	ModSize uiLocPageUsedUnit = 0;

	ModSize uiIdLocPageTotalUnit = 0;
	ModSize uiIdLocPageUsedUnit = 0;
	
	// 先頭の物理ページを得る
	PhysicalFile::PageID uiPageID = getTopPageID(cTransaction_);
	while (uiPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		OverflowPage::PagePointer pPage = attachPage(uiPageID);

		// トータルユニット数と使用ユニット数
		switch (pPage->getType())
		{
		case OverflowPage::Type::ID:
			uiIdPageCount++;
			uiIdPageTotalUnit += pPage->getPageUnitSize();
			uiIdPageUsedUnit += pPage->getUsedUnitSize();
			break;
		case OverflowPage::Type::LOC:
			uiLocPageCount++;
			uiLocPageTotalUnit += pPage->getPageUnitSize();
			uiLocPageUsedUnit += pPage->getUsedUnitSize();
			break;
		case OverflowPage::Type::IDLOC:
			uiIdLocPageCount++;
			uiIdLocPageTotalUnit += pPage->getPageUnitSize();
			uiIdLocPageUsedUnit += pPage->getUsedUnitSize();
			break;
		}

		// 次へ
		uiPageID = getNextPageID(cTransaction_, uiPageID);
	}

	stream_ << "OverflowFile:" << ModEndl;
	stream_ << "\tPageCount=" << getUsedPageNum(cTransaction_) << ModEndl;
	stream_ << "\tUsedFileSize=" << getUsedSize(cTransaction_) << ModEndl;
	stream_ << "\tPageDataSize=" << getPageDataSize() << ModEndl;
	stream_ << "\tIdPage:" << ModEndl;
	stream_ << "\t\tPageCount=" << uiIdPageCount << ModEndl;
	stream_ << "\t\tAverageUsedPageRatio="
			<< static_cast<float>(uiIdPageUsedUnit)
						/static_cast<float>(uiIdPageTotalUnit)
			<< ModEndl;
	stream_ << "\tLocPage:" << ModEndl;
	stream_ << "\t\tPageCount=" << uiLocPageCount << ModEndl;
	stream_ << "\t\tAverageUsedPageRatio="
			<< static_cast<float>(uiLocPageUsedUnit)
						/static_cast<float>(uiLocPageTotalUnit)
			<< ModEndl;
	stream_ << "\tIdLocPage:" << ModEndl;
	stream_ << "\t\tPageCount=" << uiIdLocPageCount << ModEndl;
	stream_ << "\t\tAverageUsedPageRatio="
			<< static_cast<float>(uiIdLocPageUsedUnit)
						/static_cast<float>(uiIdLocPageTotalUnit)
			<< ModEndl;
}
#endif

//
//	Copyright (c) 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
