// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/FileAccess.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Action/Class.h"
#include "Execution/Action/Collection.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Action/Locator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/FakeError.h"
#include "Execution/Utility/Transaction.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/NotSupported.h"
#include "Exception/SchemaObjectNotFound.h"
#include "Exception/TableNotFound.h"
#include "Exception/Unexpected.h"

#include "Lock/Name.h"

#include "LogicalFile/FileDriverManager.h"

#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/Environment.h"
#include "Opt/Explain.h"

#include "Plan/AccessPlan/Limit.h"
#include "Plan/File/Parameter.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Order/Specification.h"

#include "Schema/Database.h"
#include "Schema/File.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
// Execution::Action::FileAccess

// FUNCTION public
//	Action::FileAccess::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Schema::Table* pSchemaTable_
//	Schema::File* pSchemaFile_
//	const LogicalFile::OpenOption& cOpenOption_
//	
// RETURN
//	FileAccess*
//
// EXCEPTIONS

//static
FileAccess*
FileAccess::
create(Interface::IProgram& cProgram_,
	   Schema::Table* pSchemaTable_,
	   Schema::File* pSchemaFile_,
	   const LogicalFile::OpenOption& cOpenOption_)
{
	AUTOPOINTER<This> pFileAccess = new FileAccess(pSchemaTable_,
												   pSchemaFile_,
												   cOpenOption_);
	pFileAccess->registerToProgram(cProgram_);
	return pFileAccess.release();
}

// FUNCTION public
//	Action::FileAccess::isGetByBitSet -- check open option
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileAccess::
isGetByBitSet()
{
	return m_cOpenOption.getBoolean(LogicalFile::OpenOption::KeyNumber::GetByBitSet);
}


// FUNCTION public
//	Action::FileAccess::isBitSetSort -- check open option
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileAccess::
isBitSetSort()
{
	return m_cOpenOption.getBoolean(LogicalFile::OpenOption::KeyNumber::GroupBy);
}


// FUNCTION public
//	Action::FileAccess::isCacheAllObject -- check open option
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileAccess::
isCacheAllObject()
{
	return m_cOpenOption.getBoolean(LogicalFile::OpenOption::KeyNumber::CacheAllObject);
}

// FUNCTION public
//	Action::FileAccess::addStartUp -- set startup action if needed
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iActionID_
//	bool bBeforeOpen_ = false
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
addStartUp(Interface::IProgram& cProgram_,
		   int iActionID_,
		   bool bBeforeOpen_ /* = false */)
{
	if (bBeforeOpen_) {
		m_cStartUpBeforeOpen.addID(iActionID_);
	} else {
		m_cStartUp.addID(iActionID_);
	}
}

// FUNCTION public
//	Action::FileAccess::setSearchByBitSet -- set previous bitset
//
// NOTES
//
// ARGUMENTS
//	int iBitSetID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
setSearchByBitSet(int iBitSetID_)
{
	m_cBitSet.setDataID(iBitSetID_);
}

// FUNCTION public
//	Action::FileAccess::setRankByBitSet -- set rankby bitset
//
// NOTES
//
// ARGUMENTS
//	int iBitSetID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
setRankByBitSet(int iBitSetID_)
{
	m_cRankBitSet.setDataID(iBitSetID_);
}

// FUNCTION public
//	Action::FileAccess::addLock -- add lock action
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Locker::Argument& cArgument_
//	int iDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
addLock(Interface::IProgram& cProgram_,
		const Locker::Argument& cArgument_,
		int iDataID_)
{
	if (cArgument_.isNeedLock()) {
		Action::Locker* pLocker = 0;
		if (isBitSetSort()
			&&isCacheAllObject()) {
			pLocker = Locker::BitSetSort::create(cProgram_,
													this,
													cArgument_,
													iDataID_);
		} else if (isGetByBitSet()
				   && isCacheAllObject()) {
			pLocker = Locker::GetByBitSetCacheAllObject::create(cProgram_,
																this,
																cArgument_,
																iDataID_);
		} else if (isGetByBitSet()
			||isBitSetSort()) {
			pLocker = Locker::GetByBitSet::create(cProgram_,
												  this,
												  cArgument_,
												  iDataID_);
		} else if (isCacheAllObject()) {
			pLocker = Locker::CacheAllObject::create(cProgram_,
													 this,
													 cArgument_,
													 iDataID_);
		} else {
			pLocker = Locker::Normal::create(cProgram_,
											 this,
											 cArgument_,
											 iDataID_);
		}
		m_cLocker.setID(pLocker->getID());
	}
}

