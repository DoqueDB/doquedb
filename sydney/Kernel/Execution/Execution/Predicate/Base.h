// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Predicate/Base.h --
// 
// Copyright (c) 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_PREDICATE_BASE_H
#define __SYDNEY_EXECUTION_PREDICATE_BASE_H

#include "Execution/Predicate/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Interface/IPredicate.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PREDICATE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Predicate::Base -- base class of predicate subclasses
//
//	NOTES
//		This class is not constructed directly
class Base
	: public Interface::IPredicate
{
public:
	typedef Interface::IPredicate Super;
	typedef Base This;

	// destructor
	virtual ~Base() {}

/////////////////////////////
// Interface::IPredicate::
	virtual Boolean::Value check(Interface::IProgram& cProgram_) {return m_eValue;}

/////////////////////////////
// Interface::IAction::
//	virtual void explain(Opt::Environment* pEnvironment_,
//						 Interface::IProgram& cProgram_,
//						 Opt::Explain& cExplain_);
//	virtual void initialize(Interface::IProgram& cProgram_);
//	virtual void terminate(Interface::IProgram& cProgram_);
	virtual Action::Status::Value
				execute(Interface::IProgram& cProgram_,
						Action::ActionList& cActionList_)
	{
		if (m_eValue != Boolean::NeverTrue
			&& isDone() == false) {
			m_eValue = evaluate(cProgram_, cActionList_);
			done();
		}
		return (m_eValue == Boolean::True) ? Action::Status::Success : Action::Status::False;
	}

//	virtual void finish(Interface::IProgram& cProgram_);
//	virtual void reset(Interface::IProgram& cProgram_);

protected:
	// constructor
	Base() : Super(), m_eValue(Boolean::Unknown) {}

	// base implementation
	void initializeBase(Interface::IProgram& cProgram_) {}
	void terminateBase(Interface::IProgram& cProgram_) {m_eValue = Boolean::Unknown;}

private:
	// evaluate main
	virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
									Action::ActionList& cActionList_) = 0;

	Boolean::Value m_eValue;
};

_SYDNEY_EXECUTION_PREDICATE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_PREDICATE_BASE_H

//
//	Copyright (c) 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
