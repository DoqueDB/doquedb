// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectManager.h -- manage fixed page and object.
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

#ifndef __SYDNEY_RECORD2_OBJECTMANAGER_H
#define __SYDNEY_RECORD2_OBJECTMANAGER_H

#include "Record2/Manager.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

class DataAccess;

//
//	CLASS
//	Record2::ObjectManager -- 
//		ObjectManager
//	NOTES
// Manage class for attaching page, locating the record position.
// This class provides method to allocate, seek and iterate Object.
// Maybe compact and undoExpunge should be implement by it.
//
class ObjectManager : public Manager
{

public:

	struct ObjectStatus 
	{
		enum Value 
		{
			Initialize = 0,		// Empty, can do next operation
			Inserted,				// Insert status
			Expunged,				// Has been expunged
			Free,					// Undo expunged
			Used,					// Used status, including Inserted, Expunged & Free
			Invalid					// Illegal object
		};
	};

	//SyncProgress
	struct SyncProgress 
	{
		typedef unsigned char	Value;
		enum _Value 
		{
			NotWriting = 0x00,
			WritingFirstBlock = 0x01,
			WritingSecondBlock = 0x02
		};
	};

    //constructor
    ObjectManager(File& cFileAccess_,
					Operation::Value eOperation_,
					Admin::Verification::Progress* pProgress_ = 0);

	//destructor
	~ObjectManager();

//*********************************************************************

	//allocate objectid, create a new record
    ObjectID::Value allocate();

    //find position by OID, should distinguish operation
    const char* seek(ObjectID::Value ullObjectID_, 
					Operation::Value eOperation_ = Operation::Read,
					bool bKeepAttach_ = true);

    // get the next position as follows.
    const char* next();

	//expunge objectid, used in expunged and update
    void expunge(const ObjectID::Value ullObjectID_);
    // undo expunge
    bool undoExpunge(const ObjectID::Value ullObjectID_);

	// undo update
    bool undoUpdate(const ObjectID::Value ullObjectID_);

    // compact, the data will delete really
	Utility::ObjectNum compact(const ObjectID::Value ullObjectID_ = ObjectID::m_UndefinedValue);

//*********************************************************************
    //mark
    void mark();

    //rewind
    void rewind();

    //reset
    void reset();

    //Correct one of administrative information in the file is read to the member variable. 
	// When DoRepair _ is true, it mends. 
	void reload(const bool bDoRepair_ = false);

    //The data member is written in the file.
    void sync();

    //It returns it when writing it. 
    void recover();

    //It puts it into the state of dirty.
    void touch();

	//isValid
	//bool isValid() const;

	//substance file
	void substanceFile();

//*********************************************************************
	//whether exist variable file, not variable fields
	bool existVariableFile() const;

    //The number of objects that has been inserted is acquired.
    Utility::ObjectNum getInsertedObjectNum() const;

	//set data access sometimes
	void setDataAccess(DataAccess* pDataAccess_);
//*********************************************************************

	// The page for administrative information Atatti/is Detattied. 
	void attach(Operation::Value eOperation_ = Operation::Read,
				Admin::Verification::Progress* pProgress_ = 0);
	// Page descriptor of the place of Iteratar is liberated. 
	void detach(/*ObjectID::Value ullObjectID_ = ObjectID::m_UndefinedValue,*/
				Operation::Value eOperation_ = Operation::Read,
				bool bAll_ = false);

	// All pages are Detattied.
	void detachAll(bool bSucceeded_ = true);

//*********************************************************************
	//
	Utility::ObjectNum verifyBody(Admin::Verification::Treatment::Value eTreatment_,
								  Admin::Verification::Progress& cProgress_);
	
	//The correspondence of the content is inspected. 
	void verifyHeader(Utility::ObjectNum iObjectNum_, 
						Admin::Verification::Treatment::Value eTreatment_,
						Admin::Verification::Progress& cProgress_);
	
private:

	//locate in 1 page position
	const char* locate(const char* pPointer_, const PhysicalAreaID uiAreaID_);

	// read/write Object header include in record header
	ObjectStatus::Value readObjectHeader(const ObjectID::Value ullObjectID_, 
							ObjectID::Value& ullNextObjectID_,
							Utility::TransID& iTransID_);
	ObjectStatus::Value readObjectHeader(const ObjectID::Value ullObjectID_, ObjectID::Value& ullNextObjectID_);
	ObjectStatus::Value readObjectHeader(const ObjectID::Value ullObjectID_);

	void writeObjectHeader(const ObjectID::Value ullObjectID_, 
							const ObjectID::Value ullNextObjectID_, 
							const Utility::TransID iTransID_ = 0,
							Operation::Value eOperation_ = Operation::Read);

	//find a special object from expunged or free list
	int findObjectFromList(ObjectID::Value ullObjectID_, 
							ObjectID::Value& ullNextObjectID_,
							ObjectID::Value& ullPrevObjectID_, 
							bool bExpunged_ = true);

	// Iteratar obtains and the following front one at pointed position use inside obtains object ID. 
	ObjectID::Value getNextObjectID(const ObjectID::Value ullObjectID_, 
							ObjectStatus::Value& eStatus_, 
							ObjectID::Value iMaxObjectID_ = ObjectID::m_UndefinedValue);

	// attach/detach linked page
	PhysicalPage* attachLinkedPage(PhysicalPageID iPageID_, 
					bool isReadOnly_ = true,
					bool bKeepAttach_ = true);
	void detachLinkedPage(PhysicalPage* pPage_);

