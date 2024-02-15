// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectAreaFile.cpp --
//		物理エリア管理機能付き物理ファイル関連の関数定義
// 
// Copyright (c) 2005, 2006, 2007, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "PhysicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "PhysicalFile/Types.h"
#include "PhysicalFile/DirectAreaFile.h"
#include "PhysicalFile/DirectAreaTree.h"
#include "PhysicalFile/DirectAreaPage.h"
#include "PhysicalFile/DirectArea.h"
#include "PhysicalFile/Message_CanNotFixNode.h"
#ifdef DEBUG
#include "PhysicalFile/Parameter.h"
#include "PhysicalFile/Message.h"
#endif

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/BadArgument.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

namespace
{
//
//	VARIABLE local
//	_$$::_cParameterPageSize -- PysicalFile's page size for debug.[byte]
//
//	NOTES
//
#ifdef DEBUG
ParameterInteger _cParameterPageSize("PhysicalFile_PageSize", 0);
#endif

//
//	FUNCTION local
//	_$$::_printDetachPageID -- Print detaching PageID for debug.
//
//	NOTES
//
//	ARGUMENTS
//	PageID							uiPageID_
//
//	RETURN
//
//	EXCEPTIONS
// 
void
_printDetachPageID(PageID uiPageID_)
{
#ifdef DEBUG
	_SYDNEY_PHYSICALFILE_DEBUG_MESSAGE
		<< "Detaching PageID is " << uiPageID_ << " ." << ModEndl;
#endif
}

}	// namespace {

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaFile クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::DirectAreaFile -- Constructor
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::File::StorageStrategy&		cStorageStrategy_
//		[IN]		Reference to file storage strategy
//	const PhysicalFile::File::BufferingStrategy&	cBufferingStrategy_
//		[IN]		Reference to file buffering strategy
//	const Lock::FileName*							pLockName_
//		[IN]		Pointer to lock name of logical file of DirectAreaFile
//	bool											bBatch_
//		[IN]		The insert mode whether batch or not.
//
//	RETURN
//
//	EXCEPTIONS
//	
DirectAreaFile::DirectAreaFile(
	const File::StorageStrategy&	cFileStorageStrategy_,
	const File::BufferingStrategy&	cBufferingStrategy_,
	const Lock::FileName*			pLockName_,
	bool							bBatch_)
	: File(cFileStorageStrategy_, cBufferingStrategy_, pLockName_, bBatch_),
	  m_uiLastPageID(ConstValue::UndefinedPageID)
{
	// Verify VersionPage size.
	m_VersionPageSize = verifyVersionPageSize(
		cFileStorageStrategy_.m_VersionFileInfo._pageSize);

	// Set PhysicalFile's page size.
	m_VersionPageDataSize =	Version::Page::getContentSize(m_VersionPageSize);
#ifdef DEBUG
	PageSize size = static_cast<PageSize>(_cParameterPageSize.get());
	if (size != 0)
	{
		m_VersionPageDataSize = ModMin(size, m_VersionPageDataSize);
	}
#endif

	m_pTree = new DirectAreaTree(this);
}


//	FUNCTION public
//	PhysicalFile::DirectAreaFile::~DirectAreaFile -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

