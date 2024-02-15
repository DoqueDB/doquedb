// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Generator.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Execution/Operator/Generator.h"
#include "Execution/Operator/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Opt/Trace.h"

#include "Exception/ColumnNotFound.h"
#include "Exception/TableNotFound.h"
#include "Exception/Unexpected.h"

#include "Common/Assert.h"
#include "Common/IntegerData.h"
#include "Common/UnsignedIntegerData.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Schema/Column.h"
#include "Schema/Database.h"
#include "Schema/Table.h"
#include "Schema/TupleID.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	struct _Type
	{
		enum Value {
			RowID,
			Identity,
			RecoveryRowID,
			RecoveryIdentity,
			ValueNum
		};
	};
			
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName[] =
	{
		"generate rowid",
		"generator",
		"recover rowid",
		"recover generator"
	};
}

namespace GeneratorImpl
{
	// CLASS local
	//	Execution::Operator::GeneratorImpl::RowID -- implementation class of Generator
	//
	// NOTES
	class RowID
		: public Operator::Generator
	{
	public:
		typedef RowID This;
		typedef Operator::Generator Super;

		RowID()
			: Super(),
			  m_pSchemaTable(0),
			  m_cTableName(),
			  m_cData()
		{}
		RowID(Schema::Table* pSchemaTable_,
			  int iDataID_)
			: Super(),
			  m_pSchemaTable(pSchemaTable_),
			  m_cTableName(pSchemaTable_->getName()),
			  m_cData(iDataID_)
		{}
		~RowID()
		{}

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
		void serialize(ModArchive& archiver_);

	protected:
	private:
		Schema::Table* m_pSchemaTable;
		Schema::Object::Name m_cTableName;
		Action::RowIDHolder m_cData;
	};

	// CLASS local
	//	Execution::Operator::GeneratorImpl::Identity -- implementation class of Generator
	//
	// NOTES
	class Identity
		: public Operator::Generator
	{
	public:
		typedef Identity This;
		typedef Operator::Generator Super;

		Identity()
			: Super(),
			  m_pSchemaColumn(0),
			  m_cTableName(),
			  m_cColumnName(),
			  m_cData()
		{}
		Identity(Schema::Table* pSchemaTable_,
				 Schema::Column* pSchemaColumn_,
				 int iDataID_)
			: Super(),
			  m_pSchemaColumn(pSchemaColumn_),
			  m_cTableName(pSchemaTable_->getName()),
			  m_cColumnName(pSchemaColumn_->getName()),
			  m_cData(iDataID_)
		{}
		virtual ~Identity()
		{}

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
		void serialize(ModArchive& archiver_);

	protected:
		Schema::Column* getSchemaColumn() {return m_pSchemaColumn;}
		Action::IdentityHolder& getData() {return m_cData;}

	private:
		Schema::Column* m_pSchemaColumn;
		Schema::Object::Name m_cTableName;
		Schema::Object::Name m_cColumnName;
		Action::IdentityHolder m_cData;
	};

	// CLASS local
	//	Execution::Operator::GeneratorImpl::IdentityByInput -- implementation class of Generator
	//
	// NOTES
	class IdentityByInput
		: public Identity
	{
	public:
		typedef IdentityByInput This;
		typedef Identity Super;

		IdentityByInput()
			: Super(),
			  m_cInput(),
			  m_bGetMax(false)
		{}
		IdentityByInput(Schema::Table* pSchemaTable_,
						Schema::Column* pSchemaColumn_,
						int iInputID_,
						int iDataID_)
			: Super(pSchemaTable_,
					pSchemaColumn_,
					iDataID_),
			  m_cInput(iInputID_),
			  m_bGetMax(false)
		{}
		~IdentityByInput()
		{}

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
		Action::DataHolder m_cInput;
		bool m_bGetMax;
	};

	// CLASS local
	//	Execution::Operator::GeneratorImpl::RecoveryRowID -- implementation class of Generator
	//
	// NOTES
	class RecoveryRowID
		: public Operator::Generator
	{
	public:
		typedef RecoveryRowID This;
		typedef Operator::Generator Super;

		RecoveryRowID()
			: Super(),
			  m_pSchemaTable(0),
			  m_cData()
		{}
		RecoveryRowID(Schema::Table* pSchemaTable_,
					  int iDataID_)
			: Super(),
			  m_pSchemaTable(pSchemaTable_),
			  m_cData(iDataID_)
		{}
		~RecoveryRowID()
		{}

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
		void serialize(ModArchive& archiver_);

	protected:
	private:
		Schema::Table* m_pSchemaTable;
		Action::RowIDHolder m_cData;
	};

