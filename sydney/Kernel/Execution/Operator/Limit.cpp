// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Limit.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Operator/Limit.h"
#include "Execution/Operator/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Action/IteratorHolder.h"
#include "Execution/Action/Limit.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/Data.h"

#include "Exception/NotSupported.h"

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
	const char* const _pszOperatorName = "check limit";
}

namespace LimitImpl
{
	// CLASS local
	//	Execution::Operator::LimitImpl::Base -- base class of implementation class
	//
	// NOTES
	class Base
		: public Operator::Limit
	{
	public:
		typedef Base This;
		typedef Operator::Limit Super;

		virtual ~Base()
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
	//	int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		Base()
			: Super(),
			  m_cIterator(),
			  m_cLimit(-1, -1),
			  m_iCurrent(-1)
		{}
		Base(int iIteratorID_,
			 const PAIR<int, int>& cLimit_,
			 bool bIntermediate_)
			: Super(),
			  m_cIterator(iIteratorID_),
			  m_cLimit(cLimit_, bIntermediate_),
			  m_iCurrent(-1)
		{}

		// accessor
		Action::IteratorHolder& getIterator() {return m_cIterator;}

	private:
		// do something when iteration reaches limit
		virtual void reachLimit(Interface::IProgram& cProgram_) = 0;

		// iterator which result data is limited
		Action::IteratorHolder m_cIterator;
		// limit holder
		Action::Limit m_cLimit;

		// current position
		int m_iCurrent;
	};

	// CLASS local
	//	Execution::Operator::LimitImpl::Normal -- implementation class of limit
	//
	// NOTES
	class Normal
		: public Base
	{
	public:
		typedef Normal This;
		typedef Base Super;

		Normal()
			: Super()
		{}
		Normal(int iIteratorID_,
			   const PAIR<int, int>& cLimit_,
			   bool bIntermediate_)
			: Super(iIteratorID_, cLimit_, bIntermediate_)
		{}
		~Normal()
		{}

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
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
	//////////////////////////////
	// LimitImpl::Base::
		virtual void reachLimit(Interface::IProgram& cProgram_);
	};

	// CLASS local
	//	Execution::Operator::LimitImpl::Partial -- implementation class of limit
	//
	// NOTES
	class Partial
		: public Base
	{
	public:
		typedef Partial This;
		typedef Base Super;

		Partial()
			: Super(),
			  m_cKey(),
			  m_pLimitKey()
		{}
		Partial(int iIteratorID_,
				const PAIR<int, int>& cLimit_,
				int iKeyID_)
			: Super(iIteratorID_, cLimit_, true /* intermediate */),
			  m_cKey(iKeyID_),
			  m_pLimitKey()
		{}
		~Partial()
		{}

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
		const Common::Data* getKey() {return m_cKey.getData();}

	//////////////////////////////
	// LimitImpl::Base::
		virtual void reachLimit(Interface::IProgram& cProgram_);

		Action::DataHolder m_cKey;
		Common::Data::Pointer m_pLimitKey;
	};
}

/////////////////////////////////////////////
// Execution::Operator::LimitImpl::Base

// FUNCTION public
//	Operator::LimitImpl::Base::explain -- 
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
LimitImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszOperatorName);
	cExplain_.put(" ");
	m_cLimit.explain(pEnvironment_,
					 cProgram_,
					 cExplain_);
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Operator::LimitImpl::Base::initialize -- 
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
LimitImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	m_cIterator.initialize(cProgram_);
	m_cLimit.initialize(cProgram_);
	m_iCurrent = 0;
}

// FUNCTION public
//	Operator::LimitImpl::Base::terminate -- 
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
LimitImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	// don't terminate m_cIterator, because it terminates iterator itself
	m_cIterator.clearIterator();
	m_cLimit.terminate(cProgram_);
	m_iCurrent = -1;
}

// FUNCTION public
//	Operator::LimitImpl::Base::execute -- 
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
LimitImpl::Base::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		m_cLimit.setValues(cProgram_);
		if (++m_iCurrent < m_cLimit.getOffset()) {
			// go to next iteration without doing more
			return Action::Status::Continue;
		}
		if (m_cLimit.getLimit() <= m_iCurrent) {
			// if current position reaches limit, set iterator as end-of-data at next iteration
			reachLimit(cProgram_);
			return Action::Status::Success;
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::LimitImpl::Base::finish -- 
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
LimitImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	m_iCurrent = 0; // do nothing
}

