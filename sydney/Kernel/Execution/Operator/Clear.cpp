// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Clear.cpp --
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "Execution/Operator/Clear.h"
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
	const char* const _pszOperatorName = "clear";
}

namespace Impl
{

	// CLASS local
	//	Execution::Operator::Impl::ClearImpl -- implementation class of Clear
	//
	// NOTES
	class ClearImpl
		: public Operator::Clear
	{
	public:
		typedef ClearImpl This;
		typedef Operator::Clear Super;

		ClearImpl()
			: Super(),
			  m_cArrayData()
		{}
		ClearImpl(int iDataID_)
			: Super(),
			  m_cArrayData(iDataID_)
		{}
		~ClearImpl()
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
		Action::ArrayDataHolder& getData() {return m_cArrayData;}
		
	private:
		Action::ArrayDataHolder m_cArrayData;
	};

	class ArrayClearImpl
		: public ClearImpl
	{
	public:
		typedef ArrayClearImpl This;
		typedef ClearImpl Super;

		ArrayClearImpl()
			: Super()
		{}

		ArrayClearImpl(int iDataID_)
			: Super(iDataID_)
		{}

		~ArrayClearImpl()
		{}

		/////////////////////////////
		// Interface::IAction::
		virtual Action::Status::Value
		execute(Interface::IProgram& cProgram_,
				Action::ActionList& cActionList_);
		
		///////////////////////////////
		// Common::Externalizable
		int getClassID() const;		
	};
}

/////////////////////////////////////////////
// Execution::Operator::Impl::ClearImpl

// FUNCTION public
//	Operator::Impl::ClearImpl::explain -- 
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
Impl::ClearImpl::
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
//	Operator::Impl::ClearImpl::initialize -- 
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
Impl::ClearImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cArrayData.isInitialized() == false) {
		m_cArrayData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::Impl::ClearImpl::terminate -- 
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
Impl::ClearImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_cArrayData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::Impl::ClearImpl::execute -- 
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
Impl::ClearImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		m_cArrayData->clear();
		m_cArrayData->setNull();
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::Impl::ClearImpl::finish -- 
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
Impl::ClearImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::Impl::ClearImpl::reset -- 
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
Impl::ClearImpl::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::Impl::ClearImpl::getClassID -- 
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
Impl::ClearImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Clear);
}

// FUNCTION public
//	Operator::Impl::ClearImpl::serialize -- 
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
Impl::ClearImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cArrayData.serialize(archiver_);
}

// FUNCTION public
//	Operator::Impl::ClearImpl::execute -- 
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
Impl::ArrayClearImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		
		int n = getData()->getCount();
		for (int i = 0; i < n; ++i) {
			Common::Data::Pointer pData = getData()->getElement(i);
			if (pData->getType() == Common::DataType::Array
				&& pData->getElementType() == Common::DataType::Data) {
				Common::DataArrayData* pArrayData =
					_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData.get());
				pArrayData->clear();
			}
			pData->setNull();
		}
		done();
	}
	return Action::Status::Success;
}


// FUNCTION public
//	Operator::Impl::ClearImpl::getClassID -- 
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
Impl::ArrayClearImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::ArrayClear);
}



//////////////////////////////
// Operator::Clear::

// FUNCTION public
//	Operator::Clear::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	
// RETURN
//	Clear*
//
// EXCEPTIONS

//static
Clear*
Clear::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::ClearImpl(iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::Clear::Array::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	
// RETURN
//	Clear*
//
// EXCEPTIONS

//static
Clear*
Clear::Array::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::ArrayClearImpl(iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}


// FUNCTION public
//	Operator::Clear::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Clear*
//
// EXCEPTIONS

//static
Clear*
Clear::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::Clear:
		return new Impl::ClearImpl;
	case Class::Category::ArrayClear:
		return new Impl::ArrayClearImpl;
	default:
		;_SYDNEY_ASSERT(false);
	}
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
