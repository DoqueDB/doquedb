// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectAreaTree.cpp -- Managing Tree-structure for DirectAreaFile
// 
// Copyright (c) 2007, 2009, 2023 Ricoh Company, Ltd.
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

#include "PhysicalFile/DirectArea.h"
#include "PhysicalFile/DirectAreaTree.h"
#include "PhysicalFile/DirectAreaPage.h"
#include "PhysicalFile/Message_DiscordAreaUseSituation1.h"
#include "PhysicalFile/Message_DiscordAreaUseSituation2.h"
#include "PhysicalFile/Message_DiscordPageUseSituation3.h"
#include "PhysicalFile/Message_CorrectedAreaUseSituation3.h"
#include "PhysicalFile/Message_DiscordNodeFreeArea.h"
#include "PhysicalFile/Message_CorrectedNodeFreeArea.h"
#ifdef DEBUG
#include "PhysicalFile/Message.h"
#endif
#include "PhysicalFile/FakeError.h"

#include "Common/Assert.h"

#include "Exception/FileManipulateError.h"
#include "Exception/Unexpected.h"

#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

namespace
{

//	FUNCTION local
//	_$$::_printTruncate -- Print truncate message for debug.
//
//	NOTES
//
//	ARGUMENTS
//	PageID							lastPageID_
//	const Version::File*			pFile_
//
//	RETURN
//
//	EXCEPTIONS
// 
void
_printTruncate(PageID lastPageID_, const Version::File* pFile_)
{
#ifdef DEBUG
	_SYDNEY_PHYSICALFILE_DEBUG_MESSAGE
		<< "Last PhysicalFile's PageID is " << lastPageID_ << " ."
		<< "Version::File's size is " << pFile_->getSize() << " ." << ModEndl;
#endif
}

}	// namespace {

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaTree's public member function
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::Tree -- Constructor
//
//	NOTES
//
//	ARGUMENTS
//	PageID							uiLastPageID_ = ConstValue::UndefinedPageID
//		[IN]		PageID pointing to last page in the File
//
//	RETURN
//
//	EXCEPTIONS
//
DirectAreaTree::DirectAreaTree(DirectAreaFile* pFile_)
	:m_pFile(pFile_)
{
	m_uiMaxChildren =
		(pFile_->getPageSize() - FileHeader::getSize(DirectAreaType))
		/ sizeof(DirectArea::Size);

	m_uiSubtree[LeafStep - 1] = 1;
	for (int i = LeafStep - 2; i >= 0; --i)
	{
		m_uiSubtree[i] = 1 + m_uiMaxChildren * m_uiSubtree[i + 1];
	}
}

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::~Tree -- Destructor
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
// 
DirectAreaTree::~DirectAreaTree()
{
}

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::initialize --
//		Initialize during DirectAreaFile creating
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//
//	RETURN
//
//	EXCEPTIONS
//
PageID
DirectAreaTree::initialize(const Trans::Transaction& cTransaction_)
{
	// Root has been initialized in File::create().
	// Page's memory is set by 0.
	PageID id = FileHeader::VersionPageID + 1;

	// Reserve two VersionPages for Node.
	// These are used when Tree structure grows up to two or three steps.
	while (id < MinLeafID)
	{
		m_pFile->fixVersionPage(
			cTransaction_, id++, Buffer::Page::FixMode::Allocate);
	}
	return id;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::getLeaf -- Get Leaf which has enough space.
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	AreaSize								uiSize_
//		[IN]		The size of the area
//
//	RETURN
//	DirectAreaPage*
//
//	EXCEPTIONS
//
DirectAreaPage*
DirectAreaTree::getLeaf(const Trans::Transaction&	cTransaction_,
						AreaSize					uiSize_,
						bool						bBatch_)
{
	// Check last Leaf whose free space size is not managed under tree.
	PageID lastPageID = m_pFile->getLastPageID(cTransaction_);
	DirectAreaPage* pLeaf = m_pFile->attachPage(
		cTransaction_, lastPageID, Buffer::Page::FixMode::ReadOnly);
	AreaSize freeSize = pLeaf->getFreeSize();
	if (freeSize >= uiSize_)
	{
		// Last Leaf has enough free space size.

		// Not need to detachPage().
		// Because DirectAreaPage::changeFixMode is executed in attachPage().
		return m_pFile->attachPage(
			cTransaction_, lastPageID, File::DiscardableWriteFixMode);
	}
	Page* temp = pLeaf;
	m_pFile->detachPage(temp);

	// Check Tree
	if (bBatch_ == false && lastPageID > MinLeafID)
	{
		// Allocate an area in last or new page in batch insert mode.
		pLeaf = attachLeafInTree(
			cTransaction_, static_cast<DirectArea::Size>(uiSize_));
		if (pLeaf != 0)
		{
			// Found Page.
			return pLeaf;
		}
	}

	// Allocate new Leaf.

	// Get new LastLeafID (and allocate Nodes if necessary.)
	PageID newLastPageID = getNextLeafID(cTransaction_, lastPageID);
	// Set new LastPageID.
	m_pFile->setLastPageID(cTransaction_, newLastPageID);
	// Append previous last Page to tree.
	append(cTransaction_, lastPageID, newLastPageID, freeSize);

	// Allocate new page and alocate the area in it.
	pLeaf = m_pFile->attachPage(
		cTransaction_, newLastPageID, File::DiscardableAllocateFixMode);
	pLeaf->initialize();
	return pLeaf;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::update -- Update tree
//
//	NOTES
//	For Page::freeArea and Page::changeAreaSize
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	PageID									uiPageID_
//		[IN]		Leaf's PageID
//	AreaSize								uiSize_
//		[IN]		The free space area size of the Leaf
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaTree::update(const Trans::Transaction&		cTransaction_,
					   PageID							uiPageID_,
					   AreaSize							uiSize_,
					   Admin::Verification::Progress*	pProgress_,
					   DirectArea::Size*				usSecondSizes_)
{
	; _SYDNEY_ASSERT(getStep(uiPageID_) == LeafStep);
	; _SYDNEY_ASSERT(static_cast<DirectArea::Size>(uiSize_) !=
					 DirectArea::InvalidSize);
	
	if (m_pFile->getLastPageID(cTransaction_, pProgress_) == uiPageID_)
	{
		// Last Page is not managed under Tree.
		return;
	}

	DirectArea::Size size = static_cast<DirectArea::Size>(uiSize_);

	// [OPTIMIZE!] This is similer to 'decontrol'.
	while (size != DirectArea::InvalidSize)
	{
		// Update tree upward.

		// Get parent Node.
		PageNum index;
		uiPageID_ = getSubstantialParentPageID(
			cTransaction_, uiPageID_, index, pProgress_);

		// Set new size.
		File::PagePointer page = m_pFile->fixVersionPage(
			cTransaction_, uiPageID_, File::DiscardableWriteFixMode,
			pProgress_);
		DirectArea::Size* p = getTopIndex(page);
		DirectArea::Size prevSize = getSize(p, index);
		setSize(p, index, size);

		if (uiPageID_ == FileHeader::VersionPageID)
		{
			// This Node is Root.
			break;
		}

		// Get new maximum size.
		if (usSecondSizes_ == 0)
		{
			// Not searched sizes,
			// when Free Area, Change Area's size and Allocate new Area.
			PageNum lastIndex =
				getLastIndex(cTransaction_, uiPageID_, pProgress_);
			// When allocating new Area, prevSize is equal to 0.
			// Because the Nodes was initialized to 0.
			size = getUpdatedMaxSize(p, lastIndex, index, prevSize);
		}
		else
		{
			// Searched sizes.
			size = ModMax(usSecondSizes_[getStep(uiPageID_)], size);
		}
	}
}

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::truncate -- Truncate
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
//	Exception::FileManipulateError
//					Failure of truncate
//
void
DirectAreaTree::truncate(const Trans::Transaction&	cTransaction_,
						 Version::File*				pVersionFile_,
						 bool& 						bModified_)
{
	// Check last Page
	PageID id = m_pFile->getLastPageID(cTransaction_);
	if (id == MinLeafID)
	{
		// NOT truncate. Because tree manages NO page.
		return;
	}
	DirectAreaPage* page =
		m_pFile->attachPage(cTransaction_, id, Buffer::Page::FixMode::ReadOnly);
	AreaSize size = page->getFreeSize();
	Page* temp = page;
	m_pFile->detachPage(temp);
	if (size != m_pFile->getMaxStorableAreaSize())
	{
		// NOT truncate. Because data exists in last page.
		return;
	}

	// Check pages under tree.
	PageID prevLastPageID = id;
	id = getNonEmptyLeafID(cTransaction_, id);
	if (prevLastPageID != id)
	{
		// Decontrol pages.
		decontrol(cTransaction_, id, prevLastPageID);

		// Set new LastPageID.
		m_pFile->setLastPageID(cTransaction_, id);

		try
		{
			// Truncate pages.
			; _PHYSICALFILE_FAKE_ERROR(DirectAreaFile::truncate);

			; _printTruncate(prevLastPageID, pVersionFile_);
			pVersionFile_->truncate(cTransaction_, id + 1);
			; _printTruncate(id, pVersionFile_);
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
		{
			_SYDNEY_RETHROW;
		}
#else
		catch (...)
		{
			_SYDNEY_THROW0(Exception::FileManipulateError);
		}
#endif
		bModified_ = true;
	}
}

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::verify -- Verify tree
//
//	NOTES
//	Verify tree with depth first search.
//	ARGUMENTS
//	const Trans::Transaction&					cTransaction_
//		[IN]		Reference to transaction
//	Buffer::Page::FixMode::Value				eFixMode_
//		[IN]		FixMode
//	Admin::Verification::Progress&				cProgress_
//		[IN/OUT]	Reference to progress
//	PageID&										uiPageID_
//		[IN]		PageID pointing to Root or Node
//		[OUT]		PageID pointing to last verified Leaf
//
//	RETURN
//	DirectArea::Size
//					The free space area size
//
//	EXCEPTIONS
//
void
DirectAreaTree::verify(const Trans::Transaction& 		cTransaction_,
					   Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(pProgress_ != 0);
	; _SYDNEY_ASSERT(m_pFile->getLastPageID(cTransaction_, pProgress_) != 
					 ConstValue::UndefinedPageID);

	// Check last page which is not managed in tree.
	PageID lastPageID = m_pFile->getLastPageID(cTransaction_, pProgress_);
	verifyLeaf(cTransaction_, lastPageID, pProgress_);

	if (lastPageID > MinLeafID)
	{
		// Tree manages some pages.
		PageID rootID = FileHeader::VersionPageID;
		verifyNode(cTransaction_, rootID, pProgress_);
	}
}

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::notifyUsePage --
//		Notify using page for checking consistency
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&			cTransaction_
//		[IN]		Reference to transaction
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
DirectAreaTree::notifyUsePage(const Trans::Transaction&			cTransaction_,
							  DirectAreaPage*					pPage_,
							  Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(pProgress_ != 0);
	
	// Check PageID.
	PageID uiPageID = pPage_->getID();
	if (m_pFile->getLastPageID(cTransaction_, pProgress_) < uiPageID)
	{
		_SYDNEY_VERIFY_INCONSISTENT(
			*pProgress_, m_pFile->getFilePath(),
			Message::DiscordPageUseSituation3(uiPageID));
	}

	VerifyResult* result = getVerifyResult(uiPageID);
	if (result ==0)
	{
		// The Page has NOT verified yet.

		result = new VerifyResult;

		// Verify the page
		// [OPTIMIZE!] 'setVerifyResult' is prepared for this process.
		result->m_uiAreaSize = pPage_->verify(
			cTransaction_, *pProgress_, result->m_cAreaIDMap);

		// Set the result of verification.
		m_cVerifyMap.insert(uiPageID, result);
	}

	// A Page may be notified for some times
	// which is as same as the number of Area in the Page.
}

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::notifyUseArea --
//		Notify use area for checking consistency
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&			cTransaction_
//		[IN]		Reference to transaction
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
DirectAreaTree::notifyUseArea(const Trans::Transaction&			cTransaction_,
							  const DirectArea::ID&				cID_,
							  Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(pProgress_ != 0);

	PageID uiPageID = cID_.m_uiPageID;
	AreaID uiAreaID = cID_.m_uiAreaID;

	// Get VerifyResult.
	VerifyResult* result = getVerifyResult(uiPageID);
	if (result == 0)
	{
		// Not found result.
		// It should be notified with DirectAreaTree::notifyUsePage.
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	AreaIDMap::Iterator j = result->m_cAreaIDMap.find(uiAreaID);
	if (j != result->m_cAreaIDMap.end())
	{
		// Found the AreaID.
		// Not mind whether the area is notified two or more.
		(*j).second = true;
	}
	else
	{
		// Not found the AreaID in the Page.
		_SYDNEY_VERIFY_INCONSISTENT(
			*pProgress_, m_pFile->getFilePath(),
			Message::DiscordAreaUseSituation2(uiPageID, uiAreaID));
	}
}

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::notifyUnuseArea --
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
DirectAreaTree::notifyUnuseArea(PageID							uiPageID_,
								AreaID							uiAreaID_,
								AreaSize						uiFreeSize_,
								Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(pProgress_ != 0);
	modifyVerifyResult(uiPageID_, uiAreaID_, uiFreeSize_, pProgress_);
}

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::notifyFreeSpaceSize --
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
DirectAreaTree::notifyFreeSpaceSize(PageID							uiPageID_,
									AreaSize						uiFreeSize_,
									Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(pProgress_ != 0);
	modifyVerifyResult(
		uiPageID_, ConstValue::UndefinedAreaID, uiFreeSize_, pProgress_);
}

//	FUNCTION public
//	PhysicalFile::DirectAreaTree::clearVerifyMap -- Clear m_cVerifyMap
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
DirectAreaTree::clearVerifyMap()
{
	VerifyMap::Iterator	i = m_cVerifyMap.begin();
	for (; i != m_cVerifyMap.end(); ++i)
	{
		VerifyResult* result = (*i).second;
		delete result;
	}
	m_cVerifyMap.erase(m_cVerifyMap.begin(), m_cVerifyMap.end());
}


#ifdef DEBUG
AreaSize
DirectAreaTree::getParentIndexValue(
	const Trans::Transaction&	cTransaction_,
	PageID						uiPageID_)
{
	if (getStep(uiPageID_) == FileHeader::VersionPageID)
	{
		// Set page is root and root doesn't have parent.
		return ConstValue::UndefinedAreaSize;
	}

	PageNum index;
	uiPageID_ = getSubstantialParentPageID(cTransaction_, uiPageID_, index);
	File::PagePointer page = m_pFile->fixVersionPage(
		cTransaction_, uiPageID_, Buffer::Page::FixMode::ReadOnly);
	const DirectArea::Size* pSize = getConstTopIndex(page) + index;
	return static_cast<AreaSize>(*pSize);
}

void
DirectAreaTree::setParentIndexValue(
	const Trans::Transaction&	cTransaction_,
	PageID						uiPageID_,
	AreaSize					uiSize_)
{
	PageNum index;
	uiPageID_ = getSubstantialParentPageID(cTransaction_, uiPageID_, index);
	File::PagePointer page = m_pFile->fixVersionPage(
		cTransaction_, uiPageID_, File::DiscardableWriteFixMode);
	DirectArea::Size* pSize = getTopIndex(page) + index;
	*pSize = static_cast<DirectArea::Size>(uiSize_);
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaFile's private constant number and so on
//
///////////////////////////////////////////////////////////////////////////////

//	CONST private
//	PhysicalFile::DirectAreaTree::LeafStep -- The number of tree's steps
//
//	NOTES
//	See header file.

//static
//const DirectAreaTree::Step
//DirectAreaTree::LeafStep = 3;

//	CONST private
//	PhysicalFile::DirectAreaTree::InvalidStep -- The invalid step
//
//	NOTES
//
//static
const DirectAreaTree::Step
DirectAreaTree::InvalidStep = 0xFFFF;

//	CONST private
//	PhysicalFile::PageID::MinLeafID -- The minimum leaf's PageID
//
//	NOTES
//
//static
const PageID
DirectAreaTree::MinLeafID = FileHeader::VersionPageID + LeafStep;


///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaTree's private functions
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::attachLeafInTree -- Attach Leaf in Tree
//
//	NOTES
//	This function is searching for a page.
//	The searched page have enough free space size
//	which is equal to the given size or more than the size and minimum.
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	DirectArea::Size						usSize_
//		[IN]		The size of the area
//
//	RETURN
//	DirectAreaPage*
//
//	EXCEPTIONS
//
DirectAreaPage*
DirectAreaTree::attachLeafInTree(const Trans::Transaction&	cTransaction_,
								 DirectArea::Size			usSize_)
{
	DirectAreaPage* pLeaf = 0;
	
	// usSecondSizes is candidates of max free space size in each Nodes.
	// When the size is equal to DirectArea::InvalidSize,
	// the maximum size in the Node will be NOT changed for allocate.
	// When NOT equal to the InvalidSize,
	// the size means the second maximum size in the Node
	// (and the updated size is maximum.)
	// So, if the updated size is less than the candidate size,
	// the candidate size become maximum.
	// If the updated size is more than the candidate size,
	// the updated size keep maximum.
	DirectArea::Size usSecondSizes[LeafStep];
	for (Step i = 0; i < LeafStep; ++i)
	{
		usSecondSizes[i] = DirectArea::InvalidSize;
	}

	PageID id = search(cTransaction_, usSize_, usSecondSizes);
	if (id != ConstValue::UndefinedPageID)
	{
		// Found Page.
		
		pLeaf = m_pFile->attachPage(
			cTransaction_, id, File::DiscardableWriteFixMode);
		usSize_ = static_cast<DirectArea::Size>(
			DirectAreaPage::verifyFreeSize(
				pLeaf->getFreeSize() - static_cast<AreaSize>(usSize_)));
		// Progress* is 0.
		update(cTransaction_, id, usSize_, 0, usSecondSizes);
	}
	return pLeaf;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::search --	Search PageID from Root to Leaf
//
//	NOTES
//	This function is searching for a page.
//	The searched page have enough free space size
//	which is equal to the given size or more than the size and minimum.
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	AreaSize								uiSize_
//		[IN]		The size of the area
//
//	RETURN
//	PageID
//					The PageID pointing a page which has enogh free space area
//					ConstValue::UndefinedPageID is returnd
//					when no page has the enough space area in the tree.
//
//	EXCEPTIONS
//
PageID
DirectAreaTree::search(const Trans::Transaction&	cTransaction_,
					   DirectArea::Size				usSize_,
					   DirectArea::Size*			usSecondSizes_)
{
	; _SYDNEY_ASSERT(usSize_ != 0);

	PageID id = FileHeader::VersionPageID;
	while (getStep(id) != LeafStep)
	{
		// Access to root/node.
		File::PagePointer page = m_pFile->fixVersionPage(
			cTransaction_, id, Buffer::Page::FixMode::ReadOnly);

		// Get child node/leaf's index.
		PageNum index = 0;
		if (id == FileHeader::VersionPageID)
		{
			// This page is root, so not need to get second.
			index = getStorableIndexInRoot(cTransaction_, page, usSize_);
			if (index == ConstValue::UndefinedPageNum)
			{
				// Not found any page which can store the area.
				return ConstValue::UndefinedPageID;
			}
		}
		else
		{
			index = getStorableIndex(
				cTransaction_, page, id, usSize_, usSecondSizes_);
			if (index == ConstValue::UndefinedPageNum)
			{
				// Upper node has indicated that storable page exists.
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}

		// Get child node/leaf's PageID.
		id = getSubstantialChildPageID(cTransaction_, id, index);
	}
	return id;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::append -- Append a page
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	PageID									uiLastPageID_
//		[IN]		PageID pointing to the current last page
//	AreaSize								uiFreeSize_
//		[IN]		The free space size of the page pointed by uiPageID_
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaTree::append(const Trans::Transaction&	cTransaction_,
					   PageID						uiPrevLastPageID_,
					   PageID						uiLastPageID_,
					   AreaSize						uiFreeSize_)
{
	; _SYDNEY_ASSERT(getStep(uiPrevLastPageID_) == LeafStep);
	; _SYDNEY_ASSERT(getStep(uiLastPageID_) == LeafStep);
	; _SYDNEY_ASSERT(m_pFile->getLastPageID(cTransaction_) == uiLastPageID_);
	
	// Get current Root's step.
	Step prevStep = getRootStep(cTransaction_, uiPrevLastPageID_);
	// Get new Root's step.
	Step step = getRootStep(cTransaction_, uiLastPageID_);

	if (prevStep != step && prevStep != InvalidStep)
	{
		// Changed Root's step.
		
		// Copy Root to child.
		File::PagePointer root = m_pFile->fixVersionPage(
			cTransaction_, FileHeader::VersionPageID,
			File::DiscardableWriteFixMode);
		File::PagePointer child = m_pFile->fixVersionPage(
			cTransaction_, FileHeader::VersionPageID + prevStep,
			File::DiscardableWriteFixMode);
		char* pRoot = root->operator char*() + sizeof(FileHeader::Item_Type2);
		char* pChild = child->operator char*() + sizeof(FileHeader::Item_Type2);
		AreaSize size =
			m_pFile->getPageSize() - sizeof(FileHeader::Item_Type2);
		Os::Memory::copy(pChild, pRoot, size);

		// Initialize Root.
		const DirectArea::Size* p =
			syd_reinterpret_cast<const DirectArea::Size*>(pChild);
		DirectArea::Size max = getMaxSize(p, m_uiMaxChildren - 1);
		Os::Memory::reset(pRoot, size);
		Os::Memory::copy(pRoot, &max, sizeof(DirectArea::Size));
	}

	// Update tree with previous last page.
	if (uiFreeSize_ != 0)
	{
		// When free size is equal to 0, NOT need to update tree.
		// Because the Nodes has been initialized to 0.
		// See getNewLeafID and Version::Page::fix so on.
		update(cTransaction_, uiPrevLastPageID_, uiFreeSize_);
	}
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::decontrol -- Decontrol pages under tree
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	PageID									begin_
//		[IN]		PageID pointing to the begin page of the pages
//	PageID									end_
//		[IN]		PageID pointing to the end page of the pages
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaTree::decontrol(const Trans::Transaction&	cTransaction_,
						  PageID					begin_,
						  PageID					end_)
{
	; _SYDNEY_ASSERT(getStep(begin_) == LeafStep);
	; _SYDNEY_ASSERT(getStep(end_) == LeafStep);
	; _SYDNEY_ASSERT(begin_ >= MinLeafID);
	; _SYDNEY_ASSERT(end_ > MinLeafID);
	; _SYDNEY_ASSERT(begin_ < end_);
	
	Step prevStep = getRootStep(cTransaction_, end_);
	Step newStep = getRootStep(cTransaction_, begin_);

	//
	// Fix up tree structure.
	//
	if (newStep == InvalidStep)
	{
		// Tree manages no page.

		// Initialize Root.
		resetNode(cTransaction_, FileHeader::VersionPageID);
		// Initialize unused Nodes.
		for (Step step = prevStep + 1; step < LeafStep; ++step)
		{
			resetNode(cTransaction_, static_cast<PageID>(step));
		}
		return;
	}
	else if (newStep > prevStep)
	{
		// The logical step of Root becomes deeper.

		// Overwrite Root with Node.
		File::PagePointer root = m_pFile->fixVersionPage(
			cTransaction_, FileHeader::VersionPageID,
			File::DiscardableWriteFixMode);
		File::PagePointer node = m_pFile->fixVersionPage(
			cTransaction_, FileHeader::VersionPageID + newStep,
			File::DiscardableWriteFixMode);
		Os::Memory::copy(
			root->operator char*() + sizeof(FileHeader::Item_Type2),
			node->operator char*() + sizeof(FileHeader::Item_Type2),
			m_pFile->getPageSize()
			- sizeof(FileHeader::Item_Type2));

		for (Step step = prevStep + 1; step < LeafStep; ++step)
		{
			// Initialize unused Nodes.
			resetNode(cTransaction_, static_cast<PageID>(step));
		}
	}

	// Get leaf which will be last and has been managed under tree.
	begin_ = getPreviousLeafIDwithResettingNodes(cTransaction_, begin_);
	// begin_ became last in TREE.

	//
	// Update the size of free space area.
	//
	// [OPTIMIZE!] This is similer to 'update'.
	DirectArea::Size size = DirectArea::InvalidSize;
	while (true)
	{
		// Get parent Node.
		PageNum index;
		begin_ = getSubstantialParentPageID(cTransaction_, begin_, index);

		// Set new size.
		File::PagePointer page = m_pFile->fixVersionPage(
			cTransaction_, begin_, File::DiscardableWriteFixMode);
		DirectArea::Size* p = getTopIndex(page);
		if (size != DirectArea::InvalidSize)
		{
			// When the Node/Root is direct parent of Leaf,
			// the 'max' means Leaf's free space size, but it is not set. 
			setSize(p, index, size);
		}

		// Reset unused area where is after the index.
		Os::Memory::reset(
			p + index + 1,
			(m_uiMaxChildren - index - 1) * sizeof(DirectArea::Size));

		if (begin_ == FileHeader::VersionPageID)
		{
			break;
		}

		// Get new maximum size.
		size = getMaxSize(p, index);
	}

	return;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::resetNode -- Reset Node or Root
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	PageID									uiPageID_
//		[IN]		PageID
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaTree::resetNode(const Trans::Transaction&	cTransaction_,
						  PageID					uiPageID_)
{
	; _SYDNEY_ASSERT(getStep(uiPageID_) != LeafStep);
	
	File::PagePointer page = m_pFile->fixVersionPage(
		cTransaction_, uiPageID_, File::DiscardableWriteFixMode);
	char* p = page->operator char*() + sizeof(FileHeader::Item_Type2);
	Os::Memory::reset(
		p, m_pFile->getPageSize() - sizeof(FileHeader::Item_Type2));
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getStep -- Get the step of the page
//
//	NOTES
//
//	ARGUMENTS
//	PageID									uiPageID_
//		[IN]		PageID
//
//	RETURN
//	DirectAreaTree::Step
//					The step of the page pointed by the PageID
//	EXCEPTIONS
//
DirectAreaTree::Step
DirectAreaTree::getStep(PageID uiPageID_)
{
	Step step = 0;
	while (step < LeafStep && uiPageID_ != 0)
	{
		uiPageID_ = (uiPageID_ - 1) % m_uiSubtree[step++];
	}
	return step;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::setRootStep -- Get logical Root's step
//
//	NOTES
//
//	ARGUMENTS
//	PageID									uiPageID_
//		[IN]		last PageID which is managed in the "FILE".
//
//	RETURN
//	DirectAreaTree::Step
//					The step of Root
//					InvalidStep means NO page under tree.
//
//	EXCEPTIONS
// 
DirectAreaTree::Step
DirectAreaTree::getRootStep(const Trans::Transaction&		cTransaction_,
							PageID							uiLastPageID_,
							Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(uiLastPageID_ == ConstValue::UndefinedPageID ||
					 (uiLastPageID_ >= MinLeafID &&
					  getStep(uiLastPageID_) == LeafStep));

	// Get last PageID
	if (uiLastPageID_ == ConstValue::UndefinedPageID)
	{
		uiLastPageID_ = m_pFile->getLastPageID(cTransaction_, pProgress_);
	}

	if (uiLastPageID_ == MinLeafID)
	{
		// Tree manages NO Leaf.
		return InvalidStep;
	}
	else
	{
		// Tree manages some Leaf.

		// Get last leaf which is managed under tree.
		uiLastPageID_ = getPreviousLeafID(uiLastPageID_);

		Step step = 0;
		while (step < LeafStep - 1 &&
			   ((uiLastPageID_ - 1) / m_uiSubtree[step]) == 0)
		{
			uiLastPageID_ = (uiLastPageID_ - 1) % m_uiSubtree[step++];
		}
		
		return step;
	}
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getParentPageID --
//		Get parent's PageID
//
//	NOTES
//
//	ARGUMENTS
//	PageID									uiChildID_
//		[IN]		Child's PageID
//	PageNum									uiIndex_
//		[OUT]		Index in Parent, the index points to the Child
//
//	RETURN
//	PageID
//					Parent's PageID
//
//	EXCEPTIONS
//
PageID
DirectAreaTree::getParentPageID(PageID		uiChildID_,
								PageNum&	uiIndex_)
{
	; _SYDNEY_ASSERT(uiChildID_ != FileHeader::VersionPageID);

	PageID parentID = FileHeader::VersionPageID;

	// id represents PageID in subtree.
	PageID id = uiChildID_;
	for (Step i = 0; i < LeafStep; ++i)
	{
		// Set new index pointing to Child.
		uiIndex_ = (id - 1) / m_uiSubtree[i];
		// Set new PageID in subtree.
		id = (id - 1) % m_uiSubtree[i];

		if (id == 0)
		{
			// Found Parent.
			break;
		}
		else
		{
			// Set new Parent's PageID.
			parentID += uiIndex_ * m_uiSubtree[i] + 1;
		}
	}
	; _SYDNEY_ASSERT(id == 0);

	return parentID;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getSubstantialParentPageID --
//		Get PageID pointing to substantial parent
//
//	NOTES
//	A page is directly managed by Root
//	even if the page is not directly below Root.
//	So SUBSTANTIAL parent means page which is ACTUALLY managed the child.
//	See getSubstantialChildPageID().
//
//	ARGUMENTS
//	PageID									uiChildID_
//		[IN]		Child's PageID
//	PageNum									uiIndex_
//		[OUT]		Index in Parent, the index points to the Child
//
//	RETURN
//	PageID
//					PageID pointing to substantial parent
//
//	EXCEPTIONS
//

PageID
DirectAreaTree::getSubstantialParentPageID(
	const Trans::Transaction&		cTransaction_,
	PageID							uiChildID_,
	PageNum&						uiIndex_,
	Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(uiChildID_ != FileHeader::VersionPageID);

	PageID parentID = getParentPageID(uiChildID_, uiIndex_);
	if (parentID ==	FileHeader::VersionPageID +
		getRootStep(cTransaction_, ConstValue::UndefinedPageID, pProgress_))
	{
		// Substantial parent is Root.
		parentID = FileHeader::VersionPageID;
	}
	return parentID;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getChildPageID --
//		Get child's PageID
//
//	NOTES
//
//	ARGUMENTS
//	PageID									uiParentID_
//		[IN]		Parent's PageID
//	PageNum									uiIndex_
//		[IN]		Index pointing to child
//
//	RETURN
//	PageID
//					Child's PageID
//
//	EXCEPTIONS
//
PageID
DirectAreaTree::getChildPageID(PageID		uiParentID_,
							   PageNum	uiIndex_)
{
	; _SYDNEY_ASSERT(getStep(uiParentID_) != LeafStep);

	return uiParentID_ + uiIndex_ * m_uiSubtree[getStep(uiParentID_)] + 1;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getSubstantialChildPageID --
//		Get PageID pointing to substantial child
//
//	NOTES
//	Usually Root manages Nodes which is directly below it.
//	But when the number of Leafs is small,
//	Root manages directly Leafs or Nodes which is not directly below it.
//	So SUBSTANTIAL child means page which is ACTUALLY managed by parent.
//
//	ARGUMENTS
//	PageID									uiParentID_
//		[IN]		Parent's PageID
//	PageNum									uiIndex_
//		[IN]		Index pointing to child
//
//	RETURN
//	PageID
//					PageID pointing to substantial child
//
//	EXCEPTIONS
//
PageID
DirectAreaTree::getSubstantialChildPageID(
	const Trans::Transaction&	cTransaction_,
	PageID						uiParentID_,
	PageNum						uiIndex_)
{
	; _SYDNEY_ASSERT(getStep(uiParentID_) != LeafStep);

	if (uiParentID_ == FileHeader::VersionPageID)
	{
		// Set logical PageID
		uiParentID_ += getRootStep(cTransaction_);
	}
	return getChildPageID(uiParentID_, uiIndex_);
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getNonEmptyLeafID --
//		Get PageID of nonempty LeafID
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	PageID									uiPageID_
//		[IN]		PageID pointing to the last PageID
//
//	RETURN
//	PageID
//					PageID pointing to the last PageID with data
//
//	EXCEPTIONS
//
PageID
DirectAreaTree::getNonEmptyLeafID(const Trans::Transaction&	cTransaction_,
								  PageID					uiPageID_)
{
	; _SYDNEY_ASSERT(getStep(uiPageID_) == LeafStep);
	; _SYDNEY_ASSERT(m_pFile->getLastPageID(cTransaction_) == uiPageID_);

	const AreaSize max = m_pFile->getMaxStorableAreaSize();

	while (uiPageID_ > MinLeafID)
	{
		// Get previous LeafID
		uiPageID_ = getPreviousLeafID(uiPageID_);
		// Set uiPageID_ to Node's PageID which points to parent of Leaf.
		PageNum index;
		uiPageID_ = getSubstantialParentPageID(cTransaction_, uiPageID_, index);
		// Get Node's memory
		File::PagePointer p = m_pFile->fixVersionPage(
			cTransaction_, uiPageID_, Buffer::Page::FixMode::ReadOnly);

		const DirectArea::Size* pTop = getConstTopIndex(p);
		const DirectArea::Size* pLast = pTop + index;
		// Check forward
		for (; pLast >= pTop; --pLast)
		{
			if (*pLast != max)
			{
				// Found Leaf with data.

				return getSubstantialChildPageID(
					cTransaction_, uiPageID_,
					static_cast<PageNum>(pLast - pTop));
			}
		}
	}

	// Not found Leaf with data.
	return MinLeafID;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getPreviousLeafID --
//		Get previous Leaf's PageID
//
//	NOTES
//
//	ARGUMENTS
//	PageID									uiPageID_
//		[IN]		Base of PageID
//
//	RETURN
//	PageID
//					PageID pointing to previous Leaf's PageID
//
//	EXCEPTIONS
//

PageID
DirectAreaTree::getPreviousLeafID(PageID uiPageID_)
{
	; _SYDNEY_ASSERT(uiPageID_ > MinLeafID);
	
	while (getStep(--uiPageID_) != LeafStep) {}
	return uiPageID_;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getPreviousLeafIDwithResettingNodes --
//		Get previous Leaf's PageID with resetting Nodes
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Refference to transaction
//	PageID									uiPageID_
//		[IN]		Base of PageID
//
//	RETURN
//	PageID
//					PageID pointing to previous Leaf's PageID
//
//	EXCEPTIONS
//

PageID
DirectAreaTree::getPreviousLeafIDwithResettingNodes(
	const Trans::Transaction&	cTransaction_,
	PageID						uiPageID_)
{
	; _SYDNEY_ASSERT(getStep(uiPageID_) == LeafStep);
	; _SYDNEY_ASSERT(uiPageID_ > MinLeafID);
	
	while (getStep(--uiPageID_) != LeafStep)
	{
		// Parents of the Leaf have to be reset
		// when uiPageID_ is top of subtree.
		// These Nodes becomes to be unused.
		resetNode(cTransaction_, uiPageID_);
	}
	return uiPageID_;
}
		
//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getNextLeafID --
//		Get next Leaf's PageID with allocating Nodes
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Refference to transaction
//	PageID									uiPageID_
//		[IN]		Base of PageID
//
//	RETURN
//	PageID
//					PageID pointing to next Leaf's PageID
//
//	EXCEPTIONS
//

PageID
DirectAreaTree::getNextLeafID(const Trans::Transaction&	cTransaction_,
							  PageID					uiPageID_)
{
	; _SYDNEY_ASSERT(getStep(uiPageID_) == LeafStep);
	; _SYDNEY_ASSERT(m_pFile->getLastPageID(cTransaction_) == uiPageID_);
	
	while (getStep(++uiPageID_) != LeafStep)
	{
		// Reserve some VersionPages for Node.
		m_pFile->fixVersionPage(
			cTransaction_, uiPageID_, File::DiscardableAllocateFixMode);
	}
	return uiPageID_;
}

//	FUNCTION private static
//	PhysicalFile::DirectAreaTree::getTopIndex --
//		Get the top of the indexes with pointer
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DirectArea::Size*
DirectAreaTree::getTopIndex(File::PagePointer pPage_)
{
	return syd_reinterpret_cast<DirectArea::Size*>(
		pPage_->operator char*() + sizeof(FileHeader::Item_Type2));
}

//	FUNCTION private static
//	PhysicalFile::DirectAreaTree::getConstTopIndex --
//		Get the top of the indexes with const pointer
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
const DirectArea::Size*
DirectAreaTree::getConstTopIndex(File::PagePointer pPage_)
{
	return syd_reinterpret_cast<const DirectArea::Size*>(
		static_cast<const File::VersionPage&>(*pPage_).operator const char*()
		+ sizeof(FileHeader::Item_Type2));
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getStorableIndexInRoot --
//		Get storable page under Root.
//
//	NOTES
//	When m_usSecondSizes is DirectArea::InvalidSize,
//	NOT need parent to be updated.
//
//	ARGUMENTS
//	PagePointer							page_
//		[IN]		Pointer to memory
//	AreaSize							uiSize_
//		[IN]		The size of the area
//
//	RETURN
//	PageNum
//					The index
//					ConstValue::UndefinedPageNum is returned,
//					when NOT found a storable page.
//
//	EXCEPTIONS
//
PageNum
DirectAreaTree::getStorableIndexInRoot(
	const Trans::Transaction&	cTransaction_,
	File::PagePointer			pPage_,
	DirectArea::Size			usSize_)
{
	Step rootStep = getRootStep(cTransaction_);

	const DirectArea::Size* pSize = getConstTopIndex(pPage_);
	const DirectArea::Size* pTop = pSize;
	const DirectArea::Size* pLast =
		pSize + getLastIndex(cTransaction_, FileHeader::VersionPageID);

	// "more" is more than uiSize_ and minimum in the root.
	DirectArea::Size more = DirectArea::InvalidSize;
	PageNum index = ConstValue::UndefinedPageNum;
	for (; pSize <= pLast; ++pSize)
	{
		if (usSize_ < *pSize && *pSize < more)
		{
			// Update min.
			more = *pSize;
			index = static_cast<PageNum>(pSize - pTop);
		}
		else if (*pSize == usSize_)
		{
			index = static_cast<PageNum>(pSize - pTop);
			break;
		}
	}

	return index;
}

// private
//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getStorableIndex -- 
//		Get storable page.
//	NOTES
//	When m_usSecondSizes is DirectArea::InvalidSize,
//	NOT need parent to be updated.
//
//	ARGUMENTS
//	PagePointer							page_
//		[IN]		Pointer to memory
//	PageID								pageID_
//		[IN]		PageID
//	AreaSize							uiSize_
//		[IN]		The size of the area
//	DirectArea::Size					usSecondSizes_
//		[OUT]
//
//	RETURN
//					The index
//					ConstValue::UndefinedPageNum is returned,
//					when NOT found a storable page.
//
//	EXCEPTIONS
//
PageNum
DirectAreaTree::getStorableIndex(const Trans::Transaction&	cTransaction_,
								 File::PagePointer			page_,
								 PageID						pageID_,
								 DirectArea::Size			usSize_,
								 DirectArea::Size*			usSecondSizes_)
{
	; _SYDNEY_ASSERT(getStep(pageID_) != LeafStep);
	; _SYDNEY_ASSERT(pageID_ != FileHeader::VersionPageID);
	
	const DirectArea::Size* pSize = getConstTopIndex(page_);
	const DirectArea::Size* pTop = pSize;
	const DirectArea::Size* pLast =
		pSize + getLastIndex(cTransaction_, pageID_);

	// "moreEqual" is more than or equal to uiSize_ and minimum in the node.
	DirectArea::Size moreEqual = DirectArea::InvalidSize;
	// "less" is less than uiSize_ and maximum in the node.
	// If "less" is maximum in the node,
	// it will be new free space size in the node.
	DirectArea::Size less = 0;
	// bMax represents "moreEqual" is maximum in the node and
	// the parent need to be updated with "moreEqual".
	bool bMax = true;

	PageNum index = ConstValue::UndefinedPageNum;
	for (; pSize <= pLast; ++pSize)
	{
		if (less < *pSize && *pSize < usSize_)
		{
			// Update less.
			less = *pSize;
		}
		else if (usSize_ <= *pSize && *pSize < moreEqual)
		{
			index = static_cast<PageNum>(pSize - pTop);

			if (moreEqual != DirectArea::InvalidSize)
			{
				// This is second time when
				// the size which is more than or equal to uiSize_ is found.
				bMax = false;
				if (*pSize == usSize_)
				{
					// Had found more than "size" yet, and now found "size".
					break;
				}
			}

			// Update min
			moreEqual = *pSize;
		}
		else if (moreEqual <= *pSize)
		{
			// This is second time when
			// the size which is more than or equal to uiSize_ is found.
			bMax = false;
			if (moreEqual == usSize_)
			{
				// Had found equal to "size" yet,
				// and now found more than or equal to "size".
				break;
			}
		}
	}

	if (bMax == true)
	{
		usSecondSizes_[getStep(pageID_)] = less;
	}
	return index;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getLastIndex -- Get last index
//
//	NOTES
//
//	ARGUMENTS
//	PageID									uiPageID_
//		[IN]		PageID
//	Step									usStep_
//		[IN]		The step of the Node or Root
//
//	RETURN
//	PageNum
//					Last index in the Node or Root
//
//	EXCEPTIONS
//
PageNum
DirectAreaTree::getLastIndex(const Trans::Transaction&		cTransaction_,
							 PageID							uiPageID_,
							 Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(getStep(uiPageID_) != LeafStep);

	// Get last PageID managed in FILE.
	PageID lastPageID = m_pFile->getLastPageID(cTransaction_, pProgress_);
	// Get last PageID managed under TREE.
	lastPageID = getPreviousLeafID(lastPageID);

	while (lastPageID != FileHeader::VersionPageID)
	{
		PageNum index;
		lastPageID = getSubstantialParentPageID(
			cTransaction_, lastPageID, index, pProgress_);

		if (uiPageID_ == lastPageID)
		{
			// The page is one of last PageIDs.
			return index;
		}
	}

	// The page is none of last PageIDs.
	return m_uiMaxChildren - 1;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getUpdatedMaxSize -- Get the maximum size.
//
//	NOTES
//	For free Area or change Area's size
//
//	ARGUMENTS
//
//	RETURN
//	DirectArea::Size
//		DirectArea::InvalidSize is returned
//		when the max size is NOT changed for freeArea and chageAreaSize.
//
//	EXCEPTIONS
//
DirectArea::Size
DirectAreaTree::getUpdatedMaxSize(const DirectArea::Size*	pIndex_,
								  PageNum					uiLastIndex_,
								  PageNum					uiUpdatedIndex_,
								  DirectArea::Size			usPrevSize_) const
{
	// Get maximum size except one.
	DirectArea::Size current;
	DirectArea::Size max = getMaxSizeExceptOne(pIndex_, uiLastIndex_,
											   uiUpdatedIndex_, current);

	// previous maximum size
	usPrevSize_ = ModMax(usPrevSize_, max);
	// current maximum size
	current = ModMax(current, max);

	DirectArea::Size size = DirectArea::InvalidSize;
	if (usPrevSize_ != current)
	{
		// The maximum size was changed.
		size = current;
	}
	return size;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getMaxSize -- Get the maximum size except one.
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DirectArea::Size
DirectAreaTree::getMaxSizeExceptOne(
	const DirectArea::Size*	pIndex_,
	PageNum					uiLastIndex_,
	PageNum					uiExceptIndex_,
	DirectArea::Size&		usExceptSize_) const
{
	DirectArea::Size max = 0;

	const DirectArea::Size* pLast = pIndex_ + uiLastIndex_;
	const DirectArea::Size* pExcept = pIndex_ + uiExceptIndex_;
	for (; pIndex_ <= pLast; ++pIndex_)
	{
		if (pIndex_ == pExcept)
		{
			usExceptSize_ = *pIndex_;
		}
		else
		{
			max = ModMax(*pIndex_, max);
		}
	}
	return max;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getMaxSize -- Get the maximum size simply.
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DirectArea::Size
DirectAreaTree::getMaxSize(const DirectArea::Size*	pIndex_,
						   PageNum					uiLastIndex_) const
{
	DirectArea::Size max = 0;

	const DirectArea::Size* pLast = pIndex_ + uiLastIndex_;
	while (pIndex_ <= pLast)
	{
		max = ModMax(*pIndex_++, max);
	}
	return max;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::getVerifyResult --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DirectAreaTree::VerifyResult*
DirectAreaTree::getVerifyResult(PageID uiPageID_)
{
	VerifyResult* result = 0;

	VerifyMap::Iterator	i = m_cVerifyMap.find(uiPageID_);
	if (i != m_cVerifyMap.end())
	{
		// Found result.
		result = (*i).second;
	}
	return result;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::modifyVerifyResult --
//
//	NOTES
//	This is used only for deleting AreaIDs and changing free space size.
//	Initilizing VerifyResult is executed in DirectAreaPage::verify.
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaTree::modifyVerifyResult(PageID							uiPageID_,
								   AreaID							uiAreaID_,
								   AreaSize						uiFreeSize_,
								   Admin::Verification::Progress*	pProgress_)
{
	// Get VerifyResult.
	VerifyResult* result = getVerifyResult(uiPageID_);
	if (result == 0)
	{
		// Not found result.
		// It should be notified with DirectAreaTree::notifyUsePage.
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	if (uiAreaID_ != ConstValue::UndefinedAreaID)
	{
		// Delete the area from the lists.
		
		AreaIDMap::Iterator j = result->m_cAreaIDMap.find(uiAreaID_);
		if (j != result->m_cAreaIDMap.end())
		{
			// Found tha AreaID.
			// Not mind whether the area was notified or not.
			// Because the area may be expunged without verifyArea().
			result->m_cAreaIDMap.erase(j);
		}
		else
		{
			// Not found the AreaID
			_SYDNEY_VERIFY_INCONSISTENT(
				*pProgress_, m_pFile->getFilePath(),
				Message::DiscordAreaUseSituation2(uiPageID_, uiAreaID_));
		}
	}
	
	// Set new free space size.
	result->m_uiAreaSize = uiFreeSize_;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::verifyNode -- Verify Node/Root
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	Buffer::Page::FixMode::Value			eFixMode_
//		[IN]		FixMode
//	Admin::Verification::Progress&			cProgress_
//		[IN/OUT]	Reference to pregress
//	PageID&									uiPageID_
//		[IN]		PageID pointing to Node/Root
//
//	RETURN
//	DirectArea::Size
//					The free space area size of the page
//
//	EXCEPTIONS
//
DirectArea::Size
DirectAreaTree::verifyNode(const Trans::Transaction&		cTransaction_,
						   PageID&							uiPageID_,
						   Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(getStep(uiPageID_) != LeafStep);
	
	PageID parentPageID = uiPageID_;

	// Get Root or Node.
	File::PagePointer page = m_pFile->fixVersionPage(
		cTransaction_, uiPageID_, Buffer::Page::FixMode::ReadOnly, pProgress_);

	const DirectArea::Size* p = getConstTopIndex(page);
	const DirectArea::Size* pTop = p;
	const DirectArea::Size* pLast =
		p + getLastIndex(cTransaction_, uiPageID_, pProgress_);

	// Set logical PageID.
	if (uiPageID_ == FileHeader::VersionPageID)
	{
		uiPageID_ +=
			getRootStep(cTransaction_, ConstValue::UndefinedPageID, pProgress_);
	}

	DirectArea::Size max = 0;
	for (; p <= pLast; ++p)
	{
		// Set the top child's PageID or increment it.
		// If it is incremented in for-sentence, Node's PageID is done twice.
		PageID pageID = ++uiPageID_;

		// Get free space size.
		DirectArea::Size size;
		if (getStep(uiPageID_) == LeafStep)
		{
			// uiPageID_ points to Leaf.
			size = verifyLeaf(cTransaction_, uiPageID_, pProgress_);
		}
		else
		{
			// uiPageID_ points to Node.
			size = verifyNode(cTransaction_, uiPageID_, pProgress_);
		}

		// Check size.
		if (size != *p)
		{
			// Inconsistent!

			if (m_pFile->isCorrect() == true &&
				pProgress_->isGood() == true)
			{
				// Tree is traced with Depth-First-Search.
				// So this modification is transmitted to Root.
				setSize(
					cTransaction_, parentPageID,
					static_cast<PageNum>(p - pTop), size, pProgress_);
				_SYDNEY_VERIFY_CORRECTED(
					*pProgress_, m_pFile->getFilePath(),
					Message::CorrectedNodeFreeArea(
						parentPageID,
						static_cast<int>(p - pTop), *p, pageID, size));
			}
			else
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					*pProgress_, m_pFile->getFilePath(),
					Message::DiscordNodeFreeArea(
						parentPageID,
						static_cast<int>(p - pTop), *p, pageID, size));
			}
		}

		// Update size.
		max = ModMax(size, max);
	}

	return max;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::verifyLeaf -- Verify leaf
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	PageID&									uiPageID_
//		[IN]		PageID pointing to Leaf
//
//	RETURN
//	DirectArea::Size
//					The free space area size of the page
//
//	EXCEPTIONS
//
DirectArea::Size
DirectAreaTree::verifyLeaf(const Trans::Transaction&		cTransaction_,
						   PageID							uiPageID_,
						   Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(getStep(uiPageID_) == LeafStep);
	
	DirectAreaPage* pLeaf = 0;
	ModVector<AreaID> unnotified;

	// Get VerifyResult
	VerifyResult* result = getVerifyResult(uiPageID_);
	if (result == 0)
	{
		// The page has NOT been verified yet.
		
		if (m_pFile->isCorrect() == true)
		{
			pLeaf = m_pFile->attachPage(
				cTransaction_, uiPageID_, File::DiscardableWriteFixMode,
				pProgress_);
		}
		else
		{
			pLeaf = m_pFile->attachPage(
				cTransaction_, uiPageID_, Buffer::Page::FixMode::ReadOnly,
				pProgress_);
		}

		// VerifyResult has been set
		// in notifyUsePage via DirectFile::attachPage.
		result = (*m_cVerifyMap.find(uiPageID_)).second;

		// [OPTIMIZE!] This is difficult to understand.
	}

	// Check the result.
	AreaIDMap::Iterator j = result->m_cAreaIDMap.begin();
	for (; j != result->m_cAreaIDMap.end(); ++j)
	{
		if ((*j).second == false)
		{
			// The AreaID is NOT notified.
			if (m_pFile->isCorrect() == true)
			{
				// Delete the area later.
				// If the area is deleted now, AreaIDMap will be changed.
				unnotified.pushBack((*j).second);
			}
			else
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					*pProgress_, m_pFile->getFilePath(),
					Message::DiscordAreaUseSituation1(uiPageID_, (*j).first));
			}
		}
	}

	// Delete the areas.
	if (unnotified.getSize() != 0)
	{
		if (pLeaf == 0)
		{
			pLeaf = m_pFile->attachPage(
				cTransaction_, uiPageID_, File::DiscardableWriteFixMode,
				pProgress_);
		}

		// Freeing last Area in the Page is faster than the others.
		// So, the areas is deleted from the last one.
		ModVector<AreaID>::ConstIterator k = unnotified.end();
		while (k-- != unnotified.begin())
		{
			// Delete the area.
			pLeaf->freeArea(cTransaction_, *k, pProgress_);
			pLeaf->dirty();

			_SYDNEY_VERIFY_CORRECTED(
				*pProgress_, m_pFile->getFilePath(),
				Message::CorrectedAreaUseSituation3(uiPageID_, *k));
		}

		// 'result->m_uiAreaSize' is updated
		// in notifyUnuseArea via DirectAreaPage::freeArea.

		// [OPTIMIZE!] This is difficult to understand.
	}

	// Detach Leaf;
	if (pLeaf != 0)
	{
		Page* temp = pLeaf;
		m_pFile->detachPage(temp);
	}

	return static_cast<DirectArea::Size>(result->m_uiAreaSize);
}

//	FUNCTION private
//	PhysicalFile::DirectAreaTree::setSize --
//		Verify Node/Root's free area size
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	PageID									uiPageID_
//		[IN]
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaTree::setSize(
	const Trans::Transaction&		cTransaction_,
	PageID							uiPageID_,
	PageNum							uiIndex_,
	DirectArea::Size				usSize_,
	Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(getStep(uiPageID_) != LeafStep);

	File::PagePointer page = m_pFile->fixVersionPage(
		cTransaction_, uiPageID_, File::DiscardableWriteFixMode, pProgress_);
	DirectArea::Size* p = getTopIndex(page) + uiIndex_;
	*p = usSize_;
}

//
//	Copyright (c) 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
