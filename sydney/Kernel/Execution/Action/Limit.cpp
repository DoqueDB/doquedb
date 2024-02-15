// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/Limit.cpp --
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
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Action/Limit.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
// Execution::Action::Limit

// FUNCTION public
//	Action::Limit::explain -- explain
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Limit::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	m_cLimit.explain(cProgram_,
					 cExplain_);
	if (m_cOffset.isValid()) {
		cExplain_.put(" offset ");
		m_cOffset.explain(cProgram_,
						  cExplain_);
	}
}

// FUNCTION public
//	Action::Limit::initialize -- initialize necessary members
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
Limit::
initialize(Interface::IProgram& cProgram_)
{
	m_cLimit.initialize(cProgram_);
	if (m_cOffset.isValid()) {
		m_cOffset.initialize(cProgram_);
	}
}

// FUNCTION public
//	Action::Limit::terminate -- end using members
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
Limit::
terminate(Interface::IProgram& cProgram_)
{
	m_cLimit.terminate(cProgram_);
	if (m_cOffset.isValid()) {
		m_cOffset.terminate(cProgram_);
	}
	m_iLimit = m_iOffset = -1;
}

// FUNCTION public
//	Action::Limit::setValues -- calculate limit value
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
Limit::
setValues(Interface::IProgram& cProgram_)
{
	if (m_iLimit <= 0) {
		m_iLimit = m_cLimit->getInt();
		if (m_iLimit <= 0) {
			// invalid limit value
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		if (m_cOffset.isValid()) {
			m_iOffset = m_cOffset->getInt();
			if (m_iOffset <= 0) {
				// invalid offset value
				_SYDNEY_THROW0(Exception::NotSupported);
			}
		} else {
			m_iOffset = 1;
		}

		m_iLimit += m_iOffset - 1;
		if (m_bIntermediate) m_iOffset = 1;
	}
}

// FUNCTION public
//	Action::Limit::serialize -- serialize this class
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
Limit::
serialize(ModArchive& archiver_)
{
	m_cLimit.serialize(archiver_);
	m_cOffset.serialize(archiver_);
	archiver_(m_bIntermediate);
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
