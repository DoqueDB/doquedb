// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h -- Header file of class that secure/releases Object/Area
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

#ifndef __SYDNEY_RECORD2_MANAGER_H
#define __SYDNEY_RECORD2_MANAGER_H

#include "Record2/File.h"

_SYDNEY_BEGIN

_SYDNEY_RECORD2_BEGIN

//use a alias
typedef PhysicalFile::Page PhysicalPage;

//
//	CLASS
//	Record2::Manager -- Class that operates object/area, and maintains information
//
//	NOTES
//	
//	
class Manager : public Common::Object		// This class need re-write carefullly
{

public:

	// Kind of processing done to administrative information
	struct Operation 
	{
		enum Value 
		{
			Read = 0,				// Reading
			Write,					// Writing
			Insert,					// New insertion
			Update,					// Update
			Expunge,				// Expunge
			Verify,					// Correspondence inspection
			Batch,					// Insertion of batch mode
			Other,					// Other mode
			ValueNum
		};
	};

	// constructor
	Manager(File& cFileAccess_,
			Operation::Value eOperation_ = Operation::Read)
	: m_cFileAccess(cFileAccess_),
	  m_eOperation(eOperation_),
	  m_pFile(cFileAccess_.getFile()),
	  m_cTrans(cFileAccess_.getTransaction()),
	  m_iCurrentObjectID(ObjectID::m_UndefinedValue)
	{ 
		_SYDNEY_ASSERT(m_pFile != 0);
	}

	// destructor
	virtual ~Manager(){;}

	// get file
	const File& getFile() const
	{ return m_cFileAccess; }

	////one direct area is attached
	//virtual bool isAttached() const
	//{ return (m_cFile.m_pFile != 0); }

	////reset physical file
	//void setPhysicalFile(PhysicalFile::File* pFile_)
	//{ m_pFile = pFile_;}
//*********************************************************************
    //get current ObjectID
    ObjectID::Value getCurrentObjectID() const
	{ return m_iCurrentObjectID;}

    //set current ObjectID
    void setCurrentObjectID(ObjectID::Value ullObjectID_)
	{ m_iCurrentObjectID = ullObjectID_;}

//*********************************************************************
	// attach page/area from file
	//virtual void attach(ObjectID::Value ullObjectID_, Operation::Value eValue_ = Operation::Read){;}

    //detach page/area
	//virtual void detach(ObjectID::Value ullObjectID_, Operation::Value eValue_ = Operation::Read){;}

	// All pages/areas are Detattied. 
	virtual void detachAll(bool bSucceeded_ = true) = 0;

//*********************************************************************

	//load file or create it
	virtual void substanceFile() = 0;
	//here * read and write operation
	//......................
	//

//*********************************************************************
protected:

#ifdef DEBUG
	//get operation value when attach/detach
	static ModUnicodeString getOperationValue(Operation::Value eOperation_)
	{
		switch(eOperation_)
		{
		case Operation::Read: return "Read";
		case Operation::Write: return "Write";
		case Operation::Insert: return "Insert";
		case Operation::Update: return "Update";
		case Operation::Expunge: return "Expunge";
		case Operation::Verify: return "Verify";
		case Operation::Batch: return "Batch";
		case Operation::Other: return "Other";
		default: return "Error";
		}
	};
#endif

	// Data member

	// Transaction descriptor
	const Trans::Transaction&	m_cTrans;

	// File Handle
	File& m_cFileAccess;

	// PhysicalFile handle
	PhysicalFile::File* m_pFile;

	// Operation mode
	Operation::Value m_eOperation;

	//maybe needn't it
	//current varibale OID
	ObjectID::Value m_iCurrentObjectID;

};

namespace	//used for subclass, it's nood good, should move to Manager.cpp
{
	// Table to obtain FixMode from Operation::Value
	const Buffer::Page::FixMode::Value _FixModeTable[Manager::Operation::ValueNum] =
	{
		// Read
		Buffer::Page::FixMode::ReadOnly,
		// Write
		Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable,
		// Insert
		Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable,
		// Update
		Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable,
		// Expunge
		Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable,
		// Verify
		Buffer::Page::FixMode::ReadOnly,
		// Batch
		Buffer::Page::FixMode::Write,
		// Other
		Buffer::Page::FixMode::ReadOnly
	};

	// Table to obtain UnfixMode from Operation::Value
	const PhysicalPage::UnfixMode::Value
		_UnfixModeTable[Manager::Operation::ValueNum] =
	{
		// Read
		PhysicalPage::UnfixMode::NotDirty,
		// Write
		PhysicalPage::UnfixMode::Dirty,
		// Insert
		PhysicalPage::UnfixMode::Dirty,
		// Update
		PhysicalPage::UnfixMode::Dirty,
		// Expunge
		PhysicalPage::UnfixMode::Dirty,
		// Verify
		PhysicalPage::UnfixMode::NotDirty,
		// Batch
		PhysicalPage::UnfixMode::Dirty,
		// Other
		PhysicalPage::UnfixMode::Omit
	};
}

_SYDNEY_RECORD2_END
_SYDNEY_END

#endif // __SYDNEY_RECORD2_MANAGER_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
