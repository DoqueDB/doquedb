// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaArchiver.h -- AreaArchiver.h
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

#ifndef __SYDNEY_RECORD2_AREAARCHIVER_H
#define __SYDNEY_RECORD2_AREAARCHIVER_H

#include "ModSerialIO.h"

#include "Record2/VariableData.h"
#include "Record2/AreaManager.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

//struct VariableData::Position;

//
//	CLASS
//	Record2::AreaArchiver -- 
//		AreaArchiver
//	NOTES
//	We could take the class as the bridge from outer data to LinkedArea, and remove the old classes (linkobject...). 
//	Any data operation accessing to DirectArea must through it.
//
class AreaArchiver : public ModSerialIO
{
public:
	
	struct LinkedArea
	{
		//Direct Area ID
		ObjectID::Value m_iObjectID;
		//Area size
		Utility::AreaSize m_uiSize;
		//constructor
		LinkedArea(ObjectID::Value ullObjectID_ = ObjectID::m_UndefinedValue, 
					Utility::AreaSize uiSize_ = 0)
					:m_iObjectID(ullObjectID_),m_uiSize(uiSize_){;}
	};

	//constructor
	AreaArchiver(AreaManager* pAreaManager_);

	//destructor
	~AreaArchiver();

	//read linked areas
	//needn't write because it's writed in creating Areas
	Utility::AreaSize setFirstArea(const ObjectID::Value iFirstObjectID_, 
		Manager::Operation::Value eOperation_ = Manager::Operation::Read,
		Admin::Verification::Treatment::Value eTreatment_ = 0,
		Admin::Verification::Progress* pProgress_ = 0);
	//goto the first area and skip the linked areas
	Utility::AreaOffset skipLinkedAreas(const ObjectID::Value iFirstObjectID_);
	LinkedArea* getLinkedAreas() const;
	Utility::AreaIDNum getAreaCount() const;

	//set dump value from DirectArea to Data
	//because following 2 functions are inherit from Common module
	//so size must use ModSize
	virtual int readSerial(void* pBuf_, ModSize uiSize_, 
		ModSerialIO::DataType cType_ = ModSerialIO::dataTypeVariable);
	//dump value from Data to DirectArea
	virtual int writeSerial(const void* pBuf_, ModSize uiSize_, 
		ModSerialIO::DataType cType_ = ModSerialIO::dataTypeVariable);
	
	//skip and not dump value, for null data or others condition(decimal)
	ModSize skip(ModSize uiSize_, bool bReadOnly_ = true);

	//set position info such as index and offset
	//we should call this function first before random reading or writing
	void setPosition(const VariableData::Position& cPosition_);

	//copy src Archiver to dest Archiver
	static void copy(AreaArchiver* pDest_, AreaArchiver* pSrc_, Utility::Size uiSize_);

	//get current AreaIndex
	Utility::AreaIndex getCurrentIndex() const;
	//get current offset
	Utility::AreaOffset getCurrentOffset() const;

	//after doing something, the area should be detached
	void detachArea(bool bAll_ = false);

private:

	//get next direct area
    void getNextArea(bool isRealOnly_ = true);

	// attach DirectArea, OjectID = m_iCurrentAreaID
	// the pointer store in m_pAreaPointer, size store in m_uiAreaSize
	void attachArea(Manager::Operation::Value eOperation_);

//****************************************************************
	//attach/detach DirectArea
	AreaManager* m_pAreaManager;
		
	//DirectArea array
	LinkedArea* m_pLinkedAreas;
	// LinkedArea count
	Utility::AreaIDNum m_uiAreaCount;

	//last DirectAreaID, avoid re-attach DirectArea
	ObjectID::Value m_iCurrentAreaID;
	//current area id
	Utility::AreaIndex m_uiCurrentIndex;
	//current offset
	Utility::AreaOffset m_uiCurrentOffset;

	//current area length
	Utility::AreaSize m_uiAreaSize;
	//current data length
	//Utility::Size m_uiDumpSize;

	//only reserve the original pointer
	union	//use union to reserve read/write pointer
	{
		const char* m_pConstAreaPointer;
		char* m_pAreaPointer;
	};

	//for verify
	Admin::Verification::Progress* m_pProgress;
	Admin::Verification::Treatment::Value m_eTreatment;
};

//get linked areas
inline 
AreaArchiver::LinkedArea*   
AreaArchiver::
getLinkedAreas() const
{
	return m_pLinkedAreas;
}

//get area count
inline 
Utility::AreaIDNum 
AreaArchiver::
getAreaCount() const
{
	return m_uiAreaCount;
}

//get area index
inline 
Utility::AreaIndex  
AreaArchiver::
getCurrentIndex() const
{
	return m_uiCurrentIndex;
}

//get area offset
inline 
Utility::AreaOffset   
AreaArchiver::
getCurrentOffset() const
{
	return m_uiCurrentOffset;
}

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_AREAARCHIVER_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
