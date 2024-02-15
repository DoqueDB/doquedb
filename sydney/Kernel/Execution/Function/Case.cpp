// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Case.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Function/Case.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/PredicateHolder.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IPredicate.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	// CONST
	//	_pszFunctionName -- function name for explain
	//
	// NOTES
	const char* const _pszFunctionName = "case";
}

namespace Impl
{
	// CLASS local
	//	Execution::Function::Impl::CaseImpl -- implementation class of Case
	//
	// NOTES
	class CaseImpl
		: public Function::Case
	{
	public:
		typedef CaseImpl This;
		typedef Function::Case Super;

		CaseImpl()
			: Super(),
			  m_vecPredicate(),
			  m_vecData(),
			  m_cOutData()
		{}
		CaseImpl(const VECTOR<int>& vecDataID_,
				 const VECTOR<int>& vecPredicateID_,
				 int iOutDataID_)
			: Super(),
			  m_vecPredicate(),
			  m_vecData(),
			  m_cOutData(iOutDataID_)
		{
			setPredicate(vecPredicateID_);
			setData(vecDataID_);
		}
		~CaseImpl()
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
		// accessor
		Common::Data* getOutData() {return m_cOutData.get();}

	private:
		void setPredicate(const VECTOR<int>& vecPredicateID_);
		void setData(const VECTOR<int>& vecDataID_);

		VECTOR<Action::PredicateHolder> m_vecPredicate;
		VECTOR<Action::DataHolder> m_vecData;
		Action::DataHolder m_cOutData;
	};
}

/////////////////////////////////////////////////
// Execution::Function::Impl::CaseImpl

// FUNCTION public
//	Function::Impl::CaseImpl::explain -- 
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
Impl::CaseImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszFunctionName);
	cExplain_.popNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
		cExplain_.pushIndent();
		int n = m_vecPredicate.GETSIZE();
		int m = m_vecData.GETSIZE();
		; _SYDNEY_ASSERT(m == n || m == n + 1);
		int i = 0;
		int j = 0;
		for (; i < n && j < m; ++i, ++j) {
			cExplain_.newLine(true).put("when (");
			m_vecPredicate[i].explain(pEnvironment_,
									  cProgram_,
									  cExplain_);
			cExplain_.pushNoNewLine();
			cExplain_.put(") then ");
			m_vecData[i].explain(cProgram_,
								 cExplain_);
			cExplain_.popNoNewLine();
		}
		if (j < m) {
			cExplain_.pushNoNewLine();
			cExplain_.newLine(true).put("else ");
			m_vecData[j].explain(cProgram_,
								 cExplain_);
			cExplain_.popNoNewLine();
		}
		cExplain_.popIndent();
	}
}

// FUNCTION public
//	Function::Impl::CaseImpl::initialize -- 
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
Impl::CaseImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cOutData.isInitialized() == false) {
		Opt::ForEach(m_vecPredicate,
					 boost::bind(&Action::PredicateHolder::initialize,
								 _1,
								 boost::ref(cProgram_)));
		Opt::ForEach(m_vecData,
					 boost::bind(&Action::DataHolder::initialize,
								 _1,
								 boost::ref(cProgram_)));
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::Impl::CaseImpl::terminate -- 
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
Impl::CaseImpl::
terminate(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecPredicate,
				 boost::bind(&Action::PredicateHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	Opt::ForEach(m_vecData,
				 boost::bind(&Action::DataHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::Impl::CaseImpl::execute -- 
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
Impl::CaseImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		; _SYDNEY_ASSERT(m_vecPredicate.GETSIZE() == m_vecData.GETSIZE()
						 || m_vecPredicate.GETSIZE() + 1 == m_vecData.GETSIZE());

		VECTOR<Action::PredicateHolder>::ITERATOR iteratorP = m_vecPredicate.begin();
		const VECTOR<Action::PredicateHolder>::ITERATOR lastP = m_vecPredicate.end();
		VECTOR<Action::DataHolder>::ITERATOR iteratorD = m_vecData.begin();
		const VECTOR<Action::DataHolder>::ITERATOR lastD = m_vecData.end();

		for (; iteratorP != lastP; ++iteratorP, ++iteratorD) {
			; _SYDNEY_ASSERT(iteratorD != lastD);

			Action::Status::Value eResult = (*iteratorP)->execute(cProgram_, cActionList_);
			if (eResult == Action::Status::Success) {
				if ((*iteratorP)->check(cProgram_) == Interface::IPredicate::Boolean::True) {
					break;
				}
			}
		}
		if (iteratorD != lastD) {
			getOutData()->assign((*iteratorD).getData());
		} else {
			getOutData()->setNull();
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::Impl::CaseImpl::finish -- 
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
Impl::CaseImpl::
finish(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecPredicate,
				 boost::bind(&Interface::IPredicate::finish,
							 boost::bind(&Action::PredicateHolder::getPredicate,
										 _1),
							 boost::ref(cProgram_)));
}

// FUNCTION public
//	Function::Impl::CaseImpl::reset -- 
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
Impl::CaseImpl::
reset(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecPredicate,
				 boost::bind(&Interface::IPredicate::reset,
							 boost::bind(&Action::PredicateHolder::getPredicate,
										 _1),
							 boost::ref(cProgram_)));
}

// FUNCTION public
//	Function::Impl::CaseImpl::getClassID -- 
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
Impl::CaseImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Case);
}

// FUNCTION public
//	Function::Impl::CaseImpl::serialize -- 
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
Impl::CaseImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	Utility::SerializeObject(archiver_, m_vecPredicate);
	Utility::SerializeObject(archiver_, m_vecData);
	m_cOutData.serialize(archiver_);
}

// FUNCTION private
//	Function::Impl::CaseImpl::setPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<int>& vecPredicateID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::CaseImpl::
setPredicate(const VECTOR<int>& vecPredicateID_)
{
	int n = vecPredicateID_.GETSIZE();
	m_vecPredicate.assign(n, Action::PredicateHolder());
	for (int i = 0; i < n; ++i) {
		m_vecPredicate[i].setPredicateID(vecPredicateID_[i]);
	}
}


// FUNCTION private
//	Function::Impl::CaseImpl::setData -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<int>& vecDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::CaseImpl::
setData(const VECTOR<int>& vecDataID_)
{
	int n = vecDataID_.GETSIZE();
	m_vecData.assign(n, Action::DataHolder());
	for (int i = 0; i < n; ++i) {
		m_vecData[i].setDataID(vecDataID_[i]);
	}
}

//////////////////////////////
// Function::Case::

// FUNCTION public
//	Function::Case::create -- constructor -- N-adic coalese
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecDataID_
//	const VECTOR<int>& vecPredicateID_
//	int iOutDataID_
//	
// RETURN
//	Case*
//
// EXCEPTIONS

//static
Case*
Case::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecDataID_,
	   const VECTOR<int>& vecPredicateID_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::CaseImpl(vecDataID_,
												   vecPredicateID_,
												   iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Case::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Case*
//
// EXCEPTIONS

//static
Case*
Case::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::Case:
		{
			return new Impl::CaseImpl;
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
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
