// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/IsSubstringOf.cpp --
// 
// Copyright (c) 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Predicate/IsSubstringOf.h"
#include "Execution/Predicate/Class.h"

#include "Execution/Action/Collection.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PREDICATE_BEGIN

namespace
{
	const char* const _pszOperatorName = "is substring of";
}

namespace Impl
{
	// CLASS local
	//	Execution::Predicate::Impl::IsSubstringOfImpl -- implementation class of IsSubstringOf
	//
	// NOTES
	class IsSubstringOfImpl
		: public Predicate::IsSubstringOf
	{
	public:
		typedef IsSubstringOfImpl This;
		typedef Predicate::IsSubstringOf Super;

		// constructor
		IsSubstringOfImpl()
			: Super(),
			  m_cData0(),
			  m_cData1()
		{}
		IsSubstringOfImpl(int iDataID0_,
						  int iDataID1_)
			: Super(),
			  m_cData0(iDataID0_),
			  m_cData1(iDataID1_)
		{}

		// destructor
		virtual ~IsSubstringOfImpl() {}

	///////////////////////////
	// Predicate::IsSubstringOf::

	/////////////////////////////
	// Interface::IPredicate::
	//	virtual Boolean::Value check(Interface::IProgram& cProgram_);

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
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		// compare data
		static bool compare(const Common::DataArrayData* pArray0_,
							const Common::DataArrayData* pArray1_,
							ModSize iPosition0_,
							ModSize iPosition1_);

	/////////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		// operands
		Action::ArrayDataHolder m_cData0;
		Action::ArrayDataHolder m_cData1;
	};
} // namespace Impl

///////////////////////////////////////////////////
// Execution::Predicate::Impl::IsSubstringOfImpl

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::explain -- 
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
Impl::IsSubstringOfImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cData0.explain(cProgram_, cExplain_);
		cExplain_.put(" ");
	}
	cExplain_.put(_pszOperatorName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cData1.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::initialize -- 
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
Impl::IsSubstringOfImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData0.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData0.initialize(cProgram_);
		m_cData1.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::terminate -- 
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
Impl::IsSubstringOfImpl::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData0.isInitialized()) {
		m_cData0.terminate(cProgram_);
		m_cData1.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::finish -- 
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
Impl::IsSubstringOfImpl::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::reset -- 
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
Impl::IsSubstringOfImpl::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::getClassID -- 
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
Impl::IsSubstringOfImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::IsSubstringOf);
}

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::serialize --
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::IsSubstringOfImpl::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cData0.serialize(cArchive_);
	m_cData1.serialize(cArchive_);
}

// FUNCTION private
//	Predicate::Impl::IsSubstringOfImpl::compare -- compare data
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData* pArray0_
//	const Common::DataArrayData* pArray1_
//	ModSize iPosition0_
//	ModSize iPosition1_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Impl::IsSubstringOfImpl::
compare(const Common::DataArrayData* pArray0_,
		const Common::DataArrayData* pArray1_,
		ModSize iPosition0_,
		ModSize iPosition1_)
{
	return pArray0_->getElement(iPosition0_)->equals(pArray1_->getElement(iPosition1_).get());
}

// FUNCTION private
//	Predicate::Impl::IsSubstringOfImpl::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Boolean::Value
//
// EXCEPTIONS

//virtual
IsSubstringOf::Boolean::Value
Impl::IsSubstringOfImpl::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	// check whether data1 contains data0 as its substring

	if (m_cData0->isNull() || m_cData1->isNull()) {
		return Boolean::Unknown;
	}

	const Common::DataArrayData* pArray0 = m_cData0.getData();
	const Common::DataArrayData* pArray1 = m_cData1.getData();
	if (pArray0 == 0 || pArray1 == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	ModSize n0 = pArray0->getCount();
	ModSize n1 = pArray1->getCount();

	if (n0 == 0 || n1 == 0) {
		// empty array never become substring and never have substring
		return Boolean::False;
	}

	ModSize i1 = 0;

	while (true) {
		// find element of data1 which is same to the first element of data0
		ModSize i0 = 0;
		for (; i1 < n1; ++i1) {
			if (compare(pArray0, pArray1, i0, i1)) {
				// same element found
				break;
			}
			if (n1 - i1 < n0) {
				// not enough elements are left
				return Boolean::False;
			}
		}
		if (n1 - i1 < n0) {
			// not enough elements are left
			return Boolean::False;
		}

		// check successive elements
		++i1;
		++i0;
		ModSize iCandid = 0;
		for (;
			 i0 < n0
			 && i1 < n1
			 && compare(pArray0, pArray1, i0, i1);
			 ++i0, ++i1) {
			if (iCandid == 0
				&& compare(pArray0, pArray1, 0, i1)) {
				iCandid = i1;
			}
		}
		if (i0 == n0) {
			// found
			return Boolean::True;
		}
		// not found
		if (iCandid != 0) {
			i1 = iCandid;
		}
	}
	return Boolean::False;
}

/////////////////////////////////////
// Predicate::IsSubstringOf

// FUNCTION public
//	Predicate::IsSubstringOf::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iCollectionID_
//	int iDataID_
//	
// RETURN
//	IsSubstringOf*
//
// EXCEPTIONS

//static
IsSubstringOf*
IsSubstringOf::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iCollectionID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult =
		new Impl::IsSubstringOfImpl(iCollectionID_,
									  iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::IsSubstringOf::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	IsSubstringOf*
//
// EXCEPTIONS

//static
IsSubstringOf*
IsSubstringOf::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::IsSubstringOf);
	return new Impl::IsSubstringOfImpl;
}

_SYDNEY_EXECUTION_PREDICATE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
