// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectArea.cpp -- 物理エリア関連の関数定義
// 
// Copyright (c) 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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
#include "PhysicalFile/DirectAreaPage.h"

#include "Common/Assert.h"

#include "Exception/Unexpected.h"
#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectArea's public constant number
//
///////////////////////////////////////////////////////////////////////////////

//	CONST public PhysicalFile::DirectArea::InvalidSize --
//		The invalid size of an area
//
//	NOTES

//static
const DirectArea::Size
DirectArea::InvalidSize = 0xFFFF;


///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectArea クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::DirectArea::DirectArea -- Constructor
//
//	NOTES
//
//	ARGUMENTS
//	Page*									pPage_
//		[IN]		Pointer to Page
//	AreaID									uiAreaID_
//		[IN]		AreaID
//
//	RETURN
//
//	EXCEPTIONS
//
DirectArea::DirectArea(DirectAreaPage*	pPage_,
					   AreaID			uiAreaID_)
	: m_pPage(pPage_), m_uiAreaID(uiAreaID_), m_bOwner(true)
{
}

//	FUNCTION public
//	PhysicalFile::DirectArea::~DirectArea -- Destructor
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DirectArea::~DirectArea()
{
	detach();
}

//	FUNCTION public
//	PhysicalFile::DirectArea::DirectArea -- Copy constructor
//
//	NOTES
//
//	ARGUMENTS
//	const DirectArea&						cArea_
//		[IN]		Reference to DirectArea
//
//	RETURN
//
//	EXCEPTIONS
//
DirectArea::DirectArea(const DirectArea& cArea_)
	: m_pPage(cArea_.m_pPage), m_uiAreaID(cArea_.m_uiAreaID)
{
	moveOwner(cArea_);
}

//	FUNCTION public
//	PhysicalFile::DirectArea::operator= -- Operator of substitution
//
//	NOTES
//
//	ARGUMENTS
//	const DirectArea&						cArea_
//		[IN]		Reference to DirectArea
//
//	RETURN
//
//	EXCEPTIONS
//
DirectArea&
DirectArea::
operator =(const DirectArea& cArea_)
{
	m_pPage = cArea_.m_pPage;
	m_uiAreaID = cArea_.m_uiAreaID;
	moveOwner(cArea_);
	return *this;
}

//	FUNCTION public
//	PhysicalFile::DirectArea::getID -- Get ID
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	DirectArea::ID
//				ID of the area
//
//	EXCEPTIONS
//
DirectArea::ID
DirectArea::getID() const
{
	; _SYDNEY_ASSERT(m_pPage != 0);

	ID id;
	id.m_uiPageID = m_pPage->getID();
	id.m_uiAreaID = m_uiAreaID;
	return id;
}

//	FUNCTION public
//	PhysicalFile::DirectArea::isWritable -- Check the area is writable
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	bool
//				The case of the area being Writable
//
//	EXCEPTIONS
//
bool
DirectArea::isWritable() const
{
	; _SYDNEY_ASSERT(m_pPage != 0);

	return !(m_pPage->getFixMode() & Buffer::Page::FixMode::ReadOnly);
}

//	FUNCTION public
//	PhysicalFile::DirectArea:: -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DirectArea::operator void*()
{
	; _SYDNEY_ASSERT(m_pPage != 0);
	; _SYDNEY_ASSERT(isWritable() == true);

	PageOffset offset = m_pPage->getAreaOffset(m_uiAreaID);
	if (offset == ConstValue::UndefinedAreaOffset)
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_pPage->operator char*() + offset;
}

//	FUNCTION public
//	PhysicalFile::DirectArea:: -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
DirectArea::operator const void*() const
{
	; _SYDNEY_ASSERT(m_pPage != 0);

	PageOffset offset = m_pPage->getAreaOffset(m_uiAreaID);
	if (offset == ConstValue::UndefinedAreaOffset)
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return static_cast<const Page&>(*m_pPage).operator const char*()+offset;
}

//	FUNCTION public
//	PhysicalFile::DirectArea::dirty -- Notify the data of the area being changed
//
//	NOTES
//	This function should be called after the data of the area is changed.
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectArea::dirty()
{
	; _SYDNEY_ASSERT(m_pPage != 0);

	m_pPage->dirty();
}

