// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Locator.cpp --
// 
// Copyright (c) 2011, 2012, 2016, 2023 Ricoh Company, Ltd.
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

#include "Execution/Operator/Locator.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/Argument.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Action/Locator.h"
#include "Execution/Function/SubString.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Transaction.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Os/Limits.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	struct _Type
	{
		enum Value {
			Length,
			Get,
			Append,
			Replace,
			Truncate,
			ValueNum
		};
	};
	const char* const _pszExplainName[] =
	{
		"length",
		"get",
		"append",
		"replace",
		"truncate",
	};
}

namespace LocatorImpl
{
	// CLASS local
	//	Execution::Operator::LocatorImpl::Base -- base class of implementation classes of Locator
	//
	// NOTES
	class Base
		: public Operator::Locator
	{
	public:
		typedef Base This;
		typedef Operator::Locator Super;

		virtual ~Base() {}

	///////////////////////////
	// Operator::Locator::

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
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
		Base()
			: Super(),
			  m_cLocator()
		{}
		explicit Base(int iLocatorID_)
			: Super(),
			  m_cLocator(iLocatorID_)
		{}

		// base implementation
		void initializeBase(Interface::IProgram& cProgram_);
		void terminateBase(Interface::IProgram& cProgram_);
		void serializeBase(ModArchive& archiver_);

		// accessor
		Action::LocatorHolder& getLocator() {return m_cLocator;}

		// check whether function is valid
		virtual bool isValid();

	private:
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;
		virtual void explainOption(Interface::IProgram& cProgram_,
								   Opt::Explain& cExplain_) = 0;

		virtual void doExecuteValid(Interface::IProgram& cProgram_) = 0;
		virtual void doExecuteInvalid(Interface::IProgram& cProgram_) = 0;

		Action::LocatorHolder m_cLocator;
	};

	// CLASS local
	//	Execution::Operator::LocatorImpl::Length -- implementation class of locator
	//
	// NOTES
	class Length
		: public Base
	{
	public:
		typedef Length This;
		typedef Base Super;

		Length()
			: Super()
		{}
		Length(int iLocatorID_,
			   int iDataID_)
			: Super(iLocatorID_),
			  m_cData(iDataID_)
		{}
		~Length() {}

	///////////////////////////
	// Operator::Locator::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

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
		void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////
	// Base::
	//	virtual bool isValid();

	private:
	///////////////////////////
	// Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void explainOption(Interface::IProgram& cProgram_,
								   Opt::Explain& cExplain_);
		virtual void doExecuteValid(Interface::IProgram& cProgram_);
		virtual void doExecuteInvalid(Interface::IProgram& cProgram_);

