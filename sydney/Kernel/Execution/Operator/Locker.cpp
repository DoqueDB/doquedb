// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Locker.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Operator";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Operator/Locker.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/Argument.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Action/Locker.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Transaction.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/TableNotFound.h"
#include "Exception/Unexpected.h"

#include "Lock/Name.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Schema/Table.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	struct _Type
	{
		enum Value {
			Table,
			ConvertTable,
			Tuple,
			BitSet,
			ConvertTuple,
			UnlockTuple,
			ValueNum
		};
	};
	const char* const _pszExplainName[] =
	{
		"lock table",
		"convert lock table",
		"lock tuple",
		"lock tuple(bitset)",
		"convert lock tuple",
		"unlock tuple",
	};
}

namespace LockerImpl
{
	// CLASS local
	//	Execution::Operator::LockerImpl::Base -- base class of implementation classes of Locker
	//
	// NOTES
	class Base
		: public Operator::Locker
	{
	public:
		typedef Base This;
		typedef Operator::Locker Super;

		virtual ~Base() {}

	///////////////////////////
	// Operator::Locker::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
		Base()
			: Super(),
			  m_cArgument()
		{}
		explicit Base(const Action::LockerArgument& cArgument_)
			: Super(),
			  m_cArgument(cArgument_)
		{}

		// base implementation
		void initializeBase(Interface::IProgram& cProgram_);
		void terminateBase(Interface::IProgram& cProgram_);
		void serializeBase(ModArchive& archiver_);

		// accessor
		Schema::Object::ID::Value getDatabaseID() {return m_cArgument.m_iDatabaseID;}
		Schema::Object::ID::Value getTableID() {return m_cArgument.m_iTableID;}
		const Action::LockerArgument& getArgument() {return m_cArgument;}
		const Schema::Object::Name& getTableName() {return m_cArgument.m_cTableName;}

		// get lock mode when prepared
		void setLockMode(Interface::IProgram& cProgram_,
						 bool bIsReadOnly_,
						 Action::LockerArgument& cArgument_);

	private:
		virtual Lock::Name::Category::Value getLockCategory() = 0;
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;

		Action::LockerArgument m_cArgument;
	};

	// CLASS local
	//	Execution::Operator::LockerImpl::Table -- implementation class of LockTable
	//
	// NOTES
	class Table
		: public Base
	{
	public:
		typedef Table This;
		typedef Base Super;

		Table()
			: Super()
		{}
		explicit Table(const Action::LockerArgument& cArgument_)
			: Super(cArgument_)
		{}
		~Table() {}

	///////////////////////////
	// Operator::Locker::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual Lock::Name::Category::Value getLockCategory();
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};

	// CLASS local
	//	Execution::Operator::LockerImpl::ConvertTable -- implementation class of Locker
	//
	// NOTES
	class ConvertTable
		: public Base
	{
	public:
		typedef ConvertTable This;
		typedef Base Super;

		ConvertTable()
			: Super(),
			  m_cPrevArgument(),
			  m_bConverted(false) {}
		ConvertTable(const Action::LockerArgument& cArgument_,
					 const Action::LockerArgument& cPrevArgument_)
			: Super(cArgument_),
			  m_cPrevArgument(cPrevArgument_),
			  m_bConverted(false)
		{}
		~ConvertTable() {}

	///////////////////////////
	// Operator::Locker::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);
		virtual void undone(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		void revertLock(Interface::IProgram& cProgram_);

	///////////////////////////
	// Base::
		virtual Lock::Name::Category::Value getLockCategory();
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);

		Action::LockerArgument m_cPrevArgument;
		bool m_bConverted;
	};

	// CLASS local
	//	Execution::Operator::LockerImpl::Tuple -- implementation class of Locker
	//
	// NOTES
	class Tuple
		: public Base
	{
	public:
		typedef Tuple This;
		typedef Base Super;

		Tuple() : Super() {}
		Tuple(const Action::LockerArgument& cArgument_,
			  int iRowIDID_)
			: Super(cArgument_),
			  m_cRowID(iRowIDID_)
		{}
		~Tuple() {}

	///////////////////////////
	// Operator::Locker::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual Lock::Name::Category::Value getLockCategory();
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);

