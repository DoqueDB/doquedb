// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaArchiver.cpp -- AreaArchiver.cpp
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
#include "Exception/Unexpected.h"
#include "Common/Message.h"

#include "Record2/AreaArchiver.h"
#include "Record2/Debug.h"

_SYDNEY_USING

_SYDNEY_RECORD2_USING

namespace
{
	//ObjectID::m_ArchiveSize
	const ModSize _ObjectIDArchiveSize = sizeof(Common::ObjectIDData::FormerType) + 
										 sizeof(Common::ObjectIDData::LatterType);
}

//	FUNCTION public
//	Record2::AreaArchiver::AreaArchiver -- constructor
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

AreaArchiver::
AreaArchiver(AreaManager* pAreaManager_)
	:m_pAreaManager(pAreaManager_),
	m_pLinkedAreas(0),
	m_uiAreaCount(0),
	m_pAreaPointer(0),
	m_uiAreaSize(0),
	m_uiCurrentOffset(0),
	m_pProgress(0),
	m_iCurrentAreaID(ObjectID::m_UndefinedValue)
{
 
}

//	FUNCTION public
//	Record2::AreaArchiver::AreaArchiver -- destructor
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

AreaArchiver::
~AreaArchiver()
{
	detachArea();
	if(m_pLinkedAreas)
	{
		delete[] m_pLinkedAreas;
		m_pLinkedAreas = 0;
	}
}

//	FUNCTION public
//	Record2::AreaArchiver::read -- read data from area
//
//	NOTES
//		-- read data from area
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

int 
AreaArchiver::
readSerial(void* pBuf_, ModSize uiSize_, ModSerialIO::DataType cType_)
{
	; _SYDNEY_ASSERT(pBuf_ && uiSize_ != 0);
	; _SYDNEY_ASSERT(m_pAreaPointer);
	int iDumpSize = uiSize_;

	// will be re-write
	char* pBuffer = syd_reinterpret_cast<char*>(pBuf_);
	//get the buffer position
	const char* pConstPointer = m_pConstAreaPointer;
	pConstPointer += m_uiCurrentOffset;
	Utility::AreaSize uiRestSize = m_uiAreaSize - m_uiCurrentOffset;
	while (uiSize_)
	{
		//get the longer length
		Utility::AreaSize len = static_cast<Utility::AreaSize>
			((uiRestSize < uiSize_) ? uiRestSize : uiSize_);
		Os::Memory::copy(pBuffer, pConstPointer, len);

		//
		uiSize_ -= len;
		uiRestSize -= len;
		pBuffer += len;
		pConstPointer += len;

		//if need more Area, get next and do it
		if (uiRestSize == 0 && uiSize_ != 0)
		{
			getNextArea(true);
			pConstPointer = m_pConstAreaPointer;
			uiRestSize = m_uiAreaSize;
		}
	}

	//reset offset, used in sequence access
	m_uiCurrentOffset = m_uiAreaSize - uiRestSize;

	//the size must == 0
	; _SYDNEY_ASSERT(uiSize_ == 0);
	//return rest size
	return iDumpSize;
}

//	FUNCTION public
//	Record2::AreaArchiver::write -- write data into area
//
//	NOTES
//		-- write data into area
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

int 
AreaArchiver::
writeSerial(const void* pBuf_, ModSize uiSize_, ModSerialIO::DataType cType_)
{
	; _SYDNEY_ASSERT(pBuf_ && uiSize_ != 0);
	; _SYDNEY_ASSERT(m_pAreaPointer);
	int iDumpSize = uiSize_;

	// will be re-write

	const char* pBuffer = syd_reinterpret_cast<const char*>(pBuf_);
	//get the buffer position
	char* pPointer = m_pAreaPointer;
	pPointer += m_uiCurrentOffset;
	Utility::AreaSize uiRestSize = m_uiAreaSize - m_uiCurrentOffset;
	while (uiSize_)
	{
		Utility::AreaSize len = static_cast<Utility::AreaSize>
			((uiRestSize < uiSize_) ? uiRestSize : uiSize_);
		Os::Memory::copy(pPointer, pBuffer, len);

		uiSize_ -= len;
		uiRestSize -= len;
		pBuffer += len;
		pPointer += len;

		//if need more Area, get next and do it
		if (uiRestSize == 0 && uiSize_ != 0)
		{
			getNextArea(false);
			pPointer = m_pAreaPointer;
			uiRestSize = m_uiAreaSize;
		}
	}


	//reset offset, used in sequence access
	m_uiCurrentOffset = m_uiAreaSize - uiRestSize;

	//the size must == 0
	; _SYDNEY_ASSERT(uiSize_ == 0);
	//return rest size
	return iDumpSize;
}