		Action::DataHolder m_cData;
	};

	// CLASS local
	//	Execution::Operator::LocatorImpl::Get -- implementation class of locator
	//
	// NOTES
	class Get
		: public Base
	{
	public:
		typedef Get This;
		typedef Base Super;

		Get()
			: Super()
		{}
		Get(int iLocatorID_,
			const VECTOR<int>& vecOptionID_,
			int iDataID_)
			: Super(iLocatorID_),
			  m_cStart(),
			  m_cLength(),
			  m_cData(iDataID_)
		{
			setOption(vecOptionID_);
		}
		~Get() {}

	///////////////////////////
	// Operator::Locator::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

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
		void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////
	// Base::
		virtual bool isValid();

	private:
		void setOption(const VECTOR<int>& vecOptionID_);

	///////////////////////////
	// Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void explainOption(Interface::IProgram& cProgram_,
								   Opt::Explain& cExplain_);
		virtual void doExecuteValid(Interface::IProgram& cProgram_);
		virtual void doExecuteInvalid(Interface::IProgram& cProgram_);

		Action::DataHolder m_cData;
		Action::DataHolder m_cStart;
		Action::DataHolder m_cLength;
	};

	// CLASS local
	//	Execution::Operator::LocatorImpl::Append -- implementation class of locator
	//
	// NOTES
	class Append
		: public Base
	{
	public:
		typedef Append This;
		typedef Base Super;

		Append()
			: Super()
		{}
		Append(int iLocatorID_,
			   int iOptionID_)
			: Super(iLocatorID_),
			  m_cAppendData(iOptionID_)
		{}
		~Append() {}

	///////////////////////////
	// Operator::Locator::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

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
		void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////
	// Base::
		virtual bool isValid();

	private:
		void setOption(const VECTOR<int>& vecOptionID_);

	///////////////////////////
	// Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void explainOption(Interface::IProgram& cProgram_,
								   Opt::Explain& cExplain_);
		virtual void doExecuteValid(Interface::IProgram& cProgram_);
		virtual void doExecuteInvalid(Interface::IProgram& cProgram_);

		Action::DataHolder m_cAppendData;
	};

	// CLASS local
	//	Execution::Operator::LocatorImpl::Truncate -- implementation class of locator
	//
	// NOTES
	class Truncate
		: public Base
	{
	public:
		typedef Truncate This;
		typedef Base Super;

		Truncate()
			: Super()
		{}
		Truncate(int iLocatorID_,
			   int iOptionID_)
			: Super(iLocatorID_),
			  m_cTruncateLength(iOptionID_)
		{}
		~Truncate() {}

	///////////////////////////
	// Operator::Locator::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

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
		void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////
	// Base::
		virtual bool isValid();

	private:
		void setOption(const VECTOR<int>& vecOptionID_);

	///////////////////////////
	// Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void explainOption(Interface::IProgram& cProgram_,
								   Opt::Explain& cExplain_);
		virtual void doExecuteValid(Interface::IProgram& cProgram_);
		virtual void doExecuteInvalid(Interface::IProgram& cProgram_);

		Action::DataHolder m_cTruncateLength;
	};

	// CLASS local
	//	Execution::Operator::LocatorImpl::Replace -- implementation class of locator
	//
	// NOTES
	class Replace
		: public Base
	{
	public:
		typedef Replace This;
		typedef Base Super;

		Replace()
			: Super()
		{}
		Replace(int iLocatorID_,
				const VECTOR<int>& vecOptionID_)
			: Super(iLocatorID_),
			  m_cPlaceData(),
			  m_cStart(),
			  m_cLength()
		{
			setOption(vecOptionID_);
		}
		~Replace() {}

	///////////////////////////
	// Operator::Locator::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

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
		void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////
	// Base::
		virtual bool isValid();

	private:
		void setOption(const VECTOR<int>& vecOptionID_);

	///////////////////////////
	// Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
		virtual void explainOption(Interface::IProgram& cProgram_,
								   Opt::Explain& cExplain_);
		virtual void doExecuteValid(Interface::IProgram& cProgram_);
		virtual void doExecuteInvalid(Interface::IProgram& cProgram_);

		Action::DataHolder m_cPlaceData;
		Action::DataHolder m_cStart;
		Action::DataHolder m_cLength;
	};
}

///////////////////////////////////////////////
// Execution::Operator::LocatorImpl::Base

// FUNCTION public
//	Operator::LocatorImpl::Base::explain -- 
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
LocatorImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	explainOperator(cProgram_, cExplain_);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cLocator.explain(cProgram_, cExplain_);
		explainOption(cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Operator::LocatorImpl::Base::initialize -- 
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
LocatorImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
}

// FUNCTION public
//	Operator::LocatorImpl::Base::terminate -- 
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
LocatorImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
}

// FUNCTION public
//	Operator::LocatorImpl::Base::execute -- 
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
LocatorImpl::Base::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (isValid()) {
			doExecuteValid(cProgram_);
		} else {
			doExecuteInvalid(cProgram_);
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::LocatorImpl::Base::finish -- 
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
LocatorImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::LocatorImpl::Base::reset -- 
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
LocatorImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::LocatorImpl::Base::undone -- 
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
LocatorImpl::Base::
undone(Interface::IProgram& cProgram_)
{
	m_cLocator.clear();
	Super::undone(cProgram_);
}

// FUNCTION public
//	Operator::LocatorImpl::Base::serialize -- 
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
LocatorImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
}

// FUNCTION protected
//	Operator::LocatorImpl::Base::initializeBase -- 
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

void
LocatorImpl::Base::
initializeBase(Interface::IProgram& cProgram_)
{
	m_cLocator.initialize(cProgram_);
}

// FUNCTION protected
//	Operator::LocatorImpl::Base::terminateBase -- 
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

void
LocatorImpl::Base::
terminateBase(Interface::IProgram& cProgram_)
{
	m_cLocator.terminate(cProgram_);
}

// FUNCTION protected
//	Operator::LocatorImpl::Base::serializeBase -- 
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
LocatorImpl::Base::
serializeBase(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cLocator.serialize(archiver_);
}

// FUNCTION protected
//	Operator::LocatorImpl::Base::isValid -- check whether function is valid
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LocatorImpl::Base::
isValid()
{
	// default: locator is valid
	return getLocator()->isValid();
}

