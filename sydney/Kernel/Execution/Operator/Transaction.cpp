// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Transaction.cpp --
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

#include "Execution/Operator/Transaction.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/Argument.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/TableNotFound.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Schema/Table.h"

#include "Trans/AutoLatch.h"
#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	const char* const _pszExplainName =	"start batch";
}

namespace TransactionImpl
{
	// CLASS local
	//	Execution::Operator::TransactionImpl::StartBatch -- implementation class of Transaction
	//
	// NOTES
	class StartBatch
		: public Operator::Transaction
	{
	public:
		typedef StartBatch This;
		typedef Operator::Transaction Super;

		StartBatch()
			: Super(),
			  m_cTableName(),
			  m_pSchemaTable(0)
		{}
		explicit StartBatch(const Schema::Table* pSchemaTable_)
			: Super(),
			  m_cTableName(pSchemaTable_->getName()),
			  m_pSchemaTable(pSchemaTable_)
		{}

		~StartBatch() {}

	///////////////////////////
	// Operator::Transaction::

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
		Schema::Object::Name m_cTableName;
		const Schema::Table* m_pSchemaTable;
	};
}

//////////////////////////////////////////////////////////
// Execution::Operator::TransactionImpl::StartBatch

// FUNCTION public
//	Operator::TransactionImpl::StartBatch::explain -- 
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
TransactionImpl::StartBatch::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName);
}

// FUNCTION public
//	Operator::TransactionImpl::StartBatch::initialize -- 
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
TransactionImpl::StartBatch::
initialize(Interface::IProgram& cProgram_)
{
	if (m_pSchemaTable == 0) {
		// get schema::table from name
		m_pSchemaTable =
			cProgram_.getDatabase()->getTable(m_cTableName,
											  *cProgram_.getTransaction());
		if (m_pSchemaTable == 0) {
			_SYDNEY_THROW2(Exception::TableNotFound,
						   m_cTableName,
						   cProgram_.getDatabase()->getName());
		}
	}
}

// FUNCTION public
//	Operator::TransactionImpl::StartBatch::terminate -- 
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
TransactionImpl::StartBatch::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::TransactionImpl::StartBatch::execute -- 
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
TransactionImpl::StartBatch::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		Trans::Transaction& cTrans = *cProgram_.getTransaction();
		Trans::AutoLatch latch(cTrans,
							   cTrans.getLogInfo(Trans::Log::File::Category::Database).getLockName());
		cTrans.startBatchInsert(*m_pSchemaTable);
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::TransactionImpl::StartBatch::finish -- 
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
TransactionImpl::StartBatch::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::TransactionImpl::StartBatch::reset -- 
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
TransactionImpl::StartBatch::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::TransactionImpl::StartBatch::getClassID -- 
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
TransactionImpl::StartBatch::
getClassID() const
{
	return Class::getClassID(Class::Category::StartBatch);
}

// FUNCTION public
//	Operator::TransactionImpl::StartBatch::serialize -- 
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
TransactionImpl::StartBatch::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	archiver_(m_cTableName);
}

////////////////////////////////////////
// Operator::Transaction::StartBatch

// FUNCTION public
//	Operator::Transaction::StartBatch::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const Schema::Table* pSchemaTable_
//	
// RETURN
//	Transaction*
//
// EXCEPTIONS

//static
Transaction*
Transaction::StartBatch::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const Schema::Table* pSchemaTable_)
{
	AUTOPOINTER<This> pResult = new TransactionImpl::StartBatch(pSchemaTable_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::Transaction::

// FUNCTION public
//	Operator::Transaction::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Transaction*
//
// EXCEPTIONS

//static
Transaction*
Transaction::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::StartBatch:
		{
			return new TransactionImpl::StartBatch;
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
