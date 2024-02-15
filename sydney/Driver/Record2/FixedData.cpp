// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FixedData.cpp -- Class for fixed data operation, managed fixed fields header and data accessing.
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
#include "Record2/FixedData.h"

_SYDNEY_USING

_SYDNEY_RECORD2_USING

//	FUNCTION public
//	Record2::FixedData::FixedData -- constructor
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

FixedData::
FixedData(const FileID::TargetFields& cTargetFields_)
	:FieldData(cTargetFields_),
	m_iCurrentObjectID(ObjectID::m_UndefinedValue)
{
}

//	FUNCTION public
//	Record2::FixedData::~FixedData -- destructor
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

FixedData::
~FixedData()
{

}

//	FUNCTION public
//	Record2::FixedData::read -- read
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
FixedData::
read(Common::DataArrayData* pTuple_, const char* pPointer_)
{
	; _SYDNEY_ASSERT(pTuple_ && pPointer_);

	const char* pPointer = pPointer_;

	//read old null bitmap
	pPointer = readNullBitMap(pPointer, m_iFieldNum);

	//read data from selected fields
	//Utility::FieldNum selFieldNum = m_cTargetFields.getSize();
	//allocate space
	Utility::FieldLength uiOffset = 0;
	for(Utility::FieldNum i = 0; i < m_iFieldNum; ++ i)
	{
		FieldInfo& cInfo = const_cast<FieldInfo&>(*(m_cTargetFields[i]));
		if(cInfo.isSelected())
		{
			//get data index
			int index = cInfo.getDataIndex();
			Common::Data& cData =  *pTuple_->getElement(index);
			if(!m_pNulls[i])
			{
				//if is ObjectID, heap file will store objectid
				if(i == 0 && ObjectID::isObjectIDType(pTuple_, index))
				{
					; _SYDNEY_ASSERT(cInfo.getDataType()._name == 
						pTuple_->getElement(index)->getType());

					ObjectID::Value objValue;
					ObjectID::readValue(pPointer, objValue);
					LogicalFile::ObjectID* pID = _SYDNEY_DYNAMIC_CAST(
						LogicalFile::ObjectID*, &cData);
					//maybe needn't get it
					pID->setValue(objValue);
				}
				else
				{
					readFixedField(pPointer + uiOffset,
						cInfo.getDataType(),
						cData);
				}
			}
			else	//if is null data
			{
				cData.setNull();
			}
#ifdef DEBUG
		SydRecordDebugMessage
			<< "Read fixed field: "
			<< i
			<< " data: "
			<< cData.getString()
			<< ModEndl;
#endif
		}
		//move offset
		uiOffset += cInfo.getLength();
	}
}

//	FUNCTION public
//	Record2::FixedData::insert -- insert
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

void
FixedData::
insert(const Common::DataArrayData* pTuple_, char* pPointer_)
{
	; _SYDNEY_ASSERT(pTuple_ && pPointer_);
	
	//make null bitmap first
	makeNullBitMap(pTuple_);

	char* pPointer = pPointer_;
	//write null bitmap
	pPointer = writeNullBitMap(pPointer, m_iFieldNum);

	//write data for all selected fields
	for(Utility::FieldNum i = 0; i < m_iFieldNum; ++ i)
	{
		FieldInfo& cInfo = const_cast<FieldInfo&>(*(m_cTargetFields[i]));
		if(!m_pNulls[i])	//if not null
		{
			//get data index
			int index = cInfo.getDataIndex();
			const Common::DataArrayData::Pointer& pData = pTuple_->getElement(index);
			//if is ObjectID
			if(ObjectID::isObjectIDType(pTuple_, index))
			{
				; _SYDNEY_ASSERT(cInfo.getDataType()._name == 
					pTuple_->getElement(index)->getType());

				if(i == 0)	//the first objectid
				{
					pPointer += ObjectID::writeValue(pPointer, m_iCurrentObjectID);
				}
				else 
				{
					pPointer += ObjectID::writeValue(pPointer, _SYDNEY_DYNAMIC_CAST(
						LogicalFile::ObjectID*, &*pData)->getValue());
				}
			}
			else	//insert data really
			{
				pPointer = writeFixedField(pPointer, cInfo.getDataType(), *pData);
			}
#ifdef DEBUG
		SydRecordDebugMessage
			<< "Insert fixed field: "
			<< i
			<< " data: "
			<< pData->getString()
			<< ModEndl;
#endif
		}
		else	//null and skip position
		{
			pPointer += cInfo.getLength();
#ifdef DEBUG
		SydRecordDebugMessage
			<< "Skip fixed field during insertion for null-data field: "
			<< i
			<< ModEndl;
#endif
		}
	}
}

//	FUNCTION public
//	Record2::FixedData::update -- update
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
FixedData::
update(const Common::DataArrayData* pTuple_, char* pPointer_)
{
	; _SYDNEY_ASSERT(pTuple_ && pPointer_);

	//read data from selected fields
	char* pPointer = pPointer_;
	
	//read old null bitmap
	readNullBitMap(pPointer, m_iFieldNum);
	//reset null bitmap by data
	makeNullBitMap(pTuple_, false, true);
	//write null bitmap to new buffer
	pPointer = writeNullBitMap(pPointer, m_iFieldNum);

	//needn't update the first field (objectid)
	pPointer += ObjectID::m_ArchiveSize;

	for(Utility::FieldNum i = 1/*, j = 0*/; i < m_iFieldNum; ++ i)
	{
		FieldInfo& cInfo = const_cast<FieldInfo&>(*(m_cTargetFields[i]));
		if(!m_pNulls[i] && cInfo.isSelected())
		{
			//get data index
			int index = cInfo.getDataIndex();
			const Common::DataArrayData::Pointer& pData = 
					pTuple_->getElement(index);
			//if is ObjectID
			if(ObjectID::isObjectIDType(pTuple_, index))
			{
				; _SYDNEY_ASSERT(cInfo.getDataType()._name == 
					pTuple_->getElement(index)->getType());

				ObjectID::writeValue(pPointer, _SYDNEY_DYNAMIC_CAST(
					LogicalFile::ObjectID*, &*pData)->getValue());
			}
			else	//insert data really
			{
				writeFixedField(pPointer, cInfo.getDataType(), *pData);
			}

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Update fixed field: "
			<< i
			<< " data: "
			<< pData->getString()
			<< ModEndl;
#endif
		}
#ifdef DEBUG
		else	//copy old data
		{
		SydRecordDebugMessage
			<< "Copy old fixed data during updation for not changed field: "
			<< i
			<< ModEndl;
		}
#endif
		//move pointer
		pPointer += cInfo.getLength();
	}
}

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
