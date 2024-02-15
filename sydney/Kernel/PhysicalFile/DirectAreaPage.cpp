// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectAreaPage.cpp --
//		物理エリア管理機能付き物理ファイルの物理ページ関連の関数定義
// 
// Copyright (c) 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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
#include "PhysicalFile/DirectAreaPage.h"
#include "PhysicalFile/DirectAreaFile.h"
#include "PhysicalFile/Message_DiscordLeafHeader.h"
#include "PhysicalFile/Message_DisorderedLeafIndexKey.h"
#include "PhysicalFile/Message_DisorderedLeafIndexOffset.h"
#include "PhysicalFile/Message_DiscordAreaOffset.h"
#include "PhysicalFile/Message_DiscordAreaUseSituation1.h"
#include "PhysicalFile/Message_DiscordAreaUseSituation2.h"
#include "PhysicalFile/Message_CorrectedAreaUseSituation3.h"
#include "PhysicalFile/Message_DiscordLeafHeaderOffset.h"
#include "PhysicalFile/Message_CorrectedLeafHeaderOffset.h"
#include "PhysicalFile/Message_DiscordLeafIndexOffset.h"

#include "Common/Assert.h"

#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaPage クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::DirectAreaPage -- Constructor
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	File*									pFile_
//		[IN]		Pointer to File
//	const PageID							uiPageID_
//		[IN]		PageID
//	const Buffer::Page::FixMode::Value		eFixMode_
//		[IN]		FixMode
//	const Buffer::ReplacementPriority::Value eReplacementPriority_
//		[IN]		ReplacementPriority
//
//	RETURN
//
//	EXCEPTIONS
//
DirectAreaPage::DirectAreaPage(
	const Trans::Transaction&					cTransaction_,
	DirectAreaFile*								pFile_,
	const PageID								uiPageID_,
	const Buffer::Page::FixMode::Value			eFixMode_,
	const Buffer::ReplacementPriority::Value	eReplacementPriority_)
	:Page(cTransaction_, pFile_, uiPageID_, eFixMode_, eReplacementPriority_),
	 m_pFile(pFile_)
{}

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::DirectAreaPage -- Constructor
//
//	NOTES
//	For checking consistency
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	File*									pFile_
//		[IN]		Pointer to File
//	const PageID							uiPageID_
//		[IN]		PageID
//	const Buffer::Page::FixMode::Value		eFixMode_
//		[IN]		FixMode
//	Admin::Verification::Progress&			cProgress_
//		[IN]		Progress
//
//	RETURN
//
//	EXCEPTIONS
//
DirectAreaPage::DirectAreaPage(
	const Trans::Transaction&					cTransaction_,
	DirectAreaFile*								pFile_,
	const PageID								uiPageID_,
	const Buffer::Page::FixMode::Value			eFixMode_,
	Admin::Verification::Progress&				cProgress_)
	:Page(cTransaction_, pFile_, uiPageID_, eFixMode_, cProgress_),
	 m_pFile(pFile_)
{}

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::~DirectAreaPage -- Destructor
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DirectAreaPage::~DirectAreaPage()
{}

