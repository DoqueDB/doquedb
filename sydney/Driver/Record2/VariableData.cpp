// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VariableData.cpp -- 
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Record2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Common/AutoArrayPointer.h"
#include "Common/DecimalData.h"
#include "Exception/Unexpected.h"

#include "Record2/Debug.h"
#include "Record2/AreaArchiver.h"
#include "Record2/AreaManager.h"
#include "Record2/VariableData.h"
#include "Record2/Message_InconsistentVariableSize.h"

#include "ModAutoPointer.h"

_SYDNEY_USING

_SYDNEY_RECORD2_USING

namespace
{
	const Utility::AreaSize _AlignmentBytes = 4;
}

//	FUNCTION public
//	Record2::VariableData::VariableData -- constructor
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

VariableData::
VariableData(const FileID::TargetFields& cTargetFields_, AreaManager* pAreaManager_)
	: FieldData(cTargetFields_),
	  m_pAreaManager(pAreaManager_),
	  m_pPositions(0)
{
	m_pArchiver = new AreaArchiver(m_pAreaManager);
}

//	FUNCTION public
//	Record2::VariableData::~VariableData -- destructor
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

VariableData::
~VariableData()
{
	if(m_pPositions)
	{
		delete[] m_pPositions;
		m_pPositions = 0;
	}

	if(m_pArchiver)
	{
		delete m_pArchiver;
		m_pArchiver = 0;
	}
}

//	FUNCTION public
//	Record2::VariableData::read -- read
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

void
VariableData::
read(Common::DataArrayData* pTuple_, const ObjectID::Value ullObjectID_)
{
	; _SYDNEY_ASSERT(pTuple_ && !ObjectID::isInvalid(ullObjectID_));

	//read linked direct area first
	m_pArchiver->setFirstArea(ullObjectID_);

	//release first
	if(m_pPositions != 0) delete[] m_pPositions;
	//read header info (positions) second
	m_pPositions = readPositions(m_iFieldNum, false, *m_pArchiver);

	//need not read unselected fields
	//Utility::FieldNum selFieldNum = m_cTargetFields.getSize();
	//read value from DirectArea
	for(Utility::FieldNum i = 0; i < m_iFieldNum; ++ i)
	{
		const FieldInfo& cInfo = *(m_cTargetFields[i]);
		if(cInfo.isSelected())
		{
			Common::Data& cData =  *pTuple_->getElement(cInfo.getDataIndex());
			//if is array,including fixed and variable
			if(cInfo.isArray())
			{
				//read variable array
				if(cInfo.isVariableArray())
					getVariableArray(cInfo, cData);
				//read fixed array
				else getFixedArray(cInfo, cData);
			}
			//get simple variable
			else getSimpleVariable(cInfo, cData);
#ifdef DEBUG
		SydRecordDebugMessage
			<< "Read varible field: "
			<< i
			<< " data: "
			<< cData.getString()
			<< ModEndl;
#endif
		}
	}
	
	//free the resource
	m_pArchiver->detachArea(true);
}

//	FUNCTION public
//	Record2::VariableData::insert -- insert
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
VariableData::
insert(const Common::DataArrayData* pTuple_)
{
	; _SYDNEY_ASSERT(pTuple_);

	//get insert data size
	Utility::Size uiSize = makeInsertSize(pTuple_);

	//allocate new space
	ObjectID::Value ullObjectID = m_pAreaManager->createArea(uiSize);

	//read linked direct area first
	m_pArchiver->setFirstArea(ullObjectID);
	
	//selected fields == all fields
	Utility::FieldNum selFieldNum = m_cTargetFields.getSize();
	; _SYDNEY_ASSERT(selFieldNum == m_iFieldNum);

	//release first
	if(m_pPositions != 0) delete[] m_pPositions;
	//allocate header first, their content are assigned in set--value
	m_pPositions = readPositions(m_iFieldNum, false, *m_pArchiver, true);

	//read value from DirectArea
	for(Utility::FieldNum i = 0; i < selFieldNum; ++ i)
	{
		const FieldInfo& cInfo = *(m_cTargetFields[i]);
		const Common::DataArrayData::Pointer& pData = pTuple_->getElement(cInfo.getDataIndex());
		//if is array,including fixed and variable
		if(cInfo.isArray())
		{
			//write variable array
			if(cInfo.isVariableArray())
				setVariableArray(cInfo, pData);
			//write fixed array
			else setFixedArray(cInfo, pData);
		}
		//set simple variable
		else setSimpleVariable(cInfo, pData);
#ifdef DEBUG
		SydRecordDebugMessage
			<< "Insert varible field: "
			<< i
			<< " data: "
			<< pData->getString()
			<< ModEndl;
#endif
	}
	
	//set the last position info
	//m_pPositions[m_iFieldNum].m_uiIndex = m_pArchiver->getCurrentIndex();
	m_pPositions[m_iFieldNum].m_uiOffset = m_pArchiver->getCurrentOffset();
	//go to the header postion
	//skip area info header
	m_pArchiver->skipLinkedAreas(ullObjectID);
	//m_pArchiver->setPosition(m_pPositions[0]);

	//wirte position header info
	writePositions(m_pPositions, m_iFieldNum, false);

	//free the resource
	m_pArchiver->detachArea(true);

	return ullObjectID;
}


