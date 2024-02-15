// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/Argument.cpp --
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
const char moduleName[] = "Opt";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Opt/Argument.h"

#include "Plan/File/Parameter.h"

_SYDNEY_USING
_SYDNEY_OPT_USING

// FUNCTION public
//	Opt::ExplainFileArgument::setValues -- 
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
ExplainFileArgument::
setValues()
{
	if (m_pParameter) {
		m_bIsSearch = (m_pParameter->getPredicate() != 0);
		m_bIsGetByBitSet = m_pParameter->isGetByBitSet();
		m_bIsSearchByBitSet = m_pParameter->isSearchByBitSet();
		m_bIsLimited = m_pParameter->isLimited();
	}
}

// FUNCTION public
//	Opt::ExplainFileArgument::serialize -- 
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

void
ExplainFileArgument::
serialize(ModArchive& cArchive_)
{
	cArchive_(m_bIsSearch);
	cArchive_(m_bIsGetByBitSet);
	cArchive_(m_bIsSearchByBitSet);
	cArchive_(m_bIsLimited);
	cArchive_(m_bIsSimple);
}

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
