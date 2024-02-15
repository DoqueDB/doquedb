// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaManager.h -- Header file of class that secure/releases DirectArea
// 
// Copyright (c) 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD2_AREAMANAGER_H
#define __SYDNEY_RECORD2_AREAMANAGER_H

#include "PhysicalFile/DirectArea.h"

#include "Record2/Manager.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

//use a alias
typedef PhysicalFile::DirectArea PhysicalArea;

//
//	CLASS
//	Record2::AreaManager -- Class that operates direct area, and maintains information on direct area
//
//	NOTES
//
// It has the method of doing the allocation and the release of the area. 
//	
class AreaManager : public Manager		// This class need re-write carefullly
{

public:

	// constructor
	AreaManager(File& cFile_);

	// destructor
	~AreaManager();

//*********************************************************************
	// attach DirectArea from variable file
	PhysicalArea attach(ObjectID::Value ullObjectID_, 
						Operation::Value eOperation_ = Operation::Read,
						Admin::Verification::Treatment::Value eTreatment_ = 0,
						Admin::Verification::Progress* pProgress_ = 0);

    //detach DirectArea
    void detach(ObjectID::Value ullObjectID_, 
				Operation::Value eOperation_ = Operation::Read);

	// All areas are Detattied. 
	void detachAll(bool bSucceeded_ = true);

	// create some DirectArea when insert a record which including variable fields.
	// may be return multy area when request size is big enough.
	ObjectID::Value createArea(Utility::Size uiRequiredSize_);

	//free the space when it isn't necessary
	void freeArea(ObjectID::Value ullObjectID_, bool bRemoveFromMap_ = true);

	// Free all the area, it call physicalfile's function to do it 
	void freeAreaAll();

	//load file or create it
	void substanceFile();
	//here * read and write operation
	//......................
	//

//*********************************************************************


private:

	//create one area, may be private function
	PhysicalArea createOneArea(Utility::Size uiRequiredSize_);

	//store header info 
	void storeMultiAreaInfo(PhysicalArea cFirstArea_, ModVector<PhysicalArea>& vecArea_);

	//convert ObjectID to DirectArea::ID
	ObjectID::Value getObjectIDFromAreaID(const PhysicalArea::ID& cAreaID_);
	void setObjectIDToAreaID(ObjectID::Value ullObjectID_, PhysicalArea::ID& cAreaID_);

//*********************************************************************

	// Data member

	//DirectArea cache 
	typedef ModMap<ObjectID::Value, PhysicalArea, ModLess<ObjectID::Value> > AttachedAreaMap;
	AttachedAreaMap m_mapAttachedArea;
	
};

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_AREAMANAGER_H

//
//	Copyright (c) 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
