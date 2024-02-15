// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaManager.cpp -- 
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

#include "Checkpoint/Database.h"
#include "Exception/NotSupported.h"

#include "Record2/Debug.h"
#include "Record2/AreaManager.h"

_SYDNEY_USING
_SYDNEY_RECORD2_USING

namespace 
{
	// header cell size
	//if need multi-area, the size of LinkedArea
	const Utility::Size _uiHeaderCellSize = sizeof(Common::ObjectIDData::FormerType) + 
											sizeof(Common::ObjectIDData::LatterType) + 
											sizeof(Utility::AreaSize);
}

//
//	FUNCTION public
//	Record2::AreaManager::AreaManager -- constructor
//
//	NOTES
//	constructor
//
//	ARGUMENTS
//	const Trans::Transaction&	pTransaction_
//		Transaction descriptor
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
AreaManager::
AreaManager(File& cFileAccess_)
	: Manager(cFileAccess_, Operation::Read)
{
}

//
//	FUNCTION public
//	Record2::AreaManager::~AreaManager -- destructor
//
//	NOTES
//	destructor
//
//	ARGUMENTS
//	None
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
AreaManager::~AreaManager()
{
}

//	FUNCTION public
//	Record2::AreaManager::createArea -- 
//
//	NOTES
//		create some DirectArea when insert a record which including variable fields.
//		may be return multy area when request size is big enough.
//
//	ARGUMENTS
//		Utility::Size uiRequiredSize_
//			Necessary number of bytes
//
//	REUTRN
//		ID of the first object
//
//	EXCEPTIONS

ObjectID::Value  
AreaManager::
createArea(Utility::Size uiRequiredSize_)
{
	ObjectID::Value ullObjectID = ObjectID::m_UndefinedValue;
	//may be < 0
	int iRequiredSize = uiRequiredSize_;
	try
	{
		//allocate the first one
		PhysicalArea cFirstArea = createOneArea(uiRequiredSize_);
		ullObjectID = getObjectIDFromAreaID(cFirstArea.getID());
		//calculate rest size
		iRequiredSize -= cFirstArea.getSize();
		//including the first Area
		Utility::AreaIDNum areaCount = 1;
		
		//rest space should be re-allocate
		if(iRequiredSize > 0)
		{
			//the first area size must bigger than ...
			if(cFirstArea.getSize() <	Utility::m_AreaIDNumArchiveSize + ObjectID::m_ArchiveSize)
			{
				SydErrorMessage
					<< "Allocated directArea size is too small, the minimum size is: "
					<< (int)(Utility::m_AreaIDNumArchiveSize + ObjectID::m_ArchiveSize)
					<< "allocated size is: "
					<< (int)(cFirstArea.getSize())
					<< ModEndl;

				//may be Exception::ObjectSizeError
				_SYDNEY_THROW0(Exception::NotSupported);
			}
			//; _SYDNEY_ASSERT(cFirstArea.getSize() >= 
			//		Utility::m_AreaIDNumArchiveSize +
			//		ObjectID::m_ArchiveSize);

			ModVector<PhysicalArea> vecArea;
			do
			{
				//continue allocating
				iRequiredSize += _uiHeaderCellSize;
				PhysicalArea cNextArea = createOneArea(iRequiredSize);
				iRequiredSize -= cNextArea.getSize();
				vecArea.pushBack(cNextArea);
				++ areaCount;
			}
			while (iRequiredSize > 0);
			
			//store multi area header info
			storeMultiAreaInfo(cFirstArea, vecArea);
		}

		//reserve the area count number
		Os::Memory::copy(cFirstArea.operator void*(), &areaCount, Utility::m_AreaIDNumArchiveSize);
	
#ifdef DEBUG
		SydRecordDebugMessage
			<< "AreaManager create DirectArea ObjectID="
			<< ullObjectID
			<< " the first AreaSize="
			<< cFirstArea.getSize()
			<< " all of required size="
			<< uiRequiredSize_
			<< ModEndl;
#endif

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		freeAreaAll();
		_SYDNEY_RETHROW;
	}

	return ullObjectID;
}

