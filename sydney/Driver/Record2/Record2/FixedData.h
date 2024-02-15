// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FixedData.h -- Class for fixed data operation, managed fixed fields header and data accessing.
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD2_FIXEDDATA_H
#define __SYDNEY_RECORD2_FIXEDDATA_H

#include "Record2/FieldData.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

//
//	CLASS
//	Record2::FixedData -- 
//		Class for fixed data operation, managed fixed fields header and data accessing.
//
//	NOTES
// 
//
class FixedData : public FieldData
{

public:
    //constructor
    FixedData(const FileID::TargetFields& cTargetFields_);

	//destructor
	~FixedData();

    //read
    void read(Common::DataArrayData* pTuple_, const char* pPointer_);

    //insert
    void insert(const Common::DataArrayData* pTuple_, char* pPointer_);

    //update
    void update(const Common::DataArrayData* pTuple_, char* pPointer_);

	//reset objectid
	void setCurrentObjectID(const ObjectID::Value& ullObjectID_);

private:

	//current fixed object position
    ObjectID::Value m_iCurrentObjectID;
};

//setCurrentObjectID
inline
void 
FixedData::
setCurrentObjectID(const ObjectID::Value& ullObjectID_)
{ 
	m_iCurrentObjectID = ullObjectID_;
}

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_FIXEDDATA_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
