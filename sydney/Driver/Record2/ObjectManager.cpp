// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectManager.cpp -- manage fixed page and object.
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

//#include "Exception/BadArgument.h"
//#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Record2/Message_VerifyFailed.h"
#include "Record2/Message_ObjectNotFound.h"
#include "Record2/Message_ExistObjectBeforeTop.h"
#include "Record2/Message_ExistObjectAfterLast.h"
#include "Record2/Message_DiscordObjectNum.h"
#include "Record2/Message_VerifyOnGoing.h"
#include "Record2/Message_BadExpungedList.h"
#include "Record2/Message_BadFreeList.h"
#include "Record2/Message_InconsistentHeader.h"
#include "Record2/Message_InconsistentPageObjectNumber.h"

#include "Record2/Debug.h"
#include "Record2/DataAccess.h"
#include "Record2/ObjectManager.h"

_SYDNEY_USING

_SYDNEY_RECORD2_USING

namespace 
{
	const Utility::Size _ObjectNumArchiveSize = sizeof(Utility::ObjectNum);;

	// Top pageID
	const PhysicalPageID _uiTopPageID = 0;
	const ObjectID::Value _iFirstObjectID = 0x10000;

	// Replacement priority on header page of ObjectManager
	const Buffer::ReplacementPriority::Value
		_ePriority = Buffer::ReplacementPriority::Middle;

	// The size is obtained though it records on the header page. 
	const Utility::Size _SyncProgressSize = sizeof(ObjectManager::SyncProgress::Value);
	const Utility::Size _TimeSize = Common::DateTimeData::getArchiveSize();
	const Utility::Size _BlockSize =	sizeof(int) +	// VersionNumber
										_TimeSize +		// LastModifiedTime
										_ObjectNumArchiveSize +// ObjectNumber
										ObjectID::m_ArchiveSize * 3; // Last、FirstFree、FirstExpunged
	const Utility::Size _AreaSize = _SyncProgressSize + (_BlockSize << 1);

	const ModOffset _SyncProgressValueOffset = 0;
	const ModOffset _FirstBlockOffset = _SyncProgressValueOffset + _SyncProgressSize;
	const ModOffset _SecondBlockOffset = _FirstBlockOffset + _BlockSize;

} // namespace

//	FUNCTION public
//	Record2::ObjectManager::ObjectManager -- constructor
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

ObjectManager::
ObjectManager(File& cFileAccess_,
		Operation::Value eOperation_,
		Admin::Verification::Progress* pProgress_)
	: Manager(cFileAccess_, eOperation_),
	  m_pProgress(pProgress_),
	  m_eStatus(NotDirty),
	  m_pTopPage(0),
	  m_pPage(0),
	  m_pBatchPage(0),
	  m_iMarkedObjectID(ObjectID::m_UndefinedValue),
	  m_iObjectNumber(0),
	  m_pDataAccess(0),
	  m_pTopPointer(0)
{
	; _SYDNEY_ASSERT(m_eOperation != Operation::Verify || m_pProgress);
}

//	FUNCTION public
//	Record2::ObjectManager::~ObjectManager -- destructor
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

ObjectManager::
~ObjectManager()
{
	if (m_pPage)
		detach(Operation::Write);

	if (m_pTopPage)
		detachTopPage(true);
}

//	FUNCTION public
//	Record2::ObjectManager::allocate -- allocate
//
//	NOTES
//		-- allocate
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