//	FUNCTION public
//	Record2::AreaManager::createOneArea -- createOneArea
//
//	NOTES
//		-- createOneArea
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

PhysicalArea 
AreaManager::
createOneArea(Utility::Size uiRequiredSize_)
{
	//the apply size can't bigger than maxareasize
	Utility::Size uiMaxSize = m_pFile->getMaxStorableAreaSize();
	//if so then should change it
	if(uiRequiredSize_>uiMaxSize) uiRequiredSize_ = uiMaxSize;

	//allocate
	PhysicalArea cArea = m_pFile->allocateArea(m_cTrans, uiRequiredSize_);

#ifdef DEBUG
			SydRecordDebugMessage
			<< "Allocate one direct area, requiredsize="
			<< uiRequiredSize_
			<< " allocated size="
			<< (int)cArea.getSize()
			<< ModEndl;
#endif

	//attach area and reserved in map
	return attach(getObjectIDFromAreaID(cArea.getID()), Operation::Write);

	//return pArea;
}

//	FUNCTION public
//	Record2::AreaManager::storeMultiArea -- storeMultiArea
//
//	NOTES
//		-- storeMultiArea
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
AreaManager::
storeMultiAreaInfo(PhysicalArea cFirstArea_, ModVector<PhysicalArea>& vecArea_)
{
	//current Area which which are writing header info now
	PhysicalArea& cCurrentArea = cFirstArea_;
	Utility::Size iCurrentSize = cCurrentArea.getSize();
	//get buffer pointer
	char* pAreaPointer = static_cast<char*> (cCurrentArea.operator void*());	
	//store ID count
	pAreaPointer += Utility::m_AreaIDNumArchiveSize;
	iCurrentSize -= Utility::m_AreaIDNumArchiveSize;

	//reserve header info (areaid and areasize)
	ModVector<PhysicalArea>::Iterator iterator = vecArea_.begin();
	for(int i = 0; iterator != vecArea_.end(); ++ iterator)
	{
		ObjectID::Value ullNextObjectID = getObjectIDFromAreaID((*iterator).getID());
		Utility::AreaSize iNextSize = (*iterator).getSize();

		//if the DirectArea can store header info
		if(iCurrentSize >= _uiHeaderCellSize)
		{
			//store header info
			pAreaPointer += ObjectID::writeValue(pAreaPointer, ullNextObjectID);
			Os::Memory::copy(pAreaPointer, &iNextSize, Utility::m_AreaSizeArchiveSize);
			pAreaPointer += Utility::m_AreaSizeArchiveSize;
	
			//change size
			iCurrentSize -= _uiHeaderCellSize;
		}
		//if the DirectArea can't store header info
		else	//other way: allocate a temporary buffer...
		{
			//allocate a temporary space
			char cBuf[_uiHeaderCellSize];
			char* pBuf = cBuf;
			pBuf += ObjectID::writeValue(pBuf, ullNextObjectID);
			Os::Memory::copy(pBuf , &iNextSize, Utility::m_AreaSizeArchiveSize);
			
			//move to header
			pBuf = cBuf;

			//store the 2 parts memory
			//copy the former parts of Size
			Os::Memory::copy(pAreaPointer, pBuf, iCurrentSize);
			//move to next
			pBuf += iCurrentSize;
			
			//move to next area to write
			cCurrentArea = *(vecArea_.begin() + i++);
			pAreaPointer = static_cast<char*> (cCurrentArea.operator void*());	

			//caculate the rest size
			iCurrentSize = _uiHeaderCellSize - iCurrentSize;
			//copy the latter parts of Size
			Os::Memory::copy(pAreaPointer, pBuf, iCurrentSize);
			pAreaPointer += iCurrentSize;

			//get the rest size
			iCurrentSize = cCurrentArea.getSize() - iCurrentSize;
		}
	}

}

