// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Control/Status.cpp --
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
const char moduleName[] = "Execution::Control";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Control/Status.h"
#include "Execution/Control/Class.h"

#include "Execution/Action/ActionList.h"

#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_CONTROL_BEGIN

namespace
{
	struct _Explain
	{
		enum Value
		{
			Continue,
			Break,
			ValueNum
		};
	};
	const char* const _pszExplainName[] = {
		"continue",
		"break",
		0
	};
}

namespace StatusImpl
{
	// CLASS local
	//	Execution::Control::StatusImpl::Continue -- implementation class of Status control
	//
	// NOTES
	class Continue
		: public Control::Status
	{
	public:
		typedef Continue This;
		typedef Control::Status Super;

		// constructor
		Continue()
			: Super()
		{}

		// destructor
		~Continue() {}

	///////////////////////////
	// Control::Status::

	/////////////////////////////
	// Interface::IControl::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
	};
	// CLASS local
	//	Execution::Control::StatusImpl::Break -- implementation class of Status control
	//
	// NOTES
	class Break
		: public Control::Status
	{
	public:
		typedef Break This;
		typedef Control::Status Super;

		// constructor
		Break()
			: Super()
		{}

		// destructor
		~Break() {}

	///////////////////////////
	// Control::Status::

	/////////////////////////////
	// Interface::IControl::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
	};

} // namespace StatusImpl

///////////////////////////////////////
// Control::StatusImpl::Continue

// FUNCTION public
//	Control::StatusImpl::Continue::explain -- 
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
StatusImpl::Continue::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::Continue]);
}

// FUNCTION public
//	Control::StatusImpl::Continue::execute -- 
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
StatusImpl::Continue::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	return Action::Status::Continue;
}

// FUNCTION public
//	Control::StatusImpl::Continue::getClassID -- 
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
StatusImpl::Continue::
getClassID() const
{
	return Class::getClassID(Class::Category::Continue);
}

// FUNCTION public
//	Control::StatusImpl::Continue::serialize -- for serialize
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

//virtual
void
StatusImpl::Continue::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	serializeBase(cArchive_);
}

///////////////////////////////////////
// Control::StatusImpl::Break

// FUNCTION public
//	Control::StatusImpl::Break::explain -- 
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
StatusImpl::Break::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::Break]);
}

// FUNCTION public
//	Control::StatusImpl::Break::execute -- 
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
StatusImpl::Break::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	return Action::Status::Break;
}

// FUNCTION public
//	Control::StatusImpl::Break::getClassID -- 
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
StatusImpl::Break::
getClassID() const
{
	return Class::getClassID(Class::Category::Break);
}

// FUNCTION public
//	Control::StatusImpl::Break::serialize -- for serialize
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

//virtual
void
StatusImpl::Break::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	serializeBase(cArchive_);
}

/////////////////////////////////
// Control::Status::Continue

// FUNCTION public
//	Control::Status::Continue::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	
// RETURN
//	Status*
//
// EXCEPTIONS

//static
Status*
Status::Continue::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_)
{
	AUTOPOINTER<This> pResult = new StatusImpl::Continue;
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Control::Status::Break

// FUNCTION public
//	Control::Status::Break::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	
// RETURN
//	Status*
//
// EXCEPTIONS

//static
Status*
Status::Break::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_)
{
	AUTOPOINTER<This> pResult = new StatusImpl::Break;
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Control::Status::

// FUNCTION public
//	Control::Status::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Status*
//
// EXCEPTIONS

//static
Status*
Status::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Continue:
		{
			return new StatusImpl::Continue;
		}
	case Class::Category::Break:
		{
			return new StatusImpl::Break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_EXECUTION_CONTROL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
