// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/SetNull.cpp --
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

#include "Execution/Operator/SetNull.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/Data.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "set null";
}

namespace Impl
{

	// CLASS local
	//	Execution::Operator::Impl::SetNullImpl -- implementation class of SetNull
	//
	// NOTES
	class SetNullImpl
		: public Operator::SetNull
	{
	public:
		typedef SetNullImpl This;
		typedef Operator::SetNull Super;

		SetNullImpl()
			: Super(),
			  m_cArrayData()
		{}
		SetNullImpl(int iDataID_)
			: Super(),
			  m_cArrayData(iDataID_)
		{}
		~SetNullImpl()
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
		Action::ArrayDataHolder m_cArrayData;
	};
}

/////////////////////////////////////////////
// Execution::Operator::Impl::SetNullImpl

// FUNCTION public
//	Operator::Impl::SetNullImpl::explain -- 
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
Impl::SetNullImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszOperatorName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cArrayData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Operator::Impl::SetNullImpl::initialize -- 
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
Impl::SetNullImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cArrayData.isInitialized() == false) {
		m_cArrayData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::Impl::SetNullImpl::terminate -- 
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
Impl::SetNullImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_cArrayData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::Impl::SetNullImpl::execute -- 
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
Impl::SetNullImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		int n = m_cArrayData->getCount();
		for (int i = 0; i < n; ++i) {
			m_cArrayData->getElement(i)->setNull();
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::Impl::SetNullImpl::finish -- 
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
Impl::SetNullImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::Impl::SetNullImpl::reset -- 
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
Impl::SetNullImpl::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::Impl::SetNullImpl::getClassID -- 
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
Impl::SetNullImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::SetNull);
}

// FUNCTION public
//	Operator::Impl::SetNullImpl::serialize -- 
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
Impl::SetNullImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cArrayData.serialize(archiver_);
}

//////////////////////////////
// Operator::SetNull::

// FUNCTION public
//	Operator::SetNull::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	
// RETURN
//	SetNull*
//
// EXCEPTIONS

//static
SetNull*
SetNull::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::SetNullImpl(iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::SetNull::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	SetNull*
//
// EXCEPTIONS

//static
SetNull*
SetNull::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::SetNull);
	return new Impl::SetNullImpl;
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
