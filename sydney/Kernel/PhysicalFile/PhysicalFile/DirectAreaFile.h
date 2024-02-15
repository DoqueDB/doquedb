// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectAreaFile.h --
//		Class definition and functions declare of DirectAreaFile
// 
// Copyright (c) 2005, 2006, 2007, 2009, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_DIRECTAREAFILE_H
#define __SYDNEY_PHYSICALFILE_DIRECTAREAFILE_H

#include "PhysicalFile/File.h"

_SYDNEY_BEGIN
_SYDNEY_PHYSICALFILE_BEGIN

class DirectAreaTree;

//	CLASS
//	PhysicalFile::DirectAreaFile --
//		The file of managing area directly.
//
//	NOTES
//	
class DirectAreaFile : public File {
	
public:

	// Constructor
	DirectAreaFile(const File::StorageStrategy&		cStorageStrategy_,
				   const File::BufferingStrategy&	cBufferingStrategy_,
				   const Lock::FileName*			pFileName_,
				   bool								bBatch_);

	// Destructor
	~DirectAreaFile();

	// Allocate DirectArea
	DirectArea allocateArea(const Trans::Transaction&		cTransaction_,
							AreaSize						uiSize_);

	// Free DirectArea
	void freeArea(const Trans::Transaction&			cTransaction_,
				  const DirectArea::ID&				cID_,
				  Admin::Verification::Progress*	pProgress_ = 0);

	// Attach DirectArea
	DirectArea attachArea(const Trans::Transaction&		cTransaction_,
						  const DirectArea::ID&			cID_,
						  Buffer::Page::FixMode::Value	eFixMode_);

	// Detach All attached DirectAreas
	void detachAllAreas();

	// Recover All attached DirectAreas
	void recoverAllAreas();

	// Get the max size of storable DirectArea.
	AreaSize getMaxStorableAreaSize() const;

	// detach page
	virtual void
	detachPage(
		Page*&					Page_,
		Page::UnfixMode::Value	UnfixMode_ = Page::UnfixMode::Omit,
		const bool				SavePage_ = false);
	
	//
	//	For checking consistency
	//
	
	// Attach DirectArea with verify mode
	DirectArea verifyArea(const Trans::Transaction&			cTransaction_,
						  const DirectArea::ID&				cID_,
						  Buffer::Page::FixMode::Value		eFixMode_,
						  Admin::Verification::Progress&	cProgress_);
	
	//
	// For preventing misusage
	// These functions overwrites File's and returns 'Not supported'.
	//
	
	// Detach all pages.
	void detachPageAll();

	// Recover all pages.
	void recoverPageAll();


	//
	//	For debug
	//

#ifdef DEBUG
	// Get parent index value
	// which indicates given page's free space size for allocate.
	AreaSize getParentIndexValue(const Trans::Transaction&	cTransaction_,
								 PageID						uiPageID_);

	// Set parent index value
	void setParentIndexValue(const Trans::Transaction&	cTransaction_,
							 PageID						uiPageID_,
							 AreaSize					uiSize_);

	// Set the number of Area in the Page
	void setLeafHeaderNumber(const Trans::Transaction&	cTransaction_,
							 PageID						uiPageID_,
							 AreaNum					uiNum_);
	// Set the free space offset in the Page
	void setLeafHeaderOffset(const Trans::Transaction&	cTransaction_,
							 PageID						uiPageID_,
							 AreaOffset					uiOffset_);

	// Set the AreaID in the Page
	void setLeafIndexAreaID(const Trans::Transaction&	cTransaction_,
							PageID						uiPageID_,
							AreaID						uiAreaID_,
							AreaNum						uiIndex_);
	// Set the area offset in the Page
	void setLeafIndexOffset(const Trans::Transaction&	cTransaction_,
							PageID						uiPageID_,
							AreaOffset					uiOffset_,
							AreaNum						uiIndex_);
#endif

	//
	// The following functions are 'public',
	// but these are not used by other than this module.
	//
	
	// Get the size of all DirectAreaFile's pages
	FileSize getUsedSize(const Trans::Transaction&	cTransaction_);

	// Get the size of max storable area.
	// File's pure virtual function
	PageSize getPageDataSize(const AreaNum	uiDummy_ = 1) const;

#ifdef OBSOLETE
	// Get the size of all DirectAreas
	FileSize getAllocatedSize(const Trans::Transaction&	cTransaction_);
#endif


	//
	// These functions exist in ONLY DirectAreaPage.
	//
	
	// Get the size of PhysicalFile's page.
	PageSize getPageSize() const { return m_VersionPageDataSize; };

	// Get the size of max storable area.
	static PageSize	getPageDataSize(const Os::Memory::Size	uiPageSize_,
									const AreaNum			uiAreaNum_);

	// Get last PageID.
	PageID getLastPageID(const Trans::Transaction&		cTransaction_,
						 Admin::Verification::Progress*	pProgress_ = 0);
	// Set last PageID.
	void setLastPageID(const Trans::Transaction&	cTransaction_,
					   PageID						uiLastPageID_);
	
