// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectArea.h -- The function declear and class definition of DirectArea
// 
// Copyright (c) 2005, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_DIRECTAREA_H
#define __SYDNEY_PHYSICALFILE_DIRECTAREA_H

#include "PhysicalFile/Module.h"
#include "PhysicalFile/Types.h"

#include "Os/Memory.h"

_SYDNEY_BEGIN
_SYDNEY_PHYSICALFILE_BEGIN

//	CLASS
//	PhysicalFile::DirectArea -- Area of PhysicalFile::DirectAreaFile
//
//	NOTES
//	
//

class Page;
class DirectAreaPage;

class DirectArea {

public:

	//	STRUCT public
	//	PhysicalFile::DirectArea::ID -- The ID of DirectArea
	//
	//	NOTES

	struct ID {
		// Constructor
		ID() : m_uiPageID(ConstValue::UndefinedPageID),
			   m_uiAreaID(ConstValue::UndefinedAreaID) {}
		
		// The ID of VersionPage
		PageID	m_uiPageID;
		// The ID in a page
		AreaID	m_uiAreaID;
	};

	// Constructor
	SYD_PHYSICALFILE_FUNCTION
	DirectArea(DirectAreaPage*	pPage_ = static_cast<DirectAreaPage*>(0),
			   AreaID			uiAreaID_ = ConstValue::UndefinedAreaID);

	// Destructor
	SYD_PHYSICALFILE_FUNCTION
	~DirectArea();

	// Copy constructor
	SYD_PHYSICALFILE_FUNCTION
	DirectArea(const DirectArea& cArea_);

	// Operator of substitution
	SYD_PHYSICALFILE_FUNCTION
	DirectArea& operator = (const DirectArea& cArea_);

	// Get ID of DirectArea
	SYD_PHYSICALFILE_FUNCTION
	ID getID() const;

	// Check the writability of DirectArea
	SYD_PHYSICALFILE_FUNCTION
	bool isWritable() const;

	// Cast Operator to 'void*'
	SYD_PHYSICALFILE_FUNCTION
	operator void*();

	// Cast Operator to 'const void*'
	SYD_PHYSICALFILE_FUNCTION
	operator const void*() const;

	// Notify that DirectArea has been updated
	SYD_PHYSICALFILE_FUNCTION
	void dirty();

	// Get the size of DirectArea
	SYD_PHYSICALFILE_FUNCTION
	AreaSize getSize() const;

	// Detache DirectArea
	SYD_PHYSICALFILE_FUNCTION
	void detach();

	// Expunge DirectArea
	SYD_PHYSICALFILE_FUNCTION
	void expunge(const Trans::Transaction&		cTransaction_,
				 Admin::Verification::Progress*	pProgress_ = 0);

	// Change the size of DirectArea
	SYD_PHYSICALFILE_FUNCTION
	bool changeSize(const Trans::Transaction&		cTransaction_,
					AreaSize						uiSize_,
					Admin::Verification::Progress*	pProgress_ = 0);

	// Get the max expandable size of DirectArea
	SYD_PHYSICALFILE_FUNCTION
	AreaSize getMaxExpandableSize() const;

	// Reset area with 0;
	SYD_PHYSICALFILE_FUNCTION
	void reset() { Os::Memory::reset(operator void*(), getSize()); };


	//
	// The following functions are 'public',
	// but these are not used by other than this module.
	//

	//	TYPEDEF public
	//	PhysicalFile::DirectArea::IndexKey --
	//		The ID in a page which is actually written in memory.
	//
	//	NOTES
	//	AreaID is cast as this type when data is actually written in memory.

	typedef unsigned short	IndexKey;

	//	TYPEDEF private
	//	PhysicalFile::DirectArea::Offset -- The offset of DirectArea
	//
	//	NOTES
	//	It starts from top of page which is 0.

	typedef unsigned short	Offset;

	//	TYPEDEF private
	//	PhysicalFile::DirectArea::Size -- The size of DirectArea
	//
	//	NOTES

	typedef unsigned short	Size;

	//	CONST public
	//	PhysicalFile::DirectAreaFile::InvalidSize --
	//		The invalid size of an area
	//
	//	NOTES

	static const Size		InvalidSize;

	//	TYPEDEF private
	//	PhysicalFile::DirectArea::Num -- The number of DirectArea
	//
	//	NOTES
	//	It is used for the number of Node/Leaf too.
	//	ex: DirectAreaFile::getMaxChildren()

	typedef unsigned short	Num;

	
	//
	// For debug
	//

	// Is the area valid?
	SYD_PHYSICALFILE_FUNCTION
	bool isValid() const { return (m_pPage != 0); };

#ifdef DEBUG
	// Get Page
	SYD_PHYSICALFILE_FUNCTION
	Page* getPage() const;
#endif
	
private:
	// Move owner
	void moveOwner(const DirectArea& cArea_) const;

	// Pointer to DirectAreaPage
	DirectAreaPage* m_pPage;

	// ID in DirectAreaFile's page
	AreaID	m_uiAreaID;

	// Owner of myself
	mutable bool m_bOwner;
		
};	// end of class PhysicalFile::DirectArea

_SYDNEY_PHYSICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_DIRECTAREA_H

//
//	Copyright (c) 2005, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