//	FUNCTION public
//	Record2::VariableData::update -- update
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
VariableData::
update(const Common::DataArrayData* pTuple_,	const ObjectID::Value iOldObjectID_)
{
	; _SYDNEY_ASSERT(pTuple_ && !ObjectID::isInvalid(iOldObjectID_));

	//used to read old data
	ModAutoPointer<AreaArchiver> pArchiverAutoPointer = new AreaArchiver(m_pAreaManager);
	AreaArchiver* pOldArchiver = pArchiverAutoPointer.get();

	//get the old linked areas
	pOldArchiver->setFirstArea(iOldObjectID_);
	//read old header first
	Common::AutoArrayPointer<Position> pPositionsAutoPointer = readPositions(m_iFieldNum, false, *pOldArchiver);
	Position* pOldPositions = pPositionsAutoPointer.get();

	//caculate changed size
	Utility::Size uiSize = makeUpdateSize(pTuple_, *pOldArchiver, pOldPositions);

	//allocate new space, including update
	ObjectID::Value iNewObject = m_pAreaManager->createArea(uiSize);
	//get the linked areas
	m_pArchiver->setFirstArea(iNewObject);
	
	//release first
	if(m_pPositions != 0) delete[] m_pPositions;
	//allocate header first, their content are assigned in set--value
	m_pPositions = readPositions(m_iFieldNum, false, *m_pArchiver, true);
	//read value from DirectArea
	for(Utility::FieldNum i = 0; i < m_iFieldNum; ++ i)
	{
		const FieldInfo& cInfo = *(m_cTargetFields[i]);
		if(cInfo.isSelected())	//insert into new data
		{
			const Common::DataArrayData::Pointer& pData = pTuple_->getElement(cInfo.getDataIndex());
			//if is array,including fixed and variable
			if(cInfo.isArray())
			{
				//write variable array
				if(cInfo.isVariableArray())
					setVariableArray(cInfo, pData);
				//write fixed array
				else setFixedArray(cInfo, pData);
			}
			//set simple variable
			else setSimpleVariable(cInfo, pData);
			
#ifdef DEBUG
		SydRecordDebugMessage
			<< "Update varible field: "
			<< i
			<< " data: "
			<< pData->getString()
			<< ModEndl;
#endif
		}
		else	//copy the old data
		{
			//get last position info and set them
			m_pPositions[i].m_uiIndex = m_pArchiver->getCurrentIndex();
			m_pPositions[i].m_uiOffset = m_pArchiver->getCurrentOffset();
			//get old field length
			Utility::FieldLength uiOldLength = getLengthFromArea(i, *pOldArchiver, pOldPositions);
			if(uiOldLength != 0)	//maybe have null data
			{
				//set to real position
				pOldArchiver->setPosition(pOldPositions[i]);
				//copy old data
				AreaArchiver::copy(m_pArchiver, pOldArchiver, uiOldLength);

				//variable array header info should be updated
				if(cInfo.isVariableArray())
				{
					//for updating the field positions
					resetVariableArrayPositions(m_pPositions[i], *pOldArchiver);
				}
#ifdef DEBUG
				SydRecordDebugMessage
				<< "Copy old variable data during updation for not changed field: "
				<< i
				<< ModEndl;
#endif
			}

#ifdef DEBUG
			else
			{
				SydRecordDebugMessage
					<< "copy data: FieldID = " 
					<< i
					<< " skip NULL"
					<< ModEndl;
			}
#endif
		}
	}
	
	//set the last position info
	//m_pPositions[m_iFieldNum].m_uiIndex = m_pArchiver->getCurrentIndex();
	m_pPositions[m_iFieldNum].m_uiOffset = m_pArchiver->getCurrentOffset();
	//go to the header postion
	//skip area info header
	m_pArchiver->skipLinkedAreas(iNewObject);
	//m_pArchiver->setPosition(m_pPositions[0]);
	writePositions(m_pPositions, m_iFieldNum, false);

	//free the resource
	pOldArchiver->detachArea(true);
	m_pArchiver->detachArea(true);

	return iNewObject;
}

//	FUNCTION public
//	Record2::VariableData::erase -- erase
//
//	NOTES
//		-- free direct area
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
VariableData::
erase(const ObjectID::Value ullObjectID_)
{
	; _SYDNEY_ASSERT( !ObjectID::isInvalid(ullObjectID_));

	//read linked direct area first
	m_pArchiver->setFirstArea(ullObjectID_, Manager::Operation::Expunge);

	//get linked areas
	AreaArchiver::LinkedArea* pLinkedAreas = m_pArchiver->getLinkedAreas();
	Utility::AreaIDNum uiAreaCount = m_pArchiver->getAreaCount();
	
	//check the areasize
	for(Utility::AreaIDNum i = 0; i < uiAreaCount; ++i)
	{
		//free direct area
		m_pAreaManager->freeArea(pLinkedAreas[i].m_iObjectID);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Erase variable data operation: ObjectID="
			<< pLinkedAreas[i].m_iObjectID
			<< " Utility::AreaSize="
			<< (int) pLinkedAreas[i].m_uiSize
			<< ModEndl;
#endif

	}
}

//	FUNCTION public
//	Record2::VariableData::verify -- verify
//
//	NOTES
//		-- verify
//	ARGUMENTS
//		ObjectID::Value ullObjectID_
//			verify area's objectid
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
VariableData::
verify(ObjectID::Value ullObjectID_, 
	   Admin::Verification::Treatment::Value eTreatment_,
	   Admin::Verification::Progress& cProgress_)
{
	; _SYDNEY_ASSERT( !ObjectID::isInvalid(ullObjectID_));

	//read linked direct area first
	Utility::AreaSize uiFirstAreaSize = m_pArchiver->setFirstArea(
					ullObjectID_,  Manager::Operation::Verify, eTreatment_, &cProgress_);

	//get linked areas
	AreaArchiver::LinkedArea* pLinkedAreas = m_pArchiver->getLinkedAreas();
	Utility::AreaIDNum uiAreaCount = m_pArchiver->getAreaCount();
	
	//check the areasize
	Utility::Size uiAreaSize = 0;
	for(Utility::AreaIDNum i = 0; i < uiAreaCount; ++i)
	{
		Utility::AreaSize uiCurrentSize = uiFirstAreaSize;
		if(i != 0)
		{
			PhysicalArea cPhysicalArea = m_pAreaManager->attach(pLinkedAreas[i].m_iObjectID, 
										Manager::Operation::Verify,eTreatment_, &cProgress_);
			uiCurrentSize = static_cast<Utility::AreaSize>(cPhysicalArea.getSize());
			//detach it soon
			m_pAreaManager->detach(pLinkedAreas[i].m_iObjectID, Manager::Operation::Verify);
		}

		if(uiCurrentSize != pLinkedAreas[i].m_uiSize)
		{
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_,
				m_pAreaManager->getFile().getPath(),
				Message::InconsistentVariableSize(pLinkedAreas[i].m_iObjectID,
							pLinkedAreas[i].m_uiSize, uiCurrentSize));
			return;
		}
		uiAreaSize += uiCurrentSize;

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Verify variable data operation: ObjectID="
			<< pLinkedAreas[i].m_iObjectID
			<< " Utility::AreaSize="
			<< (int) uiCurrentSize
			<< ModEndl;
#endif
	}