///////////////////////////////////////////////
// Execution::Operator::LocatorImpl::Length

// FUNCTION public
//	Operator::LocatorImpl::Length::initialize -- 
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
LocatorImpl::Length::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cData.initialize(cProgram_);
}
	
// FUNCTION public
//	Operator::LocatorImpl::Length::terminate -- 
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
LocatorImpl::Length::
terminate(Interface::IProgram& cProgram_)
{
	m_cData.terminate(cProgram_);
	terminateBase(cProgram_);
}

// FUNCTION public
//	Operator::LocatorImpl::Length::getClassID -- 
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
LocatorImpl::Length::
getClassID() const
{
	return Class::getClassID(Class::Category::LocatorLength);
}

// FUNCTION public
//	Operator::LocatorImpl::Length::serialize -- 
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

//virtual
void
LocatorImpl::Length::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cData.serialize(archiver_);
}

// FUNCTION private
//	Operator::LocatorImpl::Length::explainOperator -- 
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
LocatorImpl::Length::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Length]);
}

// FUNCTION private
//	Operator::LocatorImpl::Length::explainOption -- 
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
LocatorImpl::Length::
explainOption(Interface::IProgram& cProgram_,
			  Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(" to ");
	m_cData.explain(cProgram_, cExplain_);
	cExplain_.popNoNewLine();
}

// FUNCTION private
//	Operator::LocatorImpl::Length::doExecuteValid -- 
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
LocatorImpl::Length::
doExecuteValid(Interface::IProgram& cProgram_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Locator [" << getLocator().getID() << "] "
			<< _pszExplainName[_Type::Length]
			<< ModEndl;
	}
#endif
	Common::UnsignedIntegerData cResult;
	getLocator()->length(&cResult);
	m_cData->assign(&cResult);
}

// FUNCTION private
//	Operator::LocatorImpl::Length::doExecuteInvalid -- 
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
LocatorImpl::Length::
doExecuteInvalid(Interface::IProgram& cProgram_)
{
	m_cData->setNull();
}

///////////////////////////////////////////////
// Execution::Operator::LocatorImpl::Get

// FUNCTION public
//	Operator::LocatorImpl::Get::initialize -- 
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
LocatorImpl::Get::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cData.initialize(cProgram_);
	m_cStart.initialize(cProgram_);
	m_cLength.initialize(cProgram_);
}
	
// FUNCTION public
//	Operator::LocatorImpl::Get::terminate -- 
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
LocatorImpl::Get::
terminate(Interface::IProgram& cProgram_)
{
	m_cData.terminate(cProgram_);
	m_cStart.terminate(cProgram_);
	m_cLength.terminate(cProgram_);
	terminateBase(cProgram_);
}

// FUNCTION public
//	Operator::LocatorImpl::Get::getClassID -- 
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
LocatorImpl::Get::
getClassID() const
{
	return Class::getClassID(Class::Category::LocatorGet);
}

// FUNCTION public
//	Operator::LocatorImpl::Get::serialize -- 
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

//virtual
void
LocatorImpl::Get::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cData.serialize(archiver_);
	m_cStart.serialize(archiver_);
	m_cLength.serialize(archiver_);
}

// FUNCTION protected
//	Operator::LocatorImpl::Get::isValid -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LocatorImpl::Get::
isValid()
{
	return Super::isValid()
		&& (!m_cStart.isInitialized() || !m_cStart->isNull())
		&& (!m_cLength.isInitialized() || !m_cLength->isNull());
}

// FUNCTION private
//	Operator::LocatorImpl::Get::setOption -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<int>& vecOptionID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LocatorImpl::Get::
setOption(const VECTOR<int>& vecOptionID_)
{
	switch (vecOptionID_.GETSIZE()) {
	case 2:
		{
			m_cLength.setDataID(vecOptionID_[1]);
			// thru.
		}
	case 1:
		{
			m_cStart.setDataID(vecOptionID_[0]);
			// thru.
		}
	case 0:
		{
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

// FUNCTION private
//	Operator::LocatorImpl::Get::explainOperator -- 
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
LocatorImpl::Get::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Get]);
}

// FUNCTION private
//	Operator::LocatorImpl::Get::explainOption -- 
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
LocatorImpl::Get::
explainOption(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (m_cStart.isValid()) {
		cExplain_.put(" from ");
		m_cStart.explain(cProgram_, cExplain_);
	}
	if (m_cLength.isValid()) {
		cExplain_.put(" for ");
		m_cLength.explain(cProgram_, cExplain_);
	}
	cExplain_.put(" to ");
	m_cData.explain(cProgram_, cExplain_);
	cExplain_.popNoNewLine();
}

