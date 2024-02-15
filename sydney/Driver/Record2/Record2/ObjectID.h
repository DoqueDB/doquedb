// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectID.h -- lightweight of Common::ObjectIDData
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

#ifndef __SYDNEY_RECORD2_OBJECTID_H
#define __SYDNEY_RECORD2_OBJECTID_H

#include "Common/ObjectIDData.h"
#include "PhysicalFile/Types.h"
#include "LogicalFile/ObjectID.h"

#include "Record2/Module.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

//define alias
typedef PhysicalFile::PageID PhysicalPageID;
typedef PhysicalFile::AreaID PhysicalAreaID;

//
//	Record2::ObjectID --
//
class ObjectID
{

public:	
	
	//define Object Value as unsigned __int64
	typedef ModUInt64 Value;

	//const value
	static const ModSize m_ArchiveSize;
	static const Value m_UndefinedValue;
	
	//default constructor
	ObjectID();
	
	//assign constructor
	ObjectID(const Value& uiValue_);
	
	//override constructor
	ObjectID(const PhysicalPageID	uiPageID_,
			const PhysicalAreaID uiAreaID_);
	
	//override ObjectID equal symbol
	//bool operator==(const ObjectID&	cObjectID_) const;

	// is invalied
	static bool isInvalid(const Value& uiValue_);
	//the data type is objectid type
	static bool isObjectIDType(const Common::DataArrayData* pTuple_, int iIndex_);
	
	//get logical objectid from data
	static const LogicalFile::ObjectID* getLogicalObjectID(const Common::Data* pData_);

	//main interface for outside
	Value getValue() const;
	//static function
	static Value getValue(const PhysicalPageID uiPageID_,
						const PhysicalAreaID uiAreaID_);

	// Function concerning ObjectID value read in fixed page
	static ModSize readValue(const char* pSrcPointer_, Value& cDestValue_);
	// Function concerning ObjectID value write in fixed page
	static ModSize writeValue(char* pDestPointer_, Value cSrcValue_);


	//real value
	PhysicalPageID m_uiPageID;
	PhysicalAreaID m_uiAreaID;
};

_SYDNEY_RECORD2_END

_SYDNEY_END

#endif // __SYDNEY_RECORD2_OBJECTID_H

//
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