#ifdef DEBUG
	SydRecordDebugMessage
		<< "Verify variable data operation: AreaCount="
		<< (int)uiAreaCount
		<< " AllAreaSize="
		<< uiAreaSize
		<< ModEndl;

	Utility::Size uiPrevDataSize = 0;

#endif
	//check the field position
	//read header info (positions) second
	Utility::Size uiDataSize = 0;
	//release first
	if(m_pPositions != 0) delete[] m_pPositions;
	m_pPositions = readPositions(m_iFieldNum, false, *m_pArchiver);
	for(Utility::FieldNum i = 0; i < m_iFieldNum; ++i)
	{
		uiDataSize += getLengthFromArea(i, *m_pArchiver, m_pPositions);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Verify variable data operation: FieldID="
			<< i
			<< " FieldSize="
			<< uiDataSize - uiPrevDataSize
			<< ModEndl;

		uiPrevDataSize = uiDataSize;
#endif
	}

	uiDataSize += Utility::m_AreaIDNumArchiveSize;
	uiDataSize += (ObjectID::m_ArchiveSize  
			+ Utility::m_AreaSizeArchiveSize)
			* (uiAreaCount - 1);	//areacount not include the first area

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Verify variable data operation: AreaHeaderSize="
			<< uiDataSize - uiPrevDataSize
			<< ModEndl;

		uiPrevDataSize = uiDataSize;
#endif

	uiDataSize += getPositionSize(m_iFieldNum, false);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Verify variable data operation: FieldHeaderSize="
			<< uiDataSize - uiPrevDataSize
			<< ModEndl;

		SydRecordDebugMessage
			<< "Verify variable data operation: RealDataSize="
			<< uiDataSize
			<< ModEndl;

#endif

	//the last Area maybe have padding size, so...
	Utility::AreaSize uiRemainder = uiDataSize % _AlignmentBytes;
	if (uiRemainder != 0)
	{
		uiDataSize += _AlignmentBytes - uiRemainder;
	}

	//check the data size
	if(uiAreaSize != uiDataSize)
	{
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			m_pAreaManager->getFile().getPath(),
			Message::InconsistentVariableSize(ullObjectID_,
						uiDataSize, uiAreaSize));
		return;
	}

	//free the resource
	m_pArchiver->detachArea(true);
}

//	FUNCTION public
//	Record2::VariableData::readPositions -- readPositions
//
//	NOTES
//		-- readPositions
//	ARGUMENTS
//		bool bSkip_:	if read content
//
//	RETURN
//		None
//
//	EXCEPTIONS

VariableData::Position* 
VariableData::
readPositions(Utility::FieldNum& uiPositionCount_, 
			  bool bIsArray_, AreaArchiver& cArchiver_, bool bSkip_)
{
	//if is variable array
	if(bIsArray_)
	{
		if(!bSkip_)
		{
			//get element count
			cArchiver_.readSerial(&uiPositionCount_, Utility::m_ElementNumArchiveSize);
		}
		else cArchiver_.skip(Utility::m_ElementNumArchiveSize);
		//position count is know!!!
		//else cArchiver_.writeSerial(&uiPositionCount_, Utility::m_ElementNumArchiveSize);
	}
	
	//allocate field offset memory space
	//for convenience in caculating field length, Utility::AreaIndex + 1
	//the last offset must exist, so + 1
	Common::AutoArrayPointer<Position> pPositionsAutoPointer = new Position[uiPositionCount_ + 1];
	Position* pPositions = pPositionsAutoPointer.get();

	//if read content
	if(!bSkip_)
	{
		//get the filed offset
		for(Utility::FieldNum i = 0; i < uiPositionCount_; ++ i)
		{
			//get AreaID value
			cArchiver_.readSerial(&(pPositions[i].m_uiIndex), Utility::m_AreaIndexArchiveSize);

			//get offset value
			cArchiver_.readSerial(&(pPositions[i].m_uiOffset), Utility::m_AreaOffsetArchiveSize);
		}
		
		if(bIsArray_) 
		{	//get the last index value
			cArchiver_.readSerial(&(pPositions[uiPositionCount_].m_uiIndex), Utility::m_AreaIndexArchiveSize);
		}
		else 
		{
			//for convenience in caculating field length, but needn't store it
			pPositions[uiPositionCount_].m_uiIndex = cArchiver_.getAreaCount() - 1;
		}
		//get the last offset value
		cArchiver_.readSerial(&(pPositions[uiPositionCount_].m_uiOffset), Utility::m_AreaOffsetArchiveSize);
	}
	else	//skip
	{
		//because has read areaID, so - it
		cArchiver_.skip(getPositionSize(uiPositionCount_, bIsArray_));
	}

	pPositionsAutoPointer.release();
	return pPositions;
}

//	FUNCTION public
//	Record2::VariableData::writePositions -- writePositions
//
//	NOTES
//		-- writePositions
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void  
VariableData::
writePositions(VariableData::Position* pPositions, Utility::FieldNum uiPositionCount_, bool bIsArray_)
{
	//if is variable array
	if(bIsArray_)
	{
		//set element count
		m_pArchiver->writeSerial(&uiPositionCount_, Utility::m_ElementNumArchiveSize);
	}
	
	//set the filed offset
	for(Utility::FieldNum i = 0; i < uiPositionCount_; ++ i)
	{
		//set AreaID value
		m_pArchiver->writeSerial(&(pPositions[i].m_uiIndex), Utility::m_AreaIndexArchiveSize);

		//set offset value
		m_pArchiver->writeSerial(&(pPositions[i].m_uiOffset), Utility::m_AreaOffsetArchiveSize);
	}
	
	//for convenience in caculating field length, but needn't store it
	if(bIsArray_) m_pArchiver->writeSerial(&(pPositions[uiPositionCount_].m_uiIndex), Utility::m_AreaIndexArchiveSize);
	//get the last offset value
	m_pArchiver->writeSerial(&(pPositions[uiPositionCount_].m_uiOffset), Utility::m_AreaOffsetArchiveSize);
}