//	FUCNTION public
//	PhysicalFile::PageManagePage::reset -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DirectAreaPage::reset(const Trans::Transaction&			cTransaction_,
					  PageID							uiPageID_,
					  Buffer::Page::FixMode::Value		eFixMode_,
					  Buffer::ReplacementPriority::Value  eReplacementPriority_)
{
	Page::reset(
		cTransaction_, m_pFile, uiPageID_, eFixMode_, eReplacementPriority_);
}

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::getAreaOffset -- Get the offset of an area
//
//	NOTES
//
//	ARGUMENTS
//	AreaID									uiAreaID_
//		[IN]		AreaID
//
//	RETURN
//	PageOffset
//					The offset of the area pointed by the AreaID
//					ConstValue::UndefinedAreaOffset is returned,
//					when the area does NOT exist.
//
//	EXCEPTIONS
//
PageOffset
DirectAreaPage::getAreaOffset(AreaID uiAreaID_) const
{
	PageOffset offset = ConstValue::UndefinedAreaOffset;

	const LeafHeader* pHeader = getConstLeafHeader();
	if (pHeader->m_usTotal != 0)
	{
		// Get LeafIndex
		// pIndex and pLastIndex has to be used with const.
		// But binsearch() does not support 'const LeafIndex*'.
		LeafIndex* pIndex = getLeafIndex(0);
		LeafIndex* pLastIndex = pIndex - (pHeader->m_usTotal - 1);
		pIndex = binsearch(pIndex, pLastIndex, uiAreaID_);
		// Get offset
		if (pIndex != 0)
		{
			offset = static_cast<PageOffset>(pIndex->m_usOffset);
		}
	}

	return offset;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::getAreaSize -- Get the size of the area
//
//	NOTES
//
//	ARGUMENTS
//	AreaID									uiAreaID_
//		[IN]		AreaID
//
//	RETURN
//	AreaSize
//					The size of the area pointed by the AreaID
//					ConstValue::UndefinedAreaSize is returned,
//					when the area does NOT exist.
//
//	EXCEPTIONS
//
AreaSize
DirectAreaPage::getAreaSize(AreaID uiAreaID_) const
{
	AreaSize size = ConstValue::UndefinedAreaSize;

	const LeafHeader* pHeader = getConstLeafHeader();
	if (pHeader->m_usTotal != 0)
	{
		// Get LeafIndex
		// pIndex and pLastIndex has to be used with const.
		// But binsearch() does not support 'const LeafIndex*'.
		LeafIndex* pIndex = getLeafIndex(0);
		LeafIndex* pLastIndex = pIndex - (pHeader->m_usTotal - 1);
		pIndex = binsearch(pIndex, pLastIndex, uiAreaID_);
		// Get size
		if (pIndex != 0)
		{
			size = getAreaSize(pHeader, pIndex, pLastIndex);
		}
	}

	return size;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaPage's public functions
//
//	These functions exist in ONLY DirectAreaPage
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::initialize -- Initialize page
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
DirectAreaPage::initialize()
{
	LeafHeader* p = getLeafHeader();
	p->m_usTotal = 0;
	p->m_usSpaceOffset = sizeof(LeafHeader);
}

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::allocateArea -- Allocate an area
//
//	NOTES
//
//	ARGUMENTS
//	AreaSize							uiSize_
//		[IN]		The size of the area
//
//	RETURN
//	DirectArea
//
//	EXCEPTIONS
//
DirectArea
DirectAreaPage::allocateArea(AreaSize uiSize_)
{
	LeafHeader* pHeader = getLeafHeader();
	LeafIndex* pIndex = getLeafIndex(0);
	LeafIndex* pLastIndex = pIndex - (pHeader->m_usTotal - 1);

	DirectArea::IndexKey key = 0;
	if (pHeader->m_usTotal != 0)
	{
		// Search position for new area.
		pIndex = binsearchForUnusedIndexKey(pIndex, pLastIndex, key);
	}

	// Update areas and indexes fields.
	if (pHeader->m_usTotal != 0 && pLastIndex <= pIndex)
	{
		// Add area in the gap.

		// Update areas
		// The range of uiSize_ is expected to be normal.
		insertArea(pHeader, pIndex, uiSize_);
		// Update indexes
		insertLeafIndex(pHeader, pIndex, pLastIndex, key, uiSize_);
	}
	else
	{
		// Add area at the end.
		
		// Set the new index.
		pIndex->m_usIndexKey = key;
		pIndex->m_usOffset = pHeader->m_usSpaceOffset;
	}
	// Update header field.
	pHeader->m_usSpaceOffset += static_cast<DirectArea::Offset>(uiSize_);
	++(pHeader->m_usTotal);

	// Allocate area
	DirectArea area = attachArea(static_cast<AreaID>(key));
	area.reset();
	return area;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::attachArea -- Attach an area
//
//	NOTES
//
//	ARGUMENTS
//	AreaID									uiAreaID_
//		[IN]		AreaID
//
//	RETURN
//	DirectArea
//
//	EXCEPTIONS
//
DirectArea
DirectAreaPage::attachArea(AreaID uiAreaID_)
{
	return DirectArea(this, uiAreaID_);
}

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::freeArea -- Free an area
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	AreaID									uiAreaID_
//		[IN]		AreaID
//	bool									bUpdate_ = true
//		[IN]		The case where tree has to be updated.
//
//	RETURN
//
//	EXCEPTIONS
//	Exception::BadArgument
//					The area pointed by the given AreaID does NOT exist.
//
void
DirectAreaPage::freeArea(const Trans::Transaction&		cTransaction_,
						 AreaID							uiAreaID_,
						 Admin::Verification::Progress*	pProgress_)
{
	LeafHeader* pHeader = getLeafHeader();
	if (pHeader->m_usTotal == 0)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	LeafIndex* pIndex = getLeafIndex(0);
	LeafIndex* pLastIndex = pIndex - (pHeader->m_usTotal - 1);

	// Get LeafIndex
	pIndex = binsearch(pIndex, pLastIndex, uiAreaID_);
	if (pIndex == 0)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	if (pIndex == pLastIndex)
	{
		// The freed area is the end of areas, so don't need to move anything.

		// Update header.
		pHeader->m_usSpaceOffset = pIndex->m_usOffset;
	}
	else
	{
		// Update Areas.
		DirectArea::Size size = expungeArea(pHeader, pIndex);
		// Update Indexes.
		expungeLeafIndex(pHeader, pIndex, pLastIndex, size);
		//Update header.
		pHeader->m_usSpaceOffset -= size;
	}
	// Update header.
	--(pHeader->m_usTotal);

	// Update tree
	AreaSize freeSize = getFreeSize();
	m_pFile->updateTree(cTransaction_, m_ID, freeSize, pProgress_);

	if (pProgress_ != 0)
	{
		// Notify unusing area
		m_pFile->notifyUnuseArea(m_ID, uiAreaID_, freeSize, pProgress_);
	}
}

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::changeAreaSize -- Change the size of a area
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&					cTransaction_
//		[IN]		Reference to transaction
//	AreaID										uiAreaID_
//		[IN]		AreaID
//	AreaSize									uiSize_
//		[IN]		The new size of the area
//	bool										bDummy_
//		[]			Dummy
//
//	RETURN
//	bool
//					Success to change the size
//
//	EXCEPTIONS
//	Exception::BadArgument
//					The area pointed by the given AreaID does NOT exist.
//
bool
DirectAreaPage::changeAreaSize(const Trans::Transaction&		cTransaction_,
							   AreaID							uiAreaID_,
							   AreaSize							uiRequestSize_,
							   Admin::Verification::Progress*	pProgress_)
{
	LeafHeader* pHeader = getLeafHeader();
	if (pHeader->m_usTotal == 0)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// Get LeafIndex.
	LeafIndex* pIndex = getLeafIndex(0);
	LeafIndex* pLastIndex = pIndex - (pHeader->m_usTotal - 1);
	pIndex = binsearch(pIndex, pLastIndex, uiAreaID_);
	if (pIndex == 0)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// Check each sizes.
	AreaSize formerSize = getAreaSize(pHeader, pIndex, pLastIndex);
	AreaSize newSize = DirectAreaFile::roundUp(uiRequestSize_);
	AreaSize freeSize = getFreeSize(false);
	int diffSize = newSize - formerSize;
	if (diffSize == 0)
	{
		// New size is equal to former size.
		initializeArea(pIndex, uiRequestSize_, newSize - uiRequestSize_);
		return true;
	}
	else if	(diffSize > static_cast<int>(freeSize))
	{
		// Not enough space.
		return false;
	}

	// Update areas and indexes.
	if (pIndex != pLastIndex)
	{
		// Update Areas.
		// The range of diffSize is '-formerSize < diffSize <= freeSize'.
		resizeArea(pHeader, pIndex, diffSize);
		// Update Indexes.
		updateOffsets(pIndex - 1, pLastIndex, diffSize);
	}
	// Update header.
	pHeader->m_usSpaceOffset += diffSize;
	
	// Initialize area.
	if (diffSize > 0)
	{
		// A part of expanded space
		initializeArea(pIndex, formerSize, diffSize);
	}
	else
	{
		// A part of unused space which is expanded by roundUp().
		initializeArea(pIndex, uiRequestSize_, newSize - uiRequestSize_);
	}

	// Update tree.
	freeSize = verifyFreeSize(freeSize - diffSize);
	m_pFile->updateTree(cTransaction_, m_ID, freeSize, pProgress_);

	if (pProgress_ != 0)
	{
		// Notify free space size
		m_pFile->notifyFreeSpaceSize(m_ID, freeSize, pProgress_);
	}

	return true;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::changeFixMode -- Change FixMode
//
//	NOTES
//	Previous FixMode is ReadOnly and New FixMode is NOT ReadOnly.
//
//	ARGUMENTS
//	const Trans::Transaction&					cTransaction_
//		[IN]		Reference to transaction
//	File*										pFile_
//		[IN/OUT]	Pointer to file
//	PageID										uiPageID_
//		[IN]		PageID
//	Buffer::Page::FixMode::Value				eFixMode_
//		[IN]		FixMode
//	Buffer::ReplacementPriority::Value			ePriority_
//		[IN]		ReplacementPriority
//	Admin::Verification::Progress*				pProgress_
//		[IN]		Progress
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaPage::changeFixMode(
	const Trans::Transaction&					cTransaction_,
	File*										pFile_,
	PageID										uiPageID_,
	Buffer::Page::FixMode::Value				eFixMode_,
	const Buffer::ReplacementPriority::Value*	pPriority_,
	Admin::Verification::Progress*				pProgress_)
{
	; _SYDNEY_ASSERT(m_ID == uiPageID_ && m_pFile == pFile_
					 && m_FixMode == Buffer::Page::FixMode::ReadOnly);
	
	m_Memory.unfix(false);

	m_FixMode = eFixMode_;
	if (pProgress_ == 0)
	{
		m_Memory = Version::Page::fix(
			cTransaction_, *m_pFile->getVersionFile(),
			m_pFile->convertToVersionPageID(m_ID), m_FixMode, *pPriority_);
	}
	else
	{
		m_Memory = Version::Page::verify(
			cTransaction_, *m_pFile->getVersionFile(),
			m_pFile->convertToVersionPageID(m_ID), m_FixMode, *pProgress_);
	}
	// For Page::operator char*() which is used in DirectArea::operator void*()
	m_PhysicalPageTop = m_Memory.operator char*();
}

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::getFreeSize -- Get the free space size
//
//	NOTES
//	When bAllocate_ is equal to true,
//	the size of LeafIndex for the new allocating area is removed.
//
//	ARGUMENTS
//	bool								bAllocate_
//		[IN]	   	The case of allocating new area
//
//	RETURN
//	AreaSize
//					The free space size of the page
//
//	EXCEPTIONS
//
AreaSize
DirectAreaPage::getFreeSize(bool bAllocate_) const
{
	const LeafHeader* pHeader = getConstLeafHeader();
	AreaSize size =	static_cast<AreaSize>(m_pFile->getPageSize());
	AreaSize usingSize = static_cast<AreaSize>(
		pHeader->m_usSpaceOffset + pHeader->m_usTotal * sizeof(LeafIndex));
	if (usingSize > size)
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	size -= usingSize;

	if (bAllocate_ == true)
	{
		size = verifyFreeSize(size);
	}

	return size;
}

//	FUNCTION public static
//	PhysicalFile::DirectAreaPage::verifyFreeSize
//		-- Verify the free space size for allocating an area
//
//	NOTES
//
//	ARGUMENTS
//	AreaSize									uiFreeSize_
//		[IN]		The free space size for resizing an area
//
//	RETURN
//	AreaSize
//					The free space size for allocating an area
//
//	EXCEPTIONS
//
AreaSize
DirectAreaPage::verifyFreeSize(AreaSize uiFreeSize_)
{
	if (uiFreeSize_ > sizeof(LeafIndex))
	{
		uiFreeSize_ -= sizeof(LeafIndex);
	}
	else
	{
		uiFreeSize_ = 0;
	}
	return uiFreeSize_;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaPage::verify -- Verify the page
//
//	NOTES
//
//	ARGUMENTS
//	Admin::Verification::Progress&			cProgress_
//		[IN/OUT]	Reference to progress
//	DirectAreaFile::AreaIDMap&				cAreaIDMap_
//		[IN/OUT]	Map of AreaIDs
//
//	RETURN
//	AreaSize
//					The free space size
//
//	EXCEPTIONS
//
AreaSize
DirectAreaPage::verify(const Trans::Transaction&		cTransaction_,
					   Admin::Verification::Progress&	cProgress_,
					   DirectAreaTree::AreaIDMap&		cAreaIDMap_)
{
	// Check verify mode.
	; _SYDNEY_ASSERT(m_pFile->isCorrect() == false
					 || getFixMode() != Buffer::Page::FixMode::ReadOnly);
	
	const LeafHeader* pHeader = getConstLeafHeader();
	DirectArea::Num total = pHeader->m_usTotal;
	DirectArea::Offset spaceOffset = pHeader->m_usSpaceOffset;

	// Initialize AreaIDMap.
	cAreaIDMap_.reserve(total);

	// Check Header
	if (total == 0 && spaceOffset != sizeof(LeafHeader))
	{
		if (m_pFile->isCorrect() == true)
		{
			LeafHeader* p = getLeafHeader();
			p->m_usSpaceOffset = sizeof(LeafHeader);
			dirty();
			_SYDNEY_VERIFY_CORRECTED(
				cProgress_,	m_pFile->getFilePath(),
				Message::CorrectedLeafHeaderOffset(
					m_ID, 0, sizeof(LeafHeader)));
		}
		else
		{
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_,	m_pFile->getFilePath(),
				Message::DiscordLeafHeaderOffset(m_ID, 0, spaceOffset));
		}
	}
	DirectArea::Offset lastIndexPos =
		static_cast<DirectArea::Offset>(m_pFile->getPageSize())
		- static_cast<DirectArea::Offset>(total) * sizeof(LeafIndex);
	if (lastIndexPos < spaceOffset)
	{
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_, m_pFile->getFilePath(),
			Message::DiscordLeafHeader(m_ID, total, lastIndexPos, spaceOffset));
	}

	// Check Indexes
	const LeafIndex* pIndex = getConstLeafIndex(0);
	const LeafIndex* pTop = pIndex;
	const LeafIndex* pLast = pIndex - total + 1;
	DirectArea::IndexKey prevKey = 0;
	DirectArea::Offset prevOffset = 0;
	for (; pIndex >= pLast; --pIndex)
	{
		// Check offset
		DirectArea::Offset offset = pIndex->m_usOffset;

		if (pIndex == pTop)
		{
			if (offset != sizeof(LeafHeader))
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					cProgress_, m_pFile->getFilePath(),
					Message::DiscordLeafIndexOffset(m_ID, 0, offset));
			}
		}
		else if	(offset <= prevOffset)
		{
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_, m_pFile->getFilePath(),
				Message::DisorderedLeafIndexOffset(
					m_ID, static_cast<int>(pTop - pIndex), offset, prevOffset));
		}
		prevOffset = offset;

		// Check IndexKey
		DirectArea::IndexKey key = pIndex->m_usIndexKey;
		if (pIndex != pTop && key <= prevKey)
		{
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_,	m_pFile->getFilePath(),
				Message::DisorderedLeafIndexKey(
					m_ID, static_cast<int>(pTop - pIndex), key, prevKey));
		}
		prevKey = key;

		// Set using area.
		// 'false' means the AreaID has NOT been notified yet.
		cAreaIDMap_.insert(static_cast<AreaID>(key), false);
	}

	// Check free space size
	if (total != 0 && prevOffset >= spaceOffset)
	{
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_, m_pFile->getFilePath(),
			Message::DiscordAreaOffset(
				m_ID, total - 1, prevOffset, spaceOffset));
	}
	return static_cast<DirectArea::Size>(getFreeSize());
}

