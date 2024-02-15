// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Control/Base.h --
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

#ifndef __SYDNEY_EXECUTION_CONTROL_BASE_H
#define __SYDNEY_EXECUTION_CONTROL_BASE_H

#include "Execution/Control/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Interface/IControl.h"

#include "ModArchive.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_CONTROL_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Control::Base -- base class of control subclasses
//
//	NOTES
//		This class is not constructed directly
class Base
	: public Interface::IControl
{
public:
	typedef Interface::IControl Super;
	typedef Base This;

	// destructor
	virtual ~Base() {}

/////////////////////////////
// Interface::IControl::
	virtual void setEnd(int iPosition_) {m_iEnd = iPosition_;}
	virtual void setElse(int iPosition_) {m_iElse = iPosition_;}

/////////////////////////////
// Interface::IAction::
//	virtual void explain(Interface::IProgram& cProgram_,
//						 Opt::Explain& cExplain_);
	virtual void initialize(Interface::IProgram& cProgram_) {initializeBase(cProgram_);}
	virtual void terminate(Interface::IProgram& cProgram_) {terminateBase(cProgram_);}
//	virtual Action::Status::Value
//				execute(Interface::IProgram& cProgram_,
//						Action::ActionList& cActionList_);
	virtual void finish(Interface::IProgram& cProgram_) {}
	virtual void reset(Interface::IProgram& cProgram_) {}

protected:
	// constructor
	Base() : Super(), m_iEnd(-1), m_iElse(-1) {}

	// base implementation
	void initializeBase(Interface::IProgram& cProgram_) {}
	void terminateBase(Interface::IProgram& cProgram_) {}
	void serializeBase(ModArchive& archiver_) {archiver_(m_iEnd);archiver_(m_iElse);}

	// accessor
	int getEnd() {return m_iEnd;}
	int getElse() {return (m_iElse < 0) ? m_iEnd : m_iElse;}

private:
	int m_iEnd;
	int m_iElse;
};

_SYDNEY_EXECUTION_CONTROL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_CONTROL_BASE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
