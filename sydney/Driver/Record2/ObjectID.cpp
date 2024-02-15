// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectID.cpp --
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Record2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Exception/BadArgument.h"
#include "Common/Assert.h"


#include "Record2/ObjectID.h"

_SYDNEY_USING
_SYDNEY_RECORD2_USING

namespace
{
	const int _iAreaIDBits = sizeof(Common::ObjectIDData::LatterType) * 8;
	const ObjectID::Value _ullAreaIDMask = ~((~static_cast<ObjectID::Value>(0)) << _iAreaIDBits);
}

// static
const ObjectID::Value
ObjectID::m_UndefinedValue = Common::ObjectIDData::getMaxValue();

// static
const ModSize
ObjectID::m_ArchiveSize = sizeof(Common::ObjectIDData::FormerType) + sizeof(Common::ObjectIDData::LatterType);

//	FUNCTION public
//	Record2::Object::ObjectID -- ObjectID
//
//	NOTES
//		-- default constructor

//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

ObjectID::
ObjectID()
  : m_uiPageID(PhysicalFile::ConstValue::UndefinedPageID),
	m_uiAreaID(PhysicalFile::ConstValue::UndefinedAreaID)
{
	;//do nothing
}
	
//	FUNCTION public
//	Record2::Object::ObjectID -- ObjectID
//
//	NOTES
//		-- copy constructor

//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

ObjectID::
ObjectID(const Value& uiValue_)
   :m_uiPageID(static_cast<PhysicalPageID>(uiValue_ >> _iAreaIDBits)),
	m_uiAreaID(static_cast<PhysicalAreaID>(uiValue_ & _ullAreaIDMask))
{
	;//do nothing
}
	
//	FUNCTION public
//	Record2::Object::ObjectID -- ObjectID
//
//	NOTES
//		-- override constructor

//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

ObjectID::
ObjectID(const PhysicalPageID	uiPageID_,
		const PhysicalAreaID uiAreaID_)
	: m_uiPageID(uiPageID_), m_uiAreaID(uiAreaID_) 
{
	;//do nothing
}

//	FUNCTION public
//	Record2::Object::equal -- equal
//
//	NOTES
//		-- :operator==

//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

//bool
//ObjectID::
//operator==(const ObjectID&	cObjectID_) const
//{
//	return (m_uiPageID == cObjectID_.m_uiPageID &&
//			m_uiAreaID == cObjectID_.m_uiAreaID);
//}

//	FUNCTION public
//	Record2::Object::isInvalid -- isInvalid
//
//	NOTES
//		-- main interface for outside

//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

bool 
ObjectID::
isInvalid(const Value& uiValue_)
{
	return uiValue_ > 0 && uiValue_ < m_UndefinedValue	? false : true;
}

//	FUNCTION public
//	Record2::Object::isObjectIDType -- isObjectIDType
//
//	NOTES
//		-- judge data is this type

//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

bool 
ObjectID::
isObjectIDType(const Common::DataArrayData* pTuple_, int iIndex_)
{
	return pTuple_->getElement(iIndex_)->getType() == 
		Common::DataType::ObjectID	? true : false;
}

//	FUNCTION public
//	Record2::Object::getLogicalObjectID -- getLogicalObjectID
//
//	NOTES
//		-- judge data is this type

//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

const LogicalFile::ObjectID* 
ObjectID::
getLogicalObjectID(const Common::Data* pData_)
{
	const LogicalFile::ObjectID* pID = 0;
	if (pData_->getType() == Common::DataType::Array) 
	{
		const Common::DataArrayData* pKey =
			_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_);
		if (pKey->getElement(0)->getType() != LogicalFile::ObjectID().getType()) 
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		pID = _SYDNEY_DYNAMIC_CAST(LogicalFile::ObjectID*, pKey->getElement(0).get());

	} 
	else 
	{
		if (pData_->getType() != LogicalFile::ObjectID().getType()) 
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		pID = _SYDNEY_DYNAMIC_CAST(const LogicalFile::ObjectID*, pData_);
	}

	; _SYDNEY_ASSERT(pID);

	return pID;	
}

//	FUNCTION public
//	Record2::Object::getValue -- getValue
//
//	NOTES
//		-- main interface for outside

//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

ObjectID::Value 
ObjectID::
getValue() const
{ 
	return getValue(m_uiPageID, m_uiAreaID);
}

//	FUNCTION public
//	Record2::Object::getValue -- getValue
//
//	NOTES
//		-- main interface for outside

//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS
	
ObjectID::Value 
ObjectID::
getValue(PhysicalPageID uiPageID_,
		PhysicalAreaID uiAreaID_)
{
	return (static_cast<Value>(uiPageID_) << _iAreaIDBits)
			| static_cast<Value>(uiAreaID_);
}

//	FUNCTION public
//	Record2::ObjectID::readValue -- get data from fixed page
//
//	NOTES
//		-- get data from fixed page

//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

ModSize 
ObjectID::
readValue(const char* pSrcPointer_, Value& cDestValue_)
{
	//; _SYDNEY_ASSERT(pSrcPointer_);

	/*
	//is a simple method
	Os::Memory::copy(&cDestValue_, pSrcPointer_, m_ArchiveSize);
	return m_ArchiveSize;
	*/

	Common::ObjectIDData::FormerType Former;
	Os::Memory::copy(&Former, pSrcPointer_, sizeof(Former));
	Common::ObjectIDData::LatterType Latter;
	Os::Memory::copy(&Latter, pSrcPointer_+sizeof(Former), sizeof(Latter));

	cDestValue_ = (static_cast<Value>(Former) << _iAreaIDBits)
				| static_cast<Value>(Latter);

	//equal to m_ArchiveSize
	return sizeof(Common::ObjectIDData::FormerType)
		+ sizeof(Common::ObjectIDData::LatterType);
	
}

//	FUNCTION public
//	Record2::ObjectID::writeValue -- write data to fixed page
//
//	NOTES
//		-- write data to fixed page

//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

ModSize 
ObjectID::
writeValue(char* pDestPointer_, Value cSrcValue_)
{
	//; _SYDNEY_ASSERT(pDestPointer_);

	/*
	//is a simple method
	Os::Memory::copy(pDestPointer_, &cSrcValue_, m_ArchiveSize);
	return m_ArchiveSize;
	*/
	
	Common::ObjectIDData::FormerType Former
		= static_cast<Common::ObjectIDData::FormerType>(cSrcValue_ >> _iAreaIDBits);
	Os::Memory::copy(pDestPointer_, &Former, sizeof(Former));
	Common::ObjectIDData::LatterType Latter
		= static_cast<Common::ObjectIDData::LatterType>(cSrcValue_ & _ullAreaIDMask);
	Os::Memory::copy(pDestPointer_+sizeof(Former), &Latter, sizeof(Latter));

	//equal to m_ArchiveSize
	return sizeof(Common::ObjectIDData::FormerType)
		+ sizeof(Common::ObjectIDData::LatterType);
}

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