		Action::RowIDHolder m_cRowID;
	};

	// CLASS local
	//	Execution::Operator::LockerImpl::BitSet -- implementation class of Locker
	//
	// NOTES
	class BitSet
		: public Base
	{
	public:
		typedef BitSet This;
		typedef Base Super;

		BitSet() : Super() {}
		BitSet(const Action::LockerArgument& cArgument_,
			   int iBitSetID_)
			: Super(cArgument_),
			  m_cBitSet(iBitSetID_)
		{}
		~BitSet() {}

	///////////////////////////
	// Operator::Locker::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual Lock::Name::Category::Value getLockCategory();
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);

		Action::BitSetHolder m_cBitSet;
	};

	// CLASS local
	//	Execution::Operator::LockerImpl::ConvertTuple -- implementation class of Locker
	//
	// NOTES
	class ConvertTuple
		: public Base
	{
	public:
		typedef ConvertTuple This;
		typedef Base Super;

		ConvertTuple()
			: Super(),
			  m_cPrevArgument(),
			  m_cRowID()
		{}
		ConvertTuple(const Action::LockerArgument& cArgument_,
					 const Action::LockerArgument& cPrevArgument_,
					 int iRowIDID_)
			: Super(cArgument_),
			  m_cPrevArgument(cPrevArgument_),
			  m_cRowID(iRowIDID_)
		{}
		~ConvertTuple() {}

	///////////////////////////
	// Operator::Locker::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual Lock::Name::Category::Value getLockCategory();
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);

		Action::LockerArgument m_cPrevArgument;
		Action::RowIDHolder m_cRowID;
	};

	// CLASS local
	//	Execution::Operator::LockerImpl::UnlockTuple -- implementation classes of Locker
	//
	// NOTES
	class UnlockTuple
		: public Operator::Locker
	{
	public:
		typedef UnlockTuple This;
		typedef Operator::Locker Super;

		UnlockTuple()
			: Super(),
			  m_cLocker()
		{}
		UnlockTuple(int iLockerID_)
			: Super(),
			  m_cLocker(iLockerID_)
		{}

		~UnlockTuple() {}

	///////////////////////////
	// Operator::Locker::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
		Action::LockerHolder m_cLocker;
	};
}

///////////////////////////////////////////////
// Execution::Operator::LockerImpl::Base

// FUNCTION public
//	Operator::LockerImpl::Base::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	explainOperator(cProgram_, cExplain_);
}

// FUNCTION public
//	Operator::LockerImpl::Base::initialize -- 
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
LockerImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
}

// FUNCTION public
//	Operator::LockerImpl::Base::terminate -- 
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
LockerImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
}

// FUNCTION public
//	Operator::LockerImpl::Base::finish -- 
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
LockerImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::LockerImpl::Base::reset -- 
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
LockerImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::LockerImpl::Base::serialize -- 
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
LockerImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
}

// FUNCTION protected
//	Operator::LockerImpl::Base::initializeBase -- 
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
LockerImpl::Base::
initializeBase(Interface::IProgram& cProgram_)
{
	setLockMode(cProgram_,
				!m_cArgument.m_bIsUpdate,
				m_cArgument);

	if (m_cArgument.m_eMode != Lock::Mode::N) {
		if (getTableID() == Schema::ObjectID::Invalid) {
			// get tableID from name
			Schema::Table* pSchemaTable = 
				cProgram_.getDatabase()->getTable(getTableName(),
												  *cProgram_.getTransaction());
			if (pSchemaTable == 0) {
				_SYDNEY_THROW2(Exception::TableNotFound,
							   getTableName(),
							   cProgram_.getDatabase()->getName());
			}
			m_cArgument.m_iDatabaseID = pSchemaTable->getDatabaseID();
			m_cArgument.m_iTableID = pSchemaTable->getID();
		}
	}
}

