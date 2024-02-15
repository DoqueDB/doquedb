// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/FileAccess.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_FILEACCESS_H
#define __SYDNEY_EXECUTION_ACTION_FILEACCESS_H

#include "Execution/Action/Module.h"
#include "Execution/Action/ActionList.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/Locker.h"
#include "Execution/Interface/IObject.h"
#include "Execution/Declaration.h"

#include "Common/Externalizable.h"

#include "Lock/Mode.h"
#include "Lock/Duration.h"

#include "LogicalFile/AutoLogicalFile.h"

#include "Opt/Algorithm.h"
#include "Opt/Declaration.h"

#include "Schema/File.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Action::FileAccess -- wrapping class for file access in various actions
//
//	NOTES
class FileAccess
	: public Interface::IObject
{
public:
	typedef Interface::IObject Super;
	typedef FileAccess This;

	// constructor
	static This* create(Interface::IProgram& cProgram_,
						Schema::Table* pSchemaTable_,
						Schema::File* pSchemaFile_,
						const LogicalFile::OpenOption& cOpenOption_);

	// destructor
	~FileAccess() {}

	// check open option
	bool isGetByBitSet();
	// check open option
	bool isBitSetSort();
	bool isCacheAllObject();

	// set startup action if needed
	void addStartUp(Interface::IProgram& cProgram_,
					int iActionID_,
					bool bBeforeOpen_ = false);
	// set previous bitset
	void setSearchByBitSet(int iBitSetID_);
	// set rankby bitset
	void setRankByBitSet(int iBitSetID_);

	// add lock action
	void addLock(Interface::IProgram& cProgram_,
				 const Locker::Argument& cArgument_,
				 int iDataID_);

	// explain fileaccess
	void explain(Opt::Environment* pEnvironment_,
				 Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_,
				 const Opt::ExplainFileArgument& cArgument_);
	void explainStartUp(Opt::Environment* pEnvironment_,
						Interface::IProgram& cProgram_,
						Opt::Explain& cExplain_);
	// initialize necessary members
	void initialize(Interface::IProgram& cProgram_);
	// end using members
	void terminate(Interface::IProgram& cProgram_);
	// create logicalfile instance
	void attach(Interface::IProgram& cProgram_);
	// open file
	bool open(Interface::IProgram& cProgram_);
	// get data
	bool getData(Interface::IProgram& cProgram_,
				 Common::DataArrayData* pData_);
	// get locator
	bool getLocator(Interface::IProgram& cProgram_,
					Common::DataArrayData* pKey_,
					Action::Locator* pLocator_);
	// get property
	void getProperty(Interface::IProgram& cProgram_,
					 Common::DataArrayData* pKey_,
					 Common::DataArrayData* pValue_);
	// insert
	void insert(Interface::IProgram& cProgram_,
				Common::DataArrayData* pData_);
	// update
	void update(Interface::IProgram& cProgram_,
				const Common::DataArrayData* pKey_,
				Common::DataArrayData* pData_);
	// expunge
	void expunge(Interface::IProgram& cProgram_,
				 const Common::DataArrayData* pKey_);
	// undo update
	void undoUpdate(Interface::IProgram& cProgram_,
					const Common::DataArrayData* pKey_);
	// undo expunge
	void undoExpunge(Interface::IProgram& cProgram_,
					 const Common::DataArrayData* pKey_);

	// reset iteration
	void reset();
	// close file
	void close();
	// fetch
	void fetch(const Common::DataArrayData* pOption_);

	// is opened
	bool isOpened();

	// accessor
	LogicalFile::OpenOption& getOpenOption() {return m_cOpenOption;}
	Schema::File* getSchemaFile() {return m_pSchemaFile;}
	LogicalFile::AutoLogicalFile& getLogicalFile() {return m_cLogicalFile;}
	ModUnicodeString getTableName() {return m_cTableName;}
	ModUnicodeString getName() {return m_cFileName;}
	int getLocker() {return m_cLocker.getID();}

	// for serialize
	static This* getInstance(int iCategory_);

///////////////////////////////
// Common::Externalizable
	int getClassID() const;

///////////////////////////////
// ModSerializer
	void serialize(ModArchive& archiver_);

protected:
private:
	//constructor
	FileAccess()
		: Super(),
		  m_pSchemaFile(0), // assigned in open
		  m_iDatabaseID(Schema::ObjectID::Invalid),
		  m_iSchemaID(Schema::ObjectID::Invalid),
		  m_cTableName(),
		  m_cFileName(),
		  m_cLogicalFile(),
		  m_cOpenOption(),
		  m_cStartUp(),
		  m_cStartUpBeforeOpen(),
		  m_cBitSet(),
		  m_cRankBitSet(),
		  m_cLocker()
	{}
	FileAccess(Schema::Table* pSchemaTable_,
			   Schema::File* pSchemaFile_,
			   const LogicalFile::OpenOption& cOpenOption_)
		: Super(),
		  m_pSchemaFile(pSchemaFile_),
		  m_iDatabaseID(pSchemaTable_->getDatabaseID()),
		  m_iSchemaID(pSchemaFile_->getID()),
		  m_cTableName(pSchemaTable_->getName()),
		  m_cFileName(pSchemaFile_->getName()),
		  m_cLogicalFile(),
		  m_cOpenOption(cOpenOption_),
		  m_cStartUp(),
		  m_cStartUpBeforeOpen(),
		  m_cBitSet(),
		  m_cRankBitSet(),
		  m_cLocker()
	{}

	// do once before open
	void startUpBeforeOpen(Interface::IProgram& cProgram_);
	// do once after open
	void startUp(Interface::IProgram& cProgram_);

	// register to program
	void registerToProgram(Interface::IProgram& cProgram_);

	Schema::File* m_pSchemaFile;
	Schema::Object::ID::Value m_iDatabaseID;
	Schema::Object::ID::Value m_iSchemaID;
	Schema::Object::Name m_cTableName;
	Schema::Object::Name m_cFileName;
	LogicalFile::AutoLogicalFile m_cLogicalFile;
	LogicalFile::OpenOption m_cOpenOption;

	Action::ActionList m_cStartUp;
	Action::ActionList m_cStartUpBeforeOpen;
	Action::BitSetHolder m_cBitSet;
	Action::BitSetHolder m_cRankBitSet;

	Action::LockerHolder m_cLocker;
};