	// Attach page with Progress
	DirectAreaPage* attachPage(
		const Trans::Transaction&		cTransaction_,
		PageID							uiPageID_,
		Buffer::Page::FixMode::Value	eFixMode_,
		Admin::Verification::Progress*	pProgress_ = 0);

	// Update tree.
	void updateTree(const Trans::Transaction&		cTransaction_,
					PageID							uiPageID_,
					AreaSize						uiSize_,
					Admin::Verification::Progress*	pProgress_ = 0);

	// Fix VersionPage with Progress
	PagePointer fixVersionPage(
		const Trans::Transaction&			cTransaction_,
		Version::Page::ID					uiPageID_,
		Buffer::Page::FixMode::Value		eFixMode_,
		Admin::Verification::Progress*		pProgress_ = 0);

	// Round size up.
	static AreaSize roundUp(AreaSize	uiSize_);

	// Convert Physical::PageID to Version::Page::ID.
	Version::Page::ID convertToVersionPageID(const PageID	PageID_) const
		{ return PageID_; };
	
	//
	//	For checking consistency
	//
	
	// Notify unusing DirectArea
	void notifyUnuseArea(PageID							uiPageID_,
						 AreaID							uiAreaID_,
						 AreaSize						uiAreaSize_,
						 Admin::Verification::Progress*	pProgress_);

	// Notify free space size of the page
	void notifyFreeSpaceSize(PageID							uiPageID_,
							 AreaSize						uiAreaSize_,
							 Admin::Verification::Progress*	pProgress_);

private:

	//	CONST private
	//	PhysicalFile::DirectAreaFile::AlignmentBytes --
	//		Boundaries of alignment [byte]
	//
	//	NOTES

	static const DirectArea::Size	AlignmentBytes;

	//	CONST private
	//	PhysicalFile::DirectAreaFile::MaxPageSize --
	//		Max page size which is supported by DirectAreaFile [byte]
	//
	//	NOTES

	static const Os::Memory::Size	MaxPageSize;


	// Get attached page.
	// File's virtual function
	Page* getAttachedPage(
		const Trans::Transaction&					cTransaction_,
		const PageID								uiPageID_,
		const Buffer::Page::FixMode::Value			eFixMode_,
		const Buffer::ReplacementPriority::Value*	pPriority_,
		Admin::Verification::Progress*				pProgress_);

	// Allocate Page instance.
	// File's pure virtual function
	Page* allocatePageInstance(
		const Trans::Transaction&			cTransaction_,
		PageID								uiPageID_,
		Buffer::Page::FixMode::Value		eFixMode_,
		Admin::Verification::Progress*		pProgress_ = 0,
		Buffer::ReplacementPriority::Value	eReplacementPriority_
											= Buffer::ReplacementPriority::Low);
	
	// Initialize during DirectAreaFile creating.
	void initialize(const Trans::Transaction&	cTransaction_,
					void*						pFileHeader_);

	// Truncate DirectAreaFile's unused pages.
	void truncate(const Trans::Transaction&		cTransaction_,
				  bool&							bModified_);

	// Verify the size of VersionPage.
	static Os::Memory::Size verifyVersionPageSize(Os::Memory::Size uiPageSize_);

	// Get header page with Progress
	VersionPage* getHeaderPage(const Trans::Transaction&		cTransaction_,
							   Buffer::Page::FixMode::Value		eFixMode_,
							   Admin::Verification::Progress*	pProgress_);

	// Get version page with Progress
	PagePointer getVersionPage(const Trans::Transaction&		cTransaction_,
							   Version::Page::ID				uiPageID_,
							   Buffer::Page::FixMode::Value		eFixMode_,
							   Admin::Verification::Progress*	pProgress_ = 0);
	
	//
	//	For cheking consistency
	//

	// Check all Nodes and not attached Leafs
	void verifyAllPages(const Trans::Transaction&		cTransaction_,
						Admin::Verification::Progress&	cProgress_);

	// Verify attached pages, but nothing to do in this file type
	void verifyAttachedPage(const Trans::Transaction&		cDummy1_,
							Admin::Verification::Progress&	cDummy2_) const;

	// Verify file header, but nothing to do in this file type
	void verifyFileHeader(const Trans::Transaction&			cDummy1_,
						  Admin::Verification::Progress&	cDummy2_);

	// Check DirectAreaFile
	// File's pure virtual function
	void checkPhysicalFile(const Trans::Transaction&		cDummy1_,
						   Admin::Verification::Progress&	cDummy2_);

	// Preprocess of verification
	void initializeVerification(const Trans::Transaction&		cTransaction_,
								const unsigned int				uiTreatment_,
								Admin::Verification::Progress&	cDummy_);

	// Postprocess of verification
	void terminateVerification();


	// Last PageID
	PageID	m_uiLastPageID;
	
	// status of tree
	DirectAreaTree*	m_pTree;

}; // end of class PhysicalFile::DirectAreaFile

_SYDNEY_PHYSICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_DIRECTAREAFILE_H

//
//	Copyright (c) 2005, 2006, 2007, 2009, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
