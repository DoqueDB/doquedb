// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/Tuples.cpp --
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
const char moduleName[] = "Execution::Iterator";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Iterator/Tuples.h"
#include "Execution/Iterator/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/ICollection.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/Serialize.h"

#include "Exception/NotSupported.h"

#include "Common/Assert.h"
#include "Common/Data.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "tuples";
}

namespace Impl
{
	// CLASS local
	//	Execution::Iterator::Impl::TuplesImpl -- implementation classes of Tuples
	//
	// NOTES
	class TuplesImpl
		: public Iterator::Tuples
	{
	public:
		typedef TuplesImpl This;
		typedef Iterator::Tuples Super;

		TuplesImpl()
			: Super(),
			  m_cTuple(),
			  m_vecData(),
			  m_iCursor(-1)
		{}
		~TuplesImpl()
		{}

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual bool next(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
	//	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void setWasLast(Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();
		virtual int getOutData(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////
	//Iterator::Base::
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);

		virtual void addOutData(Interface::IProgram& cProgram_,
								const Action::Argument& cAction_);
		virtual void addInData(Interface::IProgram& cProgram_,
							   const Action::Argument& cAction_);

	private:
		Action::ArrayDataHolder m_cTuple;
		VECTOR<Action::ArrayDataHolder> m_vecData;
		int m_iCursor;
	};
}

/////////////////////////////////////////////
// Execution::Iterator::Impl::TuplesImpl

// FUNCTION public
//	Iterator::Impl::TuplesImpl::initialize -- 
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
Impl::TuplesImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cTuple.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cTuple.initialize(cProgram_);
		Opt::ForEach(m_vecData,
					 boost::bind(&Action::ArrayDataHolder::initialize,
								 _1,
								 boost::ref(cProgram_)));
	}
}

// FUNCTION public
//	Iterator::Impl::TuplesImpl::terminate -- 
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
Impl::TuplesImpl::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cTuple.isInitialized() == true) {
		m_cTuple.terminate(cProgram_);
		Opt::ForEach(m_vecData,
					 boost::bind(&Action::ArrayDataHolder::terminate,
								 _1,
								 boost::ref(cProgram_)));
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Iterator::Impl::TuplesImpl::next -- go to next tuple
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

//virtual
bool
Impl::TuplesImpl::
next(Interface::IProgram& cProgram_)
{
	// read one tuple and assign to result data
	if (hasData()) {
		if (++m_iCursor < static_cast<int>(m_vecData.GETSIZE())) {
			Utility::DataType::assignElements(m_cTuple.get(),
											  m_vecData[m_iCursor].getData());
#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
				_OPT_EXECUTION_MESSAGE
					<< "Tuples (#" << m_iCursor
					<< ") assign = "
					<< Opt::Trace::toString(*(m_cTuple.getData()))
					<< ModEndl;
			}
#endif
		} else {
			setHasData(false);
		}
	}
	return hasData();
}

// FUNCTION public
//	Iterator::Impl::TuplesImpl::reset -- 
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
Impl::TuplesImpl::
reset(Interface::IProgram& cProgram_)
{
	m_iCursor = -1;
	resetBase(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::TuplesImpl::finish -- 
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
Impl::TuplesImpl::
finish(Interface::IProgram& cProgram_)
{
	m_iCursor = -1;
	finishBase(cProgram_);
}

// FUNCTION public
//	Iterator::Impl::TuplesImpl::setWasLast -- 
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
Impl::TuplesImpl::
setWasLast(Interface::IProgram& cProgram_)
{
	m_iCursor = static_cast<int>(m_vecData.GETSIZE());
}

// FUNCTION public
//	Iterator::Impl::TuplesImpl::getOutData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::TuplesImpl::
getOutData(Interface::IProgram& cProgram_)
{
	return m_cTuple.getDataID();
}

// FUNCTION public
//	Iterator::Impl::TuplesImpl::getClassID -- 
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
Impl::TuplesImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Tuples);
}

// FUNCTION public
//	Iterator::Impl::TuplesImpl::serialize -- 
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
Impl::TuplesImpl::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cTuple.serialize(archiver_);
	Utility::SerializeObject(archiver_, m_vecData);
}

// FUNCTION protected
//	Iterator::Impl::TuplesImpl::explainThis -- 
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
Impl::TuplesImpl::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.pushNoNewLine();
		cExplain_.put(" to ");
		m_cTuple.explain(cProgram_, cExplain_);
		cExplain_.popNoNewLine();
	}
}

// FUNCTION protected
//	Iterator::Impl::TuplesImpl::addOutData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::TuplesImpl::
addOutData(Interface::IProgram& cProgram_,
		   const Action::Argument& cAction_)
{
	// set result id
	m_cTuple.setDataID(cAction_.getInstanceID());
}

// FUNCTION protected
//	Iterator::Impl::TuplesImpl::addInData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::TuplesImpl::
addInData(Interface::IProgram& cProgram_,
		  const Action::Argument& cAction_)
{
	// data vector is set through addAction(Tuples)
	m_vecData.PUSHBACK(Action::ArrayDataHolder(cAction_.getInstanceID()));
}

/////////////////////////////////
// Iterator::Tuples::

// FUNCTION public
//	Iterator::Tuples::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Tuples*
//
// EXCEPTIONS

//static
Tuples*
Tuples::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::TuplesImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Iterator::Tuples::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Tuples*
//
// EXCEPTIONS

//static
Tuples*
Tuples::
getInstance(int iCategory_)
{
	;_SYDNEY_ASSERT(iCategory_ == Class::Category::Tuples);
	return new Impl::TuplesImpl;
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