// FUNCTION public
//	Action::FileAccess::explain -- explain fileaccess
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_,
		const Opt::ExplainFileArgument& cArgument_)
{
	cExplain_.pushNoNewLine();

	if (pEnvironment_ && cArgument_.m_pTable) {
		const STRING& cstrTableName = cArgument_.m_pTable->getCorrelationName(*pEnvironment_);
		cExplain_.put(cstrTableName).put(".");
	}
	cExplain_.put(m_cFileName);

	if (cArgument_.m_bIsGetByBitSet) {
		cExplain_.put("[get by bitset]");
	}
	if (cArgument_.m_bIsSearchByBitSet) {
		cExplain_.put("[search by bitset");
		if (cExplain_.isOn(Opt::Explain::Option::Data)) {
			cExplain_.put(":#").put(m_cBitSet.getDataID());
		}
		cExplain_.put("]");
	}
	if (cArgument_.m_bIsLimited
		&& !cArgument_.m_bIsGetByBitSet) {
		cExplain_.put("[limited]");
	}
	if (m_cLocker.isValid()) {
		if (cProgram_.getLocker(m_cLocker.getID())->isPrepare()) {
			cExplain_.put("[lock?]");
		} else {
			cExplain_.put("[locked]");
		}
	}
	if (cExplain_.isOn(Opt::Explain::Option::Cost) && cArgument_.m_pParameter) {
		cArgument_.m_pParameter->getCost().explain(cExplain_);
	}
	bool bFirst = true;
	if (pEnvironment_ && cArgument_.m_pParameter) {
		if (cArgument_.m_pParameter->getPredicate()) {
			cExplain_.pushIndent();
			cExplain_.put(bFirst ? " for" : ",").newLine(true /* force */);
			cExplain_.popNoNewLine();

			cArgument_.m_pParameter->getPredicate()->explain(pEnvironment_, cExplain_);
			bFirst = false;

			cExplain_.pushNoNewLine();
			cExplain_.popIndent();
		}
		if (cArgument_.m_pParameter->getOrder()
			&& !cArgument_.m_bIsGetByBitSet) {
			cExplain_.pushIndent();
			cExplain_.put(bFirst ? " for" : ",").newLine(true /* force */);
			cExplain_.popNoNewLine();

			cArgument_.m_pParameter->getOrder()->explain(pEnvironment_, cExplain_);
			bFirst = false;

			cExplain_.pushNoNewLine();
			cExplain_.popIndent();
		}
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Action::FileAccess::explainStartUp -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
explainStartUp(Opt::Environment* pEnvironment_,
			   Interface::IProgram& cProgram_,
			   Opt::Explain& cExplain_)
{
	if (m_cStartUpBeforeOpen.getSize()) {
		cExplain_.pushIndent();
		cExplain_.newLine(true /* force */).put("do before open:");
		m_cStartUpBeforeOpen.explain(pEnvironment_, cProgram_, cExplain_);
		cExplain_.popIndent();
	}
	if (m_cStartUp.getSize()) {
		cExplain_.pushIndent();
		cExplain_.newLine(true /* force */).put("do after open:");
		m_cStartUp.explain(pEnvironment_, cProgram_, cExplain_);
		cExplain_.popIndent();
	}
}

// FUNCTION public
//	Action::FileAccess::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileAccess::
initialize(Interface::IProgram& cProgram_)
{
	if (m_iSchemaID == Schema::ObjectID::Invalid) {
		// get schemaID from names
		Schema::Table* pSchemaTable =
			cProgram_.getDatabase()->getTable(m_cTableName,
											  *cProgram_.getTransaction());
		if (pSchemaTable == 0) {
			_SYDNEY_THROW2(Exception::TableNotFound,
						   m_cTableName,
						   cProgram_.getDatabase()->getName());
		}

		Schema::File* pSchemaFile =
			pSchemaTable->getFile(m_cFileName,
								  *cProgram_.getTransaction());
		if (pSchemaFile == 0) {
			_SYDNEY_THROW0(Exception::SchemaObjectNotFound);
		}
		m_iDatabaseID = pSchemaFile->getDatabaseID();
		m_iSchemaID = pSchemaFile->getID();
		m_pSchemaFile = pSchemaFile;
	}
	if (m_cBitSet.isValid()) {
		m_cBitSet.initialize(cProgram_);
	}
	if (m_cRankBitSet.isValid()) {
		m_cRankBitSet.initialize(cProgram_);
	}
	if (m_cStartUp.getSize()) {
		m_cStartUp.initialize(cProgram_);
	}
	if (m_cStartUpBeforeOpen.getSize()) {
		m_cStartUpBeforeOpen.initialize(cProgram_);
	}
	if (m_cLocker.isValid()) {
		m_cLocker.initialize(cProgram_);
		m_cLocker->setFileAccess(this);
	}
}

// FUNCTION public
//	Action::FileAccess::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileAccess::
terminate(Interface::IProgram& cProgram_)
{
	if (m_pSchemaFile) {
		m_cLocker.terminate(cProgram_);

		m_cLogicalFile.close();
		m_pSchemaFile = 0;

		m_cStartUp.terminate(cProgram_);
		m_cStartUpBeforeOpen.terminate(cProgram_);
		m_cBitSet.terminate(cProgram_);
		m_cRankBitSet.terminate(cProgram_);
	}
}

// FUNCTION public
//	Action::FileAccess::attach -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
attach(Interface::IProgram& cProgram_)
{
	if (!m_pSchemaFile) {
		m_pSchemaFile = Schema::File::get(m_iSchemaID,
										  m_iDatabaseID,
										  *cProgram_.getTransaction());
	}
	if (!m_cLogicalFile.isAttached()) {
		; _SYDNEY_ASSERT(m_pSchemaFile);
		Utility::Transaction::attachLogicalFile(*cProgram_.getTransaction(),
												*m_pSchemaFile,
												m_cLogicalFile);
	}
	if (m_cBitSet.isValid()) {
		m_cOpenOption.setObjectPointer(
				   LogicalFile::OpenOption::KeyNumber::SearchByBitSet,
				   syd_reinterpret_cast<const Common::Object*>(m_cBitSet.getData()));
	}
	if (m_cRankBitSet.isValid()) {
		m_cOpenOption.setObjectPointer(
				   LogicalFile::OpenOption::KeyNumber::RankByBitSet,
				   syd_reinterpret_cast<const Common::Object*>(m_cRankBitSet.getData()));
	}
}

// FUNCTION public
//	Action::FileAccess::open -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileAccess::
open(Interface::IProgram& cProgram_)
{
	if (!m_cLogicalFile.isOpened()) {

		attach(cProgram_);
		_EXECUTION_FAKE_ERROR(Filie::open, Unexpected);

		startUpBeforeOpen(cProgram_);

		m_cLogicalFile.open(*cProgram_.getTransaction(),
							m_cOpenOption);

		if (m_cBitSet.isValid() && m_cBitSet->none())
			return false;

		startUp(cProgram_);
	}
	return true;
}

// FUNCTION public
//	Action::FileAccess::getData -- get data
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileAccess::
getData(Interface::IProgram& cProgram_,
		Common::DataArrayData* pData_)
{
	_EXECUTION_FAKE_ERROR(Filie::get, Unexpected);
	return m_cLocker.isInitialized()
		? m_cLocker->getData(cProgram_, pData_) : m_cLogicalFile.getData(pData_);
}

// FUNCTION public
//	Action::FileAccess::getLocator -- get locator
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pKey_
//	Action::Locator* pLocator_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileAccess::
getLocator(Interface::IProgram& cProgram_,
		   Common::DataArrayData* pKey_,
		   Action::Locator* pLocator_)
{
	// get locator don't need locking tuple
	return pLocator_->getLocator(m_cLogicalFile,
								 pKey_);
}

// FUNCTION public
//	Action::FileAccess::getProperty -- get property
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pKey_
//	Common::DataArrayData* pValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
getProperty(Interface::IProgram& cProgram_,
			Common::DataArrayData* pKey_,
			Common::DataArrayData* pValue_)
{
	m_cLogicalFile.getProperty(pKey_, pValue_);
}

// FUNCTION public
//	Action::FileAccess::insert -- insert
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
insert(Interface::IProgram& cProgram_,
	   Common::DataArrayData* pData_)
{
	_EXECUTION_FAKE_ERROR(Filie::insert, Unexpected);
	m_cLogicalFile.insert(pData_);
}

// FUNCTION public
//	Action::FileAccess::update -- update
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::DataArrayData* pKey_
//	Common::DataArrayData* pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
update(Interface::IProgram& cProgram_,
	   const Common::DataArrayData* pKey_,
	   Common::DataArrayData* pData_)
{
	_EXECUTION_FAKE_ERROR(Filie::update, Unexpected);
	m_cLogicalFile.update(pKey_, pData_);
}

// FUNCTION public
//	Action::FileAccess::expunge -- expunge
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::DataArrayData* pKey_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
expunge(Interface::IProgram& cProgram_,
		const Common::DataArrayData* pKey_)
{
	_EXECUTION_FAKE_ERROR(Filie::expunge, Unexpected);
	m_cLogicalFile.expunge(pKey_);
}

// FUNCTION public
//	Action::FileAccess::undoUpdate -- undo update
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::DataArrayData* pKey_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
undoUpdate(Interface::IProgram& cProgram_,
		   const Common::DataArrayData* pKey_)
{
	_EXECUTION_FAKE_ERROR(Filie::undoUpdate, Unexpected);
	m_cLogicalFile.undoUpdate(pKey_);
}

// FUNCTION public
//	Action::FileAccess::undoExpunge -- undo expunge
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::DataArrayData* pKey_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
undoExpunge(Interface::IProgram& cProgram_,
			const Common::DataArrayData* pKey_)
{
	_EXECUTION_FAKE_ERROR(Filie::undoExpunge, Unexpected);
	m_cLogicalFile.undoExpunge(pKey_);
}

// FUNCTION public
//	Action::FileAccess::reset -- reset iteration
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
reset()
{
	_EXECUTION_FAKE_ERROR(Filie::reset, Unexpected);
	if (m_cLocker.isInitialized()) {
		m_cLocker->reset();
	} else {
		m_cLogicalFile.reset();
	}
}

// FUNCTION public
//	Action::FileAccess::close -- close file
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
close()
{
	_EXECUTION_FAKE_ERROR(Filie::close, Unexpected);
	if (m_cLocker.isInitialized()) {
		m_cLocker->close();
	} else {
		m_cLogicalFile.close();
	}
}

// FUNCTION public
//	Action::FileAccess::fetch -- fetch
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData* pOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
fetch(const Common::DataArrayData* pOption_)
{
	_EXECUTION_FAKE_ERROR(Filie::fetch, Unexpected);
	if (m_cLocker.isInitialized()) {
		m_cLocker->fetch(pOption_);
	} else {
		m_cLogicalFile.fetch(pOption_);
	}
}

// FUNCTION public
//	Action::FileAccess::isOpened -- is opened
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileAccess::
isOpened()
{
	return m_cLogicalFile.isOpened();
}

// FUNCTION public
//	Action::FileAccess::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	FileAccess*
//
// EXCEPTIONS

//static
FileAccess*
FileAccess::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::FileAccess);
	return new FileAccess;
}

