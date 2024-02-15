// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectAreaPage.h --
//		物理エリア管理機能付き物理ファイルの
//		物理ページ関連のクラス定義、関数宣言
// 
// Copyright (c) 2005, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_DIRECTAREAPAGE_H
#define __SYDNEY_PHYSICALFILE_DIRECTAREAPAGE_H

#include "SyDynamicCast.h"

#include "PhysicalFile/Page.h"
#include "PhysicalFile/DirectAreaFile.h"
#include "PhysicalFile/DirectAreaTree.h"

_SYDNEY_BEGIN
_SYDNEY_PHYSICALFILE_BEGIN

//	CLASS
//	PhysicalFile::DirectAreaPage --
//		物理エリア管理機能付き物理ファイルの物理ページクラス
//
//	NOTES

class DirectAreaPage : public Page
{

public:
	// Constructor
	DirectAreaPage(const Trans::Transaction&			cTransaction_,
				   DirectAreaFile*						pFile_,
				   const PageID							uiPageID_,
				   const Buffer::Page::FixMode::Value	eFixMode_,
				   const Buffer::ReplacementPriority::Value	eReplacementPriority_ = Buffer::ReplacementPriority::Middle);
	// Constructor
	DirectAreaPage(const Trans::Transaction&			cTransaction_,
				   DirectAreaFile*						pFile_,
				   const PageID							uiPageID_,
				   const Buffer::Page::FixMode::Value	eFixMode_,
				   Admin::Verification::Progress&		cProgress_);

	// Destructor
	~DirectAreaPage();
	
	// Get DirectArea's offset.
	PageOffset getAreaOffset(AreaID uiAreaID_) const;

	// Get DirectArea's size.
	AreaSize getAreaSize(AreaID uiAreaID_) const;

	// Get the maximum size of storable area
	// File's pure virtual function
	PageSize getPageDataSize(const AreaNum	AreaNum_ = 1) const
		{ return m_pFile->getPageDataSize(AreaNum_); };

	// Get File
	// File's pure virtual function
	File* getFile() const { return m_pFile; };
	
	//
	// These functions exist in ONLY DirectAreaPage.
	//
	
	// Initialize the Leaf.
	void initialize();

	// Is this referenced from any DirectArea?
	bool isReferenced() const
		{ return (m_ReferenceCounter != 0); };

	// Detach myself, but actually not detached here.
	void detach()
		{
			Os::AutoCriticalSection latch(_latch);
			if(m_ReferenceCounter > 0)
			{
				--m_ReferenceCounter;
			}
		};
	
	// Allocate DirectArea
	DirectArea allocateArea(AreaSize uiSize_);
	
	// Attach DirectArea
	DirectArea attachArea(AreaID uiAreaID_);

	// Free area
	void freeArea(const Trans::Transaction&			cTransaction_,
				  AreaID							uiAreaID_,
				  Admin::Verification::Progress*	pProgress_ = 0);

	// Change DirectArea's size.
	bool changeAreaSize(const Trans::Transaction&		cTransaction_,
						AreaID							uiAreaID_,
						AreaSize						uiRequestSize_,
						Admin::Verification::Progress*	pProgress = 0);

	// Change FixMode
	void changeFixMode(
		const Trans::Transaction&					cTransaction_,
		File*										pFile_,
		PageID										uiPageID_,
		Buffer::Page::FixMode::Value				eFixMode_,
		const Buffer::ReplacementPriority::Value*	pPriority_,
		Admin::Verification::Progress*				pProgress_);

	// Get free space size.
	AreaSize getFreeSize(bool bAllocate_ = true) const;

	// Verify the free space size for allocating an area
	static AreaSize verifyFreeSize(AreaSize uiFreeSize_);
	
	// Get previous page.
	DirectAreaPage* getPrev() const
		{ return _SYDNEY_DYNAMIC_CAST(DirectAreaPage*, m_Prev); };

	// Get unused size of leaf.
	static AreaSize getUnusedSize(DirectArea::Num usNum_ = 1)
		{ return sizeof(LeafHeader) + sizeof(LeafIndex) * usNum_; };
	
	//
	// For checking consistency
	//

	// Verify the leaf
	AreaSize verify(const Trans::Transaction&		cTransaction_,
					Admin::Verification::Progress&	cProgress_,
					DirectAreaTree::AreaIDMap&		cAreaIDMap_);

	//
	// For debug
	//

#ifdef DEBUG
	// Set the number of Area in the Page
	void setLeafHeaderNumber(AreaNum	uiNum_);
	// Set the free space offset in the Page
	void setLeafHeaderOffset(AreaOffset	uiOffset_);