///////////////////////////////////
// CLASS
//	Action::FileAccessHolder --
//
// NOTES

class FileAccessHolder
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef FileAccessHolder This;

	FileAccessHolder()
		: Super(),
		  m_iID(-1),
		  m_pFileAccess(0)
	{}
	FileAccessHolder(int iID_)
		: Super(),
		  m_iID(iID_),
		  m_pFileAccess(0)
	{}
	~FileAccessHolder() {}

	void explain(Opt::Environment* pEnvironment_,
				 Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_,
				 const Opt::ExplainFileArgument& cArgument_);
	void explainStartUp(Opt::Environment* pEnvironment_,
						Interface::IProgram& cProgram_,
						Opt::Explain& cExplain_);

	int getID() {return m_iID;}
	void setID(int iID_) {m_iID = iID_;}

	// initialize FileAccess instance
	void initialize(Interface::IProgram& cProgram_);
	// terminate FileAccess instance
	void terminate(Interface::IProgram& cProgram_);

	// -> operator
	FileAccess* operator->() const {return m_pFileAccess;}

	// accessor
	FileAccess* get() const {return m_pFileAccess;}
	bool isInitialized() {return m_pFileAccess != 0;}

	// serializer
	void serialize(ModArchive& archiver_);

protected:
private:
	int m_iID;
	FileAccess* m_pFileAccess;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_FILEACCESS_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