//	FUNCTION public
//	PhysicalFile::DirectArea::getSize -- Get the size of itself
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	AreaSize
//				The size of the area
//
//	EXCEPTIONS
//
AreaSize
DirectArea::getSize() const
{
	; _SYDNEY_ASSERT(m_pPage != 0);

	AreaSize size = m_pPage->getAreaSize(m_uiAreaID);
	if (size == ConstValue::UndefinedAreaSize)
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return size;
}

//	FUNCTION public
//	PhysicalFile::DirectArea::detach -- detach the area
//
//	NOTES
//	It is mandatory that File::detachAllAreas is executed at the end of
//	a sequences of processes, EVEN IF this function is used for all DirectAreas.
//
//	Example code:
//	-----
//	for()
//	{
//		area = file.attachArea();
//		...
//		area.detach();		// suggested
//	}
//	file.detachAllAreas();	// mandatory
//	-----
//
//	For implementation, the area is NOT detached soon.
//	Actually, the area is detached by File::detachAllAreas or File::attachArea.
//	See these functions for actually detaching.
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectArea::detach()
{
	if (m_bOwner == true)
	{
		if (m_pPage != 0)
		{
			m_pPage->detach();
			m_pPage = 0;
		}
		m_bOwner = false;
	}
}

//	FUNCTION public
//	PhysicalFile::DirectArea::expunge -- expunge the area
//
//	NOTES
//	This function name is different from the other modules,
//	File::freeArea and Page::freeArea.
//	The reason of not using 'free' is preventing a compiler or a user
//	from misinterpreting this function and Standard C Library's one.
//
//	ARGUMENTS
//	const Trans::Transaction&			cTransaction_
//		[IN]		Reference to transaction
//	Admin::Verification::Progress*		pProgress_
//		[IN/OUT]	Rointer to progress
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectArea::expunge(const Trans::Transaction&		cTransaction_,
					Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(m_pPage != 0);
	; _SYDNEY_ASSERT(isWritable() == true);
	
	m_pPage->freeArea(cTransaction_, m_uiAreaID, pProgress_);
	dirty();
}

//	FUNCTION public
//	PhysicalFile::DirectArea::changeSize -- Change the size of the area
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&				cTransaction_
//		[IN]		Reference to transaction
//	AreaSize								uiSize_
//		[IN]		The new requested size of the area
//	Admin::Verification::Progress*		pProgress_
//		[IN/OUT]	Rointer to progress
//
//	RETURN
//
//	EXCEPTIONS
//	Exception::BadArgument
//					uiSize_ is invalid, 0.
//
bool
DirectArea::changeSize(const Trans::Transaction&		cTransaction_,
					   AreaSize							uiSize_,
					   Admin::Verification::Progress*	pProgress_)
{
	; _SYDNEY_ASSERT(m_pPage != 0);
	; _SYDNEY_ASSERT(isWritable() == true);

	if (uiSize_ == 0)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	bool result = false;
	if (m_pPage->changeAreaSize(
			cTransaction_, m_uiAreaID, uiSize_, pProgress_))
	{
		result = true;
		dirty();
	}

	return result;
}

//	FUNCTION public
//	PhysicalFile::DirectArea::getMaxExpandableSize --
//		Get the max expandable size of area 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	AreaSize
//		The max expandable size of area
//
//	EXCEPTIONS
//
AreaSize
DirectArea::getMaxExpandableSize() const
{
	; _SYDNEY_ASSERT(m_pPage != 0);

	return m_pPage->getFreeSize(false);
}

#ifdef DEBUG
// Get Page
Page*
DirectArea::getPage() const
{
	return m_pPage;
};
#endif

//	FUNCTION private
//	PhysicalFile::DirectArea::moveOwner -- Move owner
//
//	NOTES
//
//	ARGUMENTS
//	const DirectArea&						cArea_
//		[IN]		Reference to DirectArea
//
//	RETURN
//
//	EXCEPTIONS
//
void
DirectArea::moveOwner(const DirectArea& cArea_) const
{
	if (cArea_.m_bOwner == true)
	{
		m_bOwner = true;
		cArea_.m_bOwner = false;
	}
	else
	{
		m_bOwner = false;
	}
}

//
//	Copyright (c) 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