// FUNCTION protected
//	Operator::LockerImpl::Base::terminateBase -- 
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
LockerImpl::Base::
terminateBase(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION protected
//	Operator::LockerImpl::Base::serializeBase -- 
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
LockerImpl::Base::
serializeBase(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cArgument.serialize(archiver_);
}

// FUNCTION protected
//	Operator::LockerImpl::Base::setLockMode -- get lock mode when prepared
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	bool bIsReadOnly_
//	Action::LockerArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::Base::
setLockMode(Interface::IProgram& cProgram_,
			bool bIsReadOnly_,
			Action::LockerArgument& cArgument_)
{
	if (cArgument_.m_bIsPrepare) {
		cArgument_.m_bIsPrepare = false;
		Utility::Transaction::getAdequateLock(*cProgram_.getTransaction(),
											  getLockCategory(),
											  bIsReadOnly_,
											  cProgram_.isBatchMode(),
											  cArgument_);
	}
}

///////////////////////////////////////////////
// Execution::Operator::LockerImpl::Table

// FUNCTION public
//	Operator::LockerImpl::Table::terminate -- 
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
LockerImpl::Table::
terminate(Interface::IProgram& cProgram_)
{
	if (getArgument().m_eMode != Lock::Mode::N
		&& getArgument().m_bIsSimple) {
		// convert lock here
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
			_OPT_EXECUTION_MESSAGE
				<< "convertLock: " << getTableName()
				<< " " << Lock::Mode::S
				<< ":" << Lock::Duration::Inside
				<< "->" << getArgument().m_eMode
				<< ModEndl;
		}
#endif

		cProgram_.getTransaction()->convertLock(Lock::TableName(getDatabaseID(),
																getTableID()),
												Lock::Mode::S,
												Lock::Duration::Inside,
												getArgument().m_eMode,
												getArgument().m_eDuration,
												Lock::Timeout::Unlimited);
	}
	Super::terminate(cProgram_);
}

// FUNCTION public
//	Operator::LockerImpl::Table::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
LockerImpl::Table::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (getArgument().m_eMode != Lock::Mode::N) {
			Lock::Mode::Value eMode = getArgument().m_eMode;
			Lock::Duration::Value eDuration = getArgument().m_eDuration;

			if (getArgument().m_bIsSimple) {
				eMode = Lock::Mode::S;
				eDuration = Lock::Duration::Inside;
			}

#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
				_OPT_EXECUTION_MESSAGE
					<< "lockTable: " << getTableName()
					<< " " << eMode
					<< ":" << eDuration
					<< ModEndl;
			}
#endif

			cProgram_.getTransaction()->lock(Lock::TableName(getDatabaseID(),
															 getTableID()),
											 eMode,
											 eDuration,
											 Lock::Timeout::Unlimited);
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::LockerImpl::Table::getClassID -- 
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
LockerImpl::Table::
getClassID() const
{
	return Class::getClassID(Class::Category::LockTable);
}

// FUNCTION private
//	Operator::LockerImpl::Table::getLockCategory -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Lock::Name::Category::Value
//
// EXCEPTIONS

//virtual
Lock::Name::Category::Value
LockerImpl::Table::
getLockCategory()
{
	return Lock::Name::Category::Table;
}

// FUNCTION private
//	Operator::LockerImpl::Table::explainOperator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Table::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Table]);
}

///////////////////////////////////////////////
// Execution::Operator::LockerImpl::ConvertTable

// FUNCTION public
//	Operator::LockerImpl::ConvertTable::initialize -- 
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
LockerImpl::ConvertTable::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	setLockMode(cProgram_,
				true /* read only */,
				m_cPrevArgument);
	m_bConverted = false;
}

// FUNCTION public
//	Operator::LockerImpl::ConvertTable::terminate -- 
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
LockerImpl::ConvertTable::
terminate(Interface::IProgram& cProgram_)
{
	revertLock(cProgram_);
	terminateBase(cProgram_);
}

// FUNCTION public
//	Operator::LockerImpl::ConvertTable::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
LockerImpl::ConvertTable::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (getArgument().m_eMode != m_cPrevArgument.m_eMode) {
#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
				_OPT_EXECUTION_MESSAGE
					<< "convertLock: " << getTableName()
					<< " " << m_cPrevArgument.m_eMode
					<< ":" << m_cPrevArgument.m_eDuration
					<< "->" << getArgument().m_eMode
					<< ModEndl;
			}
#endif

			cProgram_.getTransaction()->convertLock(Lock::TableName(getDatabaseID(),
																	getTableID()),
													m_cPrevArgument.m_eMode,
													m_cPrevArgument.m_eDuration,
													getArgument().m_eMode,
													Lock::Timeout::Unlimited);
			m_bConverted = true;
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::LockerImpl::ConvertTable::undone -- 
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
LockerImpl::ConvertTable::
undone(Interface::IProgram& cProgram_)
{
	if (isDone()) {
		revertLock(cProgram_);
		Super::undone(cProgram_);
	}
}