//	FUNCTION public
//	Record2::VariableData::getSimpleVariable -- getSimpleVariable
//
//	NOTES
//		-- getSimpleVariable
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
VariableData::
getSimpleVariable(const FieldInfo& cInfo_, Common::Data& cData_)
{
	//get the index of selected field
	Utility::FieldNum uiFieldIndex = cInfo_.getFieldIndex();
	//get fieldlength
	Utility::FieldLength iFieldLength = getLengthFromArea(uiFieldIndex, *m_pArchiver, m_pPositions);
	//if is null data
	if(iFieldLength == 0)
	{
		cData_.setNull();

#ifdef DEBUG
		SydRecordDebugMessage
			<< "getSimpleVariable: FieldID = " << uiFieldIndex
			<< " skip NULL"
			<< ModEndl;
#endif
		return;
	}


	//reset DirectArea position for random accessing
	m_pArchiver->setPosition(m_pPositions[uiFieldIndex]);

#ifdef DEBUG
	SydRecordDebugMessage
		<< "getSimpleVariable: FieldID = " << uiFieldIndex
		<< " size = " << iFieldLength
		<< ModEndl;
#endif
	//read UncompressedSize first
	Utility::FieldLength iUncompressedSize = 0;
	m_pArchiver->readSerial(&iUncompressedSize, Utility::m_FieldLengthArchiveSize);

	//remove uncompressed size
	iFieldLength -= Utility::m_FieldLengthArchiveSize;
	//get real value
	if (cInfo_.getDataType()._name == Common::DataType::String)
	{
		Common::StringData& cStringData
			= _SYDNEY_DYNAMIC_CAST(Common::StringData&, cData_);
		//get real value
		cStringData.setDumpedValue(*m_pArchiver, iFieldLength, cInfo_.getDataType()._encodingForm);
	}
	else cData_.setDumpedValue(*m_pArchiver, iFieldLength);
	//m_pArchiver->readSerial(&cData_, iFieldLength);
}


//	FUNCTION public
//	Record2::VariableData::setSimpleVariable -- setSimpleVariable
//
//	NOTES
//		-- setSimpleVariable
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
VariableData::
setSimpleVariable(const FieldInfo& cInfo_, const Common::DataArrayData::Pointer& pData_)
{
	//get the index of selected field
	Utility::FieldNum uiFieldIndex = cInfo_.getFieldIndex();
	//get last position info and set them
	m_pPositions[uiFieldIndex].m_uiIndex = m_pArchiver->getCurrentIndex();
	m_pPositions[uiFieldIndex].m_uiOffset = m_pArchiver->getCurrentOffset();

	if(pData_->isNull())	//if is null data then do nothing
	{
#ifdef DEBUG
		SydRecordDebugMessage
			<< "setSimpleVariable: FieldID = " << uiFieldIndex
			<< " skip NULL"
			<< ModEndl;
#endif
		return;
	}

	// There is a size before and after compression in the head for variable-length. 
	// but may be split into 2 areas, so...
	m_pArchiver->writeSerial(&(cInfo_.getAdditionInfo().m_iUncompressedSize), Utility::m_FieldLengthArchiveSize);

	//set real value
	if (cInfo_.getDataType()._name == Common::DataType::String)
	{
		Common::StringData& cStringData
			= _SYDNEY_DYNAMIC_CAST(Common::StringData&, *pData_);
		//get real value
		cStringData.dumpValue(*m_pArchiver, cInfo_.getDataType()._encodingForm);
	}
	else
	{
		pData_->dumpValue(*m_pArchiver);
	}
	//m_pArchiver->writeSerial(&cData_, iFieldLength);

#ifdef DEBUG
	SydRecordDebugMessage
		<< "setSimpleVariable: FieldID = " << uiFieldIndex
		<< " size = " << const_cast<FieldInfo&>(cInfo_).getLength()
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Record2::VariableData::getFixedArray -- getFixedArray
//
//	NOTES
//		-- getFixedArray
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
VariableData::
getFixedArray(const FieldInfo& cInfo_, Common::Data& cData_)
{
	//get the index of selected field
	Utility::FieldNum uiFieldIndex = cInfo_.getFieldIndex();
	//get fieldlength
	Utility::FieldLength iFieldLength = getLengthFromArea(uiFieldIndex, *m_pArchiver, m_pPositions);
	//if is null data
	if(iFieldLength == 0)
	{
		cData_.setNull();

#ifdef DEBUG
		SydRecordDebugMessage
			<< "getFixedArray: FieldID = " << uiFieldIndex
			<< " skip NULL"
			<< ModEndl;
#endif
		return;
	}

	// The array is always DataArrayData. 
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Array);
	// is fixed array
	const DataManagerType& cElementType = cInfo_.getElementType();
	; _SYDNEY_ASSERT(!cElementType.isVariable());

	Common::DataArrayData& cDataArray = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData&, cData_);
	cDataArray.clear();	// initializes it. 
	cDataArray.setNull(false);

	//get schema element count, but maybe != stored count
	//Utility::ElementNum eleNum = cInfo_.getElementNumber();
	//if (eleNum == 0) return;

	//reset DirectArea position for seqence accessing
	m_pArchiver->setPosition(m_pPositions[uiFieldIndex]);

	// read element number
	Utility::FieldNum realNum = 0;
	m_pArchiver->readSerial(&realNum, Utility::m_ElementNumArchiveSize);
	//; _SYDNEY_ASSERT( eleNum >= realNum);
	if (realNum == 0) return;	//array has no element bug not null

	//There is null bitmap in the head. 
	//there is not good for temporary memory
	Utility::Size bitmapSize = getBitmapSize(realNum);
	{
	Common::AutoArrayPointer<char> pPointer = new char[bitmapSize];

	//read null info from area
	m_pArchiver->readSerial(pPointer.get(), bitmapSize);
	//read null array
	readNullBitMap(pPointer.get(), realNum);
	}

	//allocate space
	cDataArray.reserve(realNum);
	Utility::FieldLength iElementLength = cInfo_.getFixedElementLength();

	// The element is made. 
	for (Utility::ElementNum i = 0; i < realNum; ++i) 
	{
		Common::Data::Pointer pElement = FileCommon::DataManager::createCommonData(cElementType);
		if (m_pNulls[i]) 
		{
			// It is NullData. 
			pElement->setNull();
			// The buffer should be able to be read in case of the fixed length. 
			m_pArchiver->skip(iElementLength);

#ifdef DEBUG
			SydRecordDebugMessage
				<< "getFixedArray: ElementID = " << i
				<< " skip NULL"
				<< ModEndl;
#endif
		}
		else // In case of the fixed length
		{
			//set real value
			pElement->setDumpedValue(*m_pArchiver, iElementLength);
			//m_pArchiver->readSerial(&pElement, iElementLength);
		}
		cDataArray.pushBack(pElement);
	}

