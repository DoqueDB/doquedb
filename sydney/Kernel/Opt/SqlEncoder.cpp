// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Explain.cpp -- Output plan tree explanation
// 
// Copyright (c) 2007, 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Opt";
	const char srcFile[] = __FILE__;
}

#include <stdio.h>

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Opt/SqlEncoder.h"
#include "Common/Assert.h"

#include "Exception/SQLSyntaxError.h"

#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_OPT_USING


// FUNCTION public
//	Opt::Explain::add
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrValue_
//	
// RETURN
//	Explain&
//
// EXCEPTIONS

void
SqlEncoder::
appendString(const ModUnicodeString& cstrValue_)
{
	m_bNode = false;
	m_cStream << cstrValue_;
}


// FUNCTION public
//	Opt::Explain::appendNode
//
// NOTES
//
// ARGUMENTS
//	const Plan::Interface::IScalar* pScalar_
//	
// RETURN
//	void
//
// EXCEPTIONS

void
SqlEncoder::
appendNode(Plan::Interface::IScalar* pScalar_)
{
	if (m_bNode) {
		appendSpace();
		appendDelimiter();
		appendSpace();
	}
	
	appendString(pScalar_->createSQL());
	m_bNode = true;
}

// FUNCTION public
//	Opt::SqlEncoder::appendRow
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrValue_
//	
// RETURN
//	Explain&
//
// EXCEPTIONS
/*
void
SqlEncoder::
appendRow(const Plan::Interface::IRow* pRow_)
{
	appendLeftParenthesis();
	for (int i = 0; i < getSize(); ++i) {
		appendNode(getOperandi(i));
	}
	appendRightParenthesis();
}
*/

//
// Copyright (c) 2007, 2008, 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
