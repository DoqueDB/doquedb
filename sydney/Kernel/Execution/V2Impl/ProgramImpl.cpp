// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// V2Impl/ProgramImpl.cpp -- Execution program(v2)
// 
// Copyright (c) 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::V2Impl";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/V2Impl/ProgramImpl.h"
#include "Execution/Interface/IProgram.h"

#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_V2IMPL_BEGIN

///////////////////////////////////////////////////
// Execution::V2Impl::ProgramImpl::V2Interface

// FUNCTION public
//	V2Impl::ProgramImpl::V2Interface::~V2Interface -- destructor
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

ProgramImpl::V2Interface::
~V2Interface()
{
	try { delete m_pProgram, m_pProgram = 0; } catch (...) {}
}

// FUNCTION public
//	V2Impl::ProgramImpl::V2Interface::getProgram -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IProgram*
//
// EXCEPTIONS

Interface::IProgram*
ProgramImpl::V2Interface::
getProgram()
{
	if (m_pProgram == 0) {
		m_pProgram = Interface::IProgram::create();
	}
	return m_pProgram;
}

// FUNCTION public
//	V2Impl::ProgramImpl::V2Interface::setProgram -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram* pProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ProgramImpl::V2Interface::
setProgram(Interface::IProgram* pProgram_)
{
	if (m_pProgram != 0) {
		delete m_pProgram;
	}
	m_pProgram = pProgram_;
}

// FUNCTION public
//	V2Impl::ProgramImpl::V2Interface::releaseProgram -- 
//
// NOTES
//	Return value should be deleted by the caller.
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IProgram*
//
// EXCEPTIONS

Interface::IProgram*
ProgramImpl::V2Interface::
releaseProgram()
{
	Interface::IProgram* pResult = m_pProgram;
	m_pProgram = 0;
	return pResult;
}

//////////////////////////////////////
// Execution::V2Impl::ProgramImpl

// FUNCTION public
//	V2Impl::ProgramImpl::getV1Interface -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IV1Program*
//
// EXCEPTIONS

//virtual
Interface::IV1Program*
ProgramImpl::
getV1Interface()
{
	return 0;
}

_SYDNEY_EXECUTION_V2IMPL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
//	Copyright (c) 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
