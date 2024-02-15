// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Assign.cpp --
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

#include "Execution/Operator/Assign.h"
#include "Execution/Operator/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "assign";
}

namespace Impl
{

	// CLASS local
	//	Execution::Operator::Impl::AssignImpl -- implementation class of Assign
	//
	// NOTES
	class AssignImpl
		: public Operator::Assign
	{
	public:
		typedef AssignImpl This;
		typedef Operator::Assign Super;

		AssignImpl()
			: Super(),
			  m_cInData(),
			  m_cOutData(),
			  m_pInData(0),
			  m_pOutData(0),
			  m_pInArrayData(0),
			  m_pOutArrayData(0)
		{}
		AssignImpl(int iInDataID_,
				   int iOutDataID_)
			: Super(),
			  m_cInData(iInDataID_),
			  m_cOutData(iOutDataID_),
			  m_pInData(0),
			  m_pOutData(0),
			  m_pInArrayData(0),
			  m_pOutArrayData(0)
		{}
		~AssignImpl()
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
		void setData();
		void assign();
		void assignArray();

		Action::DataHolder m_cInData;
		Action::DataHolder m_cOutData;

		const Common::Data* m_pInData;
		Common::Data* m_pOutData;
		const Common::DataArrayData* m_pInArrayData;
		Common::DataArrayData* m_pOutArrayData;
	};
}

/////////////////////////////////////////////
// Execution::Operator::Impl::AssignImpl

// FUNCTION public
//	Operator::Impl::AssignImpl::explain -- 
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
Impl::AssignImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszOperatorName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cInData.explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Operator::Impl::AssignImpl::initialize -- 
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
Impl::AssignImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cInData.isInitialized() == false) {
		m_cInData.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::Impl::AssignImpl::terminate -- 
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
Impl::AssignImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_cInData.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
	m_pInArrayData = 0;
	m_pOutArrayData = 0;
}

// FUNCTION public
//	Operator::Impl::AssignImpl::execute -- 
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
Impl::AssignImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		setData();
		if (m_pInArrayData)
			assignArray();
		else
			assign();
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::Impl::AssignImpl::finish -- 
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
Impl::AssignImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::Impl::AssignImpl::reset -- 
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
Impl::AssignImpl::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::Impl::AssignImpl::getClassID -- 
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
Impl::AssignImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Assign);
}

// FUNCTION public
//	Operator::Impl::AssignImpl::serialize -- 
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
Impl::AssignImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cInData.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

// FUNCTION private
//	Operator::Impl::AssignImpl::setData -- 
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
Impl::AssignImpl::
setData()
{
	if (m_pInData == 0 && m_pInArrayData == 0) {
		const Common::Data* pInData = m_cInData.getData();
		Common::Data* pOutData = m_cOutData.get();

		if (pInData && pOutData) {
			if (pInData->getType() != Common::DataType::Null
				&& pOutData->getType() != Common::DataType::Data
				&& ((pInData->isScalar()
					 && pOutData->isScalar()
					 && !Common::Data::isCompatible(pInData->getType(), pOutData->getType()))
					|| (pInData->getType() == Common::DataType::Array
						&& pInData->getElementType() != pOutData->getElementType()))) {
				// data type inconsistent
				_SYDNEY_THROW0(Exception::NotSupported);
			}
			if (pInData->getType() == Common::DataType::Array
				&& pInData->getElementType() == Common::DataType::Data) {
				m_pInArrayData = _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
													  pInData);
				m_pOutArrayData = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*,
													   pOutData);

				if (m_pInArrayData->getCount() != m_pOutArrayData->getCount()) {
					_SYDNEY_THROW0(Exception::NotSupported);
				}
			} else {
				m_pInData = pInData;
				m_pOutData = pOutData;
			}
		}
	}
}

// FUNCTION private
//	Operator::Impl::AssignImpl::assign -- 
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
Impl::AssignImpl::
assign()
{
	; _SYDNEY_ASSERT(m_pInData);
	; _SYDNEY_ASSERT(m_pOutData);

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Variable)) {
		_OPT_EXECUTION_MESSAGE
			<< "Assign "
			<< Opt::Trace::toString(*m_pInData)
			<< ModEndl;
	}
#endif

	m_pOutData->assign(m_pInData);
}

// FUNCTION private
//	Operator::Impl::AssignImpl::assignArray -- 
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
Impl::AssignImpl::
assignArray()
{
	; _SYDNEY_ASSERT(m_pInArrayData);
	; _SYDNEY_ASSERT(m_pOutArrayData);
	; _SYDNEY_ASSERT(m_pInArrayData->getCount() == m_pOutArrayData->getCount());

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Variable)) {
		_OPT_EXECUTION_MESSAGE
			<< "Assign "
			<< Opt::Trace::toString(*m_pInArrayData)
			<< ModEndl;
	}
#endif

	Utility::DataType::assignElements(m_pOutArrayData,
									  m_pInArrayData);
}

//////////////////////////////
// Operator::Assign::

// FUNCTION public
//	Operator::Assign::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iInDataID_
//	int iOutDataID_
//	
// RETURN
//	Assign*
//
// EXCEPTIONS

//static
Assign*
Assign::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::AssignImpl(iInDataID_,
													 iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::Assign::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Assign*
//
// EXCEPTIONS

//static
Assign*
Assign::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Assign);
	return new Impl::AssignImpl;
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
