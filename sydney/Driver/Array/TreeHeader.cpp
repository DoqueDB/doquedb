// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TreeHeader.cpp --
// 
// Copyright (c) 2007, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Array/TreeHeader.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

//
//	FUNCTION public
//	Array::TreeHeader::TreeHeader --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
TreeHeader::TreeHeader()
{
	initialize();
}

//
//	FUNCTION public
//	Array::TreeHeader::~TreeHeader --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
TreeHeader::~TreeHeader()
{
}

//
//	FUNCTION public
//	Array::TreeHeader::initialize --
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
TreeHeader::initialize()
{
	m_uiRootPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_uiLeftLeafPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_uiRightLeafPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_uiCount = 0;
	m_uiStepCount = 0;
}

//
//	FUNCTION public
//	Array::TreeHeader::incrementCount --
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
TreeHeader::incrementCount()
{
	++m_uiCount;
}

//
//	FUNCTION public
//	Array::TreeHeader::decrementCount --
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
TreeHeader::decrementCount()
{
	; _SYDNEY_ASSERT(m_uiCount > 0);
	--m_uiCount;
}

//
//	FUNCTION public
//	Array::TreeHeader::decrementStepCount --
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
TreeHeader::decrementStepCount()
{
	; _SYDNEY_ASSERT(m_uiStepCount > 0);
	--m_uiStepCount;
}

//
//	FUNCTION public
//	Array::TreeHeader::dump --
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
TreeHeader::dump(ModUInt32*& p_) const
{
	Os::Memory::copy(p_, this, static_cast<ModSize>(sizeof(*this)));
	p_ += sizeof(*this) / sizeof(ModUInt32);
}

//
//	FUNCTION public
//	Array::TreeHeader::restore --
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
TreeHeader::restore(const ModUInt32*& p_)
{
	Os::Memory::copy(this, p_, static_cast<ModSize>(sizeof(*this)));
	p_ += sizeof(*this) / sizeof(ModUInt32);
}

//
//	Copyright (c) 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