//	FUNCTION public
//	Record2::AreaArchiver::copy -- copy data
//
//	NOTES
//		-- copy data
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
AreaArchiver::
copy(AreaArchiver* pDest_, AreaArchiver* pSrc_, Utility::Size uiSize_)
{
	; _SYDNEY_ASSERT(pDest_ && pSrc_ && uiSize_ != 0);

	//get the dest buffer
	char* pDestBuf = pDest_->m_pAreaPointer;
	pDestBuf += pDest_->m_uiCurrentOffset;
	Utility::AreaSize uiDestRestSize = pDest_->m_uiAreaSize - pDest_->m_uiCurrentOffset;

	//get the src buffer
	const char* pSrcBuf = syd_reinterpret_cast<const char*>(pSrc_->m_pAreaPointer);
	pSrcBuf += pSrc_->m_uiCurrentOffset;
	Utility::AreaSize uiSrcRestSize = pSrc_->m_uiAreaSize - pSrc_->m_uiCurrentOffset;

	while (uiSize_)
	{
		//get the smaller size
		Utility::AreaSize len = static_cast<Utility::AreaSize>
			((uiDestRestSize < uiSize_) ? uiDestRestSize : uiSize_);
		len = (uiSrcRestSize < len) ? uiSrcRestSize : len;

		//memory copy
		Os::Memory::copy(pDestBuf, pSrcBuf, len);

		//re-caculate size
		uiSize_ -= len;
		uiDestRestSize -= len;
		uiSrcRestSize -= len;

		//move pointer of buffer
		pDestBuf += len;
		pSrcBuf += len;

		//if need more Area, get next and do it
		if (uiDestRestSize == 0 && uiSize_ != 0)
		{
			pDest_->getNextArea(false);
			pDestBuf = pDest_->m_pAreaPointer;
			uiDestRestSize = pDest_->m_uiAreaSize;
		}

		//if need more Area, get next and do it
		if (uiSrcRestSize == 0 && uiSize_ != 0)
		{
			pSrc_->getNextArea(true);
			pSrcBuf = pSrc_->m_pAreaPointer;
			uiSrcRestSize = pSrc_->m_uiAreaSize;
		}
	}


	//reset offset, used in sequence access
	pDest_->m_uiCurrentOffset = pDest_->m_uiAreaSize - uiDestRestSize;
	pSrc_->m_uiCurrentOffset = pSrc_->m_uiAreaSize - uiSrcRestSize;

	//the size must == 0
	; _SYDNEY_ASSERT(uiSize_ == 0);
}


//	FUNCTION public
//	Record2::AreaArchiver::skip -- skip
//
//	NOTES
//		-- skip
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

ModSize  
AreaArchiver::
skip(ModSize uiSize_, bool bReadOnly_)
{
	//should consider skip multi-area
	Utility::AreaSize uiRestSize = m_uiAreaSize - m_uiCurrentOffset;
	while (uiSize_)
	{
		Utility::AreaSize len = static_cast<Utility::AreaSize>
			((uiRestSize < uiSize_) ? uiRestSize : uiSize_);
		uiSize_ -= len;
		uiRestSize -= len;

		//if need more Area, get next and do it
		if (uiRestSize == 0 && uiSize_ != 0)
		{
			getNextArea(bReadOnly_);
			uiRestSize = m_uiAreaSize;
		}
	}

	//reset offset, used in sequence access
	m_uiCurrentOffset = m_uiAreaSize - uiRestSize;

	return m_uiCurrentOffset;
}