	// attach/detach page really
	PhysicalPage* doAttach(PhysicalPageID uiPageID_, 
					PhysicalPage* pPrevPage_, 
					Operation::Value eOperation_,
					Admin::Verification::Progress* pProgress_ = 0);
	void doDetach(PhysicalPage*& pPage_, //should set detach page to null
					Operation::Value eOperation_,
					bool bForce_ = false);

	//find one page if exist
	PhysicalPage* findPageFromCache(PhysicalPageID iPageID_);

    //If it is necessary as the header information is kept correct by 
	//processing where it goes to the object, it corrects it. 
	void validate(const ObjectID::Value ullObjectID_ = ObjectID::m_UndefinedValue, 
				Operation::Value eOperation_ = Operation::Read);
//*********************************************************************
	//verify the page content
	Utility::ObjectNum verifyPageData(Admin::Verification::Treatment::Value eTreatment_,
									  Admin::Verification::Progress& cProgress_);

	// Subcontract of verify
	void verifyLastObjectID(Admin::Verification::Treatment::Value eTreatment_,
								Admin::Verification::Progress& cProgress_);
	Utility::ObjectNum verifyExpungedList(Admin::Verification::Treatment::Value eTreatment_,
								Admin::Verification::Progress& cProgress_);
	Utility::ObjectNum verifyFreeList(Admin::Verification::Treatment::Value eTreatment_,
								Admin::Verification::Progress& cProgress_);
	//
	PhysicalPageID nextPage(PhysicalPageID uiPageID_, Admin::Verification::Progress* pProgress_);

//*********************************************************************
	// The page for administrative information Atatti/is Detattied. 
	void attachTopPage(Operation::Value eOperation_);
	void detachTopPage(bool bForce_ = false);

	//make page header (object number or more)
	const char* makePageHeader(PhysicalPage* pPage_, Operation::Value eOperation_ = Operation::Read);

	// The data member is read and written in the first block (the second block). 
	void readFirstBlock();
	void writeFirstBlock();

	void readSecondBlock();
	void writeSecondBlock();

	// The block is read and written. 
	void readBlock(const char* pszPointer_);
	void writeBlock(char* pszPointer_);

	// It accesses the value that shows the advancement condition of function sync. 
	SyncProgress::Value readSyncProgress();
	void writeSyncProgress(SyncProgress::Value eProgress_);

//*********************************************************************

	struct TopPage
	{
		//inserted object number
		Utility::ObjectNum m_uiInsertedObjectNum;

		//last fixed OID
		ObjectID::Value m_iLastObjectID;

		//first free fixed OID
		ObjectID::Value m_iFirstFreeObjectID;

		//first expunged OID
		ObjectID::Value m_iFirstExpungedObjectID;

		//file version
		FileID::Version::Value m_eFileVersion;

		//Has the content changed?
		bool m_bDirty;

		//default constructor
		TopPage()
			: m_uiInsertedObjectNum(0),
			  m_iLastObjectID(ObjectID::m_UndefinedValue),
			  m_iFirstFreeObjectID(ObjectID::m_UndefinedValue),
			  m_iFirstExpungedObjectID(ObjectID::m_UndefinedValue),
			  m_eFileVersion(FileID::Version::CurrentVersion),
			  m_bDirty(false)
		{}

		//assign
		TopPage& operator= (const TopPage& cTopPage_)
		{
			m_uiInsertedObjectNum = cTopPage_.m_uiInsertedObjectNum;
			m_iLastObjectID = cTopPage_.m_iLastObjectID;
			m_iFirstFreeObjectID = cTopPage_.m_iFirstFreeObjectID;
			m_iFirstExpungedObjectID = cTopPage_.m_iFirstExpungedObjectID;
			m_eFileVersion = cTopPage_.m_eFileVersion;
			m_bDirty = cTopPage_.m_bDirty;
			return *this;
		}
	};

	//current headerpage
	TopPage m_cCurrentTopPage;
	//backup headerpage
	TopPage m_cBackupTopPage;

//*********************************************************************

	// Variable in which result is written when correspondence is inspected
	Admin::Verification::Progress* m_pProgress;

	//attached page cache
	typedef ModMap<PhysicalPageID, PhysicalPage*, ModLess<PhysicalPageID> > AttachedPageMap;
	AttachedPageMap m_mapAttachedPage;

	// Top physical page
	PhysicalPage* m_pTopPage;

	// Descriptor on physical page in which administrative information is written
	PhysicalPage* m_pPage;

	// Cache page for batch mode
	PhysicalPage* m_pBatchPage;

	//Marked ObjectID
	ObjectID::Value m_iMarkedObjectID;

	//inserted object count of one page
	Utility::ObjectNum m_iObjectNumber;

	//data access pointer
	DataAccess* m_pDataAccess;

	// Head of address accessed page
	union 
	{
		char* m_pTopPointer;
		const char* m_pConstTopPointer;
	};

	// Was sync done? whether writing had been generated
	enum 
	{
		NotDirty,
		Dirty,
		Sync
	} m_eStatus;
};

//setDataAccess
inline
void 
ObjectManager::
setDataAccess(DataAccess* pDataAccess_)
{ 
	m_pDataAccess = pDataAccess_;
}


//getInsertedObjectNum
inline
Utility::ObjectNum 
ObjectManager::
getInsertedObjectNum() const
{
	return m_cCurrentTopPage.m_uiInsertedObjectNum;
}

//touch
inline
void
ObjectManager::
touch()
{
	m_cCurrentTopPage.m_bDirty = true;
}

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_OBJECTMANAGER_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