// FUNCTION public
//	Operator::LockerImpl::ConvertTable::getClassID -- 
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
LockerImpl::ConvertTable::
getClassID() const
{
	return Class::getClassID(Class::Category::LockConvertTable);
}

// FUNCTION public
//	Operator::LockerImpl::ConvertTable::serialize -- 
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
LockerImpl::ConvertTable::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cPrevArgument.serialize(archiver_);
}

// FUNCTION private
//	Operator::LockerImpl::ConvertTable::revertLock -- 
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
LockerImpl::ConvertTable::
revertLock(Interface::IProgram& cProgram_)
{
	if (m_bConverted) {
		// convert lock to original mode
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
			_OPT_EXECUTION_MESSAGE
				<< "convertLock: " << getTableName()
				<< " " << getArgument().m_eMode
				<< ":" << m_cPrevArgument.m_eDuration
				<< "->" << m_cPrevArgument.m_eMode
				<< ModEndl;
		}
#endif
		cProgram_.getTransaction()->convertLock(Lock::TableName(getDatabaseID(),
																getTableID()),
												getArgument().m_eMode,
												m_cPrevArgument.m_eDuration,
												m_cPrevArgument.m_eMode,
												Lock::Timeout::Unlimited);

		m_bConverted = false;
	}
}

// FUNCTION private
//	Operator::LockerImpl::ConvertTable::getLockCategory -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Lock::Name::Category::Value
//
// EXCEPTIONS

//virtual
Lock::Name::Category::Value
LockerImpl::ConvertTable::
getLockCategory()
{
	return Lock::Name::Category::Table;
}

// FUNCTION private
//	Operator::LockerImpl::ConvertTable::explainOperator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::ConvertTable::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::ConvertTable]);
}

///////////////////////////////////////////////
// Execution::Operator::LockerImpl::Tuple

// FUNCTION public
//	Operator::LockerImpl::Tuple::initialize -- 
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
LockerImpl::Tuple::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cRowID.initialize(cProgram_);
}

// FUNCTION public
//	Operator::LockerImpl::Tuple::terminate -- 
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
LockerImpl::Tuple::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cRowID.terminate(cProgram_);
}

// FUNCTION public
//	Operator::LockerImpl::Tuple::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
LockerImpl::Tuple::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (getArgument().m_eMode != Lock::Mode::N) {
#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
				_OPT_EXECUTION_MESSAGE
					<< "lockTuple: " << getTableName()
					<< " " << m_cRowID->getValue()
					<< " " << getArgument().m_eMode
					<< ":" << getArgument().m_eDuration
					<< ModEndl;
			}
#endif

			cProgram_.getTransaction()->lock(Lock::TupleName(getDatabaseID(),
															 getTableID(),
															 m_cRowID->getValue()),
											 getArgument().m_eMode,
											 getArgument().m_eDuration,
											 Lock::Timeout::Unlimited);
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::LockerImpl::Tuple::getClassID -- 
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
LockerImpl::Tuple::
getClassID() const
{
	return Class::getClassID(Class::Category::LockTuple);
}

// FUNCTION public
//	Operator::LockerImpl::Tuple::serialize -- 
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
LockerImpl::Tuple::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cRowID.serialize(archiver_);
}

// FUNCTION private
//	Operator::LockerImpl::Tuple::getLockCategory -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Lock::Name::Category::Value
//
// EXCEPTIONS

//virtual
Lock::Name::Category::Value
LockerImpl::Tuple::
getLockCategory()
{
	return Lock::Name::Category::Tuple;
}

// FUNCTION private
//	Operator::LockerImpl::Tuple::explainOperator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Tuple::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Tuple]);
}

///////////////////////////////////////////////
// Execution::Operator::LockerImpl::BitSet

// FUNCTION public
//	Operator::LockerImpl::BitSet::initialize -- 
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
LockerImpl::BitSet::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cBitSet.initialize(cProgram_);
}

// FUNCTION public
//	Operator::LockerImpl::BitSet::terminate -- 
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
LockerImpl::BitSet::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cBitSet.terminate(cProgram_);
}