// FUNCTION public
//	Action::FileAccess::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
FileAccess::
getClassID() const
{
	return Class::getClassID(Class::Category::FileAccess);
}

// FUNCTION public
//	Action::FileAccess::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	archiver_(m_cTableName);
	archiver_(m_cFileName);
	archiver_(m_cOpenOption);
	m_cStartUp.serialize(archiver_);
	m_cStartUpBeforeOpen.serialize(archiver_);
	m_cBitSet.serialize(archiver_);
	m_cRankBitSet.serialize(archiver_);
	m_cLocker.serialize(archiver_);
}

// FUNCTION private
//	Action::FileAccess::startUpBeforeOpen -- do once before open
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
startUpBeforeOpen(Interface::IProgram& cProgram_)
{
	(void)m_cStartUpBeforeOpen.execute(cProgram_);
}

// FUNCTION private
//	Action::FileAccess::startUp -- do once after open
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
startUp(Interface::IProgram& cProgram_)
{
	(void)m_cStartUp.execute(cProgram_);
}

// FUNCTION private
//	Action::FileAccess::registerToProgram -- register to program
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccess::
registerToProgram(Interface::IProgram& cProgram_)
{
	// Instance ID is obtained by registerFileAccess method.
	setID(cProgram_.registerFileAccess(this));
}

