// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/OperandElement.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Iterator/OperandElement.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Exception/Unexpected.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

/////////////////////////////////////////////
// Execution::Iterator::OperandElement

// FUNCTION public
//	Iterator::OperandElement::explain -- 
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

void
OperandElement::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	m_cIterator.explain(pEnvironment_, cProgram_, cExplain_);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		if (m_cData.isValid()) {
			cExplain_.pushIndent();
			cExplain_.newLine(true /* force */);
			cExplain_.pushNoNewLine();
			cExplain_.put(" to ");
			m_cData.explain(cProgram_, cExplain_);
			cExplain_.popNoNewLine();
			cExplain_.popIndent();
		}
	}
}

// FUNCTION public
//	Iterator::OperandElement::initialize -- 
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
OperandElement::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cIterator.isInitialized() == false) {
		m_cIterator.initialize(cProgram_);
		m_cData.initialize(cProgram_);
		// initialize iterator here
		cProgram_.initialize(m_cIterator.getIterator());
		m_bHasData = true;
	}
}

// FUNCTION public
//	Iterator::OperandElement::terminate -- 
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
OperandElement::
terminate(Interface::IProgram& cProgram_)
{
	m_cIterator.terminate(cProgram_);
	m_cData.terminate(cProgram_);
}

// FUNCTION public
//	Iterator::OperandElement::startUp -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

Action::Status::Value
OperandElement::
startUp(Interface::IProgram& cProgram_)
{
	return cProgram_.startUp(m_cIterator.getIterator());
}

// FUNCTION public
//	Iterator::OperandElement::finish -- 
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
OperandElement::
finish(Interface::IProgram& cProgram_)
{
	cProgram_.finish(m_cIterator.getIterator());
	m_bHasData = true;
}

// FUNCTION public
//	Iterator::OperandElement::next -- 
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

bool
OperandElement::
next(Interface::IProgram& cProgram_)
{
	// if getnext is specified, call next of operand iterator
	return (m_bHasData = cProgram_.next(m_cIterator.getIterator()));
}

// FUNCTION public
//	Iterator::OperandElement::reset -- 
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
OperandElement::
reset(Interface::IProgram& cProgram_)
{
	if (m_cIterator.isInitialized()) {
		cProgram_.reset(m_cIterator.getIterator());
		m_bHasData = true;
	}
}

// FUNCTION public
//	Iterator::OperandElement::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::DataArrayData*
//
// EXCEPTIONS

const Common::DataArrayData*
OperandElement::
getData() const
{
	if (m_cData.isInitialized() == false) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_cData.getData();
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