	// CLASS local
	//	Execution::Operator::GeneratorImpl::RecoveryIdentity -- implementation class of Generator
	//
	// NOTES
	class RecoveryIdentity
		: public Operator::Generator
	{
	public:
		typedef RecoveryIdentity This;
		typedef Operator::Generator Super;

		RecoveryIdentity()
			: Super(),
			  m_pSchemaColumn(0),
			  m_cData()
		{}
		RecoveryIdentity(Schema::Table* pSchemaTable_,
						 Schema::Column* pSchemaColumn_,
						 int iDataID_)
			: Super(),
			  m_pSchemaColumn(pSchemaColumn_),
			  m_cData(iDataID_)
		{}
		~RecoveryIdentity()
		{}

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
		void serialize(ModArchive& archiver_);

	protected:
		Schema::Column* getSchemaColumn() {return m_pSchemaColumn;}
		Action::IdentityHolder& getData() {return m_cData;}

	private:
		Schema::Column* m_pSchemaColumn;
		Action::IdentityHolder m_cData;
	};
}

/////////////////////////////////////////////
// Execution::Operator::GeneratorImpl::RowID

// FUNCTION public
//	Operator::GeneratorImpl::RowID::explain -- 
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
GeneratorImpl::RowID::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::RowID]);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" to ");
		m_cData.explain(cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Operator::GeneratorImpl::RowID::initialize -- 
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
GeneratorImpl::RowID::
initialize(Interface::IProgram& cProgram_)
{
	if (m_pSchemaTable == 0) {
		// get schema table from name
		m_pSchemaTable =
			cProgram_.getDatabase()->getTable(m_cTableName,
											  *cProgram_.getTransaction());
		if (m_pSchemaTable == 0) {
			_SYDNEY_THROW2(Exception::TableNotFound,
						   m_cTableName,
						   cProgram_.getDatabase()->getName());
		}
	}
	m_cData.initialize(cProgram_);
}

// FUNCTION public
//	Operator::GeneratorImpl::RowID::terminate -- 
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
GeneratorImpl::RowID::
terminate(Interface::IProgram& cProgram_)
{
	m_pSchemaTable = 0;
	m_cData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::GeneratorImpl::RowID::execute -- 
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
GeneratorImpl::RowID::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		m_cData->setValue(Schema::TupleID::assign(*m_pSchemaTable,
												  *cProgram_.getTransaction()));
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::GeneratorImpl::RowID::finish -- 
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
GeneratorImpl::RowID::
finish(Interface::IProgram& cProgram_)
{
	// persist generator
	Schema::TupleID::persist(*m_pSchemaTable, *cProgram_.getTransaction());
}

// FUNCTION public
//	Operator::GeneratorImpl::RowID::reset -- 
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
GeneratorImpl::RowID::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::GeneratorImpl::RowID::getClassID -- 
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
GeneratorImpl::RowID::
getClassID() const
{
	return Class::getClassID(Class::Category::GeneratorRowID);
}

// FUNCTION public
//	Operator::GeneratorImpl::RowID::serialize -- 
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
GeneratorImpl::RowID::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	archiver_(m_cTableName);
	m_cData.serialize(archiver_);
}

/////////////////////////////////////////////
// Execution::Operator::GeneratorImpl::Identity

// FUNCTION public
//	Operator::GeneratorImpl::Identity::explain -- 
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
GeneratorImpl::Identity::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::Identity]);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" to ");
		m_cData.explain(cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Operator::GeneratorImpl::Identity::initialize -- 
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
GeneratorImpl::Identity::
initialize(Interface::IProgram& cProgram_)
{
	if (m_pSchemaColumn == 0) {
		// get schema table from name
		Schema::Table* pSchemaTable =
			cProgram_.getDatabase()->getTable(m_cTableName,
											  *cProgram_.getTransaction());
		if (pSchemaTable == 0) {
			_SYDNEY_THROW2(Exception::TableNotFound,
						   m_cTableName,
						   cProgram_.getDatabase()->getName());
		}
		m_pSchemaColumn =
			pSchemaTable->getColumn(m_cColumnName,
									*cProgram_.getTransaction());
		if (m_pSchemaColumn == 0) {
			_SYDNEY_THROW1(Exception::ColumnNotFound,
						   m_cColumnName);
		}
	}
	m_cData.initialize(cProgram_);
}

// FUNCTION public
//	Operator::GeneratorImpl::Identity::terminate -- 
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
GeneratorImpl::Identity::
terminate(Interface::IProgram& cProgram_)
{
	m_pSchemaColumn = 0;
	m_cData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::GeneratorImpl::Identity::execute -- 
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
GeneratorImpl::Identity::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		m_cData->setValue(Schema::Identity::assign(*m_pSchemaColumn,
												   *cProgram_.getTransaction()));
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::GeneratorImpl::Identity::finish -- 
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
GeneratorImpl::Identity::
finish(Interface::IProgram& cProgram_)
{
	// persist generator
	Schema::Identity::persist(*m_pSchemaColumn, *cProgram_.getTransaction());
}

// FUNCTION public
//	Operator::GeneratorImpl::Identity::reset -- 
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
GeneratorImpl::Identity::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::GeneratorImpl::Identity::getClassID -- 
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
GeneratorImpl::Identity::
getClassID() const
{
	return Class::getClassID(Class::Category::GeneratorIdentity);
}

// FUNCTION public
//	Operator::GeneratorImpl::Identity::serialize -- 
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
GeneratorImpl::Identity::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	archiver_(m_cTableName);
	archiver_(m_cColumnName);
	m_cData.serialize(archiver_);
}

/////////////////////////////////////////////
// Execution::Operator::GeneratorImpl::IdentityByInput

// FUNCTION public
//	Operator::GeneratorImpl::IdentityByInput::explain -- 
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
GeneratorImpl::IdentityByInput::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	Super::explain(pEnvironment_,
				   cProgram_,
				   cExplain_);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		m_cInput.explain(cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Operator::GeneratorImpl::IdentityByInput::initialize -- 
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
GeneratorImpl::IdentityByInput::
initialize(Interface::IProgram& cProgram_)
{
	Super::initialize(cProgram_);
	m_cInput.initialize(cProgram_);
	m_bGetMax = Schema::Identity::isGetMax(*getSchemaColumn(),
										   *cProgram_.getTransaction());
}

// FUNCTION public
//	Operator::GeneratorImpl::IdentityByInput::terminate -- 
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
GeneratorImpl::IdentityByInput::
terminate(Interface::IProgram& cProgram_)
{
	m_cInput.terminate(cProgram_);
	Super::terminate(cProgram_);
}

// FUNCTION public
//	Operator::GeneratorImpl::IdentityByInput::execute -- 
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
GeneratorImpl::IdentityByInput::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (m_cInput->isNull() == false && m_cInput->isDefault() == false) {
			Schema::Identity::Value iValue = m_cInput->getInt();
			if (m_bGetMax) {
				// take max value comparing to current next value
				iValue = Schema::Identity::assign(*getSchemaColumn(),
												  iValue,
												  *cProgram_.getTransaction());
			}
			getData()->setValue(iValue);

		} else {
			getData()->setValue(Schema::Identity::assign(*getSchemaColumn(),
														 *cProgram_.getTransaction()));
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::GeneratorImpl::IdentityByInput::getClassID -- 
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
GeneratorImpl::IdentityByInput::
getClassID() const
{
	return Class::getClassID(Class::Category::GeneratorIdentityByInput);
}

// FUNCTION public
//	Operator::GeneratorImpl::IdentityByInput::serialize -- 
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
GeneratorImpl::IdentityByInput::
serialize(ModArchive& archiver_)
{
	Super::serialize(archiver_);
	m_cInput.serialize(archiver_);
}

/////////////////////////////////////////////////////////
// Execution::Operator::GeneratorImpl::RecoveryRowID

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryRowID::explain -- 
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
GeneratorImpl::RecoveryRowID::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::RecoveryRowID]);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		m_cData.explain(cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryRowID::initialize -- 
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
GeneratorImpl::RecoveryRowID::
initialize(Interface::IProgram& cProgram_)
{
	// never used in prepare statement
	; _SYDNEY_ASSERT(m_pSchemaTable);
	m_cData.initialize(cProgram_);
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryRowID::terminate -- 
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
GeneratorImpl::RecoveryRowID::
terminate(Interface::IProgram& cProgram_)
{
	m_cData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryRowID::execute -- 
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
GeneratorImpl::RecoveryRowID::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		Schema::TupleID::assign(*m_pSchemaTable,
								m_cData->getValue(),
								*cProgram_.getTransaction());
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Variable)) {
			_OPT_EXECUTION_MESSAGE
				<< "Recovery RowID of " << m_pSchemaTable->getName()
				<< " by "
				<< Opt::Trace::toString(*m_cData)
				<< ModEndl;
		}
#endif
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryRowID::finish -- 
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
GeneratorImpl::RecoveryRowID::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryRowID::reset -- 
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
GeneratorImpl::RecoveryRowID::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryRowID::getClassID -- 
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
GeneratorImpl::RecoveryRowID::
getClassID() const
{
	return Class::getClassID(Class::Category::GeneratorRecoveryRowID);
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryRowID::serialize -- 
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
GeneratorImpl::RecoveryRowID::
serialize(ModArchive& archiver_)
{
	// never called
	; _SYDNEY_ASSERT(false);
}

////////////////////////////////////////////////////////////
// Execution::Operator::GeneratorImpl::RecoveryIdentity

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryIdentity::explain -- 
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
GeneratorImpl::RecoveryIdentity::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[_Type::RecoveryIdentity]);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		m_cData.explain(cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryIdentity::initialize -- 
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
GeneratorImpl::RecoveryIdentity::
initialize(Interface::IProgram& cProgram_)
{
	// never used in prepare statement
	; _SYDNEY_ASSERT(m_pSchemaColumn);
	m_cData.initialize(cProgram_);
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryIdentity::terminate -- 
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
GeneratorImpl::RecoveryIdentity::
terminate(Interface::IProgram& cProgram_)
{
	m_cData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryIdentity::execute -- 
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
GeneratorImpl::RecoveryIdentity::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		Schema::Identity::assign(*m_pSchemaColumn,
								 m_cData->getValue(),
								 *cProgram_.getTransaction());
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Variable)) {
			_OPT_EXECUTION_MESSAGE
				<< "Recovery Identity column " << m_pSchemaColumn->getName()
				<< " by "
				<< Opt::Trace::toString(*m_cData)
				<< ModEndl;
		}
#endif
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryIdentity::finish -- 
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
GeneratorImpl::RecoveryIdentity::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryIdentity::reset -- 
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
GeneratorImpl::RecoveryIdentity::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryIdentity::getClassID -- 
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
GeneratorImpl::RecoveryIdentity::
getClassID() const
{
	return Class::getClassID(Class::Category::GeneratorRecoveryIdentity);
}

// FUNCTION public
//	Operator::GeneratorImpl::RecoveryIdentity::serialize -- 
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
GeneratorImpl::RecoveryIdentity::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cData.serialize(archiver_);
}

/////////////////////////////////
// Operator::Generator::RowID::

// FUNCTION public
//	Operator::Generator::RowID::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Table* pSchemaTable_
//	int iDataID_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::RowID::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Table* pSchemaTable_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new GeneratorImpl::RowID(pSchemaTable_, iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

///////////////////////////////////////////
// Operator::Generator::RowID::Recovery::

// FUNCTION public
//	Operator::Generator::RowID::Recovery::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Table* pSchemaTable_
//	int iDataID_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::RowID::Recovery::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Table* pSchemaTable_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new GeneratorImpl::RecoveryRowID(pSchemaTable_, iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::Generator::Identity::

// FUNCTION public
//	Operator::Generator::Identity::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Column* pSchemaColumn_
//	int iDataID_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::Identity::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Column* pSchemaColumn_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult =
		new GeneratorImpl::Identity(pSchemaColumn_->getTable(*cProgram_.getTransaction()),
									pSchemaColumn_,
									iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::Generator::Identity::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Column* pSchemaColumn_
//	int iInputID_
//	int iDataID_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::Identity::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Column* pSchemaColumn_,
	   int iInputID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult =
		new GeneratorImpl::IdentityByInput(pSchemaColumn_->getTable(*cProgram_.getTransaction()),
										   pSchemaColumn_,
										   iInputID_,
										   iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

///////////////////////////////////////////////
// Operator::Generator::Identity::Recovery::

// FUNCTION public
//	Operator::Generator::Identity::Recovery::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	Schema::Column* pSchemaColumn_
//	int iDataID_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::Identity::Recovery::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   Schema::Column* pSchemaColumn_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult =
		new GeneratorImpl::RecoveryIdentity(pSchemaColumn_->getTable(*cProgram_.getTransaction()),
											pSchemaColumn_,
											iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::Generator::

// FUNCTION public
//	Operator::Generator::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::GeneratorRowID:
		{
			return new GeneratorImpl::RowID;
		}
	case Class::Category::GeneratorIdentity:
		{
			return new GeneratorImpl::Identity;
		}
	case Class::Category::GeneratorIdentityByInput:
		{
			return new GeneratorImpl::IdentityByInput;
		}
	case Class::Category::GeneratorRecoveryRowID:
		{
			return new GeneratorImpl::RecoveryRowID;
		}
	case Class::Category::GeneratorRecoveryIdentity:
		{
			return new GeneratorImpl::RecoveryIdentity;
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
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
