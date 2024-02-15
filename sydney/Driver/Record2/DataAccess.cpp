// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataAccess.cpp -- Realize data accessing of record file
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Record2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Record2/Debug.h"
#include "Record2/DataAccess.h"
#include "Record2/ObjectManager.h"
#include "Record2/AreaManager.h"

_SYDNEY_USING

_SYDNEY_RECORD2_USING

//	FUNCTION public
//	Record2::DataAccess::DataAccess -- constructor
//
//	NOTES
//		-- constructor
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

DataAccess::
DataAccess(ObjectManager* pObjectManager_, 
			AreaManager* pAreaManager_,
			Common::DataArrayData* pTuple_)
	:m_pObjectManager(pObjectManager_),
	 m_pAreaManager(pAreaManager_),
	 m_pTuple(pTuple_),
	 m_pFixedData(0),
	 m_pVariableData(0)
{
}

//	FUNCTION public
//	Record2::DataAccess::DataAccess -- destructor
//
//	NOTES
//		-- destructor
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

DataAccess::
~DataAccess()
{
	//delete field data
	if(m_pFixedData)
	{
		delete m_pFixedData;
		m_pFixedData = 0;
	}

	if(m_pVariableData)
	{
		delete m_pVariableData;
		m_pVariableData = 0;
	}
}

//	FUNCTION public
//	Record2::DataAccess::DataAccess -- read
//
//	NOTES
//		-- read
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