//////////////////////////////////////////
// Execution::Action::FileAccessHolder

// FUNCTION public
//	Action::FileAccessHolder::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	const Opt::ExplainFileArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccessHolder::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_,
		const Opt::ExplainFileArgument& cArgument_)
{
	if (m_pFileAccess) {
		m_pFileAccess->explain(pEnvironment_, cProgram_, cExplain_, cArgument_);
	} else {
		cProgram_.getFileAccess(m_iID)->explain(pEnvironment_, cProgram_, cExplain_, cArgument_);
	}
}

// FUNCTION public
//	Action::FileAccessHolder::explainStartUp -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccessHolder::
explainStartUp(Opt::Environment* pEnvironment_,
			   Interface::IProgram& cProgram_,
			   Opt::Explain& cExplain_)
{
	if (m_pFileAccess) {
		m_pFileAccess->explainStartUp(pEnvironment_, cProgram_, cExplain_);
	} else {
		cProgram_.getFileAccess(m_iID)->explainStartUp(pEnvironment_, cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Action::FileAccessHolder::initialize -- initialize FileAccess instance
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccessHolder::
initialize(Interface::IProgram& cProgram_)
{
	if (m_pFileAccess == 0) {
		m_pFileAccess = cProgram_.getFileAccess(m_iID);
		if (m_pFileAccess == 0) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		m_pFileAccess->initialize(cProgram_);
	}
}

// FUNCTION public
//	Action::FileAccessHolder::terminate -- terminate FileAccess instance
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccessHolder::
terminate(Interface::IProgram& cProgram_)
{
	if (m_pFileAccess) {
		m_pFileAccess->terminate(cProgram_);
		m_pFileAccess = 0;
	}
}

// FUNCTION public
//	Action::FileAccessHolder::serialize -- serializer
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileAccessHolder::
serialize(ModArchive& archiver_)
{
	archiver_(m_iID);
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
