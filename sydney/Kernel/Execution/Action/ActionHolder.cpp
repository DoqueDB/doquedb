// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/ActionHolder.cpp --
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

#include "Execution/Action/ActionHolder.h"
#include "Execution/Interface/IAction.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Explain.h"

#include "ModArchive.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

///////////////////////////////////////
// Execution::Action::ActionHolder::

// FUNCTION public
//	Action::ActionHolder::explain -- 
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
ActionHolder::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	if (m_pAction != 0) {
		m_pAction->explain(pEnvironment_,
						   cProgram_,
						   cExplain_);
	} else {
		cProgram_.getAction(m_iID)->explain(pEnvironment_,
											cProgram_,
											cExplain_);
	}
}

// FUNCTION public
//	Action::ActionHolder::initialize -- 
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
ActionHolder::
initialize(Interface::IProgram& cProgram_)
{
	if (m_pAction == 0) {
		m_pAction = cProgram_.getAction(m_iID);
		m_pAction->initialize(cProgram_);
	}
}

// FUNCTION public
//	Action::ActionHolder::terminate -- 
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
ActionHolder::
terminate(Interface::IProgram& cProgram_)
{
	if (m_pAction != 0) {
		m_pAction->terminate(cProgram_);
		m_pAction = 0;
	}
}

// FUNCTION public
//	Action::ActionHolder::serialize -- for serialize
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
ActionHolder::
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
