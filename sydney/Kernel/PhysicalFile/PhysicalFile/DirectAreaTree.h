// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectAreaTree.h -- Class definition and functions declare of Tree
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_DIRECTAREATREE_H
#define __SYDNEY_PHYSICALFILE_DIRECTAREATREE_H

#include "PhysicalFile/Module.h"
#include "PhysicalFile/Types.h"
#include "PhysicalFile/DirectArea.h"
#include "PhysicalFile/DirectAreaFile.h"

#include "Common/VectorMap.h"

#include "ModMap.h"

_SYDNEY_BEGIN

namespace Admin {
	namespace Verification {
		class Progress;
	}
}
namespace Version {
	class File;
}
namespace Trans {
	class Transaction;
}

_SYDNEY_PHYSICALFILE_BEGIN

class DirectAreaPage;

//	CLASS
//	PhysicalFile::DirectAreaTree --	Managing tree-structure for DirectAreaFile
//
//	NOTES
//	DirectAreaFile can manage Area directly. But the implementation uses Page.
//	And The Page is managed with Tree-structure.
//	This class provides functions which make the File manage the structure.
//
class DirectAreaTree {

public:

	// Constructor
	DirectAreaTree(DirectAreaFile* pFile_);

	// Destructor
	~DirectAreaTree();

	// Initialize
	PageID initialize(const Trans::Transaction&	cTransaction_);

	// Attach Leaf.
	DirectAreaPage* getLeaf(const Trans::Transaction&	cTransaction_,
							AreaSize					uiSize_,
							bool						bBatch_);

	// Update tree.
	void update(const Trans::Transaction&		cTransaction_,
				PageID							uiPageID_,
				AreaSize						uiSize_,
				Admin::Verification::Progress*	pProgress_ = 0,
				DirectArea::Size*				usSecondSizes_ = 0);

	// Truncate tree.
	void truncate(const Trans::Transaction&	cTransaction_,
				  Version::File*			pVersionFile_,
				  bool&						bModified_);

	//
	//	For checking consistency
	//
	
	//	TYPEDEF public
	//	PhysicalFile::DirectAreaTree::AreaMap --
	//		The Map of Areas which are stored in a same page
	//
	//	NOTES
	//	'bool' means whether the area is notified or not.
	//
	typedef Common::VectorMap< AreaID, bool, ModLess<AreaID> > AreaIDMap;

	// Verify tree/subtree.
	void verify(const Trans::Transaction&		cTransaction_,
				Admin::Verification::Progress*	pProgress_);

	// Notify using DirectAreaPage
	void notifyUsePage(const Trans::Transaction&		cTransaction_,
					   DirectAreaPage*					pPage_,
					   Admin::Verification::Progress*	pProgress_);

	// Notify using DirectArea
	void notifyUseArea(const Trans::Transaction&		cTransaction_,
					   const DirectArea::ID&			cID_,
					   Admin::Verification::Progress*	pProgress_);

	// Notify unusing DirectArea
	void notifyUnuseArea(PageID							uiPageID_,
						 AreaID							uiAreaID_,
						 AreaSize						uiAreaSize_,
						 Admin::Verification::Progress*	pProgress_);

	// Notify free space size of the page
	void notifyFreeSpaceSize(PageID							uiPageID_,
							 AreaSize						uiAreaSize_,
							 Admin::Verification::Progress*	pProgress_);

	// Clear m_mapVerifyMap
	void clearVerifyMap();

	//
	// For debug
	//

#ifdef DEBUG
	// Get parent index value.
	AreaSize
	getParentIndexValue(const Trans::Transaction&	cTransaction_,
						PageID						uiPageID_);
	// Set parent index value.
	void setParentIndexValue(const Trans::Transaction&	cTransaction_,
							 PageID						uiPageID_,
							 AreaSize					uiSize_);
#endif

private:

	//	TYPEDEF private
	//	PhysicalFile::Step --
	//		The number of tree's step and the position of Root/Node/Leaf in tree
	//
	//	NOTES
	//	0 means the position of root is at the top of tree,
	//	1 means the position is beneath the top, ...
	//	The position of root is always at the top of tree.
	//	But when the number of managed Leaf under tree is small,
	//	the position moves LOGICALLY downward
	//	for managing Leafs or Nodes directly.
	//
	typedef unsigned short	Step;

	//	CONST private
	//	PhysicalFile::DirectAreaTree::LeafStep --
	//		The step of Leaf
	//
	//	NOTES
	//
	//static const Step	LeafStep;
	enum { LeafStep = 3 };
	
	//	CONST private
	//	PhysicalFile::DirectAreaTree::InvalidStep -- The invalid step
	//
	//	NOTES
	//
	static const Step	InvalidStep;

	//	CONST private
	//	PhysicalFile::PageID::MinLeafID -- The minimum leaf's PageID
	//
	//	NOTES
	//
	static const PageID	MinLeafID;


	// Attach Leaf in Tree
	DirectAreaPage* attachLeafInTree(
		const Trans::Transaction&		cTransaction_,
		DirectArea::Size				usSize_);

	// Search for free page.
	PageID search(const Trans::Transaction&	cTransaction_,
				  DirectArea::Size			usSize_,
				  DirectArea::Size*			usSecondSizes_);

	// Append a page to tree
	void append(const Trans::Transaction&	cTransaction_,
				PageID						uiPrevLastPageID_,
				PageID						uiLastPageID_,
				AreaSize					uiFreeSize_);

	// Decontrol pages under tree
	void decontrol(const Trans::Transaction&	cTransaction_,
				   PageID						begin_,
				   PageID						end_);

	// Reset Node/Root.
	void resetNode(const Trans::Transaction&	cTransaction_,
				   PageID						uiPageID_);

	//
	// Absolute Position
	//

	// Get page's step
	Step getStep(PageID uiPageID_);
	// Get logical Root's step .
	Step getRootStep(
		const Trans::Transaction&	cTransaction_,
		PageID 						uiLastPageID_ = ConstValue::UndefinedPageID,
		Admin::Verification::Progress*	pProgress_ = 0);

	//
	// Parent-Child relative position
	//

	// Get parent's PageID
	PageID getParentPageID(PageID	uiChildID_,
						   PageNum&	uiIndex_);
	// Get parent's substantial PageID
	PageID getSubstantialParentPageID(
		const Trans::Transaction&		cTransaction_,
		PageID							uiChildID_,
		PageNum&						uiIndex_,
		Admin::Verification::Progress*	pProgress_ = 0);

	// Get child's PageID.
	PageID getChildPageID(PageID	uiParentID_,
						  PageNum	uiIndex_);
	// Get child's substantial PageID
	PageID getSubstantialChildPageID(
		const Trans::Transaction&	cTransaction_,
		PageID						uiParentID_,
		PageNum						uiIndex_);

	//
	//	Sibling relative position
	//

	// Get PageID of not empty Leaf
	PageID getNonEmptyLeafID(const Trans::Transaction&	cTransaction_,
							 PageID						uiPageID_);
	// Get previous Leaf's PageID
	PageID getPreviousLeafID(PageID uiPageID_);
	// Get previous Leaf's PageID with resetting Nodes
	PageID getPreviousLeafIDwithResettingNodes(
		const Trans::Transaction&	cTransaction_,
		PageID						uiPageID_);

   	// Get next Leaf's PageID with allocating Nodes
	PageID getNextLeafID(const Trans::Transaction&	cTransaction_,
						 PageID						uiPageID_);
	

	//
	// Simple Accessor
	//

	// Get the pointer of the top of the indexes
	static DirectArea::Size* getTopIndex(File::PagePointer pPage_);
	static const DirectArea::Size* getConstTopIndex(File::PagePointer pPage_);