// FUNCTION public
//	Operator::LockerImpl::BitSet::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
LockerImpl::BitSet::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (getArgument().m_eMode != Lock::Mode::N) {
#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
				_OPT_EXECUTION_MESSAGE
					<< "lockTuple(bitset): " << getTableName()
					<< " " << m_cBitSet->toString()
					<< " " << getArgument().m_eMode
					<< ":" << getArgument().m_eDuration
					<< ModEndl;
			}
#endif
			const Common::BitSet* pBitSet = m_cBitSet.getData();
			Common::BitSet::ConstIterator iterator = pBitSet->begin();
			const Common::BitSet::ConstIterator last = pBitSet->end();
			for (; iterator != last; ++iterator) {
				// Wait lock for ever
				cProgram_.getTransaction()->lock(Lock::TupleName(getDatabaseID(),
																 getTableID(),
																 *iterator),
												 getArgument().m_eMode,
												 getArgument().m_eDuration,
												 Lock::Timeout::Unlimited);
			}
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::LockerImpl::BitSet::getClassID -- 
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
LockerImpl::BitSet::
getClassID() const
{
	return Class::getClassID(Class::Category::LockBitSet);
}

// FUNCTION public
//	Operator::LockerImpl::BitSet::serialize -- 
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
LockerImpl::BitSet::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cBitSet.serialize(archiver_);
}

// FUNCTION private
//	Operator::LockerImpl::BitSet::getLockCategory -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Lock::Name::Category::Value
//
// EXCEPTIONS

//virtual
Lock::Name::Category::Value
LockerImpl::BitSet::
getLockCategory()
{
	return Lock::Name::Category::Tuple;
}

// FUNCTION private
//	Operator::LockerImpl::BitSet::explainOperator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::BitSet::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::BitSet]);
}

//////////////////////////////////////////////////
// Execution::Operator::LockerImpl::ConvertTuple

// FUNCTION public
//	Operator::LockerImpl::ConvertTuple::initialize -- 
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
LockerImpl::ConvertTuple::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cRowID.initialize(cProgram_);
	setLockMode(cProgram_,
				true /* read only */,
				m_cPrevArgument);
}

// FUNCTION public
//	Operator::LockerImpl::ConvertTuple::terminate -- 
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
LockerImpl::ConvertTuple::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cRowID.terminate(cProgram_);
}

// FUNCTION public
//	Operator::LockerImpl::ConvertTuple::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
LockerImpl::ConvertTuple::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (getArgument().m_eMode != m_cPrevArgument.m_eMode) {
#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
				_OPT_EXECUTION_MESSAGE
					<< "convertLockTuple: " << getTableName()
					<< " " << m_cRowID->getValue()
					<< " " << m_cPrevArgument.m_eMode
					<< ":" << getArgument().m_eDuration
					<< "->" << getArgument().m_eMode
					<< ModEndl;
			}
#endif

			cProgram_.getTransaction()->convertLock(Lock::TupleName(getDatabaseID(),
															 getTableID(),
															 m_cRowID->getValue()),
													m_cPrevArgument.m_eMode,
													getArgument().m_eDuration,
													getArgument().m_eMode,
													Lock::Timeout::Unlimited);
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::LockerImpl::ConvertTuple::getClassID -- 
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
LockerImpl::ConvertTuple::
getClassID() const
{
	return Class::getClassID(Class::Category::LockConvertTuple);
}

// FUNCTION public
//	Operator::LockerImpl::ConvertTuple::serialize -- 
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
LockerImpl::ConvertTuple::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cRowID.serialize(archiver_);
	m_cPrevArgument.serialize(archiver_);
}

// FUNCTION private
//	Operator::LockerImpl::ConvertTuple::getLockCategory -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Lock::Name::Category::Value
//
// EXCEPTIONS

//virtual
Lock::Name::Category::Value
LockerImpl::ConvertTuple::
getLockCategory()
{
	return Lock::Name::Category::Tuple;
}

// FUNCTION private
//	Operator::LockerImpl::ConvertTuple::explainOperator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::ConvertTuple::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::ConvertTuple]);
}

///////////////////////////////////////////////////
// Execution::Operator::LockerImpl::UnlockTuple