#ifdef DEBUG
		SydRecordDebugMessage
		<< "getFixedArray: ElementCount="
		<< realNum
		<< ModEndl;
#endif
}

//	FUNCTION public
//	Record2::VariableData::setFixedArray -- setFixedArray
//
//	NOTES
//		-- setFixedArray
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
VariableData::
setFixedArray(const FieldInfo& cInfo_, const Common::DataArrayData::Pointer& pData_)
{
	//get the index of selected field
	Utility::FieldNum uiFieldIndex = cInfo_.getFieldIndex();
	//get last position info and set them
	m_pPositions[uiFieldIndex].m_uiIndex = m_pArchiver->getCurrentIndex();
	m_pPositions[uiFieldIndex].m_uiOffset = m_pArchiver->getCurrentOffset();

	if(pData_->isNull())	//if is null data then do nothing
	{
#ifdef DEBUG
		SydRecordDebugMessage
			<< "setFixedArray: FieldID = " << uiFieldIndex
			<< " skip NULL"
			<< ModEndl;
#endif
		return;
	}

	// The array is always DataArrayData. 
	; _SYDNEY_ASSERT(pData_->getType() == Common::DataType::Array);
	// is fixed array
	const DataManagerType& cElementType = cInfo_.getElementType();
	; _SYDNEY_ASSERT(!cElementType.isVariable());

	//cast to array data
	const Common::DataArrayData* pDataArray =
			_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_.get());
		; _SYDNEY_ASSERT(pDataArray);

	//get schema element count, but maybe != stored count
	Utility::ElementNum eleNum = cInfo_.getElementNumber();

	//reset DirectArea position for seqence accessing
	//m_pArchiver->setPosition(m_pPositions[cInfo_.getIndex()]);

	// write element number
	Utility::ElementNum realNum = pDataArray->getCount();
	m_pArchiver->writeSerial(&realNum, Utility::m_ElementNumArchiveSize);
	; _SYDNEY_ASSERT( eleNum >= realNum);
	if (realNum == 0) return;	//array has no element bug not null

	//There is null bitmap in the head. 
	makeNullBitMap(pDataArray, true);

	//there is not good for temporary memory
	Utility::Size bitmapSize = getBitmapSize(realNum);
	{
	Common::AutoArrayPointer<char> pPointer = new char[bitmapSize];

	//write null info to bitmap
	writeNullBitMap(pPointer.get(), realNum);
	//write bitmap to area
	m_pArchiver->writeSerial(pPointer.get(), bitmapSize);
	}
	
	Utility::FieldLength iElementLength = cInfo_.getFixedElementLength();
	// The element is made. 
	for (Utility::ElementNum i = 0; i < realNum; ++i) 
	{
		const Common::Data& cElement = *pDataArray->getElement(i);
		//Common::Data& cElement = *pDataArray->getElement(i);
		//if(cElement.getType() != cElementType._name)
		//{
		//	Common::Data::Pointer pElement = cElement.cast(cElementType._name);
		//	(const_cast<Common::DataArrayData*>(pDataArray))->setElement(i, pElement);
		//	cElement = *pDataArray->getElement(i);
		//}

		if (m_pNulls[i]) 
		{
			// The buffer should be able to be read in case of the fixed length. 
			m_pArchiver->skip(iElementLength, false);
#ifdef DEBUG
			SydRecordDebugMessage
				<< "setFixedArray: ElementID = " << i
				<< " skip NULL"
				<< ModEndl;
#endif
		}
		else // In case of the fixed length
		{
			cElement.dumpValue(*m_pArchiver);
			//m_pArchiver->writeSerial(&cElement, iElementLength);
#ifdef DEBUG
			SydRecordDebugMessage
			<< "setFixedArray: ElementID = " << i
			<< " size = " << iElementLength
			<< ModEndl;
#endif
		}
	}
}