bool
DataAccess::
read(ObjectID::Value ullObjectID_)
{
	; _SYDNEY_ASSERT(m_pObjectManager);

	const char* pPointer = 0;
	if (!ObjectID::isInvalid(ullObjectID_)) 
	{
		// seek the object postion actually
		pPointer = m_pObjectManager->seek(ullObjectID_);
	} 
	else 
	{
		pPointer = m_pObjectManager->next();
	}

	//if find
	if (pPointer) 
	{
		try
		{
			//get null bitmap and variable id
			pPointer = readObjectHeader(pPointer);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "DataAccess read data operation: ObjectID="
			<< ullObjectID_
			<< " VariableObjectID="
			<< m_ullVariableObjectID
			<< ModEndl;
#endif

			// get fixed data
			m_pFixedData->read(m_pTuple, pPointer);
			
			if(m_pVariableData && !ObjectID::isInvalid(m_ullVariableObjectID))
			{
				//get data really
				m_pVariableData->read(m_pTuple, m_ullVariableObjectID);
			}
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			//detach Page, maybe change...
			m_pObjectManager->detach(Manager::Operation::Read);
			_SYDNEY_RETHROW;
		}
	}
	else
	{
		if (!ObjectID::isInvalid(ullObjectID_)) {
			SydErrorMessage
				<< "Record2::read operation can't find this ObjectID="
				<< ullObjectID_
				<< ModEndl;
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		return false;
	}

	return true;
}

//	FUNCTION public
//	Record2::DataAccess::DataAccess -- insert
//
//	NOTES
//		-- insert
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

ObjectID::Value
DataAccess::
insert()
{
	; _SYDNEY_ASSERT(m_pObjectManager);

	ObjectID::Value ullObjectID = m_pObjectManager->allocate();
	
	if (ObjectID::isInvalid(ullObjectID)) return 0;
	
	// seek the object postion actually
	char* pPointer = const_cast<char*>
		(m_pObjectManager->seek(ullObjectID, Manager::Operation::Insert));

	//if find
	if (pPointer) 
	{
#ifdef DEBUG
		Utility::FieldNum iFieldNum = m_pFixedData->getTargetFields().getSize();
#endif
		m_ullVariableObjectID = ObjectID::m_UndefinedValue;
		//get the variable id first
		if(m_pVariableData)
		{
#ifdef DEBUG
			iFieldNum += m_pVariableData->getTargetFields().getSize();
#endif
			//write data really
			m_ullVariableObjectID = m_pVariableData->insert(m_pTuple);
		}

		//set null bitmap and variable id
		pPointer = writeObjectHeader(pPointer);

#ifdef DEBUG
		; _SYDNEY_ASSERT(iFieldNum == m_pTuple->getCount());
#endif

		//maybe needn't store it
		m_pFixedData->setCurrentObjectID(ullObjectID);
		m_pFixedData->insert(m_pTuple, pPointer);

	}
	else 
	{
		ullObjectID = ObjectID::m_UndefinedValue;
	}
	
#ifdef DEBUG
		SydRecordDebugMessage
			<< "DataAccess insert data operation: ObjectID="
			<< ullObjectID
			<< " VariableObjectID="
			<< m_ullVariableObjectID
			<< ModEndl;
#endif

	//save top page
	m_pObjectManager->sync();
	//detach Page, maybe change...
	m_pObjectManager->detach(Manager::Operation::Insert);

	return ullObjectID;
}

//	FUNCTION public
//	Record2::DataAccess::update -- update
//
//	NOTES
//		-- update
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

ObjectID::Value 
DataAccess::
update(ObjectID::Value ullObjectID_)
{
	; _SYDNEY_ASSERT(m_pObjectManager);

	ObjectID::Value iNewObjectID = ObjectID::m_UndefinedValue;
	if (ObjectID::isInvalid(ullObjectID_)) return iNewObjectID;

	// seek the object postion actually
	char* pOldPointer =  const_cast<char*>
		(m_pObjectManager->seek(ullObjectID_, Manager::Operation::Update));

	//if find
	if (pOldPointer) 
	{
		//read variable objectid
		readObjectHeader(pOldPointer);

		ObjectID::Value iOldVariableObjectID = m_ullVariableObjectID;
		//if has variable
		if(m_pVariableData && !ObjectID::isInvalid(iOldVariableObjectID))
		{
			//update data really
			m_ullVariableObjectID = m_pVariableData->update(m_pTuple, iOldVariableObjectID);
		}
		//allocate new id
		iNewObjectID = m_pObjectManager->allocate();

#ifdef DEBUG
		SydRecordDebugMessage
			<< "DataAccess update data operation: "
			<< "put old data into ObjectID="
			<< iNewObjectID
			<< " VariableObjectID="
			<< iOldVariableObjectID
			<< " and update new data in ObjectID="
			<< ullObjectID_
			<< " VariableObjectID="
			<< m_ullVariableObjectID
			<< ModEndl;
#endif	

		// seek the object postion actually
		char* pNewPointer = const_cast<char*>
			(m_pObjectManager->seek(iNewObjectID, Manager::Operation::Insert));

		//copy old data to new space
		Utility::Size uiDataSize = m_pObjectManager->getFile().getFileID().getObjectSize();
		Os::Memory::copy(pNewPointer, pOldPointer, uiDataSize);

		//old variableid should be stored in new allocated fixedheader
		pOldPointer = writeObjectHeader(pOldPointer, iNewObjectID);

		//set null bitmap and variable id
		m_ullVariableObjectID = iOldVariableObjectID;
		ObjectID::Value ullNextObjectID;
		ObjectID::readValue(pNewPointer, ullNextObjectID);
		if(ullNextObjectID == iNewObjectID) ullNextObjectID = ObjectID::m_UndefinedValue;
		pNewPointer = writeObjectHeader(pNewPointer, ullNextObjectID);

		//maybe needn't store it
		m_pFixedData->setCurrentObjectID(ullObjectID_);
		//old pointer put new data
		m_pFixedData->update(m_pTuple, pOldPointer);

		//remove old record
		m_pObjectManager->expunge(iNewObjectID);
	}

	//save top page
	m_pObjectManager->sync();
	//detach Page, maybe change...
	m_pObjectManager->detach(Manager::Operation::Update, true);
	return iNewObjectID;
}

//	FUNCTION public
//	Record2::DataAccess::update -- update
//
//	NOTES
//		-- update
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
DataAccess::
undoUpdate(ObjectID::Value ullObjectID_, ObjectID::Value ullNextObjectID_)
{
	; _SYDNEY_ASSERT(m_pObjectManager);

	//ObjectID::Value iNewObjectID = ObjectID::m_UndefinedValue;
	if (ObjectID::isInvalid(ullObjectID_) || ObjectID::isInvalid(ullNextObjectID_)) return;

	// seek the object postion actually
	char* pCurrPointer =  const_cast<char*>
		(m_pObjectManager->seek(ullObjectID_, Manager::Operation::Update));

	//if find
	if (pCurrPointer) 
	{
		//read variable objectid
		readObjectHeader(pCurrPointer);

		ObjectID::Value iCurrVariableObjectID = m_ullVariableObjectID;

		// seek the object postion actually
		char* pNextPointer = const_cast<char*>
			(m_pObjectManager->seek(ullNextObjectID_, Manager::Operation::Update));
		//get next variable id
		readObjectHeader(pNextPointer);

		//copy old data to new space
		Utility::Size uiDataSize = m_pObjectManager->getFile().getFileID().getObjectSize();
		Os::Memory::copy(pCurrPointer, pNextPointer, uiDataSize);

		//old variableid should be stored in new allocated fixedheader
		pCurrPointer = writeObjectHeader(pCurrPointer, 0, true);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "DataAccess undoUpdate data operation: "
			<< "copy next data: ObjectID="
			<< ullNextObjectID_
			<< " VariableObjectID="
			<< iCurrVariableObjectID
			<< " into current data: ObjectID="
			<< ullObjectID_
			<< " VariableObjectID="
			<< m_ullVariableObjectID
			<< ModEndl;
#endif

		//set and variable id
		m_ullVariableObjectID = iCurrVariableObjectID;
		if (m_pObjectManager->existVariableFile() && m_pVariableData) 
		{
			//move to varible objectid
			pNextPointer += ObjectID::m_ArchiveSize;
			pNextPointer += Utility::m_TransIDArchiveSize;
			// There is object ID in the variable length field. 
			pNextPointer += ObjectID::writeValue(pNextPointer, m_ullVariableObjectID);
		}
	}

	//detach Page, maybe change...
	m_pObjectManager->detach(Manager::Operation::Update);

}


//	FUNCTION public
//	Record2::DataAccess::erase -- erase
//
//	NOTES
//		-- erase fields data really
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
DataAccess::
erase(ObjectID::Value ullObjectID_)
{
	; _SYDNEY_ASSERT(m_pObjectManager);

	if (ObjectID::isInvalid(ullObjectID_)) return;
	
	//free fixed part and get variable ID
	// seek the object postion actually
	char* pPointer = const_cast<char*>
		(m_pObjectManager->seek(ullObjectID_, Manager::Operation::Update));
	//const char* pPointer = m_pObjectManager->seek(ullObjectID_, Manager::Operation::Read);

	//free direct area begin...
	//free direct area end
	if (pPointer) 
	{
		//read variable objectid
		readObjectHeader(pPointer);

		 if(m_pVariableData && !ObjectID::isInvalid(m_ullVariableObjectID))
		 {
#ifdef DEBUG
			SydRecordDebugMessage
				<< "DataAccess erase data operation: ObjectID="
				<< ullObjectID_
				<< " VariableObjectID="
				<< m_ullVariableObjectID
				<< ModEndl;
#endif	
			//erase data really
			m_pVariableData->erase(m_ullVariableObjectID);

			//reset undefined value
			m_ullVariableObjectID = ObjectID::m_UndefinedValue;
			//set variable id
			//pPointer -= ObjectID::m_ArchiveSize;
			pPointer = writeObjectHeader(pPointer);
		 }
	}
}

//	FUNCTION public
//	Record2::DataAccess::verify -- verify
//
//	NOTES
//		-- verify
//	ARGUMENTS
//		ObjectID::Value ullObjectID_
//			verify record's objectid
//  		 Admin::Verification::Treatment::Value eTreatment_
//			option of corresponding verification
//  		Admin::Verification::Progress& cProgress_
//			progress of corresponding verification
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
DataAccess::
verify(ObjectID::Value ullObjectID_, 
	   Admin::Verification::Treatment::Value eTreatment_,
	   Admin::Verification::Progress& cProgress_)
{
	; _SYDNEY_ASSERT(m_pObjectManager);

	const char* pPointer = 0;
	// seek the object postion actually
	pPointer = m_pObjectManager->seek(ullObjectID_, Manager::Operation::Verify);
	//if find
	if (pPointer)
	{
		//read variable objectid
		pPointer = readObjectHeader(pPointer);
		if(m_pVariableData && !ObjectID::isInvalid(m_ullVariableObjectID))
		{
	#ifdef DEBUG
			SydRecordDebugMessage
				<< "DataAccess verify data operation: ObjectID="
				<< ullObjectID_
				<< " VariableObjectID="
				<< m_ullVariableObjectID
				<< ModEndl;
	#endif

			try
			{
				m_pVariableData->verify(m_ullVariableObjectID, eTreatment_, cProgress_);
				//detach area
				m_pAreaManager->detachAll();
			}
	#ifdef NO_CATCH_ALL
			catch (Exception::Object&)
	#else
			catch (...)
	#endif
			{
				//detach area
				m_pAreaManager->detachAll();
				//detach Page, maybe change...
				m_pObjectManager->detach(Manager::Operation::Read);
				_SYDNEY_RETHROW;
			}
		}
	}
	else
	{
		SydErrorMessage
			<< "Record2::verify operation can't find this ObjectID="
			<< ullObjectID_
			<< ModEndl;
		//_SYDNEY_THROW0(Exception::BadArgument);
	}
}

//	FUNCTION public
//	Record2::DataAccess::initializeFieldData -- initializeFieldData
//
//	NOTES
//		initializeFieldData
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
DataAccess::
initializeFieldData(const FileID::TargetFields& cFixedTargets_,
					const FileID::TargetFields& cVariableTargets_,
					const bool bHasVariable_)
{
	//should provide all selected field count
	//Utility::FieldNum allSelectedCount = cFixedTargets_.m_vecFieldInfo.getSize()
	//	+ cVariableTargets_.m_vecFieldInfo.getSize();

	//create filed data to access real data
	if(!m_pFixedData)
	{
		m_pFixedData = new FixedData(cFixedTargets_);
	}

	if(!m_pVariableData && 
		bHasVariable_ && 
		cVariableTargets_.getSize() != 0)
	{
		m_pVariableData = new VariableData(cVariableTargets_, m_pAreaManager);
	}
}

//	FUNCTION public
//	Record2::DataAccess::readObjectHeader -- readObjectHeader
//
//	NOTES
//		-- readObjectHeader
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

const char*
DataAccess::
readObjectHeader(const char* pPointer_)
{
	; _SYDNEY_ASSERT(pPointer_);

	//maybe do more thing in future...
	
	//skip linked objectid and transid
	pPointer_ += ObjectID::m_ArchiveSize;
	pPointer_ += Utility::m_TransIDArchiveSize;

	m_ullVariableObjectID = ObjectID::m_UndefinedValue;
	if (m_pObjectManager->existVariableFile()) 
	{
		if(m_pVariableData)
		{
			// There is object ID in the variable length field. 
			pPointer_ += ObjectID::readValue(pPointer_, m_ullVariableObjectID);
		}
		else pPointer_ += ObjectID::m_ArchiveSize;
	}

	return pPointer_;
}

//	FUNCTION public
//	Record2::DataAccess::writeObjectHeader -- writeObjectHeader
//
//	NOTES
//		-- update is special for that old ObjectID must be written for rollback
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

char*
DataAccess::
writeObjectHeader(char* pPointer_, const ObjectID::Value ullNextObjectID_, bool bSkip_)
{
	; _SYDNEY_ASSERT(pPointer_);

	//if updating, ullNextObjectID_ is old objectid
	//if inserting, ullNextObjectID_ is assign to undifined value
	//set objectid
	if(bSkip_) pPointer_ += ObjectID::m_ArchiveSize;	//undoUpdate
	else pPointer_ += ObjectID::writeValue(pPointer_, ullNextObjectID_);

	//get Trans ID
	Utility::TransID iTransID = Trans::IllegalID;
	if(ullNextObjectID_ != 0) iTransID = 0;
	Os::Memory::copy(pPointer_, &iTransID, Utility::m_TransIDArchiveSize);
	pPointer_ += Utility::m_TransIDArchiveSize;

	if (m_pObjectManager->existVariableFile()) 
	{
		if(m_pVariableData)
		{
			// There is object ID in the variable length field. 
			pPointer_ += ObjectID::writeValue(pPointer_, m_ullVariableObjectID);
		}
		else pPointer_ += ObjectID::m_ArchiveSize;
	}

	return pPointer_;
}

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