//	FUNCTION public
//	Record2::AreaManager::freeArea -- freeArea
//
//	NOTES
//		-- freeArea
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
AreaManager::
freeArea(ObjectID::Value ullObjectID_,
		 bool bRemoveFromMap_ /* = true */)
{
	PhysicalArea::ID aid;
	//set objectid into areaid
	setObjectIDToAreaID(ullObjectID_, aid);
	try
	{

#ifdef DEBUG
			SydRecordDebugMessage
			<< "Free direct area, ID="
			<< ullObjectID_
			<< ModEndl;
#endif
		//should have 2 modes: readonly, write
		m_pFile->freeArea(m_cTrans, aid);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (bRemoveFromMap_) {
			m_mapAttachedArea.erase(ullObjectID_);
		}
		m_pFile->unfixVersionPage(true);
		_SYDNEY_RETHROW;
	}

	if (bRemoveFromMap_) {
		//remove this Area
		m_mapAttachedArea.erase(ullObjectID_);
	}
}

//	FUNCTION public
//	Record2::AreaManager::freeAreaAll -- freeAreaAll
//
//	NOTES
//		-- freeAreaAll
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
AreaManager::
freeAreaAll()
{
	//if cache has DirectArea
	if (!m_mapAttachedArea.isEmpty()) 
	{
		//PhysicalArea::ID aid;
		AttachedAreaMap::Iterator iterator = m_mapAttachedArea.begin();
		for(; iterator != m_mapAttachedArea.end(); ++ iterator)
		{
			freeArea((*iterator).first, false /* don't remove */);
		}
		m_mapAttachedArea.erase(m_mapAttachedArea.begin(), m_mapAttachedArea.end());\
	}
}

//	FUNCTION public
//	Record2::AreaManager::attach -- attach
//
//	NOTES
//		-- attach
//	ARGUMENTS
//		ObjectID::Value ullObjectID_
//			attach area's objectid
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

PhysicalArea
AreaManager::
attach(ObjectID::Value ullObjectID_, 
	   Operation::Value eOperation_/*= Operation::Read*/, 
	   Admin::Verification::Treatment::Value eTreatment_/*=0*/,
	   Admin::Verification::Progress* pProgress_/*=0*/)
{
	//if find
	PhysicalArea cArea;
	//find from Area cache
	bool bFind = false;
	if (!m_mapAttachedArea.isEmpty()) 
	{
		AttachedAreaMap::Iterator iterator = m_mapAttachedArea.find(ullObjectID_);
		if (iterator != m_mapAttachedArea.end())
		{
			cArea = (*iterator).second;
			bFind = true;
		}
	}
	
	if(!bFind)
	{
		PhysicalArea::ID aid;
		//set objectid into areaid
		setObjectIDToAreaID(ullObjectID_, aid);
		//should have 2 modes: readonly, write
		if(eOperation_ != Operation::Verify)
		{
			cArea = m_pFile->attachArea(m_cTrans, aid, _FixModeTable[eOperation_]);

#ifdef DEBUG
			SydRecordDebugMessage
			<< "Attach direct area, ID="
			<< ullObjectID_
			<< " with operation="
			<< getOperationValue(eOperation_)
			<< ModEndl;
#endif
			//insert into cache
			m_mapAttachedArea.insert(ullObjectID_, cArea);
		}
		else 
		{
			Buffer::Page::FixMode::Value eMode = _FixModeTable[eOperation_];

			if(eTreatment_ & Admin::Verification::Treatment::Correct)
			{
				eMode = Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable;
			}
			cArea = m_pFile->verifyArea(m_cTrans, aid, eMode, *pProgress_);

#ifdef DEBUG
			SydRecordDebugMessage
			<< "Verify direct area, ID="
			<< ullObjectID_
			<< ModEndl;
#endif
		}
	}

	return cArea;
}

//	FUNCTION public
//	Record2::AreaManager::detach -- detach
//
//	NOTES
//		-- detach
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
AreaManager::
detach(ObjectID::Value ullObjectID_, Operation::Value eOperation_)
{
	//nerve excute following
	if(ObjectID::isInvalid(ullObjectID_)) return;

	//should not detach single DirectArea
	if (eOperation_ != Operation::Batch) 
	{
		//remove from Area cache
		//PhysicalArea& cArea = 0;
		AttachedAreaMap::Iterator iterator = m_mapAttachedArea.find(ullObjectID_);
		if (iterator != m_mapAttachedArea.end())
		{
			//detach, need set precompile: OBSOLETE
			(*iterator).second.detach();
			//remove from cache
			m_mapAttachedArea.erase(iterator);
#ifdef DEBUG
			SydRecordDebugMessage
			<< "Detach direct area, ID="
			<< ullObjectID_
			<< " with operation="
			<< getOperationValue(eOperation_)
			<< ModEndl;
#endif
		}
	}
}