	// Set the AreaID in the Page
	void setLeafIndexAreaID(AreaID	uiAreaID_,
							AreaNum	uiIndex_);
	// Set the area offset in the Page
	void setLeafIndexOffset(AreaOffset	uiOffset_,
							AreaNum		uiIndex_);
#endif

private:

	
	// Reset
	void reset(const Trans::Transaction&			cTransaction_,
			   PageID								uiPageID_,
			   Buffer::Page::FixMode::Value			eFixMode_,
			   Buffer::ReplacementPriority::Value	eReplacementPriority_ =
				   						Buffer::ReplacementPriority::Middle);
	
#ifdef DEBUG
	// Page's pure virtual function
	PageSize getUserAreaSize() const { return 0; };
#endif

	//	STRUCT
	//	PhysicalFile::DirectAreaPage::LeafHeader -- The header of Leaf
	//
	//	NOTES
	//	Offset starts from sizeof(LeafHeader). 0 means top of page.
	//	See initialize().
	//
	struct LeafHeader
	{
		// The total of DirectArea in the page.
		DirectArea::Num		m_usTotal;
		// The offset of free space in the page.
		DirectArea::Offset	m_usSpaceOffset;
	};

	// Get the LeafHeader.
	LeafHeader* getLeafHeader() const
		{ return syd_reinterpret_cast<LeafHeader*>(m_PhysicalPageTop); };
	const LeafHeader* getConstLeafHeader() const
		{ return syd_reinterpret_cast<const LeafHeader*>(m_PhysicalPageTop); };

	//	STRUCT
	//	PhysicalFile::DirectAreaPage::LeafHeader -- The index of Leaf
	//
	//	NOTES
	//	The top of LeafIndex is the end of page,
	//	so LeafIndex is in decending order.
	//	IndexKey starts from 0.
	//	See LeafHeader about Offset.
	//
	struct LeafIndex
	{
		// The key of DirectArea's index.
		DirectArea::IndexKey	m_usIndexKey;
		// The offset of DirectArea in the page.
		DirectArea::Offset		m_usOffset;
	};

	// Get the LeafIndex.
	LeafIndex* getLeafIndex(unsigned short index_) const;
	const LeafIndex* getConstLeafIndex(unsigned short index_) const;
	
	// Binary search for minimum unused IndexKey.
	LeafIndex* binsearchForUnusedIndexKey(LeafIndex*			begin_,
										  LeafIndex*			end_,
										  DirectArea::IndexKey&	key_) const;
	
	// Binary search
	LeafIndex* binsearch(LeafIndex*	begin_,
						 LeafIndex*	end_,
						 AreaID		key_) const;

	// Insert an area
	void insertArea(const LeafHeader*	pHeader_,
					const LeafIndex*	pIndex_,
					AreaSize			uiSize_);

	// Expunge an area
	DirectArea::Size expungeArea(const LeafHeader*	pHeader_,
								 const LeafIndex*	pIndex_);

	// Resize an area
	void resizeArea(const LeafHeader*	pHeader_,
					const LeafIndex*	pIndex_,
					int					iDiffSize_);

	// Move areas
	void moveAreas(const LeafHeader*	pHeader_,
				   const LeafIndex*		pIndex_,
				   int					iSize_);

	// Insert an LeafIndex
	void insertLeafIndex(const LeafHeader*		pHeader_,
						 LeafIndex*				pIndex_,
						 LeafIndex*				pLastIndex_,
						 DirectArea::IndexKey	uskey_,
						 AreaSize				uiSize_);

	// Expunge an LeafIndex
	void expungeLeafIndex(const LeafHeader*	pHeader_,
						  LeafIndex*		pIndex_,
						  LeafIndex*		pLastIndex_,
						  DirectArea::Size	usSize_);

	// Update the offset of LeafIndexes
	void updateOffsets(LeafIndex*		begin_,
					   const LeafIndex*	end_,
					   int				iSize_);

	// Move LeafIndexes
	void moveLeafIndexes(const LeafIndex*	begin_,
						 LeafIndex*			end_,
						 bool				bPlus_);

	// Reset an area
	void initializeArea(const LeafIndex*	pIndex_,
						AreaSize			begin_,
						AreaSize			size_);

	// Get the size of the area
	AreaSize getAreaSize(const LeafHeader*	pHeader_,
						 const LeafIndex*	pIndex_,
						 const LeafIndex*	pLastIndex_) const;


	// Pointer to File
	DirectAreaFile*		m_pFile;

	Os::CriticalSection	_latch;
};	// end of class PhysicalFile::DirectAreaPage

_SYDNEY_PHYSICALFILE_END
_SYDNEY_END

#endif // __SYDNEY_PHYSICALFILE_DIRECTAREAPAGE_H

//
//	Copyright (c) 2005, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
