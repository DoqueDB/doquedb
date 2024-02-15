// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Invoke.cpp --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Function";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Function/Invoke.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/StoredFunctionNotFound.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Schema/Database.h"
#include "Schema/Function.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	// CONST
	//	_pszFunctionName -- function name for explain
	//
	// NOTES
	const char* const _pszFunctionName = "invoke ";
}

namespace Impl
{
	// CLASS local
	//	Execution::Function::Impl::InvokeImpl -- implementation class of Invoke
	//
	// NOTES
	class InvokeImpl
		: public Function::Invoke
	{
	public:
		typedef InvokeImpl This;
		typedef Function::Invoke Super;

		InvokeImpl()
			: Super(),
			  m_cFunctionName(),
			  m_cOperandData(),
			  m_cOutData(),
			  m_pCompiledRoutine(0),
			  m_pRoutineIterator(0),
			  m_cRoutineOutput()
		{}
		InvokeImpl(const Schema::Function* pSchemaFunction_,
				   int iOperandDataID_,
				   int iOutDataID_)
			: Super(),
			  m_cFunctionName(pSchemaFunction_->getName()),
			  m_cOperandData(iOperandDataID_),
			  m_cOutData(iOutDataID_),
			  m_pCompiledRoutine(0),
			  m_pRoutineIterator(0),
			  m_cRoutineOutput()
		{}
		~InvokeImpl()
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
		virtual void undone(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		void setOperandData(const VECTOR<int>& vecOperandDataID_);

		STRING m_cFunctionName;
		Action::ArrayDataHolder m_cOperandData;
		Action::DataHolder m_cOutData;

		Interface::IProgram* m_pCompiledRoutine;
		Interface::IIterator* m_pRoutineIterator;
		Action::DataHolder m_cRoutineOutput;
	};
}

/////////////////////////////////////////////////
// Execution::Function::Impl::InvokeImpl

// FUNCTION public
//	Function::Impl::InvokeImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::InvokeImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszFunctionName);
	cExplain_.put(m_cFunctionName);
	cExplain_.popNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		m_cOperandData.explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Function::Impl::InvokeImpl::initialize -- 
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
Impl::InvokeImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cOutData.isInitialized() == false) {
		if (m_pCompiledRoutine == 0) {
			Schema::Function* pSchemaFunction =
				cProgram_.getDatabase()->getFunction(m_cFunctionName,
													 *cProgram_.getTransaction());
			if (pSchemaFunction == 0) {
				_SYDNEY_THROW2(Exception::StoredFunctionNotFound,
							   m_cFunctionName,
							   cProgram_.getDatabase()->getName());
			}
			m_pCompiledRoutine = pSchemaFunction->compile(*cProgram_.getTransaction());
			m_pRoutineIterator = m_pCompiledRoutine->getExecuteIterator(0);
			; _SYDNEY_ASSERT(m_pRoutineIterator);
			m_pCompiledRoutine->initialize(m_pRoutineIterator);
			m_cRoutineOutput.setDataID(m_pRoutineIterator->getOutData(*m_pCompiledRoutine));
			; _SYDNEY_ASSERT(m_cRoutineOutput.isValid());
			m_cRoutineOutput.initialize(*m_pCompiledRoutine);

			m_pCompiledRoutine->startUp(m_pRoutineIterator);
		}
		m_cOperandData.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::Impl::InvokeImpl::terminate -- 
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
Impl::InvokeImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_cOperandData.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
	if (m_pCompiledRoutine) {
		m_cRoutineOutput.terminate(*m_pCompiledRoutine);
		m_pCompiledRoutine->terminate(m_pRoutineIterator);
		m_pRoutineIterator = 0;
		delete m_pCompiledRoutine, m_pCompiledRoutine = 0;
	}
}

// FUNCTION public
//	Function::Impl::InvokeImpl::execute -- 
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
Impl::InvokeImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		; _SYDNEY_ASSERT(m_pCompiledRoutine);
		; _SYDNEY_ASSERT(m_pRoutineIterator);

		m_pCompiledRoutine->setParameter(m_cOperandData.getData());
		if (m_pCompiledRoutine->next(m_pRoutineIterator)) {
			m_cOutData->assign(m_cRoutineOutput.getData());
		} else {
			m_cOutData->setNull();
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::Impl::InvokeImpl::finish -- 
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
Impl::InvokeImpl::
finish(Interface::IProgram& cProgram_)
{
	if (m_pCompiledRoutine) {
		m_pCompiledRoutine->finish(m_pRoutineIterator);
	}
}

// FUNCTION public
//	Function::Impl::InvokeImpl::reset -- 
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
Impl::InvokeImpl::
reset(Interface::IProgram& cProgram_)
{
	if (m_pCompiledRoutine) {
		m_pCompiledRoutine->reset(m_pRoutineIterator);
	}
}

// FUNCTION public
//	Function::Impl::InvokeImpl::undone -- 
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
Impl::InvokeImpl::
undone(Interface::IProgram& cProgram_)
{
	Super::undone(cProgram_);
	reset(cProgram_);
}

// FUNCTION public
//	Function::Impl::InvokeImpl::getClassID -- 
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
Impl::InvokeImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Invoke);
}

// FUNCTION public
//	Function::Impl::InvokeImpl::serialize -- 
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
Impl::InvokeImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	archiver_(m_cFunctionName);
	m_cOperandData.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

//////////////////////////////
// Function::Invoke::

// FUNCTION public
//	Function::Invoke::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const Schema::Function* pSchemaFunction_
//	int iOperandData_
//	int iOutDataID_
//	
// RETURN
//	Invoke*
//
// EXCEPTIONS

//static
Invoke*
Invoke::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const Schema::Function* pSchemaFunction_,
	   int iOperandData_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::InvokeImpl(pSchemaFunction_,
													 iOperandData_,
													 iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Invoke::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Invoke*
//
// EXCEPTIONS

//static
Invoke*
Invoke::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::Invoke:
		{
			return new Impl::InvokeImpl;
		}
	default:
		{
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
