// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Control/Goto.cpp --
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
const char moduleName[] = "Execution::Control";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Control/Goto.h"
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
	const char* const _pszExplainName = "goto";
}

namespace Impl
{
	// CLASS local
	//	Execution::Control::Impl::GotoImpl -- implementation class of Goto control
	//
	// NOTES
	class GotoImpl
		: public Control::Goto
	{
	public:
		typedef GotoImpl This;
		typedef Control::Goto Super;

		// constructor
		GotoImpl()
			: Super()
		{}

		// destructor
		~GotoImpl() {}

	///////////////////////////
	// Control::Goto::

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
} // namespace Impl

///////////////////////////////////////
// Control::Impl::GotoImpl

// FUNCTION public
//	Control::Impl::GotoImpl::explain -- 
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
Impl::GotoImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszExplainName).put(" line ").put(getEnd()+1);
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Control::Impl::GotoImpl::execute -- 
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
Impl::GotoImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	cActionList_.setPointer(getEnd());
	return Action::Status::Success;
}

// FUNCTION public
//	Control::Impl::GotoImpl::getClassID -- 
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
Impl::GotoImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Goto);
}

// FUNCTION public
//	Control::Impl::GotoImpl::serialize -- for serialize
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
Impl::GotoImpl::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	serializeBase(cArchive_);
}

//////////////////////////////
// Control::Goto::

// FUNCTION public
//	Control::Goto::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	
// RETURN
//	Goto*
//
// EXCEPTIONS

//static
Goto*
Goto::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_)
{
	AUTOPOINTER<This> pResult = new Impl::GotoImpl;
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Control::Goto::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Goto*
//
// EXCEPTIONS

//static
Goto*
Goto::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Goto:
		{
			return new Impl::GotoImpl;
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
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