// FUNCTION private
//	Operator::LocatorImpl::Get::doExecuteValid -- 
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
LocatorImpl::Get::
doExecuteValid(Interface::IProgram& cProgram_)
{
	int iStart = 0;
	int iLength = Os::Limits<int>::getMax();
	if (m_cStart.isInitialized()) {
		iStart = m_cStart->getInt() - 1; // checkArgument needs 0base
	}
	if (m_cLength.isInitialized()) {
		iLength = m_cLength->getInt();
	}
	int iMaxLength = iLength;
	if (iStart > 0 && iMaxLength <= Os::Limits<int>::getMax() - iStart) {
		iMaxLength += iStart;
	}
	Function::SubString::checkArgument(&iStart, &iLength, iMaxLength);
	++iStart; // 0base->1base

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Locator [" << getLocator().getID() << "] "
			<< _pszExplainName[_Type::Get]
			<< " from " << iStart
			<< " for "  << iLength
			<< ModEndl;
	}
#endif

	Common::UnsignedIntegerData cStart(iStart);
	Common::UnsignedIntegerData cLength(iLength);
	(void) getLocator()->get(&cStart,
							 &cLength,
							 m_cData.get());
}

// FUNCTION private
//	Operator::LocatorImpl::Get::doExecuteInvalid -- 
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
LocatorImpl::Get::
doExecuteInvalid(Interface::IProgram& cProgram_)
{
	m_cData->setNull();
}

///////////////////////////////////////////////
// Execution::Operator::LocatorImpl::Append

// FUNCTION public
//	Operator::LocatorImpl::Append::initialize -- 
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
LocatorImpl::Append::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cAppendData.initialize(cProgram_);
}
	
// FUNCTION public
//	Operator::LocatorImpl::Append::terminate -- 
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
LocatorImpl::Append::
terminate(Interface::IProgram& cProgram_)
{
	m_cAppendData.terminate(cProgram_);
	terminateBase(cProgram_);
}

// FUNCTION public
//	Operator::LocatorImpl::Append::getClassID -- 
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
LocatorImpl::Append::
getClassID() const
{
	return Class::getClassID(Class::Category::LocatorAppend);
}

// FUNCTION public
//	Operator::LocatorImpl::Append::serialize -- 
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

//virtual
void
LocatorImpl::Append::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cAppendData.serialize(archiver_);
}

// FUNCTION protected
//	Operator::LocatorImpl::Append::isValid -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LocatorImpl::Append::
isValid()
{
	return Super::isValid()
		&& (!m_cAppendData.isInitialized() || !m_cAppendData->isNull());
}

// FUNCTION private
//	Operator::LocatorImpl::Append::explainOperator -- 
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
LocatorImpl::Append::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Append]);
}

// FUNCTION private
//	Operator::LocatorImpl::Append::explainOption -- 
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
LocatorImpl::Append::
explainOption(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (m_cAppendData.isValid()) {
		cExplain_.put(" by ");
		m_cAppendData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION private
//	Operator::LocatorImpl::Append::doExecuteValid -- 
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
LocatorImpl::Append::
doExecuteValid(Interface::IProgram& cProgram_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Locator [" << getLocator().getID() << "] "
			<< _pszExplainName[_Type::Append]
			<< ModEndl;
	}
#endif

	(void) getLocator()->append(m_cAppendData.get());
}

// FUNCTION private
//	Operator::LocatorImpl::Append::doExecuteInvalid -- 
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
LocatorImpl::Append::
doExecuteInvalid(Interface::IProgram& cProgram_)
{
	; // do nothing
}

///////////////////////////////////////////////
// Execution::Operator::LocatorImpl::Truncate

// FUNCTION public
//	Operator::LocatorImpl::Truncate::initialize -- 
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
LocatorImpl::Truncate::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cTruncateLength.initialize(cProgram_);
}
	
// FUNCTION public
//	Operator::LocatorImpl::Truncate::terminate -- 
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
LocatorImpl::Truncate::
terminate(Interface::IProgram& cProgram_)
{
	m_cTruncateLength.terminate(cProgram_);
	terminateBase(cProgram_);
}

// FUNCTION public
//	Operator::LocatorImpl::Truncate::getClassID -- 
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
LocatorImpl::Truncate::
getClassID() const
{
	return Class::getClassID(Class::Category::LocatorTruncate);
}

