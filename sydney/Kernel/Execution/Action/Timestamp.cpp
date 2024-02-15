// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/Timestamp.cpp --
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

#include "Execution/Action/Class.h"
#include "Execution/Action/Collection.h"
#include "Execution/Action/Timestamp.h"
#include "Execution/Interface/IProgram.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
// Execution::Action::Timestamp

// FUNCTION public
//	Action::Timestamp::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Timestamp*
//
// EXCEPTIONS

//static
Timestamp*
Timestamp::
create(Interface::IProgram& cProgram_)
{
	This* pResult = cProgram_.getTimestamp();
	if (pResult == 0) {
		AUTOPOINTER<This> pTimestamp = new Timestamp;
		cProgram_.registerTimestamp(pResult = pTimestamp.release());
	}
	return pResult;
}

// FUNCTION public
//	Action::Timestamp::assign -- assign value
//
// NOTES
//
// ARGUMENTS
//	Common::Data* pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Timestamp::
assign(Common::Data* pData_)
{
	if (m_bSet == false) {
		m_cData.setCurrent();
		m_bSet = true;
	}
	pData_->assign(&m_cData);
}

// FUNCTION public
//	Action::Timestamp::reset -- reset
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Timestamp::
reset()
{
	m_bSet = false;
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