//	FUNCTION public
//	Record2::AreaManager::detachAll --
//		All physical page Atattied descriptors are Detattied. 
//
//	NOTES
//	All physical page Atattied descriptors are Detattied. 
//
//	ARGUMENTS
//		bool bSucceeded_
//			The normal termination is processed for true. 
//			The change to the page to which attachArea is done for false is thrown away. 
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
AreaManager::
detachAll(bool bSucceeded_)
{
	try
	{
		//detach current page
		detach(m_iCurrentObjectID, Operation::Read);
		
		if (bSucceeded_	//terminate normally
			|| m_cTrans.getCategory() == Trans::Transaction::Category::ReadOnly
			|| m_cFileAccess.isBatch())
		{
			////if cache has DirectArea
			//if (!m_mapAttachedArea.isEmpty()) 
			//{
			//	//PhysicalArea& cArea = 0;
			//	AttachedAreaMap::Iterator iterator = m_mapAttachedArea.begin();
			//	for(; iterator != m_mapAttachedArea.end(); ++ iterator)
			//	{
			//		//cArea = (*iterator).second;
			//		//detach, need set precompile: OBSOLETE
			//		//pArea->detach();
			//		//remove from cache
			//		m_mapAttachedArea.erase(iterator);
			//	}
			//}

#ifdef DEBUG
			SydRecordDebugMessage
			<< "detach all direct areas."
			<< ModEndl;
#endif
			//detach all really
			m_pFile->detachAllAreas();
		}
		else		//Abnormal termination
		{
			//recover
			m_pFile->recoverAllAreas();
		}

		//clear cache
		m_mapAttachedArea.erase(m_mapAttachedArea.begin(), m_mapAttachedArea.end());
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "Fatal Error. detach page failed." << ModEndl;
		Checkpoint::Database::setAvailability(m_cFileAccess.getFileID().getLockName(), false);
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Record2::AreaManager::reload -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	
//
//	EXCEPTIONS

void 
AreaManager::
substanceFile()
{
	//may needn't create file
	if(m_cFileAccess.isMounted(m_cTrans)) 
		return;

	//substance file
	m_cFileAccess.substantiate();

	//maybe do more...
}

//	FUNCTION public
//	Record2::AreaManager::getObjectIDFromAreaID -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	
//
//	EXCEPTIONS

ObjectID::Value 
AreaManager::
getObjectIDFromAreaID(const PhysicalArea::ID& cAreaID_)
{
	//conver ObjectID to ObjectIDData
	//Common::ObjectIDData objIDData;
	//PhysicalFile::File::convertToObjectID(cAreaID_, objIDData);
	
	ObjectID objID;
	objID.m_uiPageID = static_cast<PhysicalPageID>(cAreaID_.m_uiPageID);
	objID.m_uiAreaID = static_cast<PhysicalAreaID>(cAreaID_.m_uiAreaID);

	return objID.getValue();
}

//	FUNCTION public
//	Record2::AreaManager::setObjectIDToAreaID -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	
//
//	EXCEPTIONS

void
AreaManager::
setObjectIDToAreaID(ObjectID::Value ullObjectID_, PhysicalArea::ID& cAreaID_)
{
	ObjectID objID(ullObjectID_);
	cAreaID_.m_uiPageID = static_cast<PhysicalPageID>(objID.m_uiPageID);
	cAreaID_.m_uiAreaID = static_cast<PhysicalAreaID>(objID.m_uiAreaID);

	//conver ObjectID to ObjectIDData
	//Common::ObjectIDData objIDData;
	
	//ObjectID::writeValue(&objIDData, ullObjectID_);
	//PhysicalFile::File::convertToDirectAreaID(objIDData, cAreaID_);
}

//
//	Copyright (c) 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