// FUNCTION public
//	Operator::LocatorImpl::Truncate::serialize -- 
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

//virtual
void
LocatorImpl::Truncate::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cTruncateLength.serialize(archiver_);
}

// FUNCTION protected
//	Operator::LocatorImpl::Truncate::isValid -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LocatorImpl::Truncate::
isValid()
{
	return Super::isValid()
		&& (!m_cTruncateLength.isInitialized() || !m_cTruncateLength->isNull());
}

// FUNCTION private
//	Operator::LocatorImpl::Truncate::explainOperator -- 
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
LocatorImpl::Truncate::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Truncate]);
}

// FUNCTION private
//	Operator::LocatorImpl::Truncate::explainOption -- 
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
LocatorImpl::Truncate::
explainOption(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (m_cTruncateLength.isValid()) {
		cExplain_.put(" by ");
		m_cTruncateLength.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION private
//	Operator::LocatorImpl::Truncate::doExecuteValid -- 
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
LocatorImpl::Truncate::
doExecuteValid(Interface::IProgram& cProgram_)
{
	int iTruncateLength = Os::Limits<int>::getMax();
	if (m_cTruncateLength.isInitialized()) {
		iTruncateLength = m_cTruncateLength->getInt();
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Locator [" << getLocator().getID() << "] "
			<< _pszExplainName[_Type::Truncate]
			<< " by " << iTruncateLength
			<< ModEndl;
	}
#endif
	Common::UnsignedIntegerData cTruncateLength(iTruncateLength);
	(void) getLocator()->truncate(&cTruncateLength);
}

// FUNCTION private
//	Operator::LocatorImpl::Truncate::doExecuteInvalid -- 
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
LocatorImpl::Truncate::
doExecuteInvalid(Interface::IProgram& cProgram_)
{
	; // do nothing
}

///////////////////////////////////////////////
// Execution::Operator::LocatorImpl::Replace

// FUNCTION public
//	Operator::LocatorImpl::Replace::initialize -- 
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
LocatorImpl::Replace::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cPlaceData.initialize(cProgram_);
	m_cStart.initialize(cProgram_);
	m_cLength.initialize(cProgram_);
}
	
// FUNCTION public
//	Operator::LocatorImpl::Replace::terminate -- 
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
LocatorImpl::Replace::
terminate(Interface::IProgram& cProgram_)
{
	m_cPlaceData.terminate(cProgram_);
	m_cStart.terminate(cProgram_);
	m_cLength.terminate(cProgram_);
	terminateBase(cProgram_);
}

// FUNCTION public
//	Operator::LocatorImpl::Replace::getClassID -- 
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
LocatorImpl::Replace::
getClassID() const
{
	return Class::getClassID(Class::Category::LocatorReplace);
}

// FUNCTION public
//	Operator::LocatorImpl::Replace::serialize -- 
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

//virtual
void
LocatorImpl::Replace::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cPlaceData.serialize(archiver_);
	m_cStart.serialize(archiver_);
	m_cLength.serialize(archiver_);
}

// FUNCTION protected
//	Operator::LocatorImpl::Replace::isValid -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LocatorImpl::Replace::
isValid()
{
	return Super::isValid()
		&& (!m_cPlaceData.isInitialized() || !m_cPlaceData->isNull())
		&& (!m_cStart.isInitialized() || !m_cStart->isNull())
		&& (!m_cLength.isInitialized() || !m_cLength->isNull());
}

// FUNCTION private
//	Operator::LocatorImpl::Replace::setOption -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<int>& vecOptionID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LocatorImpl::Replace::
setOption(const VECTOR<int>& vecOptionID_)
{
	switch (vecOptionID_.GETSIZE()) {
	case 3:
		{
			m_cLength.setDataID(vecOptionID_[2]);
			// thru.
		}
	case 2:
		{
			m_cStart.setDataID(vecOptionID_[1]);
			// thru.
		}
	case 1:
		{
			m_cPlaceData.setDataID(vecOptionID_[0]);
			// thru.
		}
	case 0:
		{
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

// FUNCTION private
//	Operator::LocatorImpl::Replace::explainOperator -- 
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
LocatorImpl::Replace::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Replace]);
}

