// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VariableData.h --
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

#ifndef __SYDNEY_RECORD2_VARIABLEDATA_H
#define __SYDNEY_RECORD2_VARIABLEDATA_H

#include "Record2/FieldData.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

class AreaArchiver;
class AreaManager;

//
//	CLASS
//	Record2::VariableData -- 
//		VariableData
//	NOTES
//  Class for variable data operation, managed variable fields header and data accessing. 
//  It provides random accessing to each field.
//

class VariableData : public FieldData
{
public:

	//field position
	struct Position
	{
		//DirectArea id index
		Utility::AreaIndex m_uiIndex;
		//offset in one DirectArea
		Utility::AreaOffset m_uiOffset;
		//constructor
		Position(Utility::AreaIndex uiIndex_ = 0, 
				Utility::AreaOffset uiOffset_ = 0)
				:m_uiIndex(uiIndex_),m_uiOffset(uiOffset_){;}
	};

	//constructor
	VariableData(const FileID::TargetFields& cTargetFields_, AreaManager* pAreaManager_);

	//destructor
	~VariableData();

    //read
    void read(Common::DataArrayData* pTuple_, const ObjectID::Value ullObjectID_);

	//insert into
	ObjectID::Value insert(const Common::DataArrayData* pTuple_);

    //update, get a new id
	ObjectID::Value update(const Common::DataArrayData* pTuple_, const ObjectID::Value iOldObjectID_);
 
	//free direct area
	void erase(const ObjectID::Value ullObjectID_);

	//verify variable data
    void verify(ObjectID::Value ullObjectID_, 
		Admin::Verification::Treatment::Value eTreatment_,
		Admin::Verification::Progress& cProgress_);

	//get positions block size
	static Utility::Size getPositionSize(const Utility::FieldNum uiPositionCount_, bool isArray_);

private:
	
	//read/write positions
	Position* readPositions(Utility::FieldNum& uiPositionCount_, 
				bool bIsArray_, AreaArchiver& cArchiver_, bool bSkip_ = false);
	void  writePositions(Position* pPositions, Utility::FieldNum uiPositionCount_, bool bIsArray_);

    //get/set simple variable
    void getSimpleVariable(const FieldInfo& cInfo_, Common::Data& cData_);
	void setSimpleVariable(const FieldInfo& cInfo_, const Common::DataArrayData::Pointer& pData_);

	//get/set fixed Array
    void getFixedArray(const FieldInfo& cInfo_, Common::Data& cData_);
	void setFixedArray(const FieldInfo& cInfo_, const Common::DataArrayData::Pointer& pData_);

    //get/set variable Array
    void getVariableArray(const FieldInfo& cInfo_, Common::Data& cData_);
	void setVariableArray(const FieldInfo& cInfo_, const Common::DataArrayData::Pointer& pData_);

	//calculate variable size in inserting
	Utility::Size makeInsertSize(const Common::DataArrayData* pTuple_);

	//re-calculate variable size in updating
	Utility::Size makeUpdateSize(const Common::DataArrayData* pTuple_, 
					AreaArchiver& cArchiver_, Position* pPositions_);

	// get one field size from data
	Utility::FieldLength getLengthFromData(const Common::Data& cData_, const FieldInfo& cFieldInfo_);
	
	//get one field length from Area
	Utility::FieldLength getLengthFromArea(Utility::FieldNum uiIndex_, 
					AreaArchiver& cArchiver_, Position* pPositions_);

	//reset variable array header
	void resetVariableArrayPositions(Position& cNewPosition_, AreaArchiver& cArchiver_);

	//************************************************************************************

	//m_cAreaManager
	AreaManager* m_pAreaManager;

	//access to directarea
    AreaArchiver* m_pArchiver;

     //array of Position
	Position* m_pPositions;

};

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_VARIABLEDATA_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