//	FUNCTION public
//	Record2::VariableData::getVariableArray -- getVariableArray
//
//	NOTES
//		-- getVariableArray
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
VariableData::
getVariableArray(const FieldInfo& cInfo_, Common::Data& cData_)
{
	//get the index of selected field
	Utility::FieldNum uiFieldIndex = cInfo_.getFieldIndex();
	//get fieldlength
	Utility::FieldLength iFieldLength = getLengthFromArea(uiFieldIndex, *m_pArchiver, m_pPositions);
	//if is null data
	if(iFieldLength == 0)
	{
		cData_.setNull();

#ifdef DEBUG
		SydRecordDebugMessage
			<< "getFixedArray: FieldID = " << uiFieldIndex
			<< " skip NULL"
			<< ModEndl;
#endif
		return;
	}

	// The array is always DataArrayData. 
	; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Array);
	// is variable array
	const DataManagerType& cElementType = cInfo_.getElementType();
	; _SYDNEY_ASSERT(cElementType.isVariable());

	//cast to arraydata
	Common::DataArrayData& cDataArray = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData&, cData_);
	cDataArray.clear();
	cDataArray.setNull(false);

	//get schema element count, but maybe != stored count
	//Utility::ElementNum eleNum = cInfo_.getElementNumber();
	//if (eleNum == 0) return;

	//reset DirectArea position for random accessing
	m_pArchiver->setPosition(m_pPositions[uiFieldIndex]);

	//get element header
	Utility::FieldNum realNum = 0;
	Common::AutoArrayPointer<Position> pPositionsAutoPointer = readPositions(realNum, true, *m_pArchiver);
	Position* pPositions = pPositionsAutoPointer.get();

	//; _SYDNEY_ASSERT( eleNum >= realNum);

	//allocate space
	cDataArray.reserve(realNum);

	//get element value
	Utility::FieldLength iElementLength = 0;
	Utility::FieldLength iUncompressedSize = 0;
	for (Utility::ElementNum i = 0; i < realNum; ++i) 
	{
		Common::Data::Pointer pElement = FileCommon::DataManager::createCommonData(cElementType);
		
		//set position
		m_pArchiver->setPosition(pPositions[i]);

		//read dump data
		iElementLength = getLengthFromArea(i, *m_pArchiver, pPositions);
		//may be null data
		if(iElementLength != 0)
		{
			// There is a size before and after compression in the head for variable-length. 
			// but may be split into 2 areas, so...
			m_pArchiver->readSerial(&iUncompressedSize, Utility::m_FieldLengthArchiveSize);

			//remove uncompressed size
			iElementLength -= Utility::m_FieldLengthArchiveSize;

			if (cInfo_.getElementType()._name == Common::DataType::String)
			{
				Common::StringData& cStringData
					= _SYDNEY_DYNAMIC_CAST(Common::StringData&, *pElement);
				//get real value
				cStringData.setDumpedValue(*m_pArchiver, iElementLength, cInfo_.getElementType()._encodingForm);
			}
			else pElement->setDumpedValue(*m_pArchiver, iElementLength);
			//m_pArchiver->readSerial(&pElement, iElementLength);
#ifdef DEBUG
			SydRecordDebugMessage
			<< "getVariableArray: ElementID = " << i
			<< " size = " << iElementLength
			<< ModEndl;
#endif
		}
		else	//when null
		{
			pElement->setNull();
#ifdef DEBUG
			SydRecordDebugMessage
				<< "getVariableArray: ElementID = " << i
				<< " skip NULL"
				<< ModEndl;
#endif
		}

		cDataArray.pushBack(pElement);
	}
}


//	FUNCTION public
//	Record2::VariableData::setVariableArray -- setVariableArray
//
//	NOTES
//		-- setVariableArray
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
VariableData::
setVariableArray(const FieldInfo& cInfo_, const Common::DataArrayData::Pointer& pData_)
{
	//get the index of selected field
	Utility::FieldNum uiFieldIndex = cInfo_.getFieldIndex();
	//get last position info and set them
	m_pPositions[uiFieldIndex].m_uiIndex = m_pArchiver->getCurrentIndex();
	m_pPositions[uiFieldIndex].m_uiOffset = m_pArchiver->getCurrentOffset();

	if(pData_->isNull())	//if is null data then do nothing
	{
#ifdef DEBUG
		SydRecordDebugMessage
			<< "setVariableArray: FieldID = " << uiFieldIndex
			<< " skip NULL"
			<< ModEndl;
#endif
		return;
	}

	// The array is always DataArrayData. 
	; _SYDNEY_ASSERT(pData_->getType() == Common::DataType::Array);
	// is variable array
	const DataManagerType& cElementType = cInfo_.getElementType();
	; _SYDNEY_ASSERT(cElementType.isVariable());

	//cast to array data
	const Common::DataArrayData* pDataArray =
			_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_.get());
		; _SYDNEY_ASSERT(pDataArray);

	//get schema element count, but maybe != stored count
	Utility::ElementNum eleNum = cInfo_.getElementNumber();

	// write element number
	Utility::ElementNum realNum = pDataArray->getCount();
	//m_pArchiver->writeSerial(&realNum, Utility::m_ElementNumArchiveSize);
	; _SYDNEY_ASSERT( eleNum >= realNum);

	//allocate position space
	Common::AutoArrayPointer<Position> pPositionsAutoPointer = readPositions(realNum, true, *m_pArchiver, true);
	Position* pPositions = pPositionsAutoPointer.get();

	//get element value
	Utility::FieldLength iElementLength = 0;
	Utility::FieldLength iUncompressedSize = 0;
	for (Utility::ElementNum i = 0; i < realNum; ++i) 
	{
		//set element header position
		pPositions[i].m_uiIndex = m_pArchiver->getCurrentIndex();
		pPositions[i].m_uiOffset = m_pArchiver->getCurrentOffset();
		
		const Common::Data& cElement = *pDataArray->getElement(i);
		//if is null, do nothing
		if(!cElement.isNull())
		{
			//get Uncompressed and realsize
			getVariableSize(cElement, cElementType, iUncompressedSize, iElementLength);
			// There is a size before and after compression in the head for variable-length. 
			// but may be split into 2 areas, so...
			m_pArchiver->writeSerial(&iUncompressedSize, Utility::m_FieldLengthArchiveSize);

			if (cInfo_.getElementType()._name == Common::DataType::String)
			{
				const Common::StringData& cStringData
					= _SYDNEY_DYNAMIC_CAST(const Common::StringData&, cElement);
				//get real value
				cStringData.dumpValue(*m_pArchiver, cInfo_.getElementType()._encodingForm);
			}
			else
			{
				cElement.dumpValue(*m_pArchiver);
			}
			//m_pArchiver->writeSerial(&pElement, iElementLength);
#ifdef DEBUG
			SydRecordDebugMessage
			<< "setVariableArray: ElementID = " << i
			<< " size = " << iElementLength
			<< ModEndl;
#endif
		}
#ifdef DEBUG
		else
		{
			SydRecordDebugMessage
				<< "setVariableArray: ElementID = " << i
				<< " skip NULL"
				<< ModEndl;
		}
#endif
	}

	//set the last position info
	pPositions[realNum].m_uiIndex = m_pArchiver->getCurrentIndex();
	pPositions[realNum].m_uiOffset = m_pArchiver->getCurrentOffset();

	//set element header
	m_pArchiver->setPosition(m_pPositions[uiFieldIndex]);
	writePositions(pPositions, realNum, true);
	
	//move offset to buffer end
	m_pArchiver->setPosition(pPositions[realNum]);
}