// FUNCTION public
//	Operator::LimitImpl::Base::reset -- 
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
LimitImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	m_iCurrent = 0;
}

// FUNCTION public
//	Operator::LimitImpl::Base::serialize -- 
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
LimitImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cIterator.serialize(archiver_);
	m_cLimit.serialize(archiver_);
}

/////////////////////////////////////////////
// Execution::Operator::LimitImpl::Normal

// FUNCTION public
//	Operator::LimitImpl::Normal::getClassID -- 
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
LimitImpl::Normal::
getClassID() const
{
	return Class::getClassID(Class::Category::Limit);
}

// FUNCTION private
//	Operator::LimitImpl::Normal::reachLimit -- 
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
LimitImpl::Normal::
reachLimit(Interface::IProgram& cProgram_)
{
	getIterator()->setWasLast(cProgram_);
}

/////////////////////////////////////////////
// Execution::Operator::LimitImpl::Partial

//virtual
void
LimitImpl::Partial::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	Super::explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.put(" partial key");
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		m_cKey.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Operator::LimitImpl::Partial::initialize -- 
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
LimitImpl::Partial::
initialize(Interface::IProgram& cProgram_)
{
	Super::initialize(cProgram_);
	m_cKey.initialize(cProgram_);
}

// FUNCTION public
//	Operator::LimitImpl::Partial::terminate -- 
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
LimitImpl::Partial::
terminate(Interface::IProgram& cProgram_)
{
	Super::terminate(cProgram_);
	m_cKey.terminate(cProgram_);
}

// FUNCTION public
//	Operator::LimitImpl::Partial::finish -- 
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
LimitImpl::Partial::
finish(Interface::IProgram& cProgram_)
{
	Super::finish(cProgram_);
	m_pLimitKey = Common::Data::Pointer();
}

// FUNCTION public
//	Operator::LimitImpl::Partial::reset -- 
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
LimitImpl::Partial::
reset(Interface::IProgram& cProgram_)
{
	Super::reset(cProgram_);
	m_pLimitKey = Common::Data::Pointer();
}

// FUNCTION public
//	Operator::LimitImpl::Partial::getClassID -- 
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
LimitImpl::Partial::
getClassID() const
{
	return Class::getClassID(Class::Category::LimitPartial);
}

// FUNCTION public
//	Operator::LimitImpl::Partial::serialize -- 
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
LimitImpl::Partial::
serialize(ModArchive& archiver_)
{
	Super::serialize(archiver_);
	m_cKey.serialize(archiver_);
}

// FUNCTION public
//	Operator::LimitImpl::Partial::reachLimit -- do something when iteration reaches limit
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
LimitImpl::Partial::
reachLimit(Interface::IProgram& cProgram_)
{
	if (m_pLimitKey.get() == 0) {
		m_pLimitKey = getKey()->copy();

	} else {
		if (m_pLimitKey->equals(getKey()) == false) {
			getIterator()->setWasLast(cProgram_);
		}
	}
}

//////////////////////////////
// Operator::Limit::Partial::

// FUNCTION public
//	Operator::Limit::Partial::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iIteratorID_
//	const PAIR<int, int>& cLimit_
//	int iKeyID_
//	
// RETURN
//	Limit*
//
// EXCEPTIONS

//static
Limit*
Limit::Partial::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iIteratorID_,
	   const PAIR<int, int>& cLimit_,
	   int iKeyID_)
{
	AUTOPOINTER<This> pResult = new LimitImpl::Partial(iIteratorID_,
													   cLimit_,
													   iKeyID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::Limit::

// FUNCTION public
//	Operator::Limit::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iIteratorID_
//	const PAIR<int, int>& cLimit_
//	bool bIntermediate_ = false
//	
// RETURN
//	Limit*
//
// EXCEPTIONS

//static
Limit*
Limit::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iIteratorID_,
	   const PAIR<int, int>& cLimit_,
	   bool bIntermediate_ /* = false */)
{
	AUTOPOINTER<This> pResult = new LimitImpl::Normal(iIteratorID_,
													  cLimit_,
													  bIntermediate_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::Limit::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Limit*
//
// EXCEPTIONS

//static
Limit*
Limit::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Limit:
		{
			return new LimitImpl::Normal;
		}
	case Class::Category::LimitPartial:
		{
			return new LimitImpl::Partial;
		}
	default:
		{
			_SYDNEY_ASSERT(false);
			break;
		}
	}
	return 0;
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