	// Get the size of the index.
	DirectArea::Size getSize(const DirectArea::Size*	pTopIndex_,
							 PageNum					uiIndex_) const
		{ return *(pTopIndex_ + uiIndex_); };
	// Set the size of the index.
	void setSize(DirectArea::Size*	pTopIndex_,
				 PageNum			uiIndex_,
				 DirectArea::Size	usSize_)
		{ *(pTopIndex_ + uiIndex_) = usSize_; };
	
	//
	// etc
	//

	// Get storable page's index in the root.
	PageNum getStorableIndexInRoot(const Trans::Transaction&	cTransaction_,
								   File::PagePointer			pPage_,
								   DirectArea::Size				usSize_);
	// Get storable page's index in the node.
	PageNum getStorableIndex(const Trans::Transaction&	cTransaction_,
							 File::PagePointer			page_,
							 PageID						pageID_,
							 DirectArea::Size			usSize_,
							 DirectArea::Size*			usSecondSizes_);
	// Get last index
	PageNum getLastIndex(const Trans::Transaction&		cTransaction_,
						 PageID							uiPageID_,
						 Admin::Verification::Progress*	pProgress_ = 0);
	
	// Get the new maximum size.
	DirectArea::Size getUpdatedMaxSize(
		const DirectArea::Size*	pTopIndex_,
		PageNum					uiLastIndex_,
		PageNum					uiUpdatedIndex_,
		DirectArea::Size		usPrevSize_) const;
	// Get the maximum size except one.
	DirectArea::Size getMaxSizeExceptOne(
		const DirectArea::Size*	pIndex_,
		PageNum					uiLastIndex_,
		PageNum					uiExceptIndex_,
		DirectArea::Size&		usExceptSize_) const;
	// Get the maximum size simply.
	DirectArea::Size getMaxSize(const DirectArea::Size*	pIndex_,
								PageNum					uiLastIndex_) const;


	//
	// For checking consistency
	//

	//	STRUCT public
	//	PhysicalFile::DirectAreaTree::VerifyResult --
	//		The result of Leaf's verification
	//
	struct VerifyResult {
		// The free space size of the page
		AreaSize	m_uiAreaSize;
		// The Map of Areas which are stored in a same page
		AreaIDMap	m_cAreaIDMap;
	};

	//	TYPEDEF public
	//	PhysicalFile::DirectAreaTree::VerifyMap --
	//		The Map of result of Leaf's verification
	//
	//	NOTES
	//	PageID points to Leaf.
	//
	typedef ModMap< PageID, VerifyResult*, ModLess<PageID> > VerifyMap;

	// Get VerifyResult
	VerifyResult* getVerifyResult(PageID uiPageID_);
	// Modify VerifyResult
	void modifyVerifyResult(PageID							uiPageID_,
							AreaID							uiAreaID_,
							AreaSize						uiAreaSize_,
							Admin::Verification::Progress*	pProgress_);

	// Verify Node/Root
	DirectArea::Size verifyNode(const Trans::Transaction&		cTransaction_,
								PageID&							uiPageID_,
								Admin::Verification::Progress*	pProgress_);

	// Verify leaf
	DirectArea::Size verifyLeaf(const Trans::Transaction&		cTransaction_,
								PageID							uiPageID_,
								Admin::Verification::Progress*	pProgress_);
		
	// Set Node/Root's free area size
	void setSize(const Trans::Transaction&		cTransaction_,
				 PageID							uiPageID_,
				 PageNum						uiIndex_,
				 DirectArea::Size				usSize_,
				 Admin::Verification::Progress*	pProgress_);


	// Pointer to DirectAreafile
	DirectAreaFile*	m_pFile;

	// Number of children which one node can manage.
	PageNum	m_uiMaxChildren;
	// Number of all pages under each subtree including itself
	PageNum	m_uiSubtree[LeafStep];
		
	//
	//	For cheking consistency
	//

	// Map of DirectArea::ID
	VerifyMap	m_cVerifyMap;

}; // end of class PhysicalFile::DirectAreaTree

_SYDNEY_PHYSICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_DIRECTAREATREE_H

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