//	FUNCTION public
//	Record2::DataAccess::makeInsertSize -- makeInsertSize
//
//	NOTES
//		-- only used in Insert mode
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::Size
VariableData::
makeInsertSize(const Common::DataArrayData* pTuple_)
{
	//must ==
	//; _SYDNEY_ASSERT(pTuple_->getCount() == m_cTargetFields.getSize() + m_uiSelectedFixedCount);

	//get fixed header size
	Utility::Size uiSize = Utility::m_AreaIDNumArchiveSize;
	uiSize += getPositionSize(m_iFieldNum, false);

	//get field data size
	for(Utility::FieldNum i = 0; i < m_iFieldNum; ++ i)
	{
		const FieldInfo& cInfo = *(m_cTargetFields[i]);
		//data size
		uiSize += getLengthFromData(*(pTuple_->getElement(cInfo.getDataIndex())), cInfo);
	}

	return uiSize;
}

//	FUNCTION public
//	Record2::VariableData::makeUpdateSize -- makeUpdateSize
//
//	NOTES
//		-- used in update operation
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::Size 
VariableData::
makeUpdateSize(const Common::DataArrayData* pTuple_, 
			   AreaArchiver& cArchiver_, Position* pPositions_)
{
	//get fixed header size
	Utility::Size uiSize = Utility::m_AreaIDNumArchiveSize;
	uiSize += getPositionSize(m_iFieldNum, false);
	//get field data size
	for(Utility::FieldNum i = 0, j = 0; i < m_iFieldNum; ++ i)
	{
		const FieldInfo& cInfo = *(m_cTargetFields[i]);
		//if selected, get the new data size
		if(cInfo.isSelected())
		{
			uiSize += getLengthFromData(*(pTuple_->getElement(cInfo.getDataIndex())), cInfo);
			++ j;
		}
		else	//get the old data size
		{
			uiSize += getLengthFromArea(i, cArchiver_, pPositions_);
		}
	}

	return uiSize;
}

//	FUNCTION private
//	Record::VariableIterator::getLengthFromData --
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data& cData_
//		Record::Utility::FieldNum iFieldID_
//			
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::FieldLength
VariableData::
getLengthFromData(const Common::Data& cData_, const FieldInfo& cFieldInfo_)
{
	//may be null data
	if(cData_.isNull()) return 0;

	//initialize length : uncompress or element count length
	Utility::FieldLength uiDataFieldLength = Utility::m_FieldLengthArchiveSize;
	FieldInfo::AdditionInfo cAdditionInfo;

	if (!cFieldInfo_.isArray()) 
	{
		// When it isn't an array
		; _SYDNEY_ASSERT(cData_.getType() != Common::DataType::Array);

		// The size before and after compression is obtained.
		Utility::FieldLength uiFieldLength;
		getVariableSize(cData_,
						cFieldInfo_.getDataType(),
						cAdditionInfo.m_iUncompressedSize,
						uiFieldLength);
		uiDataFieldLength += uiFieldLength;
	} 
	else 
	{
		// is array
		; _SYDNEY_ASSERT(cData_.getType() == Common::DataType::Array);

		const Common::ArrayData* pArray = _SYDNEY_DYNAMIC_CAST(const Common::ArrayData*, &cData_);
		; _SYDNEY_ASSERT(pArray);

		//get element count
		cAdditionInfo.m_uiDataElementNum = pArray->getCount();

		//is fixed array
		if (!cFieldInfo_.isVariableArray()) 
		{
			// The size calculates from the type for the fixed length element. 
			// Size of null bitmap + element number * type
			uiDataFieldLength += getBitmapSize(cAdditionInfo.m_uiDataElementNum)
							+ (cAdditionInfo.m_uiDataElementNum	* cFieldInfo_.getFixedElementLength());
		} 
		else	//is variable array
		{
			// The element is actually examined for a variable-length element. 
			// Data corresponding to the array is always DataArrayData. 
			; _SYDNEY_ASSERT(pArray->getElementType() == Common::DataType::Data);

			const DataManagerType& cElementType = cFieldInfo_.getElementType();
			const Common::DataArrayData* pDataArray = _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, &cData_);
			; _SYDNEY_ASSERT(pDataArray);

			// The dump size of each data that is not Null is totaled. 
			// The size of direct area index and id is added. 
			Utility::Size iSize = getPositionSize(cAdditionInfo.m_uiDataElementNum, true);

			for (int i = 0; i < cAdditionInfo.m_uiDataElementNum; ++i) 
			{
				const Common::Data* pData = pDataArray->getElement(i).get();
				if (pData && !pData->isNull()) 
				{
					; _SYDNEY_ASSERT(pData->isAbleToDump());
					
					//get Uncompressed and realsize
					Utility::FieldLength uiFieldLength, uiUncompressedSize;
					getVariableSize(*pData, cFieldInfo_.getElementType(), uiUncompressedSize, uiFieldLength);
					iSize += uiFieldLength;
					// should we add uncompressed size?

					//sometimes the fieldlength == 0
					//!!!!!!!!!!!
					//if a variable array include v = '', the length=0
					//may be here has a potential problem
					//!!!!!!!!!!!
					//if(uiFieldLength == 0) const_cast<Common::Data*>(pData)->setNull();
					//else iSize += Utility::m_FieldLengthArchiveSize;
					iSize += Utility::m_FieldLengthArchiveSize;
				}
			}

			uiDataFieldLength += iSize;
		}
	}

	//set it to fieldinfo
	FieldInfo& cFieldInfo = const_cast<FieldInfo&>(cFieldInfo_);
	cFieldInfo.setLength(uiDataFieldLength);
	cFieldInfo.setAdditionInfo(cAdditionInfo);

	return uiDataFieldLength;
}