// FUNCTION private
//	Operator::LocatorImpl::Replace::explainOption -- 
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
LocatorImpl::Replace::
explainOption(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (m_cPlaceData.isValid()) {
		cExplain_.put(" placing ");
		m_cPlaceData.explain(cProgram_, cExplain_);
	}
	if (m_cStart.isValid()) {
		cExplain_.put(" from ");
		m_cStart.explain(cProgram_, cExplain_);
	}
	if (m_cLength.isValid()) {
		cExplain_.put(" for ");
		m_cLength.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION private
//	Operator::LocatorImpl::Replace::doExecuteValid -- 
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
LocatorImpl::Replace::
doExecuteValid(Interface::IProgram& cProgram_)
{
	int iStart = 0;
	int iLength = Os::Limits<int>::getMax();
	if (m_cStart.isInitialized()) {
		iStart = m_cStart->getInt() - 1; // checkArgument needs 0base
	}
	if (m_cLength.isInitialized()) {
		iLength = m_cLength->getInt();
	}
	int iMaxLength = iLength;
	if (iStart > 0 && iMaxLength <= Os::Limits<int>::getMax() - iStart) {
		iMaxLength += iStart;
	}

	// first part check [0, start]
	int iFirstPartStart = 0;
	int iFirstPartLength = iStart;
	Function::SubString::checkArgument(&iFirstPartStart, &iFirstPartLength, iMaxLength);
	// last part check [start+length, <end>]
	int iLastPartStart = iStart + iLength;
	int iLastPartLength = Os::Limits<int>::getMax();
	Function::SubString::checkArgument(&iLastPartStart, &iLastPartLength, iMaxLength);

	++iStart; // 0base->1base
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Locator [" << getLocator().getID() << "] "
			<< _pszExplainName[_Type::Replace]
			<< " from " << iStart
			<< ModEndl;
	}
#endif

	Common::UnsignedIntegerData cStart(iStart);
	Common::UnsignedIntegerData cLength(iLength);

	Common::UnsignedIntegerData* pStart = &cStart;
	Common::UnsignedIntegerData* pLength = m_cLength.isInitialized() ? &cLength : 0;

	(void) getLocator()->replace(m_cPlaceData.get(),
								 pStart,
								 pLength);
}

// FUNCTION private
//	Operator::LocatorImpl::Replace::doExecuteInvalid -- 
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
LocatorImpl::Replace::
doExecuteInvalid(Interface::IProgram& cProgram_)
{
	; // do nothing
}

/////////////////////////////////
// Operator::Locator::Length

// FUNCTION public
//	Operator::Locator::Length::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const Action::LocatorArgument& cArgument_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::Length::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iLocatorID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new LocatorImpl::Length(iLocatorID_,
														iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::Locator::Get

// FUNCTION public
//	Operator::Locator::Get::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iLocatorID_
//	const VECTOR<int>& vecOptionID_
//	int iDataID_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::Get::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iLocatorID_,
	   const VECTOR<int>& vecOptionID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new LocatorImpl::Get(iLocatorID_,
													 vecOptionID_,
													 iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::Locator::Get::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iLocatorID_
//	int iDataID_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::Get::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iLocatorID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new LocatorImpl::Get(iLocatorID_,
													 VECTOR<int>(),
													 iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::Locator::Append

// FUNCTION public
//	Operator::Locator::Append::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iLocatorID_
//	int iOptionID_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::Append::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iLocatorID_,
	   int iOptionID_)
{
	AUTOPOINTER<This> pResult = new LocatorImpl::Append(iLocatorID_,
														iOptionID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::Locator::Truncate

// FUNCTION public
//	Operator::Locator::Truncate::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iLocatorID_
//	int iOptionID_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::Truncate::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iLocatorID_,
	   int iOptionID_)
{
	AUTOPOINTER<This> pResult = new LocatorImpl::Truncate(iLocatorID_,
														  iOptionID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::Locator::Replace

// FUNCTION public
//	Operator::Locator::Replace::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iLocatorID_
//	const VECTOR<int>& vecOptionID_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::Replace::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iLocatorID_,
	   const VECTOR<int>& vecOptionID_)
{
	AUTOPOINTER<This> pResult = new LocatorImpl::Replace(iLocatorID_,
														 vecOptionID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::Locator::

// FUNCTION public
//	Operator::Locator::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::LocatorLength:
		{
			return new LocatorImpl::Length;
		}
	case Class::Category::LocatorGet:
		{
			return new LocatorImpl::Get;
		}
	case Class::Category::LocatorAppend:
		{
			return new LocatorImpl::Append;
		}
	case Class::Category::LocatorTruncate:
		{
			return new LocatorImpl::Truncate;
		}
	case Class::Category::LocatorReplace:
		{
			return new LocatorImpl::Replace;
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
// Copyright (c) 2011, 2012, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
