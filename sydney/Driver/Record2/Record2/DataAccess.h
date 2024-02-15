// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.h -- Realize data accessing of record file
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

#ifndef __SYDNEY_RECORD2_DATAACCESS_H
#define __SYDNEY_RECORD2_DATAACCESS_H

#include "Record2/FileID.h"
#include "Record2/FixedData.h"
#include "Record2/VariableData.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

class ObjectManager;
class AreaManager;

//
//	CLASS
//	Record2::DataAccess -- 
//		DataAccess
//	NOTES
// Real class to deal with record file, realizes data accessing of record file.
// Data access including create, insert, get, update and erase.
//
class DataAccess : public Common::Object
{

  public:
    
	//constructor
    DataAccess(ObjectManager* pObjectManager_,
					AreaManager* pAreaManager_ = 0,
					Common::DataArrayData* pTuple_ = 0);

	//destructor
	~DataAccess();

    //initializeFieldData
	//The array of field information is initialized. 
	void initializeFieldData(const FileID::TargetFields& cFixedTargets_, 
								const FileID::TargetFields& cVariableTargets_,
								const bool bHasVariable_);

    //read data from fields
    bool read(ObjectID::Value ullObjectID_);

    //insert data into fields
    ObjectID::Value insert();

    //update fields
    ObjectID::Value update(ObjectID::Value ullObjectID_);

	//undo update, exchange data between current area and old area
	void undoUpdate(ObjectID::Value ullObjectID_, ObjectID::Value ullNextObjectID_);

	//erase fields data really
    void erase(ObjectID::Value ullObjectID_);

    //verify variable data
    void verify(ObjectID::Value ullObjectID_, 
				Admin::Verification::Treatment::Value eTreatment_,  
				Admin::Verification::Progress& cProgress_);

	//reset tuple if use the same dataaccess
	void resetTuple(Common::DataArrayData* pTuple_);

//*********************************************************************

private:

	// variable-length object ID are read
	const char* readObjectHeader(const char* pPointer_);
	//update is special for that old ObjectID must be written for rollback
	char* writeObjectHeader(char* pPointer_, 
							const ObjectID::Value ullNextObjectID_ = 0,
							bool bSkip_ = false);

//*****************************************************************

	//pointer of ObjectManager
    ObjectManager* m_pObjectManager;

	//handle of AreaManager
	AreaManager* m_pAreaManager;

	//data stored
	Common::DataArrayData* m_pTuple;

    //accessing to fixed and variable data
    FixedData* m_pFixedData;
    VariableData* m_pVariableData;

    //current fixed and variable object position
    //ObjectID::Value m_iFixedObjectID;
    ObjectID::Value m_ullVariableObjectID;

};

//resetTuple
inline
void 
DataAccess::
resetTuple(Common::DataArrayData* pTuple_)
{ 
	m_pTuple = pTuple_;
}

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_DATAACCESS_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