ObjectID::Value 
ObjectManager::
allocate()
{
	//set undefined
	ObjectID::Value ullNextObjectID = ObjectID::m_UndefinedValue;
	
	try
	{
		//get first free objectid
		if(!ObjectID::isInvalid(m_cCurrentTopPage.m_iFirstFreeObjectID))
		{
			m_iCurrentObjectID = m_cCurrentTopPage.m_iFirstFreeObjectID;
			
			readObjectHeader(m_iCurrentObjectID, ullNextObjectID);
		}
		else
		{
			ObjectStatus::Value eStatus = ObjectStatus::Initialize;
			m_iCurrentObjectID = getNextObjectID(m_cCurrentTopPage.m_iLastObjectID, eStatus);
		}
		
		//should allocate a new page
		if(ObjectID::isInvalid(m_iCurrentObjectID))
		{
			//may be batch mode
			Operation::Value eOperation = Operation::Insert;
			if(m_cFileAccess.isBatch()) eOperation = Operation::Batch;

			m_pPage = m_pFile->allocatePage2(m_cTrans, _FixModeTable[eOperation]);
			//assign current objectid
			m_iCurrentObjectID = ObjectID(m_pPage->getID(), 0).getValue();

			//insert into cache
			if (eOperation == Operation::Batch) m_pBatchPage = m_pPage;
			else m_mapAttachedPage.insert(m_pPage->getID(), m_pPage); 
#ifdef DEBUG
			SydRecordDebugMessage
			<< "Allocate new page: PageID="
			<< m_pPage->getID()
			<< " with operation="
			<< getOperationValue(eOperation)
			<< ModEndl;
#endif
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (m_pPage) 
		{
			detach();
		}
		_SYDNEY_RETHROW;
	}

	//set first free objectid
	m_cCurrentTopPage.m_iFirstFreeObjectID = ullNextObjectID;

	; _SYDNEY_ASSERT(!ObjectID::isInvalid(m_iCurrentObjectID));

	//validate, set the last ObjectID
	validate(m_iCurrentObjectID, Operation::Insert);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Allocate new object: ObjectID="
			<< m_iCurrentObjectID
			<< ModEndl;
#endif

	return m_iCurrentObjectID;
}

//	FUNCTION public
//	Record2::ObjectManager::seek -- seek
//
//	NOTES
//		-- seek
//	ARGUMENTS
//		Record::ObjectID::Value ullObjectID_
//			Moving object ID
//		bool bKeepAttach_ = false
//			Is the page left Atattied?
//		Operation::Value eOperation_ = Operation::Read
//			Mode Atattied when bKeepAttach _ is true
//
//	RETURN
//		If specified object ID is correct, it is true. 
//
//	EXCEPTIONS

const char*
ObjectManager::
seek(ObjectID::Value ullObjectID_, Operation::Value eOperation_, bool bKeepAttach_)
{
	//needn't do it for the firsttime seek
	//if (ObjectID::isInvalid(ullObjectID_)) 	return 0;

	if (!m_cFileAccess.isMounted(m_cTrans))
		return 0;

	//set current ObjectID
	m_iCurrentObjectID = ullObjectID_;

	ObjectID objID(ullObjectID_);
	; _SYDNEY_ASSERT(m_pFile->isUsedPage(m_cTrans, objID.m_uiPageID));

	//attach the current page
	attach(eOperation_);

	try 
	{
		//a temporary method for read RHP file
		if(eOperation_ == Operation::Read) 
		{
			ObjectStatus::Value eStatus = readObjectHeader(ullObjectID_);
			
			if(eStatus == ObjectStatus::Expunged)
			{
				SydErrorMessage
					<< "Want to read Expunged ObjectID: "
					<< ullObjectID_
					<< ModEndl;

				//must detach header page
				validate();
				//may be Exception::ObjectStatusError
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
		//skip to position
		const char* pPointer = locate(makePageHeader(m_pPage, eOperation_), objID.m_uiAreaID);

		// It is not changed. 
		if (!bKeepAttach_) 
			detach(); 

		return pPointer;
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detach();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Record2::ObjectManager::next -- next
//
//	NOTES
//		-- next
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

const char*
ObjectManager::
next()
{
	if (!m_cFileAccess.isMounted(m_cTrans))
		return 0;

#ifdef DEBUG
	ObjectID::Value ullPrevObjectID = m_iCurrentObjectID;
#endif

	//unvalidate value
	if (ObjectID::isInvalid(m_iCurrentObjectID) ||
		m_iCurrentObjectID < _iFirstObjectID) 
	{
		// go to the first ObjectID
		m_iCurrentObjectID = _iFirstObjectID - 1;
	} 

	// look for the next ObjectID
	ObjectStatus::Value eStatus = ObjectStatus::Inserted;
	m_iCurrentObjectID = getNextObjectID(m_iCurrentObjectID, eStatus);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Move to next object: CurrentObjectID="
			<< ullPrevObjectID
			<< " NextObjectID="
			<< m_iCurrentObjectID
			<< ModEndl;
#endif

	//havn't next objectid
	if(ObjectID::isInvalid(m_iCurrentObjectID)) 
	{
		// It puts it into the state not Atattied when failing in seek. 
		detach();
		return 0;
	}

	// Next is returned with the Atatti. 
	attach(Operation::Read);

	//skip to position
	return locate(makePageHeader(m_pPage), ObjectID(m_iCurrentObjectID).m_uiAreaID);
}

//	FUNCTION public
//	Record2::ObjectManager::expunge -- remove objectid
//
//	NOTES
//		-- remove objectid
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
ObjectManager::
expunge(const ObjectID::Value ullObjectID_)
{
	if (!m_cFileAccess.isMounted(m_cTrans))
		return;

	//for checking
	ObjectID::Value ullNextObjectID;
	ObjectStatus::Value eStatus = readObjectHeader(ullObjectID_, ullNextObjectID);
	
	if(eStatus != ObjectStatus::Inserted)
	{
		SydErrorMessage
			<< "Want to expunge Object which isn't in inserted status: "
			<< ullObjectID_
			<< ModEndl;

		//must detach header page
		validate();
		//may be Exception::ObjectStatusError
		_SYDNEY_THROW0(Exception::Unexpected);
	}
#ifdef DEBUG
	else
	{
		SydRecordDebugMessage
			<< "Expunge operation: ObjectID="
			<< ullObjectID_
			<< ModEndl;
	}
#endif
	
	//if next object is existed, this object is updated
	if(!ObjectID::isInvalid(ullNextObjectID) &&
		ullNextObjectID != m_cCurrentTopPage.m_iFirstExpungedObjectID &&
		readObjectHeader(ullNextObjectID) == ObjectStatus::Expunged)
	{
		//find from expunged list 
		ObjectID::Value ullPrevObjectID, ullTempObjectID;
		int iCount = findObjectFromList(ullNextObjectID, ullTempObjectID, ullPrevObjectID);
		if(iCount != -1)
		{
			//adjust expunged link
			writeObjectHeader(ullPrevObjectID, ullObjectID_, m_cTrans.getID(), Operation::Update);
			//reset current object status
			writeObjectHeader(ullObjectID_, ullNextObjectID, m_cTrans.getID(), Operation::Expunge);

			m_iCurrentObjectID = m_cCurrentTopPage.m_iFirstExpungedObjectID;
		}
		else
		{
			SydErrorMessage
				<< "Can't find ObjectID: "
				<< ullNextObjectID
				<< " from expunged list: "
				<< ModEndl;
			//must detach header page
			validate();
			//may be Exception::ObjectStatusError
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
	else	//expunge inserted object
	{
		//adjust expunged link
		writeObjectHeader(ullObjectID_, 
							m_cCurrentTopPage.m_iFirstExpungedObjectID, 
							m_cTrans.getID(),
							Operation::Expunge);

		//set oid to the expunged linked
		m_iCurrentObjectID = m_cCurrentTopPage.m_iFirstExpungedObjectID = ullObjectID_;
	}

	//validate, set the last ObjectID
	validate(m_iCurrentObjectID, Operation::Expunge);
	//detach this page
	//detach(Manager::Operation::Update);
}

//	FUNCTION public
//	Record2::ObjectManager::undoExpunge -- undo expunge
//
//	NOTES
//		-- undo expunge
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

bool 
ObjectManager::
undoExpunge(const ObjectID::Value ullObjectID_)
{
	if (!m_cFileAccess.isMounted(m_cTrans))
		return false;

	if(ObjectID::isInvalid(ullObjectID_)) 
		return false;

	ObjectID::Value ullNextObjectID;
	ObjectID::Value ullPrevObjectID;
	int iCount = findObjectFromList(ullObjectID_, ullNextObjectID, ullPrevObjectID);
	if(iCount != -1)
	{
		if(ullObjectID_ == m_cCurrentTopPage.m_iFirstExpungedObjectID)
		{
			m_cCurrentTopPage.m_iFirstExpungedObjectID = ullNextObjectID;
		}
		else
		{
			//set prvious's nextid to nextObjectID 
			writeObjectHeader(ullPrevObjectID, ullNextObjectID, m_cTrans.getID());
		}

		//write this object header
		if(ObjectID::isInvalid(ullNextObjectID))	//first update then expunge cause this situation
			writeObjectHeader(ullObjectID_, 0, Trans::IllegalID, Operation::Insert);
		else writeObjectHeader(ullObjectID_, ullNextObjectID, 0, Operation::Insert);

#ifdef DEBUG
		SydRecordDebugMessage
			<< "undoExpunge operation: ObjectID="
			<< ullObjectID_
			<< ModEndl;
#endif
	}
	else //not find suitable oid, should throw exception? I think needn't
	{
#ifdef DEBUG
		SydRecordDebugMessage
			<< "Can't find the expunged ObjectID: "
			<< ullObjectID_
			<< ModEndl;
		//must detach header page
		validate();
		return false;
#endif
	}

	//update first and last ObjectID
	validate(ullObjectID_, Operation::Insert);
	//detach this page
	//detach(Manager::Operation::Update);

	return true;
}

//	FUNCTION public
//	Record2::ObjectManager::undoUpdate -- undo update
//
//	NOTES
//		-- undo update
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

bool 
ObjectManager::
undoUpdate(const ObjectID::Value ullObjectID_)
{
	if (!m_cFileAccess.isMounted(m_cTrans))
		return false;

	ObjectID::Value ullNextObjectID;
	readObjectHeader(ullObjectID_, ullNextObjectID);

	ObjectID::Value ullNextNextObjectID;
	ObjectStatus::Value eStatus = readObjectHeader(ullNextObjectID, ullNextNextObjectID);
	//after updating a compact operation is run
	if(eStatus != ObjectStatus::Expunged) 
	{
		SydErrorMessage
			<< "Want to undo update unUpdated ObjectID: "
			<< ullNextObjectID
			<< ModEndl;
		//may be Exception::ObjectStatusError
		_SYDNEY_THROW0(Exception::Unexpected);
	}
#ifdef DEBUG
	else
	{
		SydRecordDebugMessage
			<< "UndoUpdate operation : ObjectID="
			<< ullObjectID_
			<< " the next ObjectID="
			<< ullNextObjectID
			<< ModEndl;
	}
#endif

	//1. copy old data into current area
	if(m_pDataAccess) m_pDataAccess->undoUpdate(ullObjectID_, ullNextObjectID);

	//2. re-link next-next object to current object
	writeObjectHeader(ullObjectID_, ullNextNextObjectID, 0);

	//3. erase next object really
	compact(ullNextObjectID);

	return true;
}

//	FUNCTION public
//	Record2::ObjectManager::compact -- compact
//
//	NOTES
//		-- compact
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

Utility::ObjectNum 
ObjectManager::
compact(const ObjectID::Value ullObjectID_)
{
	if (!m_cFileAccess.isMounted(m_cTrans))
		return 0;

	//if input valid objectid then erase the specail object
	bool bCompactAll = ObjectID::isInvalid(ullObjectID_) ? true : false;

	//move the expunged link to free link
	//may be crashed, but can't roolback?
	Utility::TransID iPrevTransID = m_cTrans.getID();
	Utility::TransID iCurrentTransID = 0;
	ObjectID::Value iCurrentObjectID = m_cCurrentTopPage.m_iFirstExpungedObjectID;
	ObjectID::Value ullNextObjectID = 0;
	ObjectID::Value ullPrevObjectID = 0;
	ObjectStatus::Value eStatus = readObjectHeader(iCurrentObjectID, ullNextObjectID, iCurrentTransID);
	bool bTransIsInProgress = false;
	//To preserve the history of transaction
	ModVector<Utility::TransID> vecPrevInProgressTransID;

	Utility::ObjectNum uiCount = 0;
	//compare
	while (!ObjectID::isInvalid(iCurrentObjectID))
	{
		//Avoid one trans is active during start compacting,
		//but finished this trans in compact on going
		if(bCompactAll)
		{
			//whether this trans is progressing
			if(!vecPrevInProgressTransID.isEmpty() && 
				vecPrevInProgressTransID.find(iCurrentTransID) != vecPrevInProgressTransID.end())
			{
				bTransIsInProgress = true;
			}
			else //reserved the history inprogress trans
			{
				bTransIsInProgress = Trans::Transaction::isInProgress(
					Trans::Transaction::ID(iCurrentTransID), Trans::Transaction::Category::ReadWrite);
				
				if(bTransIsInProgress)
					vecPrevInProgressTransID.pushBack(iCurrentTransID);
			}
		}
		//if is this trans
		if(bCompactAll && !bTransIsInProgress || 
			!bCompactAll && iCurrentTransID == m_cTrans.getID())
		{
			//check the object status
			if(eStatus != ObjectStatus::Expunged) 
			{
				SydErrorMessage
					<< "Want to compact unExpunged ObjectID: "
					<< iCurrentObjectID
					<< ModEndl;

				//must detach header page
				detachTopPage();
				//may be Exception::ObjectStatusError
				_SYDNEY_THROW0(Exception::Unexpected);
			}
#ifdef DEBUG
			else
			{
				SydRecordDebugMessage
					<< "Compact operation: ObjectID="
					<< iCurrentObjectID
					<< ModEndl;
			}
#endif
			if(bCompactAll || iCurrentObjectID == ullObjectID_)
			{
				//remove data really
				if(m_pDataAccess) m_pDataAccess->erase(iCurrentObjectID);

				//set current free id 
				writeObjectHeader(iCurrentObjectID, 
						m_cCurrentTopPage.m_iFirstFreeObjectID, 
						Trans::IllegalID);
				//adjust free list
				m_cCurrentTopPage.m_iFirstFreeObjectID = iCurrentObjectID;
				
				//remove oid really
				if(iCurrentObjectID == m_cCurrentTopPage.m_iFirstExpungedObjectID)
					m_cCurrentTopPage.m_iFirstExpungedObjectID = ullNextObjectID;
				//reset the expunged list
				else if(bCompactAll && ullPrevObjectID != 0 || !bCompactAll)
					writeObjectHeader(ullPrevObjectID, ullNextObjectID, iPrevTransID);

				//increase count
				uiCount++;

				//if special object than go out
				if(!bCompactAll) break;
			}
		}

		if(bCompactAll && bTransIsInProgress || !bCompactAll)
		{
			ullPrevObjectID = iCurrentObjectID;
			iPrevTransID = iCurrentTransID;
		}
		iCurrentObjectID = ullNextObjectID;
		//get next objectid and transid
		eStatus = readObjectHeader(iCurrentObjectID, ullNextObjectID, iCurrentTransID);
	}

	//shouldn't change first and last objectid
	m_iCurrentObjectID = ObjectID::m_UndefinedValue;
	//update first and last ObjectID
	touch();
	sync();
	return uiCount;
}

//
//	FUNCTION public
//	Record2::ObjectManager::mark -- The position of the rewinding is recorded. 
//
//	NOTES
//	The position of the rewinding is recorded. 
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
void
ObjectManager::
mark()
{
	; _SYDNEY_ASSERT(m_cFileAccess.isMounted(m_cTrans));

	m_iMarkedObjectID = m_iCurrentObjectID; 
}

//
//	FUNCTION public
//	Record2::ObjectManager::rewind -- It returns to the recorded position. 
//
//	NOTES
//	It returns to the position recorded by the rewinding. 
//
//	ARGUMENTS
//	None
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened. 
//	[YET!]
//
void
ObjectManager::
rewind()
{
	; _SYDNEY_ASSERT(m_cFileAccess.isMounted(m_cTrans));
	
	if (ObjectID::isInvalid(m_iMarkedObjectID)) 
	{
		reset();
	}
	else 
	{
		//bool bResult = seek(m_iMarkedObjectID);
		//; _SYDNEY_ASSERT(bResult);

		//do this, not sure
		m_iCurrentObjectID = m_iMarkedObjectID;
	}
}

//
//	FUNCTION public
//	Record2::ObjectManager::reset -- The cursor is reset. 
//
//	NOTES
//	The cursor is reset. 
//
//	ARGUMENTS
//	None
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	FileNotOpen
//		The record file has not been opened.
//	[YET!]
//
void
ObjectManager::
reset()
{
	if (m_cFileAccess.isMounted(m_cTrans))
		// Iteratar is annulled. 
		detach();

	m_iCurrentObjectID = ObjectID::m_UndefinedValue;
}

//	FUNCTION public
//	ObjectManager::getNextObjectID --
//		Object ID of the position at which Iteratar points is obtained 
//		while using one the next state. 
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//		Demanded object ID
//
//	EXCEPTIONS

ObjectID::Value
ObjectManager::
getNextObjectID(const ObjectID::Value ullObjectID_, 
				ObjectStatus::Value& eStatus_, 
				ObjectID::Value iMaxObjectID_)
{
	if(ObjectID::isInvalid(ullObjectID_))
		return ullObjectID_;
	
	ObjectID objID(ullObjectID_);
	ObjectStatus::Value eStatus = eStatus_;
	ObjectID::Value ullNextObjectID;
	Utility::ObjectNum iNumPerPage = 
		m_cFileAccess.getFileID().getObjectNumberPerPage();
	do
	{
		//increase areaid
		++ objID.m_uiAreaID;

		//must less than 
		if(objID.m_uiAreaID >= iNumPerPage)
		{
			if(eStatus_ == ObjectStatus::Initialize)
			{
				//should allocate new page
				ullNextObjectID = ObjectID::m_UndefinedValue;
				break;
			}
			else
			{
				++ objID.m_uiPageID;
				objID.m_uiAreaID = 0;
			}
		}
		
		ullNextObjectID = objID.getValue();
		
		//can't overflow
		if(ullNextObjectID > iMaxObjectID_ ||
			eStatus_ != ObjectStatus::Initialize && 
			ullNextObjectID > m_cCurrentTopPage.m_iLastObjectID)
		{
			ullNextObjectID = ObjectID::m_UndefinedValue;
			break;
		}
		
		//check is free or not
		eStatus = readObjectHeader(ullNextObjectID);
		//for verify...
		if(eStatus_ == ObjectStatus::Used && 
			(eStatus == ObjectStatus::Inserted ||
			eStatus == ObjectStatus::Expunged||
			eStatus == ObjectStatus::Free))
			break;

	}//if not meet for requir call again
	while(eStatus != eStatus_);

	eStatus_ = eStatus;
	return ullNextObjectID;
}

//	FUNCTION public
//	Record2::ObjectManager::existVariableFile -- existVariableFile
//
//	NOTES
//		-- existVariableFile
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

bool 
ObjectManager::
existVariableFile() const
{
	//it's not suitable
	return m_cFileAccess.getFileID().getTargetFields(true).getSize();
}

//	FUNCTION public
//	Record2::ObjectManager::readObjectHeader -- readObjectHeader
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

ObjectManager::ObjectStatus::Value 
ObjectManager::
readObjectHeader(const ObjectID::Value ullObjectID_, 
				 ObjectID::Value& ullNextObjectID_, 
				 Utility::TransID& iTransID_)
{
	//return undefined objectid
	if(ObjectID::isInvalid(ullObjectID_))
		return ObjectStatus::Invalid;

	//attached temporary page
	ObjectID objID(ullObjectID_);
	PhysicalPage* pPage = doAttach(objID.m_uiPageID, m_pPage, m_eOperation);

	//for reading
	//go to object position
	const char* pPointer = locate(makePageHeader(pPage), objID.m_uiAreaID);
	; _SYDNEY_ASSERT(pPointer);

	//read next objectid
	pPointer += ObjectID::readValue(pPointer, ullNextObjectID_);

	//cant's equal
	; _SYDNEY_ASSERT(ullObjectID_ != ullNextObjectID_);

	//read trans id
	Os::Memory::copy(&iTransID_, pPointer, Utility::m_TransIDArchiveSize);
	pPointer += Utility::m_TransIDArchiveSize;

	//maybe need push to cache
	if(pPage != m_pPage) doDetach(pPage, m_eOperation);
	
	//check status
	ObjectStatus::Value eStatus = ObjectStatus::Invalid;
	if(ullNextObjectID_ == 0)
	{
		if(iTransID_ == Trans::IllegalID) 
			eStatus = ObjectStatus::Inserted;
		else if(iTransID_ == 0) 
			eStatus = ObjectStatus::Initialize;
		
	}
	else// if(!ObjectID::isInvalid(ullNextObjectID_))
	{
		if(iTransID_ == Trans::IllegalID) 
			eStatus = ObjectStatus::Free;
		else if(iTransID_ != 0) 
			eStatus = ObjectStatus::Expunged;
		else// if(iTransID_ == 0) //must equal to zero
			eStatus = ObjectStatus::Inserted;	 //update status
	}
	
	; _SYDNEY_ASSERT(eStatus != ObjectStatus::Invalid);

	return eStatus;
}

//	FUNCTION public
//	Record2::ObjectManager::readObjectHeader -- readObjectHeader
//
//	NOTES
//		-- override function
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

ObjectManager::ObjectStatus::Value 
ObjectManager::
readObjectHeader(const ObjectID::Value ullObjectID_, ObjectID::Value& ullNextObjectID_)
{
	Utility::TransID iCurrentTransID;
	return readObjectHeader(ullObjectID_, ullNextObjectID_, iCurrentTransID);
}

//another override function
ObjectManager::ObjectStatus::Value 
ObjectManager::
readObjectHeader(const ObjectID::Value ullObjectID_)
{
	ObjectID::Value ullNextObjectID;
	Utility::TransID iCurrentTransID;
	return readObjectHeader(ullObjectID_, ullNextObjectID, iCurrentTransID);
}

//	FUNCTION public
//	Record2::ObjectManager::writeObjectHeader -- writeObjectHeader
//
//	NOTES
//		-- writeObjectHeader
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
ObjectManager::
writeObjectHeader(const ObjectID::Value ullObjectID_, 
				  const ObjectID::Value ullNextObjectID_, 
				  const Utility::TransID iTransID_,
				  Operation::Value eOperation_)
{
	if(ObjectID::isInvalid(ullObjectID_)) return;

	//cant's equal
	; _SYDNEY_ASSERT(ullObjectID_ != ullNextObjectID_);

	ObjectID objID(ullObjectID_);
	PhysicalPage* pPage = doAttach(objID.m_uiPageID, m_pPage, Operation::Update);

	//go to object position
	char* pPointer = const_cast<char*> (locate(makePageHeader(pPage, eOperation_), objID.m_uiAreaID));
	; _SYDNEY_ASSERT(pPointer);

	//write next objectid
	pPointer += ObjectID::writeValue(pPointer, ullNextObjectID_);

	//set Trans ID
	Os::Memory::copy(pPointer, &iTransID_, Utility::m_TransIDArchiveSize);
	pPointer += Utility::m_TransIDArchiveSize;

	//maybe need push to cache
	if(pPage != m_pPage) doDetach(pPage, Operation::Update);
}

//	FUNCTION public
//	Record2::ObjectManager::findObjectFromList -- findObjectFromList
//
//	NOTES
//		-- findObjectFromList
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

int 
ObjectManager::
findObjectFromList(ObjectID::Value ullObjectID_, 
				   ObjectID::Value& ullNextObjectID_, 
				   ObjectID::Value& ullPrevObjectID_, 
				   bool bExpunged_)
{
	//get the first object of list
	ObjectID::Value iCurrentObjectID = m_cCurrentTopPage.m_iFirstExpungedObjectID;
	if(!bExpunged_) iCurrentObjectID = m_cCurrentTopPage.m_iFirstFreeObjectID;
	
	//for loop and find it
	int iCount = 0;
	ObjectStatus::Value eStatus;
	//preserve previous id
	ullPrevObjectID_ = ObjectID::m_UndefinedValue;

	do
	{
		//read header info
		eStatus = readObjectHeader(iCurrentObjectID, ullNextObjectID_);
		
		if(!ObjectID::isInvalid(iCurrentObjectID))
		{
			//have to meet status
			if(bExpunged_ && eStatus != ObjectStatus::Expunged)
			{
				SydErrorMessage
					<< "Can't find ObjectID: "
					<< ullObjectID_
					<< " from expunged list: "
					<< ModEndl;
				//may be Exception::ObjectStatusError
				_SYDNEY_THROW0(Exception::Unexpected);
			}
			else if(!bExpunged_ && eStatus != ObjectStatus::Free)
			{
				SydErrorMessage
					<< "Can't find ObjectID: "
					<< ullObjectID_
					<< " from free list: "
					<< ModEndl;
				//may be Exception::ObjectStatusError
				_SYDNEY_THROW0(Exception::Unexpected);
			}
			else iCount++;
		}
		//if invalid
		else
		{
			if(!ObjectID::isInvalid(ullObjectID_)) 
				iCount = -1;
			//else iCount++;	//if object is invalid, its status shouldn't take as adjudgement
			break;
		}
		
		//whether find it
		if (iCurrentObjectID == ullObjectID_)
			break;

		//preserve current id
		ullPrevObjectID_ = iCurrentObjectID;
		iCurrentObjectID = ullNextObjectID_;

	}
	while (true);

	return iCount;
}

//	FUNCTION public
//	Record2::ObjectManager::validate -- validate
//
//	NOTES
//		If it is necessary, administrative information in the file is rewritten. 
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
ObjectManager::
validate(const ObjectID::Value ullObjectID_, Operation::Value eOperation_)
{
	if (ObjectID::isInvalid(ullObjectID_))
	{
		detachTopPage();
		return;
	}

	switch (eOperation_) 
	{
		case Operation::Read:
			// do nothing
			break;
		case Operation::Insert:
		{
			// The total of the object increases.
			++m_cCurrentTopPage.m_uiInsertedObjectNum;
			MSGLIN( SydRecordTraceMessage << "Record2 object num: " << m_cCurrentTopPage.m_uiInsertedObjectNum << ModEndl );

			// Whether maximum, minimum object ID changes is examined. 
			if (ObjectID::isInvalid(m_cCurrentTopPage.m_iLastObjectID)
				|| ullObjectID_ > m_cCurrentTopPage.m_iLastObjectID) 
			{
				m_cCurrentTopPage.m_iLastObjectID = ullObjectID_;
			}
			touch();
			break;
		}
		case Operation::Update:
		{
			// To rewrite the update time, it makes it to Dirty. 
			// touch();
			break;
		}
		case Operation::Expunge:
		{
			// The total of the object decreases. 
			--m_cCurrentTopPage.m_uiInsertedObjectNum;
			MSGLIN( SydRecordTraceMessage << "Record2 object num: " << m_cCurrentTopPage.m_uiInsertedObjectNum << ModEndl );
			//!!!!!!!!!!!!!!
			//here maybe have a problem:
			//in record2, m_iFirstObjectID will never change althought expunge a tuple
			//and m_iLastObjectID will increase ever
			//!!!!!!!!!!!!!!
			touch();
		}
		default:
			break;
	}

	//save top page
	sync();
}

//	FUNCTION public
//	Record2::ObjectManager::reload -- reload
//
//	NOTES
//		It is a rereading seeing of administrative information in the file.
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
ObjectManager::
reload(const bool bDoRepair_)
{
	//may needn't create file
	if(!m_cFileAccess.isMounted(m_cTrans)) 
		return;
	
	Operation::Value eOperation = bDoRepair_ ? Operation::Update : Operation::Read;

	//attach top page
	attachTopPage(eOperation);

	try 
	{
		SyncProgress::Value progress = readSyncProgress();

		// Administrative information in correct one is read.
		switch (progress) 
		{
			case SyncProgress::NotWriting:
			case SyncProgress::WritingSecondBlock:
			{
				// The first block must be correct. 
				readFirstBlock();

				if (bDoRepair_ && progress == SyncProgress::WritingSecondBlock) // It mends. 
				{
					// It mends. (The data member is written in the second block. )
					writeSecondBlock();
				}
				break;
			}
			case SyncProgress::WritingFirstBlock:
			{
				// The second block must be correct. 
				readSecondBlock();

				if (bDoRepair_)
				{
					// It mends. (The data member is written in the first block. )
					writeFirstBlock();
				}
			}
			default:
				; _SYDNEY_ASSERT(false);
				break;
		}

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detachTopPage();
		_SYDNEY_RETHROW;
	}

	detachTopPage();
}

//	FUNCTION public
//	Record2::ObjectManager::sync -- sync
//
//	NOTES
//		The data member is written in the file. 
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
ObjectManager::
sync()
{
	// Anything need not be done if there is no change in data. 
	if (!m_cCurrentTopPage.m_bDirty) 
		return;

	//
	// Processing that is necessary to send exception again 
	// and to be error processing when error occurs
	//

	if (!m_pTopPage) attachTopPage(Operation::Batch);

	try 
	{
		// After the status is written, the block is written. 

		// The first block
		writeSyncProgress(SyncProgress::WritingFirstBlock);
		writeFirstBlock();

		// The second block
		writeSyncProgress(SyncProgress::WritingSecondBlock);
		writeSecondBlock();

		// finished
		writeSyncProgress(SyncProgress::NotWriting);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detachTopPage();
		_SYDNEY_RETHROW;
	}

	detachTopPage();
	m_eStatus = Sync;
}

//	FUNCTION public
//	Record2::ObjectManager::recover -- recover
//
//	NOTES
//		When the error occurs after sync, the content is written and it returns it. 
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
ObjectManager::
recover()
{
	// Anything need not be done when it is Sync front.
	if (m_eStatus != Sync) 
		return;

	// It writes in the preserved data and it returns it. 
	m_cCurrentTopPage = m_cBackupTopPage;

	//
	// Processing that is necessary to send exception again 
	// and to be error processing when error occurs
	//

	if (!m_pTopPage) attachTopPage(Operation::Update);

	try 
	{
		// After the status is written, the block is written. 

		// The first block
		writeSyncProgress(SyncProgress::WritingFirstBlock);
		writeFirstBlock();

		// The second block
		writeSyncProgress(SyncProgress::WritingSecondBlock);
		writeSecondBlock();

		// Finished
		writeSyncProgress(SyncProgress::NotWriting);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detachTopPage();
		_SYDNEY_RETHROW;
	}

	detachTopPage();
}


//	FUNCTION public
//	Record2::ObjectManager::verifyPhysicalFile -- The correspondence of a physical file is inspected. 
//
//	NOTES
//
//	ARGUMENTS
//	AAdmin::Verification::Treatment::Value eTreatment_
//		Inspection method of correspondence inspection
//	Admin::Verification::Progress&	cProgress_
//		Reference to process of correspondence inspection
//	Record2::VariableFile* pVariableFile_
//		Pointer to variable-length file
//
//	RETURN
//	None
//
//	EXCEPTIONS
//
Utility::ObjectNum 
ObjectManager::
verifyBody(Admin::Verification::Treatment::Value eTreatment_,
				   Admin::Verification::Progress&	cProgress_)
{
	if (!m_cFileAccess.isMounted(m_cTrans) || !cProgress_.isGood())
		return 0;

	Utility::ObjectNum uiInsertedObjectNum = 0;
	try 
	{
		if (cProgress_.isGood()) 
		{
			// To a physical file manager in the fixed length file
			// With all physical pages used
			// The content of a variable-length file is inspected at the same time ..
			// the notification of all physical pages used and the areas in a variable-length file... 
			
			//The page of administrative information always exists. 
			m_pFile->notifyUsePage(m_cTrans, cProgress_, _uiTopPageID);
			if (!cProgress_.isGood()) return 0;

			//show the verify information
			const int n = m_pFile->getLastPageID(m_cTrans);
			m_pFile->detachPageAll();
			const int step = ModMax(5, n / 5);
			const ModUnicodeString name("physical file");
			int i = step;
			int sum = 0;
			_SYDNEY_VERIFY_INFO(cProgress_, m_cFileAccess.getPath(), Message::VerifyOnGoing(name, sum, n), eTreatment_);

			PhysicalPageID uiPageID = PhysicalFile::ConstValue::UndefinedPageID;
			// Because it is Verify, not attachPage() but verifyPage() is used in nextPage(). 
			while ((uiPageID = nextPage(uiPageID, &cProgress_)) != PhysicalFile::ConstValue::UndefinedPageID) 
			{
				if (!--i) 
				{
					sum += step;
					_SYDNEY_VERIFY_INFO(cProgress_, m_cFileAccess.getPath(), Message::VerifyOnGoing(name, sum, n), eTreatment_);
					i = step;
				}

				// Page ID used is registered in a physical file (notification). 
				m_pFile->notifyUsePage(m_cTrans, cProgress_, uiPageID);
				if (!cProgress_.isGood()) 
				{
					detach(Operation::Verify);
					return uiInsertedObjectNum;
				}
				
				//verify ObjectNum of this page and variable data
				uiInsertedObjectNum += verifyPageData(eTreatment_, cProgress_);
				if (!cProgress_.isGood()) 
				{
					detach(Operation::Verify);
					return uiInsertedObjectNum;
				}
			}
		}

		detachAll(true);
	} 
	catch (...) 
	{
		//detach all page
		detachAll(true);

		_SYDNEY_VERIFY_ABORTED(cProgress_, m_cFileAccess.getPath(), Message::VerifyFailed());
		_SYDNEY_RETHROW;
	}

	return uiInsertedObjectNum;
}

//	FUNCTION public
//	Record2::ObjectManager::verifyContents --
//		The correspondence in the fixed length file is inspected. 
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Treatment::Value eTreatment_
//		Inspection method of correspondence inspection
//   Admin::Verification::Progress& cProgress_
//		Reference to process of correspondence inspection
//	 Record2::VariableFile* pVariableFile_
//		Pointer to variable-length file
//
//	RETURN
//	None
//
//	EXCEPTIONS

void
ObjectManager::
verifyHeader(Utility::ObjectNum iObjectNum_, 
			   Admin::Verification::Treatment::Value eTreatment_,
			   Admin::Verification::Progress& cProgress_)
{
	if (!m_cFileAccess.isMounted(m_cTrans) || !cProgress_.isGood())
		return;

	//check the totalnum
	Utility::ObjectNum iTotalNum = iObjectNum_;

	// Inspect content will do following:
	// 1. The first object inspection
	// 2. The final object inspection
	// 3. Check of number of objects on each page
	// 4. Check of number of objects and total of object on each page
	// 5. Inspection of link for deletion
	// The check of the total of the object does by four and is the same thing. 
	const int n = 6;
	int i = 0;
	const ModUnicodeString name("record2 file");
	_SYDNEY_VERIFY_INFO(cProgress_, m_cFileAccess.getPath(), Message::VerifyOnGoing(name, i++, n), eTreatment_);

	//check total insertobjectnum
	if (m_cCurrentTopPage.m_uiInsertedObjectNum != iTotalNum) 
	{
		_SYDNEY_VERIFY_INCONSISTENT(cProgress_, m_cFileAccess.getPath(),
			Message::DiscordObjectNum(m_cCurrentTopPage.m_uiInsertedObjectNum,
										iObjectNum_));

		return;
	}
	// If the number of objects is 0, you may not do anything about either any further. 
	_SYDNEY_VERIFY_INFO(cProgress_, m_cFileAccess.getPath(), Message::VerifyOnGoing(name, i++, n), eTreatment_);

	//; _SYDNEY_ASSERT(m_cCurrentTopPage.m_uiInsertedObjectNum > 0);
	
	if (m_cCurrentTopPage.m_iLastObjectID >= ObjectID::m_UndefinedValue) 
	{
		// The header information is amusing. 
		// Because it is not understood whether the total of the object or 
		// top object information is correct, Correct cannot be done. 
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			m_cFileAccess.getPath(),
			Message::InconsistentHeader(m_cCurrentTopPage.m_uiInsertedObjectNum,
										_iFirstObjectID,
										m_cCurrentTopPage.m_iLastObjectID));
		return;
	}

	// 3．Final object ID inspection
	// ★Notice★
	// The final object is obtaining and has putting when the inspection of 
	// each page ends it must be on the page it. 
	_SYDNEY_VERIFY_INFO(cProgress_, m_cFileAccess.getPath(), Message::VerifyOnGoing(name, i++, n), eTreatment_);
	verifyLastObjectID(eTreatment_, cProgress_);
	if (!cProgress_.isGood()) return;

	// 4.Check of number of objects on each page
	// 5.Check of number of objects and total of object on each page
	_SYDNEY_VERIFY_INFO(cProgress_, m_cFileAccess.getPath(), Message::VerifyOnGoing(name, i++, n), eTreatment_);
	iTotalNum += verifyExpungedList(eTreatment_, cProgress_);
	if (!cProgress_.isGood()) return;

	// 6．Inspection of link for deletion
	// ★Notice★
	// Because object ID link is not the order of the page, it is not 
	// possible to do at the same time as ..the inspection of 4 and 5... 
	_SYDNEY_VERIFY_INFO(cProgress_, m_cFileAccess.getPath(), Message::VerifyOnGoing(name, i++, n), eTreatment_);
	iTotalNum += verifyFreeList(eTreatment_, cProgress_);
	if (!cProgress_.isGood()) return;

	_SYDNEY_VERIFY_INFO(cProgress_, m_cFileAccess.getPath(), Message::VerifyOnGoing(name, i++, n), eTreatment_);

	//check total objectnum
	if(iTotalNum == 0)
	{
		if(!ObjectID::isInvalid(m_cCurrentTopPage.m_iLastObjectID))
		{
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_,
				m_cFileAccess.getPath(),
				Message::InconsistentHeader(iTotalNum,
										_iFirstObjectID,
										m_cCurrentTopPage.m_iLastObjectID));
		}
	}
	else 
	{
		//may be object isn't in same page
		ObjectID firstObj(_iFirstObjectID);
		ObjectID lastObj(m_cCurrentTopPage.m_iLastObjectID);
		int iObjectCount = lastObj.m_uiAreaID - firstObj.m_uiAreaID;
		iObjectCount += (lastObj.m_uiPageID - firstObj.m_uiPageID) * 
			m_cFileAccess.getFileID().getObjectNumberPerPage();
		if (iObjectCount + 1 != iTotalNum) 
		{
			_SYDNEY_VERIFY_INCONSISTENT(cProgress_, m_cFileAccess.getPath(),
				Message::DiscordObjectNum(m_cCurrentTopPage.m_iLastObjectID - _iFirstObjectID + 1,
										iTotalNum));

			return;
		}
	}
	// If the number of objects is 0, you may not do anything about either any further. 
	_SYDNEY_VERIFY_INFO(cProgress_, m_cFileAccess.getPath(), Message::VerifyOnGoing(name, i++, n), eTreatment_);

}

//	FUNCTION private
//	Record2::ObjectManager::verifyPageData --
//		The correspondence in the fixed length file is inspected. 
//
//	NOTES
//
//	ARGUMENTS
//  		 Admin::Verification::Treatment::Value eTreatment_
//			option of corresponding verification
//  		Admin::Verification::Progress& cProgress_
//			progress of corresponding verification
//
//	RETURN
//	None
//
//	EXCEPTIONS

Utility::ObjectNum
ObjectManager::
verifyPageData(Admin::Verification::Treatment::Value eTreatment_,
			   Admin::Verification::Progress& cProgress_)
{
	if (!m_cFileAccess.isMounted(m_cTrans))
		return 0;

	// Page has been attached and after readHeader
	; _SYDNEY_ASSERT(m_pPage);
	
	//read objectnumber of one page
	const char* pPointer = static_cast<const char*>(*m_pPage);
	Os::Memory::copy(&m_iObjectNumber, pPointer, _ObjectNumArchiveSize);

	//prepare while loop
	Utility::ObjectNum iNumPerPage = 	m_cFileAccess.getFileID().getObjectNumberPerPage();
	
	ObjectID objID(m_pPage->getID(), 0);
	//skip to previous objectid then get next id
	m_iCurrentObjectID = objID.getValue() - 1;
	//get the max objectid in this page
	objID.m_uiAreaID = iNumPerPage - 1;
	ObjectID::Value iMaxObjectID = objID.getValue();
	
	Utility::ObjectNum iObjectNum = 0;
	ObjectStatus::Value eStatus = ObjectStatus::Used;
	m_iCurrentObjectID = getNextObjectID(m_iCurrentObjectID, eStatus, iMaxObjectID);

	//check the obejctdata including variable data
	while(!ObjectID::isInvalid(m_iCurrentObjectID))
	{
		if(eStatus == ObjectStatus::Inserted) iObjectNum++;
		//check the variable data
		if(m_pDataAccess)	m_pDataAccess->verify(m_iCurrentObjectID, eTreatment_, cProgress_);
		if (!cProgress_.isGood()) return iObjectNum;

		eStatus = ObjectStatus::Used;
		m_iCurrentObjectID = getNextObjectID(m_iCurrentObjectID, eStatus, iMaxObjectID);
	}

	//check the objectnumber
	if (iObjectNum != m_iObjectNumber) 
	{
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			m_cFileAccess.getPath(),
			Message::InconsistentPageObjectNumber(objID.m_uiPageID,
												m_iObjectNumber,
												iObjectNum));
		// Because the following processing is not done, detach is done. 
		detach(Operation::Verify);
	}

#ifdef DEBUG
		SydRecordDebugMessage
			<< "Verify page data operation: PageID="
			<< objID.m_uiPageID
			<< " ObjectNumber="
			<< m_iObjectNumber
			<< ModEndl;
#endif

	return m_iObjectNumber;
}

//	FUNCTION private
//	Record2::ObjectManager::verifyLastObjectID --
//		The correspondence in the fixed length file is inspected. 
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Treatment::Value eTreatment_
//		Inspection method of correspondence inspection
//   Admin::Verification::Progress& cProgress_
//		Reference to process of correspondence inspection
//
//	RETURN
//	None
//
//	EXCEPTIONS

void
ObjectManager::
verifyLastObjectID(Admin::Verification::Treatment::Value eTreatment_,
				   Admin::Verification::Progress& cProgress_)
{
	if (!m_cFileAccess.isMounted(m_cTrans))
		return;

	ObjectID::Value lastObjectID = m_cCurrentTopPage.m_iLastObjectID;

	// Seek is done to final object ID. 
	if (!seek(lastObjectID, Operation::Verify)) 
	{
		// The last object is correct. 
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			m_cFileAccess.getPath(),
			Message::ObjectNotFound(lastObjectID));
		return;
	}
	try 
	{
		ObjectStatus::Value eStatus = ObjectStatus::Used;
		ObjectID::Value iNextID = getNextObjectID(lastObjectID, eStatus);
		if (!ObjectID::isInvalid(iNextID)) 
		{
			// There is an object after finality. 
			_SYDNEY_VERIFY_INCONSISTENT(
				cProgress_,
				m_cFileAccess.getPath(),
				Message::ExistObjectAfterLast(lastObjectID, iNextID));
			detach();
			return;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detach();
		_SYDNEY_RETHROW;
	}
	detach();
}

//	FUNCTION private
//	Record2::ObjectManager::verifyExpungedList --
//		The correspondence in the fixed length file is inspected. 
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Treatment::Value eTreatment_
//		Inspection method of correspondence inspection
//   Admin::Verification::Progress& cProgress_
//		Reference to process of correspondence inspection
//
//	RETURN
//	None
//
//	EXCEPTIONS

Utility::ObjectNum 
ObjectManager::
verifyExpungedList(Admin::Verification::Treatment::Value eTreatment_,
					Admin::Verification::Progress& cProgress_)
{
	if (!m_cFileAccess.isMounted(m_cTrans))
		return 0;

	// It inspects it while sequentially tracing deletion object ID. 
	//m_iCurrentObjectID = m_cCurrentTopPage.m_iFirstExpungedObjectID;
	ObjectID::Value ullNextObjectID;
	ObjectID::Value ullPrevObjectID;
	int count = findObjectFromList(ObjectID::m_UndefinedValue, ullNextObjectID, ullPrevObjectID);
	if(count == -1)
	{
		//show inconsistent
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			m_cFileAccess.getPath(),
			Message::BadExpungedList(ullNextObjectID, ullPrevObjectID));
	}
	
	return static_cast<Utility::ObjectNum>(count);
}

//	FUNCTION private
//	Record2::ObjectManager::verifyFreeList --
//		The correspondence in the fixed length file is inspected. 
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Treatment::Value eTreatment_
//		Inspection method of correspondence inspection
//   Admin::Verification::Progress& cProgress_
//		Reference to process of correspondence inspection
//
//	RETURN
//	None
//
//	EXCEPTIONS

Utility::ObjectNum 
ObjectManager::
verifyFreeList(Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_)
{
	if (!m_cFileAccess.isMounted(m_cTrans))
		return 0;

	// It inspects it while sequentially tracing deletion object ID. 
	//m_iCurrentObjectID = m_cCurrentTopPage.m_iFirstFreeObjectID;
	ObjectID::Value ullNextObjectID;
	ObjectID::Value ullPrevObjectID;
	int count = findObjectFromList(ObjectID::m_UndefinedValue, ullNextObjectID, ullPrevObjectID, false);
	if(count == -1)
	{
		//show inconsistent
		_SYDNEY_VERIFY_INCONSISTENT(
			cProgress_,
			m_cFileAccess.getPath(),
			Message::BadFreeList(ullNextObjectID, ullPrevObjectID));
	}
	
	return static_cast<Utility::ObjectNum>(count);
}

//	FUNCTION public
//	Record2::ObjectManager::nextPage --
//
//	NOTES
//
//	ARGUMENTS
//   Admin::Verification::Progress* pProgress_ = 0
//		Pointer to process of correspondence inspection
//		When numbers except 0 are specified, verifyPage is used instead of attachPage. 

//
//	RETURN
//		Previous page moving ID
//		When it is not possible to move, Undefined is restored. 
//
//	EXCEPTIONS

PhysicalPageID
ObjectManager::
nextPage(PhysicalPageID uiPageID_, Admin::Verification::Progress* pProgress_)
{
	if (uiPageID_ == PhysicalFile::ConstValue::UndefinedPageID) 
	{
		// The header page is pointed at. 
		// The following page of the header is obtained below getNextPageID. 
		uiPageID_ = _uiTopPageID;
	} 
	else 
	{
		detach(Operation::Verify);
	}

	uiPageID_ = m_pFile->getNextPageID(m_cTrans, uiPageID_);
	if (uiPageID_ != PhysicalFile::ConstValue::UndefinedPageID) 
	{
		//get current objectid
		ObjectID objID(uiPageID_, 0);
		m_iCurrentObjectID = objID.getValue();
		attach(Operation::Verify, pProgress_);
		if (pProgress_ && !pProgress_->isGood()) 
		{
			return uiPageID_ = PhysicalFile::ConstValue::UndefinedPageID;
		}

		makePageHeader(m_pPage, Operation::Verify);

		//judge whether exist used object
		objID.m_uiAreaID = m_cFileAccess.getFileID().getObjectNumberPerPage() - 1;
		ObjectStatus::Value eStatus = ObjectStatus::Used;
		m_iCurrentObjectID = getNextObjectID(m_iCurrentObjectID - 1, eStatus, objID.getValue());

		if(ObjectID::isInvalid(m_iCurrentObjectID))
		{
			objID.m_uiAreaID = PhysicalFile::ConstValue::UndefinedAreaID;
			m_iCurrentObjectID = objID.getValue();
		} 
		else if(m_iObjectNumber == 0 && eStatus == ObjectStatus::Inserted)
		{

		}
	}

	return uiPageID_;
}

//
//	FUNCTION private
//	Record2::ObjectManager::readSyncProgress -- It accesses the area of the status. 
//
//	NOTES
//  It accesses the area of the status. 
//
//	ARGUMENTS
//	None
//
//	RETURN
//	SyncProgress::Value eProgress_
//
//	EXCEPTIONS
//	None
//
ObjectManager::SyncProgress::Value
ObjectManager::
readSyncProgress()
{
	; _SYDNEY_ASSERT(m_pConstTopPointer);
	SyncProgress::Value value;
	Os::Memory::copy(&value, m_pConstTopPointer, sizeof(SyncProgress::Value));
	return value;
//	return *syd_reinterpret_cast<const SyncProgress::Value*>(m_pConstTopPointer);
}

//
//	FUNCTION private
//	Record2::ObjectManager::writeSyncProgress -- It accesses the area of the status. 
//
//	NOTES
//  It accesses the area of the status. 
//
//	ARGUMENTS
//	SyncProgress::Value			eProgress_
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
void
ObjectManager::
writeSyncProgress(SyncProgress::Value eProgress_)
{
	; _SYDNEY_ASSERT(m_pTopPointer);
	Os::Memory::copy(m_pTopPointer, &eProgress_, sizeof(SyncProgress::Value));
//	*syd_reinterpret_cast<SyncProgress::Value*>(m_pTopPointer) = eProgress_;
}

//
//	FUNCTION private
//	Record2::ObjectManager::readPageHeader -- 
//
//	NOTES
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
const char* 
ObjectManager::
makePageHeader(PhysicalPage* pPage_, Operation::Value eOperation_)
{
	; _SYDNEY_ASSERT(pPage_);

	//get header pointer
	const char* pConstPointer = static_cast<const char*>(pPage_->operator const void*());
	Os::Memory::copy(&m_iObjectNumber, pConstPointer, sizeof(Utility::Size));
	//change object num
	if(eOperation_ == Operation::Insert || eOperation_ == Operation::Batch)
	{
		++m_iObjectNumber;
		char* pPointer = const_cast<char*>(pConstPointer);
		Os::Memory::copy(pPointer, &m_iObjectNumber, sizeof(Utility::Size));
	}
	else if(eOperation_ == Operation::Expunge)
	{
		; _SYDNEY_ASSERT(m_iObjectNumber != 0);
		--m_iObjectNumber;
		char* pPointer = const_cast<char*>(pConstPointer);
		Os::Memory::copy(pPointer, &m_iObjectNumber, sizeof(Utility::Size));
	}
	pConstPointer += sizeof(Utility::Size);

	return pConstPointer;
}

//	FUNCTION public
//	Record2::ObjectManager::locate -- locate
//
//	NOTES
//		-- move to real position
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

const char*
ObjectManager::
locate(const char* pPointer_, const PhysicalAreaID uiAreaID_)
{
	; _SYDNEY_ASSERT(pPointer_);

	//get fixed object size
	pPointer_ += uiAreaID_ * m_cFileAccess.getFileID().getObjectSize();

	//check validate

	return pPointer_;
}

//
//	FUNCTION private
//	Record2::ObjectManager::readFirstBlock -- The data member in the first block is read. 
//
//	NOTES
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
void
ObjectManager::
readFirstBlock()
{
	readBlock(m_pConstTopPointer + _FirstBlockOffset);
}

//
//	FUNCTION private
//	Record2::ObjectManager::writeFirstBlock -- The data member is written in the first block. 
//
//	NOTES
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
void
ObjectManager::
writeFirstBlock()
{
	writeBlock(m_pTopPointer + _FirstBlockOffset);
}

//
//	FUNCTION private
//	Record2::ObjectManager::readSecondBlock -- The data member in the first block is read. 

//
//	NOTES
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
void
ObjectManager::
readSecondBlock()
{
	readBlock(m_pConstTopPointer + _SecondBlockOffset);
}

//
//	FUNCTION private
//	Record2::ObjectManager::writeSecondBlock -- The data member is written in the first block. 
//
//	NOTES
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
void
ObjectManager::
writeSecondBlock()
{
	writeBlock(m_pTopPointer + _SecondBlockOffset);
}

//
//	FUNCTION private
//	Record2::ObjectManager::readBlock -- The block is read. 
//
//	NOTES
//
//	ARGUMENTS
//	const char* pszPointer_
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
void
ObjectManager::
readBlock(const char* pszPointer_)
{
	const char* pPointer = pszPointer_;

	// Version of file
	int iVersion = 0;
	Os::Memory::copy(&iVersion, pPointer, sizeof(int));
	if (iVersion < 0 || iVersion >= FileID::Version::VersionNum) 
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	m_cCurrentTopPage.m_eFileVersion = static_cast<FileID::Version::Value>(iVersion);
	pPointer += sizeof(int);
	
	// Last update time
//	pPointer = FileCommon::DataManager::readFixedCommonData(m_cCurrentTopPage.m_cLastModifiedTime, pPointer);
	pPointer += _TimeSize;

	// Number of objects that has been inserted
	Os::Memory::copy(&m_cCurrentTopPage.m_uiInsertedObjectNum, pPointer, _ObjectNumArchiveSize);
	MSGLIN( SydRecordTraceMessage << "Record2 object num: " << m_cCurrentTopPage.m_uiInsertedObjectNum << ModEndl );
	pPointer += _ObjectNumArchiveSize;

	// Object flag of the final object
	pPointer += ObjectID::readValue(pPointer, m_cCurrentTopPage.m_iLastObjectID);

	// Object flag of the first unused object
	pPointer += ObjectID::readValue(pPointer, m_cCurrentTopPage.m_iFirstFreeObjectID);

	//read first free expunged objectid
	pPointer += ObjectID::readValue(pPointer, m_cCurrentTopPage.m_iFirstExpungedObjectID);

	// The data when having finished reading is preserved.
	m_cBackupTopPage = m_cCurrentTopPage;
	m_cBackupTopPage.m_bDirty = m_cCurrentTopPage.m_bDirty = false;
}

//
//	FUNCTION private
//	Record2::ObjectManager::writeBlock -- write the block
//
//	NOTES
//
//	ARGUMENTS
//	char* pszPointer_
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None
//
void
ObjectManager::
writeBlock(char* pszPointer_)
{
	char* pPointer = pszPointer_;

	// File version
	int iFileVersion = static_cast<int>(m_cCurrentTopPage.m_eFileVersion);
	Os::Memory::copy(pPointer, &iFileVersion, sizeof(int));
//	*syd_reinterpret_cast<int*>(pPointer) = static_cast<int>(m_cCurrentTopPage.m_eFileVersion);
	pPointer += sizeof(int);

	// Last update time
//	pPointer = FileCommon::DataManager::writeFixedCommonData(m_cCurrentTopPage.m_cLastModifiedTime, pPointer);
	pPointer += _TimeSize;
	; _SYDNEY_ASSERT(pPointer == pszPointer_ + sizeof(int) + _TimeSize);

	// Number of objects that has been inserted
	Os::Memory::copy(pPointer, &m_cCurrentTopPage.m_uiInsertedObjectNum, _ObjectNumArchiveSize);
//	*syd_reinterpret_cast<Utility::ObjectNum*>(pPointer) = m_cCurrentTopPage.m_uiInsertedObjectNum;
	pPointer += _ObjectNumArchiveSize;
	; _SYDNEY_ASSERT(pPointer == pszPointer_ + sizeof(int) + 
		_TimeSize + _ObjectNumArchiveSize);

	// Object flag of the final object
	pPointer += ObjectID::writeValue(pPointer, m_cCurrentTopPage.m_iLastObjectID);
	; _SYDNEY_ASSERT(pPointer == pszPointer_ + sizeof(int) + 
		_TimeSize + _ObjectNumArchiveSize + ObjectID::m_ArchiveSize);

	// Object flag of the first unused object
	pPointer += ObjectID::writeValue(pPointer, m_cCurrentTopPage.m_iFirstFreeObjectID);
	; _SYDNEY_ASSERT(pPointer == pszPointer_ + sizeof(int) + 
		_TimeSize + _ObjectNumArchiveSize + ObjectID::m_ArchiveSize * 2);

	// write first free expunged object id
	pPointer += ObjectID::writeValue(pPointer, m_cCurrentTopPage.m_iFirstExpungedObjectID);
	; _SYDNEY_ASSERT(pPointer == pszPointer_ + sizeof(int) + 
		_TimeSize + _ObjectNumArchiveSize + ObjectID::m_ArchiveSize * 3);

	; _SYDNEY_ASSERT(pPointer == pszPointer_ + _BlockSize);

	// The page was changed.
	m_pTopPage->dirty();
}

//	FUNCTION public
//	Record2::ObjectManager::attach --
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
ObjectManager::
attach(Operation::Value eOperation_, Admin::Verification::Progress* pProgress_)
{
	//may be batch mode
	if (eOperation_ == Operation::Batch) m_pPage = m_pBatchPage;

	m_pPage = doAttach(ObjectID(m_iCurrentObjectID).m_uiPageID, m_pPage, eOperation_, pProgress_);

	//if is batch, cache it
	if (eOperation_ == Operation::Batch) m_pBatchPage = m_pPage;
}

//	FUNCTION public
//	Record2::ObjectManager::detach --
//
//	NOTES
//
//	ARGUMENTS
//		Record2::Manager::Operation::Value eOperation_
//			Operation type
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
ObjectManager::
detach(/*ObjectID::Value ullObjectID_, */Operation::Value eOperation_, bool bAll_)
{
	if (!m_cTrans.isNoVersion())
	{
		//clearCachePage();
	}
	else
	{
		//maybe in batch insert mode
		if(m_cFileAccess.isBatch()) eOperation_ = Operation::Batch;

		//detach really
		doDetach(m_pPage, eOperation_);

		if(bAll_)
		{
			AttachedPageMap::Iterator iterator = m_mapAttachedPage.begin();
			for(; iterator != m_mapAttachedPage.end(); ++ iterator)
			{
				doDetach((*iterator).second, eOperation_);
			}
		}
	}
}

//	FUNCTION private
//	Record::ObjectManager::attachTopPage -- The top page is Atattied. 
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Page::FixMode::Value eFixMode_
//			Fix mode
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None

void
ObjectManager::
attachTopPage(Operation::Value eOperation_)
{
	//attach m_pTopPage first
	if (!m_pTopPage) m_pTopPage = doAttach(_uiTopPageID, m_pTopPage, eOperation_);

	//get the pointer of buffer
	if (_FixModeTable[eOperation_] == Buffer::Page::FixMode::ReadOnly) 
	{
		m_pConstTopPointer = static_cast<const char*>(m_pTopPage->operator const void*());
	} 
	else 
	{
		m_pTopPointer = static_cast<char*>(m_pTopPage->operator void*());
	}
}

//	FUNCTION private
//	Record::ObjectManager::detachTopPage -- The top page is Detattied. 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None

void
ObjectManager::
detachTopPage(bool bForce_ /* = false */)
{
	//if batch then have to detach
	if(m_cFileAccess.isBatch()) bForce_ = true;

	doDetach(m_pTopPage, Operation::Other, bForce_);

	m_pTopPointer = 0;
	// Because it is union, it becomes 0 at the same time. 
	; _SYDNEY_ASSERT(!m_pConstTopPointer);	

	m_eStatus = NotDirty;
}

//	FUNCTION private
//	Record2::ObjectManager::doAttach --
//
//	NOTES
//
//	ARGUMENTS
//		PhysicalPageID iPageID_
//				Page Atattied ID
//				The value that Iteratar is indicating now is used when omitting it. 
//		Record2::Manager::Operation::Value eOperation_
//			operation type
//	   Admin::Verification::Progress* pProgress_ = 0
//			Pointer to process of correspondence inspection
//			When numbers except 0 are specified, verifyPage is used instead of attach. 
//
//	RETURN
//		Excluding 0 when you can acquire page descriptor
//
//	EXCEPTIONS


PhysicalPage* 
ObjectManager::
doAttach(PhysicalPageID uiPageID_, 
		PhysicalPage* pPrevPage_,
		Operation::Value eOperation_/* = Operation::Read*/,
		Admin::Verification::Progress* pProgress_)
{
	//maybe in batch insert mode
	if(m_cFileAccess.isBatch()) eOperation_ = Operation::Batch;

	PhysicalPage* pPage = 0;
	//initialize page
	//if page not equals to pPrevPage
	if (pPrevPage_ && pPrevPage_->getID() != uiPageID_ && 
		pPrevPage_ != m_pBatchPage && 
		!findPageFromCache(pPrevPage_->getID()))	
	{
		doDetach(pPrevPage_, m_eOperation, true);
		m_pBatchPage = 0;
	}

	//if required page equals to prevpage, than do...
	if(pPrevPage_ && pPrevPage_->getID() == uiPageID_)
	{
		pPage = pPrevPage_;
	}	
	else 
	{
		//find it in cache map
		pPage = findPageFromCache(uiPageID_);
		//if find and readonly, must detach it first
		//if(pPage && eOperation_ == Operation::Read)
		//{
		//	doDetach(pPage, Operation::Update, true);
		//}

		//if not find it in map
		if (!pPage)
		{
			if (eOperation_ == Operation::Verify && pProgress_) 
			{
				pPage = m_pFile->verifyPage(m_cTrans, uiPageID_, _FixModeTable[eOperation_], *pProgress_);

#ifdef DEBUG
				SydRecordDebugMessage
				<< "Verify page: PageID="
				<< uiPageID_
				<< " with operation="
				<< getOperationValue(eOperation_)
				<< ModEndl;
#endif
				// progress isn't good
				if (!pProgress_->isGood() && pPage)
				{
					doDetach(pPage, eOperation_);
				}
			}
			//attach page really
			else 
			{
				pPage = m_pFile->attachPage(m_cTrans, uiPageID_, _FixModeTable[eOperation_], _ePriority);
#ifdef DEBUG
				SydRecordDebugMessage
				<< "Attach page: PageID="
				<< uiPageID_
				<< " with operation="
				<< getOperationValue(eOperation_)
				<< ModEndl;
#endif
			}

			//sometimes not insert such as update (first read and second insert)
			 if (uiPageID_ != _uiTopPageID &&
					eOperation_ != Operation::Read && 
					eOperation_ != Operation::Verify &&
					eOperation_ != Operation::Batch) 
			{
				m_mapAttachedPage.insert(uiPageID_, pPage);
			}
		}

		//m_eOperation = eOperation_;
	}

	; _SYDNEY_ASSERT(pPage && pPage->getID() == uiPageID_);

	return pPage;
}

//	FUNCTION public
//	Record2::ObjectManager::doDetach --
//
//	NOTES
//
//	ARGUMENTS
//		Record2::Manager::Operation::Value eOperation_
//			Operation type
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
ObjectManager::
doDetach(PhysicalPage*& pPage_, Operation::Value eOperation_, bool bForce_)
{
	if(pPage_)
	{
		if (_UnfixModeTable[eOperation_] == PhysicalPage::UnfixMode::Dirty) 
		{
			// Only the Dirty flag is hoisted no Detatti actually because of discardable it here. 
			pPage_->dirty();
		}

		PhysicalPageID uiPageID = pPage_->getID();
		AttachedPageMap::Iterator iterator = m_mapAttachedPage.find(uiPageID);
		// It is not managed by the map. 
		if (bForce_ ||	iterator == m_mapAttachedPage.end())
		{
			//if batch mode then do nothing
			if(!bForce_ && m_cFileAccess.isBatch()) return;

#ifdef DEBUG
			SydRecordDebugMessage
			<< "Detach page: PageID="
			<< uiPageID
			<< " with operation="
			<< getOperationValue(eOperation_)
			<< ModEndl;
#endif
			//detach it
			m_pFile->detachPage(pPage_, _UnfixModeTable[eOperation_]);
			//if in cache then remove it
			if(bForce_ && iterator != m_mapAttachedPage.end()) 
				m_mapAttachedPage.erase(iterator);

			pPage_ = 0;
		}
	}
}

//	FUNCTION public
//	Record::ObjectManager::findPageFromCache -- find one page if exist
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	None
//
//	EXCEPTIONS
//	None

PhysicalPage* 
ObjectManager::
findPageFromCache(PhysicalPageID iPageID_)
{
	PhysicalPage* pPage = 0;
	//find it in cache map
	if (!m_mapAttachedPage.isEmpty())
	{
		AttachedPageMap::Iterator iterator = m_mapAttachedPage.find(iPageID_);
		if (iterator != m_mapAttachedPage.end())
			pPage = (*iterator).second;
	}
	
	return pPage;
}

//	FUNCTION public
//	Record2::ObjectManager::detachAll --
//		All physical page Atattied descriptors are Detattied. 
//
//	NOTES
//		All physical page Atattied descriptors are Detattied. 
//
//	ARGUMENTS
//		bool bSucceeded_
//			true: the normal termination is processed. 
//			false: the change to the page to which attach is done is thrown away. 
//
//	RETURN
//		None
//
//	EXCEPTIONS

void
ObjectManager::
detachAll(bool bSucceeded_)
{
	try
	{
		//detach current page
		if(m_eOperation == Operation::Read) detach();
		
		if (bSucceeded_
			|| m_cTrans.getCategory() == Trans::Transaction::Category::ReadOnly
			|| m_cFileAccess.isBatch())
		{
			// Normal termination etc.

			// It makes it to a success always and the same processing at the batch mode. 
			// It is returned to the version when the transaction begins by recover 
			// in a high-ranking layer in abnormal circumstances. 

			// All physical page Atattied descriptors are Detattied.

#ifdef DEBUG
			SydRecordDebugMessage
			<< "Detach all pages."
			<< ModEndl;
#endif

			m_pFile->detachPageAll();

			// Hither to refer to the same page is removed 
			// because it previously detachAll()s it. 
			m_pPage = 0; 
		}
		else
		{
			// Abnormal termination etc.

			// All physical page Atattied descriptors are Detattied, 
			// and it returns it to the state before the content of the page is Atattied. 

			//
			// The file that stores the representative object uses it. 
			// A physical file is "Physical file with the physical page 
			// maintenance function. "The transaction descriptor calls 
			// PhysicalFile::File::recoverPageAll() where there is no argument in 
			// PhysicalFile::File::recoverPageAll() of a physical file with the 
			// physical page maintenance function because it is unnecessary. 
			//

			m_pFile->recoverPageAll();

			// When failing, ObjectManager is written and it returns it. 
			m_pPage = 0; 

			recover();
		}
		
		//clear cache
		m_mapAttachedPage.erase(m_mapAttachedPage.begin(), m_mapAttachedPage.end());
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

//	FUNCTION private
//	Record2::ObjectManager::substanceFile -- substanceFile
//
//	NOTES
//		-- substanceFile
//	ARGUMENTS
//		None
//
//	RETURN
//		None
//
//	EXCEPTIONS

void 
ObjectManager::
substanceFile()
{
	//may needn't create file
	if(m_cFileAccess.isMounted(m_cTrans)) 
		return;

	m_cFileAccess.substantiate();

	try 
	{
		// The header page is made. 
		// The page is allocated and administrative information is written. 
		m_pFile->allocatePage(m_cTrans, _uiTopPageID);
		// The constructor puts an initial value in the member. 
		touch();
		sync();

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detachAll(false);
		_SYDNEY_RETHROW;
	}

	detachAll(true);

}

//
//	Copyright (c) 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