#ifdef DEBUG
void
DirectAreaPage::setLeafHeaderNumber(AreaNum uiNum_)
{
	LeafHeader* p = getLeafHeader();
	p->m_usTotal = static_cast<DirectArea::Num>(uiNum_);
	p->m_usSpaceOffset = sizeof(LeafHeader);
}

void
DirectAreaPage::setLeafHeaderOffset(AreaOffset uiOffset_)
{
	LeafHeader* p = getLeafHeader();
	p->m_usSpaceOffset = static_cast<DirectArea::Offset>(uiOffset_);
}

void
DirectAreaPage::setLeafIndexAreaID(AreaID	uiAreaID_,
								   AreaNum	uiIndex_)
{
	LeafIndex* p = getLeafIndex(uiIndex_);
	p->m_usIndexKey = static_cast<DirectArea::IndexKey>(uiAreaID_);
}

void
DirectAreaPage::setLeafIndexOffset(AreaOffset	uiOffset_,
								   AreaNum		uiIndex_)
{
	LeafIndex* p = getLeafIndex(uiIndex_);
	p->m_usOffset = static_cast<DirectArea::Offset>(uiOffset_);
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaPage クラスの private メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::getLeafIndex -- Get LeafIndex
//
//	NOTES
//	The 0 of index_ represents the top of LeafIndexes.
//
//	ARGUMENTS
//	unsigned short								index_
//		[IN]		The number of the LeafIndex from the top of the LeafIndexes
//
//	RETURN
//	DirectAreaPage::LeafIndex*
//				Pointer to LeafIndex
//
//	EXCEPTIONS
//
DirectAreaPage::LeafIndex*
DirectAreaPage::getLeafIndex(unsigned short index_) const
{
	return syd_reinterpret_cast<LeafIndex*>(
		m_PhysicalPageTop + m_pFile->getPageSize()) - index_ - 1;
}
//	FUNCTION private
//	PhysicalFile::DirectAreaPage::getConstLeafIndex -- Get const LeafIndex
//
//	NOTES
//
//	ARGUMENTS
//	unsigned short								index_
//		[IN]		The number of the LeafIndex from the top of the LeafIndexes
//
//	RETURN
//	const DirectAreaPage::LeafIndex*
//					Pointer to const LeafIndex
//
//	EXCEPTIONS
//
const DirectAreaPage::LeafIndex*
DirectAreaPage::getConstLeafIndex(unsigned short index_) const
{
	return syd_reinterpret_cast<const LeafIndex*>(
		m_PhysicalPageTop + m_pFile->getPageSize()) - index_ - 1;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::binsearchForUnusedIndexKey --
//		Binary Search for LeafIndex whose AreaID is unused and minimum
//
//	NOTES
//	[CAUTION!] NOT use this function when the no area exists in the page.
//
//	The searched AreaID is unused and minimum in the page.
//	There is first gap of AreaID
//	between the searched LeafIndex and adjasent one.
//	So, the AreaIDs of LeafIndexes from first one to the adjasent searched one
//	have no gap.
//	But, when the AreaID of first LeafIndex is NOT 0,
//	the searched LeafIndex is first one.
//
//	The LeafIndex will be moved forward when an area will be inserted.
//
//	ARGUMENTS
//	LeafIndex*								begin_
//		[IN]		The begin position of the LeafIndexes
//	LeafIndex*								end_
//		[IN]		The end position of the LeafIndexes
//	DirectArea::IndexKey&					key_
//		[OUT]		The ID which is equal to casted AreaID
//
//	RETURN
//	DirectAreaPage::LeafIndex*
//					Pointer to LeafIndex
//
//	EXCEPTIONS
//
DirectAreaPage::LeafIndex*
DirectAreaPage::binsearchForUnusedIndexKey(LeafIndex*				begin_,
										   LeafIndex*				end_,
										   DirectArea::IndexKey&	key_) const
{
	// Check first index.
	const LeafIndex* constBegin = begin_;
	if (begin_->m_usIndexKey != 0)
	{
		// The AreaID of first LeafIndex is NOT 0.
		// So the AreaID 0 is NOT used.
		key_ = 0;
		return begin_;
	}
	--begin_;
	
	// Check last index.
	if (end_->m_usIndexKey == constBegin - end_)
	{
		// The AreaID of end_ is equal to the number of LeafIndexes minus 1.
		// So there is No gap in the LeafIndexes.
		key_ = end_->m_usIndexKey + 1;
		return end_ - 1;

		// [OPTIMIZE!]
		// This if-sentence can be included below while-loop.
		// If the ratio of this situation is high, this code would be good.
		// Because, the while-loop is skipped.
		// In reverse case, this code would be bad.
		// Because, this if-sentence must always be executed.
	}
	++end_;

	while (end_ <= begin_)
	{
		LeafIndex* mid = end_ + (begin_ - end_) / 2;
		if (mid->m_usIndexKey == constBegin - mid)
		{
			// Not found any gap between begin_ and mid.
			begin_ = mid - 1;
		}
		else // (mid->m_usIndexKey > constBegin - mid)
		{
			// Found some gap
			end_ = mid + 1;
		}
	}

	key_ = end_->m_usIndexKey + 1;
	return begin_;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::binsearch -- Binary Search for LeafIndex
//
//	NOTES
//
//	ARGUMENTS
//	LeafIndex*								begin_
//		[IN]		The begin position of the LeafIndexes
//	LeafIndex*								end_
//		[IN]		The end position of the LeafIndexes
//	AreaID									key_
//		[IN]		AreaID 
//
//	RETURN
//	DirectAreaPage::LeafIndex*
//					Pointer to LeafIndex whoes key is equal to key_.
//					When the LeafIndex does NOT exists, return 0.
//
//	EXCEPTIONS
//
DirectAreaPage::LeafIndex*
DirectAreaPage::binsearch(LeafIndex*	begin_,
						  LeafIndex*	end_,
						  AreaID		key_) const
{
	DirectArea::IndexKey key = static_cast<DirectArea::IndexKey>(key_);

	if (begin_ - end_ > key)
	{
		// key is less than the number of LeafIndexes.
		// If key exists somewhere in LeafIndexes,
		// it would exists between begin_ and 'key'th LeafIndex.
		end_ = begin_ - key;
	}

	while (end_ <= begin_)
	{
		LeafIndex* mid = end_ + (begin_ - end_) / 2;
		if (key == mid->m_usIndexKey)
		{
			return mid;
		}
		else if (key > mid->m_usIndexKey)
		{
			begin_ = mid - 1;
		}
		else
		{
			end_ = mid + 1;
		}
	}
	return 0;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::insertArea -- Insert an area
//
//	NOTES
//	The areas next to the area pointed by the pIndex_ are moved backward.
//
//	ARGUMENTS
//	const LeafHeader*						pHeader_
//		[IN]		LeafHeader
//	const LeafIndex*						pIndex_
//		[IN]		LeafIndex which become to point to the inserted area
//	AreaSize								uiSize_
//		[IN]		The size of the inserted area
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaPage::insertArea(const LeafHeader*	pHeader_,
						   const LeafIndex*		pIndex_,
						   AreaSize				uiSize_)
{
	// The range of uiSize_ is expected to be normal.
	moveAreas(pHeader_, pIndex_, static_cast<int>(uiSize_));
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::expungeArea -- Expunge an area
//
//	NOTES
//	The area pointed by the LeafIndex is expunged and
//	the areas next to the area are moved forward.
//
//	ARGUMENTS
//	const LeafHeader*						pHeader_
//		[IN]		LeafHeader
//	const LeafIndex*						pIndex_
//		[IN]		LeafIndex which points to the expunged area
//
//	RETURN
//	DirectArea::Size
//					The size of the expunged area
//	EXCEPTIONS
//
DirectArea::Size
DirectAreaPage::expungeArea(const LeafHeader*	pHeader_,
							const LeafIndex*	pIndex_)
{
	// pIndex_ is NOT last LeafIndex.
	; _SYDNEY_ASSERT(pIndex_ != getLeafIndex(pHeader_->m_usTotal - 1));
	
	DirectArea::Size size = (pIndex_ - 1)->m_usOffset - pIndex_->m_usOffset;
	// The range of 'size' is expected to be normal.
	moveAreas(pHeader_, pIndex_ - 1, -1 * size);
	return size;
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::resizeArea -- Resize an area
//
//	NOTES
//	The areas next to the area pointed by the pIndex_ are moved.
//
//	ARGUMENTS
//	const LeafHeader*						pHeader_
//		[IN]		LeafHeader
//	const LeafIndex*						pIndex_
//		[IN]		LeafIndex which point to the resized area
//	AreaSize								uiSize_
//		[IN]		The new size of the area
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaPage::resizeArea(const LeafHeader*	pHeader_,
						   const LeafIndex*		pIndex_,
						   int					iDiffSize_)
{
	// pIndex_ is NOT last LeafIndex.
	; _SYDNEY_ASSERT(pIndex_ != getLeafIndex(pHeader_->m_usTotal - 1));
	// The range of iDiffSize_ should have been checked.

	moveAreas(pHeader_, pIndex_ - 1, iDiffSize_);
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::moveAreas -- Move areas
//
//	NOTES
//	The areas next to the area pointed by the pIndex_ are moved.
//
//	ARGUMENTS
//	const LeafHeader*						pHeader_
//		[IN]		LeafHeader
//	const LeafIndex*						pIndex_
//		[IN]		LeafIndex
//	DirectArea::Size						usSize_
//		[IN]		The new size of the area pointed by the LeafIndex
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaPage::moveAreas(const LeafHeader*	pHeader_,
						  const LeafIndex*	pIndex_,
						  int				iSize_)
{
	// Check the head of areas after moving.
	; _SYDNEY_ASSERT(
		pIndex_->m_usOffset + iSize_ >= sizeof(LeafHeader));
	// Check the tail of areas after moving.
	// getPageDataSize() remove the size of LeafHeader and one LeafIndex
	// from the PhysicalPage's size.
	; _SYDNEY_ASSERT(
		pHeader_->m_usSpaceOffset + iSize_
		<= static_cast<int>(
			m_pFile->getPageDataSize()
			- (pHeader_->m_usTotal - 1) * sizeof(LeafIndex)));

	char* src = m_PhysicalPageTop + pIndex_->m_usOffset;
	char* dst = src + iSize_;
	ModSize size = pHeader_->m_usSpaceOffset - pIndex_->m_usOffset;
	Os::Memory::move(dst, src, size);
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::insertLeafIndex -- Insert a LeafIndex
//
//	NOTES
//	The LeafIndexes next to the pIndex_ are moved forward.
//
//	ARGUMENTS
//	const LeafHeader*						pHeader_
//		[IN]		LeafHeader
//	LeafIndex*								pIndex_
//		[IN]		LeafIndex which become the inserted LeafIndex
//	LeafIndex*								pLastIndex_
//		[IN]		Last LeafIndex
//	DirectArea::IndexKey					usKey_
//		[IN]		AreaID of the area pointed by the inserted LeafIndex
//	AreaSize								uiSize_
//		[IN]		The size of the area pointed by the inserted LeafIndex
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaPage::insertLeafIndex(const LeafHeader*		pHeader_,
								LeafIndex*				pIndex_,
								LeafIndex*				pLastIndex_,
								DirectArea::IndexKey	usKey_,
								AreaSize				uiSize_)
{
	moveLeafIndexes(pIndex_, pLastIndex_, true);
	// Set the new index
	// Not need to set the offset of new index,
	// because it is equal to the offset of former index.
	pIndex_->m_usIndexKey = usKey_;
	updateOffsets(pIndex_ - 1, pLastIndex_ - 1, uiSize_);
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::expungeLeafIndex -- Expunge a LeafIndex
//
//	NOTES
//	pIndex_ is expunged and the LeafIndexes next to pIndex_ are moved backward.
//
//	ARGUMENTS
//	const LeafHeader*						pHeader_
//		[IN]		LeafHeader
//	LeafIndex*								pIndex_
//		[IN]		Expunged LeafIndex
//	LeafIndex*								pLastIndex_
//		[IN]		Last LeafIndex
//	DirectArea::Size						usSize_
//		[IN]		The size of the area pointed by the expunged LeafIndex
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaPage::expungeLeafIndex(const LeafHeader*	pHeader_,
								 LeafIndex*			pIndex_,
								 LeafIndex*			pLastIndex_,
								 DirectArea::Size	usSize_)
{
	moveLeafIndexes(pIndex_ - 1, pLastIndex_, false);
	updateOffsets(pIndex_, pLastIndex_ + 1, -1 * usSize_);
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::updateOffsets --
//		Update the offset of LeafIndexes
//
//	NOTES
//	The offset of the LeafIndexes from begin_ to end_ are changed.
//
//	ARGUMENTS
//	LeafIndex*									begin_
//		[IN]		The begin position of the LeafIndexes
//	const LeafIndex*							end_
//		[IN]		The end position of the LeafIndexes
//	int											iSize_
//		[IN]		The size which is added or reduced to existing offset
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaPage::updateOffsets(LeafIndex*		begin_,
							  const LeafIndex*	end_,
							  int				iSize_)
{
	; _SYDNEY_ASSERT(iSize_ != 0);

	for (; end_ <= begin_; --begin_)
	{
		begin_->m_usOffset += iSize_;
	}
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::moveLeafIndexes -- Move the LeafIndexes
//
//	NOTES
//	The LeafIndxes form begin_ to end_ are moved for the size of one LeafIndex.
//	When bPlus_ is equal to true, the LeafIndexes are moved backward,
//	so the gap is created at the begin_.
//	When false, the LeafIndexes are moved forward,
//	so the LeafIndex which is previous one pointed by begin_ is overwritten.
//
//	ARGUMENTS
//	const LeafIndex*							begin_
//		[IN]		The begin position of the LeafIndex
//	LeafIndex*									end_
//		[IN]		The end position of the LeafIndex
//	bool										bPlus_
//					The case of creating gap
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaPage::moveLeafIndexes(const LeafIndex*	begin_,
								LeafIndex*			end_,
								bool				bPlus_)
{
	const LeafIndex* src = end_;
	LeafIndex* dst = end_;
	if (bPlus_ == true)
	{
		--dst;
	}
	else
	{
		++dst;
	}
	ModSize size = static_cast<ModSize>(begin_ - end_ + 1) * sizeof(LeafIndex);
	Os::Memory::move(dst, src, size);
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::initializeArea -- Initialize a area
//
//	NOTES
//
//	ARGUMENTS
//	const LeafIndex*							pIndex_
//		[IN]		Pointer to LeafIndex
//	AreaSize									begin_
//		[IN]		The begin position in the initialized area
//	AreaSize									size_
//		[IN]		The range of the initialized area
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectAreaPage::initializeArea(const LeafIndex*	pIndex_,
							   AreaSize			begin_,
							   AreaSize			size_)
{
	if (size_ == 0)
	{
		// Not reset the space.
		return;
	}
	char* p = m_PhysicalPageTop + pIndex_->m_usOffset + begin_;
	Os::Memory::reset(p, size_);
}

//	FUNCTION private
//	PhysicalFile::DirectAreaPage::getAreaSize -- Get the size of the area
//
//	NOTES
//
//	ARGUMENTS
//	const LeafHeader*						pHeader_
//		[IN]		LeafHeader
//	const LeafIndex*						pIndex_
//		[IN]		LeafIndex
//	const LeafIndex*						pLastIndex_
//		[IN]		Last LeafIndex
//
//	RETURN
//	AreaSize
//				The size of the area pointed by pIndex_
//
//	EXCEPTIONS
//
AreaSize
DirectAreaPage::getAreaSize(const LeafHeader*	pHeader_,
							const LeafIndex*	pIndex_,
							const LeafIndex*	pLastIndex_) const
{
	DirectArea::Size begin = pIndex_->m_usOffset;
	DirectArea::Size end;
	if (pIndex_ == pLastIndex_)
	{
		end = pHeader_->m_usSpaceOffset;
	}
	else
	{
		end = (pIndex_ - 1)->m_usOffset;
	}
	return static_cast<AreaSize>(end - begin);
}

//
//	Copyright (c) 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