//	FUNCTION public
//	Record2::VariableData::readLinkedArea -- readLinkedArea
//
//	NOTES
//		-- readLinkedArea
//	ARGUMENTS
//		const ObjectID::Value iFirstObjectID_ 
//			First area's objectid
//		Operation::Value eOperation_       
//			operation on this area after attach
//  		 Admin::Verification::Treatment::Value eTreatment_
//			option of corresponding verification
//  		Admin::Verification::Progress& cProgress_
//			progress of corresponding verification
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::AreaSize  
AreaArchiver::
setFirstArea(const ObjectID::Value iFirstObjectID_, 
			 Manager::Operation::Value eOperation_ /*= Manager::Operation::Read*/,
			 Admin::Verification::Treatment::Value eTreatment_/*=0*/,
			 Admin::Verification::Progress* pProgress_/*=0*/)
{
	//set first direct id
	m_iCurrentAreaID = iFirstObjectID_;
	m_pProgress = pProgress_;
	m_eTreatment = eTreatment_;
	//attach Direct Area
	attachArea(eOperation_);
	//becuase read header migth move to the next area
	Utility::AreaSize uiFirstAreaSize = m_uiAreaSize;

	//initialize members
	m_uiCurrentOffset = 0;
	//the current area is 0
	m_uiCurrentIndex = 0;

	//free the older areas
	if(m_pLinkedAreas != 0) delete[] m_pLinkedAreas;
	//get areanum, 4 bytes
	//m_uiAreaCount be included by the first DirectArea
	readSerial(&m_uiAreaCount, Utility::m_AreaIDNumArchiveSize);
	//allocate pLinkedAreas space
	m_pLinkedAreas = new LinkedArea[m_uiAreaCount];

	//set first LinkedArea
	m_pLinkedAreas[0].m_iObjectID = iFirstObjectID_;
	m_pLinkedAreas[0].m_uiSize = m_uiAreaSize;

	//get the areaid value, from the second area
	for(Utility::AreaIDNum i = 1; i < m_uiAreaCount; ++ i)
	{
		//notice!!!
		static char cBuf[_ObjectIDArchiveSize];
		//get direct area value
		readSerial(cBuf, ObjectID::m_ArchiveSize);
		ObjectID::readValue(cBuf, m_pLinkedAreas[i].m_iObjectID);

		//get AreaSize value
		readSerial(&(m_pLinkedAreas[i].m_uiSize), Utility::m_AreaSizeArchiveSize);
	}	

	return uiFirstAreaSize;
}

//	FUNCTION public
//	Record2::AreaArchiver::setFirstObjectID -- setFirstObjectID
//
//	NOTES
//		-- setFirstObjectID
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::AreaOffset  
AreaArchiver::
skipLinkedAreas(const ObjectID::Value iFirstObjectID_)
{
	//set first direct id
	m_iCurrentAreaID = iFirstObjectID_;
	//attach Direct Area
	attachArea(Manager::Operation::Read);
	//initialize members
	m_uiCurrentOffset = 0;
	//the current area is 0
	m_uiCurrentIndex = 0;

	Utility::Size uiSize = Utility::m_AreaIDNumArchiveSize;
	uiSize += (ObjectID::m_ArchiveSize  
			+ Utility::m_AreaSizeArchiveSize)
			* (m_uiAreaCount - 1);	//areacount not include the first area

	return static_cast<Utility::AreaOffset>(skip(uiSize));
}