DirectAreaFile::~DirectAreaFile()
{
	delete m_pTree;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::allocateArea -- Allocate an area
//
//	NOTES
//	During verification, Area can NOT be allocated.
//	Because Version::Page::verify does NOT support it.
//
//	ARGUMENTS
//	const Trans::Transaction&		cTransaction_
//		[IN]		Reference to transaction
//	AreaSize						uiSize_
//		[IN]		The requested size of the area
//
//	RETURN
//	DirectArea
//
//	EXCEPTIONS
//	Exception::BadArgument
//					uiSize_ is invalid.
//
DirectArea
DirectAreaFile::allocateArea(const Trans::Transaction&		cTransaction_,
							 AreaSize						uiSize_)
{
	if (uiSize_ == 0 || uiSize_ > getMaxStorableAreaSize())
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	uiSize_ = roundUp(uiSize_);

	DirectAreaPage* pLeaf = m_pTree->getLeaf(
		cTransaction_, uiSize_, m_VersionFile->isBatchInsert());
	DirectArea area = pLeaf->allocateArea(uiSize_);
	pLeaf->dirty();

	return area;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::freeArea -- Free an area
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&			cTransaction_
//		[IN]		Reference of transaction
//	const DirectArea::ID&				cID_
//		[IN]		Reference of DirectArea::ID
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaFile::freeArea(const Trans::Transaction&		cTransaction_,
						 const DirectArea::ID&			cID_,
						 Admin::Verification::Progress*	pProgress_)
{
	DirectAreaPage* pLeaf = attachPage(
		cTransaction_, cID_.m_uiPageID,
		File::DiscardableWriteFixMode, pProgress_);
	pLeaf->freeArea(cTransaction_, cID_.m_uiAreaID, pProgress_);
	pLeaf->dirty();
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::attachArea -- Attach an area
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	const DirectArea::ID&					cID_
//		[IN]		Reference to DirectArea::ID
//	Buffer::Page::FixMode::Value			eFixMode_
//		[IN]		FixMode
//
//	RETURN
//	DirectArea
//
//	EXCEPTIONS
//
DirectArea
DirectAreaFile::attachArea(const Trans::Transaction&	cTransaction_,
						   const DirectArea::ID&		cID_,
						   Buffer::Page::FixMode::Value	eFixMode_)
{
	Os::AutoCriticalSection latch(_latch);
	
	DirectAreaPage* pLeaf =
		attachPage(cTransaction_, cID_.m_uiPageID, eFixMode_);
	return pLeaf->attachArea(cID_.m_uiAreaID);

	// If DirectArea is changed, Dirty has to be set with DirectArea::Dirty().
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::detachAllAreas -- Detach all areas
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
DirectAreaFile::detachAllAreas()
{
	// For Leaf and Node/Root
	File::detachPageAll();

	m_uiLastPageID= ConstValue::UndefinedPageID;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::recoverAllAreas -- Recover all areas
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
DirectAreaFile::recoverAllAreas()
{
	// For Leaf
	while (m_Page != 0)
	{
		detachPage(m_Page, Page::UnfixMode::NotDirty);
	}

	// For Node/Root
	File::recoverPageAll();

	m_uiLastPageID = ConstValue::UndefinedPageID;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::getMaxStorableAreaSize --
//		Get the maximum size of an area which the File can store
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	AreaSize
//					The maximum size
//
//	EXCEPTIONS
//
AreaSize
DirectAreaFile::getMaxStorableAreaSize() const
{
	return static_cast<AreaSize>(getPageDataSize());
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::verifyArea -- Attach an area with verify mode
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	const DirectArea::ID&					cID_
//		[IN]		Reference to DirectArea::ID
//	Buffer::Page::FixMode::Value			eFixMode_
//		[IN]		FixMode
//	Admin::Verification::Progress&			cProgress_
//		[IN/OUT]	Progress of verification
//
//	RETURN
//	DirectArea
//
//	EXCEPTIONS
//
DirectArea
DirectAreaFile::verifyArea(const Trans::Transaction&		cTransaction_,
						   const DirectArea::ID&			cID_,
						   Buffer::Page::FixMode::Value		eFixMode_,
						   Admin::Verification::Progress&	cProgress_)
{
	; _SYDNEY_ASSERT(&cProgress_ != 0);
	
	DirectAreaPage* pLeaf = attachPage(
		cTransaction_, cID_.m_uiPageID, eFixMode_, &cProgress_);
	m_pTree->notifyUseArea(cTransaction_, cID_, &cProgress_);
	return pLeaf->attachArea(cID_.m_uiAreaID);

	// If DirectArea is changed, Dirty has to be set with DirectArea::Dirty().
	// If much DirectArea will be attached,
	// DirectArea::detach had better to be called.
}

//  FUNCTION public
//  PhysicalFile::DirectAreaFile::detachPage
//
//	NOTES
//	物理ページ記述子を破棄する。排他制御あり。
//
//	ARGUMENTS
//	PhysicalFile::Page*&					Page_
//		物理ページ記述子への参照
//	PhysicalFile::Page::UnfixMode::Value	UnfixMode_ = Omit
//		アンフィックスモード
//	const bool								SavePage_ = false
//		利用者からは破棄したように見えても、実はキャッシュしておくか
//			true  : キャッシュしておく
//			false : 本当に破棄してしまう
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
DirectAreaFile::detachPage(Page*&					Page_,
						   Page::UnfixMode::Value	UnfixMode_/* = Omit */,
						   const bool				SavePage_/* = false */)
{
	; _SYDNEY_ASSERT(Page_ != 0);

	// ページをデタッチする
	Page_->detach();

	if (Page_->m_ReferenceCounter == 0)
	{
		// 物理ページ記述子がどこからも参照されなくなった…

		if (SavePage_ == false)
		{
			// 本当にデタッチしても構わない…

			if (UnfixMode_ != Page::UnfixMode::Omit)
			{
				// 明示的にアンフィックスモードが指定された…

				// 物理ページのバッファリング内容をアンフィックスする
				Page_->m_Memory.unfix(
					UnfixMode_ == Page::UnfixMode::Dirty);

			} else {

				// 物理ページのバッファリング内容をアンフィックスする
				Page_->m_Memory.unfix(
					Page_->m_UnfixMode == Page::UnfixMode::Dirty);
			}

			// アタッチ中の物理ページのリンクを張り替える

			Page_->m_Prev->m_Next = Page_->m_Next;

			Page_->m_Next->m_Prev = Page_->m_Prev;

			bool	isLastPage = false;
			if (Page_ == Page_->m_Next)
			{
				// 他にアタッチ中の物理ページが存在しない…

				; _SYDNEY_ASSERT(Page_ == Page_->m_Prev);

				isLastPage = true;
			}

			Page*	prevPage = isLastPage ? 0 : Page_->m_Prev;

			// デタッチしたページはファイルがdetachされるまで保存する
			Page_->m_Free = m_DetachPage;
			m_DetachPage = Page_;
			Page_ = 0;

			this->m_Page = prevPage;
		}
		else
		{
			if (Page_->m_UnfixMode != Page::UnfixMode::Dirty)
			{
				Page_->m_UnfixMode = UnfixMode_;
			}

			// ページ内容を更新する
			this->savePage(Page_,
						   Page_->m_UnfixMode == Page::UnfixMode::Dirty);
		}
	}
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::detachPageAll --
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
DirectAreaFile::detachPageAll()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::recoverPageAll --
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
DirectAreaFile::recoverPageAll()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

#ifdef DEBUG
// public
// virtual
AreaSize
DirectAreaFile::getParentIndexValue(const Trans::Transaction&	cTransaction_,
									PageID						uiPageID_)
{
	return m_pTree->getParentIndexValue(cTransaction_, uiPageID_);
}

// public
// virtual
void
DirectAreaFile::setParentIndexValue(const Trans::Transaction&	cTransaction_,
									PageID						uiPageID_,
									AreaSize					uiSize_)
{
	m_pTree->setParentIndexValue(cTransaction_, uiPageID_, uiSize_);
}

// public
// virtual
void
DirectAreaFile::setLeafHeaderNumber(const Trans::Transaction&	cTransaction_,
									PageID						uiPageID_,
									AreaNum						uiNum_)
{
	DirectAreaPage* pLeaf = 
		attachPage(cTransaction_, uiPageID_, File::DiscardableWriteFixMode);
	pLeaf->setLeafHeaderNumber(uiNum_);
	pLeaf->dirty();
}

// public
// virtual
void
DirectAreaFile::setLeafHeaderOffset(const Trans::Transaction&	cTransaction_,
									PageID						uiPageID_,
									AreaOffset					uiOffset_)
{
	DirectAreaPage* pLeaf =
		attachPage(cTransaction_, uiPageID_, File::DiscardableWriteFixMode);
	pLeaf->setLeafHeaderOffset(uiOffset_);
	pLeaf->dirty();
}

// public
// virtual
void
DirectAreaFile::setLeafIndexAreaID(const Trans::Transaction&	cTransaction_,
								   PageID						uiPageID_,
								   AreaID						uiAreaID_,
								   AreaNum						uiIndex_)
{
	DirectAreaPage* pLeaf =
		attachPage(cTransaction_, uiPageID_, File::DiscardableWriteFixMode);
	pLeaf->setLeafIndexAreaID(uiAreaID_, uiIndex_);
	pLeaf->dirty();
}

// public
// virtual
void
DirectAreaFile::setLeafIndexOffset(const Trans::Transaction&	cTransaction_,
								   PageID						uiPageID_,
								   AreaOffset					uiOffset_,
								   AreaNum						uiIndex_)
{
	DirectAreaPage* pLeaf =
		attachPage(cTransaction_, uiPageID_, File::DiscardableWriteFixMode);
	pLeaf->setLeafIndexOffset(uiOffset_, uiIndex_);
	pLeaf->dirty();
}
#endif

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::getUsedSize --
//		Get the size of all PhysicalFile's pages
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&		cTransaction_
//		[IN]		Refference to transaction
//
//	RETURN
//	PhysicalFile::FileSize
//		 The size of all PhysicalFile's pages[byte]
//
//	EXCEPTIONS
//
FileSize
DirectAreaFile::getUsedSize(const Trans::Transaction& cTransaction_)
{
	AutoUnfix autoUnfix(this);
	PageNum total = static_cast<PageNum>(getLastPageID(cTransaction_) + 1);
	autoUnfix.success();

	return static_cast<FileSize>(total) * m_VersionPageDataSize;
}

//	FUNCTION public static
//	PhysicalFile::DirectAreaFile::getPageDataSize --
//		Get the maximum size of an area which the File can store
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Memory::Size				uiVersionPageSize_
//		[IN]		The size of the VersionPage
//					with Version's and PhysicalFile's header and so on
//	const PhysicalFile::AreaNum			uiDummy_ = 1
//		[]			Dummy
//
//	RETURN
//	PhysicalFile::PageSize
//					The size of an area [byte]
//
//	EXCEPTIONS
//
PageSize
DirectAreaFile::getPageDataSize(const Os::Memory::Size	uiVersionPageSize_,
								const AreaNum			uiDummy_)
{
	return static_cast<PageSize>(
		Version::Page::getContentSize(
			verifyVersionPageSize(uiVersionPageSize_)))
		- DirectAreaPage::getUnusedSize();
}

#ifdef OBSOLETE
//	FUNCTION public
//	PhysicalFile::DirectAreaFile::getAllocatedSize --
//
//	NOTES
//	This function is not used. But it is defined as pure virtual in File.
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
FileSize
DirectAreaFile::getAllocatedSize(const Trans::Transaction&	cTransaction_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}
#endif // OBSOLETE

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::getPageDataSize --
//		Get the max size of an area which the File can store
//
//	NOTES
//	The other modules can store an area whoes size is equal to the size.
//	getMaxStorableAreSize() is recommended for hidding concept of page.
//
//	ARGUMENTS
//	const PhysicalFile::AreaNum			dummy_ = 1
//		[]			Dummy
//
//	RETURN
//	PhysicalFile::PageSize
//					The size of an area [byte]
//
//	EXCEPTIONS
//
PageSize
DirectAreaFile::getPageDataSize(const AreaNum uiDummy_) const
{
	return m_VersionPageDataSize - DirectAreaPage::getUnusedSize();
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::getLastPageID -- 	Get last PageID
//
//	NOTES
//	The last PageID always points to the page which is NOT managed under tree.
//
//	ARGUMENTS
//	const Trans::Transaction&		cTransaction_
//		[IN]	Reference of transaction
//	Admin::Verification::Progress&	pProgress_
//		[IN/OUT]	Pointer to progress
//
//	RETURN
//	PageID
//				The last PageID
//
//	EXCEPTIONS
//
PageID
DirectAreaFile::getLastPageID(const Trans::Transaction&			cTransaction_,
							  Admin::Verification::Progress*	pProgress_)
{
	if (m_uiLastPageID == ConstValue::UndefinedPageID)
	{
		// Get PagePointer of Root page.
		PagePointer p = fixVersionPage(
			cTransaction_, FileHeader::VersionPageID,
			Buffer::Page::FixMode::ReadOnly, pProgress_);
		
		// PageID starts from 0.
		m_uiLastPageID = static_cast<PageID>(
			FileHeader::getManagePageNum(
				static_cast <const VersionPage&>(*p).operator const void*()))
			- 1;
	}
	return m_uiLastPageID;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::setLastPageID -- 	Set last PageID
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
DirectAreaFile::setLastPageID(const Trans::Transaction&	cTransaction_,
							  PageID					uiLastPageID_)
{
	// Fix header
	PagePointer p = fixVersionPage(cTransaction_, FileHeader::VersionPageID,
								   File::DiscardableWriteFixMode);
	// PageID starts from 0.
	FileHeader::setManagePageNum(p->operator void*(), uiLastPageID_ + 1);
	m_uiLastPageID = uiLastPageID_;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::attachPage -- Attach page with Progress
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&					cTransaction_
//		[IN]		Reference to transaction
//	const PageID								uiPageID_
//		[IN]		Requested PageID
//	const Buffer::Page::FixMode::Value			eFixMode_
//		[IN]		Requested fix mode
//	Admin::Verification::Progress*				cProgress_
//		[IN/OUT]	For verify
//
//	RETURN
//	DirectAreaPage*
//
//	EXCEPTIONS
//
DirectAreaPage*
DirectAreaFile::attachPage(
	const Trans::Transaction&		cTransaction_,
	PageID							uiPageID_,
	Buffer::Page::FixMode::Value	eFixMode_,
	Admin::Verification::Progress*	pProgress_)
{
	// Not allocate any page during verification.
	; _SYDNEY_ASSERT(
		pProgress_ == 0 || (eFixMode_ & Buffer::Page::FixMode::Allocate) == 0);
	
	DirectAreaPage* page = 0;
	if (pProgress_ == 0)
	{
		page = _SYDNEY_DYNAMIC_CAST(
			DirectAreaPage*,
			File::attachPage(cTransaction_, uiPageID_, eFixMode_));
	}
	else
	{
		page = _SYDNEY_DYNAMIC_CAST(
			DirectAreaPage*,
			verifyPage(cTransaction_, uiPageID_, eFixMode_, *pProgress_));
		m_pTree->notifyUsePage(cTransaction_, page, pProgress_);
	}
	return page;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::updateTree -- Update tree
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&			cTransaction_
//		[IN]		Reference to transaction
//	PageID								uiPageID_
//		[IN]		The PageID of the Leaf whoes size is updated
//	AreaSize							uiSize_
//		[IN]		The free space size of the Leaf
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaFile::updateTree(
	const Trans::Transaction&		cTransaction_,
	PageID							uiPageID_,
	AreaSize						uiSize_,
	Admin::Verification::Progress*	pProgress_)
{
	m_pTree->update(cTransaction_, uiPageID_, uiSize_, pProgress_);
}

// See File::fixVersionPage and File::verifyAllTable and so on.
File::PagePointer
DirectAreaFile::fixVersionPage(
	const Trans::Transaction&		cTransaction_,
	Version::Page::ID				uiPageID_,
	Buffer::Page::FixMode::Value	eFixMode_,
	Admin::Verification::Progress*	pProgress_)
{
	PagePointer pPage = 0;
	int	retryCnt = 0;
	try
	{
		while (retryCnt < 2)
		{
			try
			{
				// For progress
				pPage = getVersionPage(
					cTransaction_, uiPageID_, eFixMode_, pProgress_);
				break;
			}
			catch (Exception::MemoryExhaust&)
			{
				switch (retryCnt)
				{
				case 0:
					if (retryStepSelf() == false)
					{
						_SYDNEY_RETHROW;
					}
					break;
				default:
					_SYDNEY_RETHROW;
				}

				++retryCnt;
			}
		}
#ifdef NO_CATCH_ALL
	} catch (Exception::Object&) {
#else
	} catch (...) {
#endif
		// Can Not fix page.
		if (pProgress_ != 0)
		{
			_SYDNEY_VERIFY_ABORTED(*pProgress_, this->m_FilePath,
								   Message::CanNotFixNode());
		}
		_SYDNEY_RETHROW;
	}

	return pPage;
}

//	FUNCTION public static
//	PhysicalFile::DirectAreaFile::roundUp -- Round up the size of an area
//
//	NOTES
//
//	ARGUMENTS
//	AreaSize								uiSize_
//		[IN]		The requested size of an area
//
//	RETURN
//	AreaSize
//					The rounded up size
//
//	EXCEPTIONS
//
AreaSize
DirectAreaFile::roundUp(AreaSize	uiSize_)
{
	AreaSize remainder = uiSize_ % DirectAreaFile::AlignmentBytes;
	AreaSize padding = 0;
	if (remainder != 0)
	{
		padding = DirectAreaFile::AlignmentBytes - remainder;
	}
	return uiSize_ + padding;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::notifyUnuseArea --
//		Notify unuse area for checking consistency
//
//	NOTES
//
//	ARGUMENTS
//	const DirectArea::ID&				cID_
//		[IN]		Reference to DirectArea::ID
//	Admin::Verification::Progress*		pProgress_
//		[IN/OUT]	Pointer to progress of verification
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaFile::notifyUnuseArea(PageID							uiPageID_,
								AreaID							uiAreaID_,
								AreaSize						uiFreeSize_,
								Admin::Verification::Progress*	pProgress_)
{
	m_pTree->notifyUnuseArea(uiPageID_, uiAreaID_, uiFreeSize_, pProgress_);
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::notifyFreeSpaceSize --
//		Notify free space size for checking consistency
//
//	NOTES
//
//	ARGUMENTS
//	const DirectArea::ID&				cID_
//		[IN]		Reference to DirectArea::ID
//	Admin::Verification::Progress*		pProgress_
//		[IN/OUT]	Pointer to progress of verification
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaFile::notifyFreeSpaceSize(PageID							uiPageID_,
									AreaSize						uiFreeSize_,
									Admin::Verification::Progress*	pProgress_)
{
	m_pTree->notifyFreeSpaceSize(uiPageID_, uiFreeSize_, pProgress_);
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaFile's private constant number and so on
//
///////////////////////////////////////////////////////////////////////////////

//	CONST private
//	PhysicalFile::DirectAreaFile::AlignmentBytes --
//		Boundaries of alignment [byte]
//
//	NOTES

// static
const DirectArea::Size
DirectAreaFile::AlignmentBytes = 4;

//	CONST private
//	PhysicalFile::DirectAreaFile::MaxPageSize --
//		Max page size which is supported by DirectAreaFile [byte]
//
//	NOTES

// static
const Os::Memory::Size
DirectAreaFile::MaxPageSize = 0x00010000; // 64K

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaFile's private member function
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION private
//	PhysicalFile::DirectAreaFile::getAttachedPage -- Get attached page
//
//	NOTES
//	When the page pointing to uiPageID_ has been attached, the page is returnd.
//	And when eFixMode_ is NOT ReadOnly and the fix mode of the page is ReadOnly,
//	the page re-fixes with eFixMode_.
//
//	ARGUMENTS
//	const Trans::Transaction&					cTransaction_
//		[IN]		Reference to transaction
//	const PageID								uiPageID_
//		[IN]		Requested PageID
//	const Buffer::Page::FixMode::Value			eFixMode_
//		[IN]		Requested fix mode
//	const Buffer::ReplacementPriority::Value*	ePriority_
//		[IN]		For Not verify
//	Admin::Verification::Progress*				cProgress_
//		[IN/OUT]	For verify
//
//	RETURN
//	Page*
//					When the page has NOT been attached, return 0.
//
//	EXCEPTIONS
//
Page*
DirectAreaFile::getAttachedPage(
	const Trans::Transaction&					cTransaction_,
	const PageID								uiPageID_,
	const Buffer::Page::FixMode::Value			eFixMode_,
	const Buffer::ReplacementPriority::Value*	pPriority_,
	Admin::Verification::Progress*				pProgress_)
{
  	; _SYDNEY_ASSERT(m_Page != 0);

	// Set last attached page.
	DirectAreaPage* page = _SYDNEY_DYNAMIC_CAST(DirectAreaPage*, m_Page);
	// Candidate page for detaching
	Page* garbage = 0;
	int count = 0;

	do
	{
		if (page->getID() == uiPageID_)
		{
			// Found requested page.

			if (eFixMode_ != Buffer::Page::FixMode::ReadOnly &&
				page->getFixMode() == Buffer::Page::FixMode::ReadOnly)
			{
				// Re-fix the page with eFixMode_.
				page->changeFixMode(cTransaction_, this, uiPageID_,
									eFixMode_, pPriority_, pProgress_);
			}

			// [OPTIMIZE!] Change page position to next to m_Page.
			
			return page;
		}
		else if (eFixMode_ == Buffer::Page::FixMode::ReadOnly &&
				 page->getFixMode() == Buffer::Page::FixMode::ReadOnly &&
				 page->isReferenced() == false)
		{
			// Set page for detaching
			garbage = page;
			++count;
		}

		// Get previous page.
		// New page is set between m_Page and m_Next and the page become m_Page.
		// So m_Prev will be attached more recently than m_Next.
		// See File::attachPage for details.
		page = page->getPrev();

	} while (page != m_Page);

	// Not found requested page.

	if (count > 3)
	{
		// The number of ReadOnly and not-referenced pages is limited to 3.
		; _printDetachPageID(garbage->getID());
		detachPage(garbage);
	}

	return 0;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaFile::allocatePageInstance -- Allocate Page instance
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Page*
DirectAreaFile::allocatePageInstance(
	const Trans::Transaction&			cTransaction_,
	PageID								uiPageID_,
	Buffer::Page::FixMode::Value		eFixMode_,
	Admin::Verification::Progress*		pProgress_,
	Buffer::ReplacementPriority::Value	eReplacementPriority_)
{
	if (pProgress_ == 0)
	{
		return new DirectAreaPage(
			cTransaction_, this, uiPageID_, eFixMode_, eReplacementPriority_);
	}
	else
	{
		// For verify
		return new DirectAreaPage(
			cTransaction_, this, uiPageID_, eFixMode_, *pProgress_);
	}
}

//	FUNCTION private
//	PhysicalFile::DirectAreaFile::initialize --
//		Initialize during DirectAreaFile creating
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	void*									pFileHeader_
//		[IN]		Pointer to Header of DirectAreaFile
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaFile::initialize(const Trans::Transaction&	cTransaction_,
						   void*						pFileHeader_)
{
	// Initialize Tree.
	PageID pageID = m_pTree->initialize(cTransaction_);

	// Initialize last Page which is Leaf.
	// NOT use File::allocatePage().
	// That is used for AreaManageType and PageManageType(2).
	DirectAreaPage* pLeaf =
		attachPage(cTransaction_, pageID, Buffer::Page::FixMode::Allocate);
	pLeaf->initialize();
	Page* temp = pLeaf;
	detachPage(temp, Page::UnfixMode::Dirty);
	
	// Update FileHeader
	FileHeader::setManagePageNum(pFileHeader_, pageID + 1);

	// Set LastPageID
	m_uiLastPageID = pageID;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaFile::truncate -- Truncate this File
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	bool&									bModified_
//		[OUT]		The case where the File is trancated
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaFile::truncate(const Trans::Transaction&	cTransaction_,
						 bool& 						bModified_)
{
	AutoUnfix	autoUnfix(this);

	m_pTree->truncate(cTransaction_, m_VersionFile, bModified_);

	if (bModified_ == true)
	{
		autoUnfix.success();
	}
}

// See File::getHeaderPage.
File::VersionPage*
DirectAreaFile::getHeaderPage(const Trans::Transaction&			cTransaction_,
							  Buffer::Page::FixMode::Value		eFixMode_,
							  Admin::Verification::Progress*	pProgress_)
{
	if (m_pHeaderPage)
	{
		if (eFixMode_ != Buffer::Page::FixMode::ReadOnly
			&& m_pHeaderPage->isUpdatable() == false)
		{
			m_pHeaderPage->unfix(false);
		}
	}
	else
	{
		m_pHeaderPage = new VersionPage;
	}

	if (m_pHeaderPage->isOwner() == false)
	{
		if (pProgress_ == 0)
		{
			*m_pHeaderPage = Version::Page::fix(
				cTransaction_, *m_VersionFile, FileHeader::VersionPageID,
				eFixMode_, Buffer::ReplacementPriority::Middle);
		}
		else
		{
			// For Progress
			*m_pHeaderPage = Version::Page::verify(
				cTransaction_, *m_VersionFile, FileHeader::VersionPageID,
				eFixMode_, *pProgress_);
		}
	}

	return m_pHeaderPage;
}

// See File::getVersionPage.
File::PagePointer
DirectAreaFile::getVersionPage(
	const Trans::Transaction&			cTransaction_,
	Version::Page::ID					uiPageID_,
	Buffer::Page::FixMode::Value		eFixMode_,
	Admin::Verification::Progress*		pProgress_)
{
	PagePointer pPage;

	if (uiPageID_ == FileHeader::VersionPageID)
	{
		pPage = getHeaderPage(cTransaction_, eFixMode_, pProgress_);
		return pPage;
	}

	PageList::Iterator i = m_cPageList.begin();
	PageList::Iterator j = m_cPageList.end();
	ModSize readOnly = 0;
	for (; i != m_cPageList.end(); ++i)
	{
		if ((*i).getPageID() == uiPageID_)
		{
			j = i;
			break;
		}
		if ((*i).isUpdatable() ==  false && (*i)._reference == 0)
		{
			++readOnly;
			if (readOnly > 3)
			{
				j = i;
			}
		}
	}

	if (j != m_cPageList.end())
	{
		if ((eFixMode_ != Buffer::Page::FixMode::ReadOnly
			 && (*j).isUpdatable() == false)
			|| (*j).getPageID() != uiPageID_)
		{
			(*j).unfix(false);
			if (pProgress_ == 0)
			{
				(*j) = Version::Page::fix(
					cTransaction_, *m_VersionFile, uiPageID_,
					eFixMode_, Buffer::ReplacementPriority::Low);
			}
			else
			{
				// For Progress
				(*j) = Version::Page::verify(
					cTransaction_, *m_VersionFile, uiPageID_,
					eFixMode_, *pProgress_);
			}
		}
		m_cPageList.splice(m_cPageList.begin(), m_cPageList, j);
		pPage = &(*j);
	}
	else
	{
		ModAutoPointer<VersionPage> p = new VersionPage;
		if (pProgress_ == 0)
		{
			*p = Version::Page::fix(
				cTransaction_, *m_VersionFile, uiPageID_,
				eFixMode_, Buffer::ReplacementPriority::Low);
		}
		else
		{
			// For Progress
			*p = Version::Page::verify(
				cTransaction_, *m_VersionFile, uiPageID_,
				eFixMode_, *pProgress_);
		}
		m_cPageList.pushFront(*(p.release()));
		pPage = p.get();
	}
	return pPage;
}

//	FUNCTION private static
//	PhysicalFile::DirectAreaFile::verifyVersionPageSize --
//		Verify the size of VersionPage.
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Memory::Size	uiPageSize_
//		[IN]	The size of default page size
//
//	RETURN
//	Os::Memory::Size
//		The size of VersionPage [byte]
//
//	EXCEPTIONS
//	Exception::BadArgument
//				Maybe uiPageSize_ is invalid.
//	Exception::NotSupported
//				The File does NOT support the verified size. 
//
Os::Memory::Size
DirectAreaFile::verifyVersionPageSize(Os::Memory::Size uiPageSize_)
{
	try
	{
		uiPageSize_ = Version::File::verifyPageSize(uiPageSize_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// (maybe) bad argument
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// verifyPageSize() might increase the size.
	if (uiPageSize_ > DirectAreaFile::MaxPageSize)
	{
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return uiPageSize_;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaFile::verifyAllPages --
//		Verify all Nodes and not attached Leafs
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&					cTransaction_
//		[IN]		Reference to transaction
//	Admin::Verification::Progress&				cProgress_
//		[IN/OUT]	Reference to progress
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaFile::verifyAllPages(const Trans::Transaction&		cTransaction_,
							   Admin::Verification::Progress&	cProgress_)
{
	// Verify tree
	m_pTree->verify(cTransaction_, &cProgress_);
}

//	FUNCTION private
//	PhysicalFile::File::verifyAttachedPage -- Verify attached pages
//
//	NOTES
//	Before File::startVerification, File::detachAllAreas should be executed.
//	So, NO attached page should exist.
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaFile::verifyAttachedPage(
	const Trans::Transaction&		cDummy1_,
	Admin::Verification::Progress&	cDummy2_) const
{
	// No attached page exist.
	; _SYDNEY_ASSERT(m_Page == 0);
}

//	FUNCTION private
//	PhysicalFile::File::verifyAttachedPage -- Verify file header
//
//	NOTES
//	File header would be attached in File::verifyAllPages.
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaFile::verifyFileHeader(const Trans::Transaction&		cDummy1_,
								 Admin::Verification::Progress&	cDummy2_)
{
	// Nothing to do
}

//	FUNCTION private
//	PhysicalFile::DirectAreaFile::checkPhysicalFile --
//		Check the consistency of the File
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&			cTransaction_
//		[IN]		Reference to transaction
//	Admin::Verification::Progress&		cProgress_
//		[IN/OUT]	Reference to progress
//
//	RETURN
//
//	EXCEPTIONS
//
// File's pure virtual function
void
DirectAreaFile::checkPhysicalFile(
	const Trans::Transaction&		cDummy1_,
	Admin::Verification::Progress&	cDummy2_)
{
	// Noting to do
}

//	FUNCTION private
//	PhysicalFile::DirectAreaFile::initializeVerification --
//		Preprocess of verification
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&			cTransaction_
//		[IN]		Reference to transaction
//	const unsigned int					uiTreatment_
//		[IN]		Checking usage. ex: Correct file or not
//					It is cast from const Admin::Verification::Treatment::Value.
//	Admin::Verification::Progress&		cDummy_
//		[]			Dummy
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaFile::initializeVerification(
	const Trans::Transaction&		cTransaction_,
	const unsigned int				uiTreatment_,
	Admin::Verification::Progress&	cDummy_)
{
	; _SYDNEY_ASSERT(m_Check == false);

	m_Check = true;
	m_Treatment = uiTreatment_;
	m_FilePath = m_VersionFile->getStorageStrategy()._path._masterData;
	m_pTree->clearVerifyMap();

	// DirectAreaFile attach NO Page in File::startVerification.
	// So, NOT need to execute File::detachAllAreas.
}

//	FUNCTION private
//	PhysicalFile::DirectAreaFile::terminateVerification --
//		Postprocess of verification
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
DirectAreaFile::terminateVerification()
{
	m_pTree->clearVerifyMap();

	// DirectAreaFile attach some Pages in File::endVerification.
	// But the upper module does NOT know it, so have to execute detach.
	detachAllAreas();
}


//
//	Copyright (c) 2005, 2006, 2007, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