// FUNCTION public
//	Operator::LockerImpl::UnlockTuple::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::UnlockTuple::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::UnlockTuple]);
}

// FUNCTION public
//	Operator::LockerImpl::UnlockTuple::initialize -- 
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
LockerImpl::UnlockTuple::
initialize(Interface::IProgram& cProgram_)
{
	m_cLocker.initialize(cProgram_);
}

// FUNCTION public
//	Operator::LockerImpl::UnlockTuple::terminate -- 
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
LockerImpl::UnlockTuple::
terminate(Interface::IProgram& cProgram_)
{
	m_cLocker.terminate(cProgram_);
}

// FUNCTION public
//	Operator::LockerImpl::UnlockTuple::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
LockerImpl::UnlockTuple::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		m_cLocker->unlock();
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::LockerImpl::UnlockTuple::finish -- 
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
LockerImpl::UnlockTuple::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::LockerImpl::UnlockTuple::reset -- 
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
LockerImpl::UnlockTuple::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::LockerImpl::UnlockTuple::serialize -- 
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
LockerImpl::UnlockTuple::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cLocker.serialize(archiver_);
}

// FUNCTION public
//	Operator::LockerImpl::UnlockTuple::getClassID -- 
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
LockerImpl::UnlockTuple::
getClassID() const
{
	return Class::getClassID(Class::Category::UnlockTuple);
}

/////////////////////////////////
// Operator::Locker::Table

// FUNCTION public
//	Operator::Locker::Table::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const Action::LockerArgument& cArgument_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::Table::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const Action::LockerArgument& cArgument_)
{
	AUTOPOINTER<This> pResult = new LockerImpl::Table(cArgument_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::Locker::ConvertTable

// FUNCTION public
//	Operator::Locker::ConvertTable::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const Action::LockerArgument& cArgument_
//	const Action::LockerArgument& cPrevArgument_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::ConvertTable::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const Action::LockerArgument& cArgument_,
	   const Action::LockerArgument& cPrevArgument_)
{
	AUTOPOINTER<This> pResult = new LockerImpl::ConvertTable(cArgument_,
															 cPrevArgument_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::Locker::Tuple

// FUNCTION public
//	Operator::Locker::Tuple::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const Action::LockerArgument& cArgument_
//	int iRowIDID_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::Tuple::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const Action::LockerArgument& cArgument_,
	   int iRowIDID_)
{
	AUTOPOINTER<This> pResult = new LockerImpl::Tuple(cArgument_, iRowIDID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::Locker::Bitset

// FUNCTION public
//	Operator::Locker::Bitset::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const Action::LockerArgument& cArgument_
//	int iRowIDID_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::BitSet::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const Action::LockerArgument& cArgument_,
	   int iBitSetID_)
{
	AUTOPOINTER<This> pResult = new LockerImpl::BitSet(cArgument_, iBitSetID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::Locker::ConvertTuple

// FUNCTION public
//	Operator::Locker::ConvertTuple::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const Action::LockerArgument& cArgument_
//	const Action::LockerArgument& cPrevArgument_
//	int iRowIDID_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::ConvertTuple::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const Action::LockerArgument& cArgument_,
	   const Action::LockerArgument& cPrevArgument_,
	   int iRowIDID_)
{
	AUTOPOINTER<This> pResult = new LockerImpl::ConvertTuple(cArgument_,
															 cPrevArgument_,
															 iRowIDID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Operator::Locker::UnlockTuple

// FUNCTION public
//	Operator::Locker::UnlockTuple::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iLockerID_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::UnlockTuple::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iLockerID_)
{
	AUTOPOINTER<This> pResult = new LockerImpl::UnlockTuple(iLockerID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::Locker::

// FUNCTION public
//	Operator::Locker::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::LockTable:
		{
			return new LockerImpl::Table;
		}
	case Class::Category::LockTuple:
		{
			return new LockerImpl::Tuple;
		}
	case Class::Category::LockBitSet:
		{
			return new LockerImpl::BitSet;
		}
	case Class::Category::LockConvertTuple:
		{
			return new LockerImpl::ConvertTuple;
		}
	case Class::Category::UnlockTuple:
		{
			return new LockerImpl::UnlockTuple;
		}
	case Class::Category::LockConvertTable:
		{
			return new LockerImpl::ConvertTable;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