//	FUNCTION public
//	Record2::VariableData::getLengthFromArea -- getLengthFromArea
//
//	NOTES
//		-- get variable field/element length
//
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::FieldLength
VariableData::
getLengthFromArea(Utility::FieldNum uiIndex_, AreaArchiver& cArchiver_, Position* pPositions_)
{
	//; _SYDNEY_ASSERT( uiIndex_ < m_iFieldNum && pPositions_);
	; _SYDNEY_ASSERT(pPositions_);

	//some temporary var
	Utility::AreaIndex		currentIndex =		pPositions_[uiIndex_].m_uiIndex;
	Utility::AreaIndex		nextIndex =			pPositions_[uiIndex_ + 1].m_uiIndex;
	Utility::AreaOffset	currentOffset =	pPositions_[uiIndex_].m_uiOffset;
	Utility::AreaOffset	nextOffset =		pPositions_[uiIndex_ + 1].m_uiOffset;
	//if in same area
	if(currentIndex == nextIndex)
	{
		return nextOffset - currentOffset;
	}
	else
	{
		if(currentIndex > nextIndex)
		{
			SydErrorMessage
				<< "DirectArea header info error, current index is: "
				<< (int)currentIndex
				<< "next index is: "
				<< (int)nextIndex
				<< ModEndl;

			//may be Exception::HeaderInfoError
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		//may be iLength < 0 during procedure, so used int instead of Utility::FieldLength
		int iLength = nextOffset - currentOffset;
		//get first area
		AreaArchiver::LinkedArea* pAreas = cArchiver_.getLinkedAreas() + currentIndex;
		//get the areaCount
		Utility::AreaIDNum uiAreaCount =  nextIndex - currentIndex;
		for(Utility::AreaIDNum i = 0; i < uiAreaCount; ++ i)
		{
			iLength += (*pAreas).m_uiSize;
			++ pAreas;
		}

		return iLength;
	}
}

//	FUNCTION public
//	Record2::VariableData::resetVariableArrayPositions -- resetVariableArrayPositions
//
//	NOTES
//		-- used in update operation
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
VariableData::
resetVariableArrayPositions(Position& cNewPosition_, AreaArchiver& cArchiver_)
{
	//set to real position
	m_pArchiver->setPosition(cNewPosition_);
	//skip element count : next step must read uiPositionCount
	//m_pArchiver->skip(Utility::m_ElementNumArchiveSize);

	//for new positions equal to old positions 
	Utility::ElementNum uiPositionCount = 0;
	Common::AutoArrayPointer<Position> pPositionsAutoPointer = readPositions(uiPositionCount, true, *m_pArchiver);
	Position* pPositions = pPositionsAutoPointer.get();
	
	//another method
	Utility::AreaIndex uiIndex = m_pArchiver->getCurrentIndex();
	Utility::AreaOffset uiOffset = m_pArchiver->getCurrentOffset();
	AreaArchiver::LinkedArea* pLinkedArea = m_pArchiver->getLinkedAreas();
	Utility::AreaSize uiAreaSize = pLinkedArea[uiIndex].m_uiSize;

	//reserve the first old pos
	Position prevPos(uiIndex, uiOffset);
	Utility::FieldLength fieldLength = 0;
	for(Utility::ElementNum i = 0; i < uiPositionCount; ++ i)
	{
		prevPos.m_uiIndex = uiIndex;
		prevPos.m_uiOffset = uiOffset;

		fieldLength = getLengthFromArea(i, cArchiver_, pPositions);

		Utility::AreaSize uiRestSize = uiAreaSize - uiOffset;
		while (fieldLength)
		{
			Utility::AreaSize len = static_cast<Utility::AreaSize>
				((uiRestSize < fieldLength) ? uiRestSize : fieldLength);
			fieldLength -= len;
			uiRestSize -= len;

			//if need more Area, get next and do it
			if (uiRestSize == 0 && fieldLength != 0)
			{
				; _SYDNEY_ASSERT(m_pArchiver->getAreaCount() > uiIndex + 1);

				uiAreaSize = pLinkedArea[++uiIndex].m_uiSize;
				uiRestSize = uiAreaSize;
			}
		}
		//reset offset, used in sequence access
		uiOffset = uiAreaSize - uiRestSize;

		//set position
		pPositions[i].m_uiIndex = prevPos.m_uiIndex;
		pPositions[i].m_uiOffset = prevPos.m_uiOffset;
	}
	//set the last position
	pPositions[uiPositionCount].m_uiIndex = uiIndex;
	pPositions[uiPositionCount].m_uiOffset = uiOffset;

	//write position data into Archiver
	m_pArchiver->setPosition(cNewPosition_);
	writePositions(pPositions, uiPositionCount, true);

	//move offset to buffer end
	m_pArchiver->setPosition(pPositions[uiPositionCount]);
}

//	FUNCTION public
//	Record2::VariableData::getPositionSize -- getPositionSize
//
//	NOTES
//		-- get positions block size
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::Size 
VariableData::
getPositionSize(const Utility::FieldNum uiPositionCount_, bool isArray_)
{
	//get fixed header size
	//if it's array, size may be 	Utility::m_ElementNumArchiveSize
	//but it equals to Utility::m_AreaIDNumArchiveSize
	//Utility::Size uiSize = Utility::m_AreaIDNumArchiveSize;

	Utility::Size uiSize = ( Utility::m_AreaIndexArchiveSize 
							+ Utility::m_AreaOffsetArchiveSize) 
							* uiPositionCount_;

	//array should store index but field need not
	if(isArray_) uiSize += Utility::m_AreaIndexArchiveSize;
	//add the last offset
	uiSize += Utility::m_AreaOffsetArchiveSize;
	return uiSize;
}

//
//	Copyright (c) 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
