// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/IteratorHolder.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#include "Execution/Action/IteratorHolder.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Explain.h"

#include "ModArchive.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

///////////////////////////////////////
// Execution::Action::IteratorHolder::

// FUNCTION public
//	Action::IteratorHolder::explain -- 
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
IteratorHolder::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	if (m_pIterator != 0) {
		m_pIterator->explain(pEnvironment_,
							 cProgram_,
							 cExplain_);
	} else {
		cProgram_.getIterator(m_iID)->explain(pEnvironment_,
											  cProgram_,
											  cExplain_);
	}
}

// FUNCTION public
//	Action::IteratorHolder::initialize -- 
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
IteratorHolder::
initialize(Interface::IProgram& cProgram_)
{
	if (m_pIterator == 0) {
		m_pIterator = cProgram_.getIterator(m_iID);
		// iterator::initialize should be called by user of this class
	}
}

// FUNCTION public
//	Action::IteratorHolder::terminate -- 
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
IteratorHolder::
terminate(Interface::IProgram& cProgram_)
{
	if (m_pIterator != 0) {
		cProgram_.terminate(m_pIterator);
		m_pIterator = 0;
	}
}

// FUNCTION public
//	Action::IteratorHolder::serialize -- for serialize
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
IteratorHolder::
serialize(ModArchive& archiver_)
{
	archiver_(m_iID);
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