//	FUNCTION public
//	Record2::AreaArchiver::setPosition -- setPosition
//
//	NOTES
//		-- setPosition
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
AreaArchiver::
setPosition(const VariableData::Position& cPosition_)
{
	//set the first LinkedArea info
	//if DirectArea has attached, needn't do it again
	if(!(m_uiCurrentIndex == cPosition_.m_uiIndex && m_pAreaPointer))
	{
		//if(cPosition_.m_uiIndex==0) can it do -1?
		//Yes, -1 and +1 will get 0
		//set area index
		m_uiCurrentIndex = cPosition_.m_uiIndex - 1;
		//get next area
		getNextArea(true);
	}		

	//reserve offset
	//used in sequence accessing
	//if(uiOffset_ != Utility::m_UndefinedAreaOffset)
	m_uiCurrentOffset = cPosition_.m_uiOffset;

}

//	FUNCTION private
//	Record2::AreaArchiver::getNextArea -- get next area
//
//	NOTES
//		-- get next area
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
AreaArchiver::
getNextArea(bool isRealOnly_)
{
	//detach last DirectArea
	detachArea();

	//index should ++
	++ m_uiCurrentIndex;

	; _SYDNEY_ASSERT(m_pLinkedAreas && m_uiCurrentIndex < m_uiAreaCount);

	//assign current id
	m_iCurrentAreaID = m_pLinkedAreas[m_uiCurrentIndex].m_iObjectID;

	Manager::Operation::Value eValue = isRealOnly_ 	? 
		Manager::Operation::Read : Manager::Operation::Write;

	//attach Direct Area
	attachArea(eValue);
	
	//must equal
	if(m_pLinkedAreas[m_uiCurrentIndex].m_uiSize != m_uiAreaSize)
	{
		SydErrorMessage
			<< "DirectArea size error, existed size is: "
			<< (int)m_uiAreaSize
			<< "wanted size is: "
			<< (int)(m_pLinkedAreas[m_uiCurrentIndex].m_uiSize)
			<< ModEndl;

		//may be Exception::ObjectStatusError
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	//; _SYDNEY_ASSERT(m_pLinkedAreas[m_uiCurrentIndex].m_uiSize == m_uiAreaSize);
	
	//set offset to begin
	m_uiCurrentOffset = 0;
}

//	FUNCTION private
//	Record2::AreaArchiver::attachArea -- attach DirectArea, 
//
//	NOTES
//		-- attach DirectArea, 
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
AreaArchiver::
attachArea(Manager::Operation::Value eOperation_)
{
	; _SYDNEY_ASSERT( !ObjectID::isInvalid(m_iCurrentAreaID));

	//attach DirectArea
	PhysicalArea cPhysicalArea = 
		m_pAreaManager->attach(m_iCurrentAreaID, eOperation_, m_eTreatment, m_pProgress);
	
	if(eOperation_ == Manager::Operation::Read || eOperation_ == Manager::Operation::Verify)
	{
		m_pConstAreaPointer = static_cast<const char*> (cPhysicalArea.operator const void*());
	}
	else 
	{
		cPhysicalArea.dirty();
		m_pAreaPointer = static_cast<char*> (cPhysicalArea.operator void*());
	}
	
	//must equal
	//; _SYDNEY_ASSERT(m_pLinkedAreas[0].m_uiSize == pPhysicalArea->getSize());
	m_uiAreaSize = static_cast<Utility::AreaSize>(cPhysicalArea.getSize());

}

//	FUNCTION private
//	Record2::AreaArchiver::detachArea -- detachArea DirectArea, 
//
//	NOTES
//		-- detachArea DirectArea, 
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
AreaArchiver::
detachArea(bool bAll_)
{
	if(!bAll_)
	{
		//detach last DirectArea
		if(m_pAreaPointer && !ObjectID::isInvalid(m_iCurrentAreaID))
		{
			m_pAreaManager->detach(m_iCurrentAreaID);
		}
	}
	else if(m_pLinkedAreas)
	{
		//get the areaid value, from the second area
		for(Utility::AreaIDNum i = 0; i < m_uiAreaCount; ++ i)
		{
			m_pAreaManager->detach(m_pLinkedAreas[i].m_iObjectID);
		}			
	}

	m_iCurrentAreaID = ObjectID::m_UndefinedValue;
	m_pAreaPointer = 0;
}

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
