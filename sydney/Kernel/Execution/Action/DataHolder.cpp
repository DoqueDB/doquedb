// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/DataHolder.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

///////////////////////////////////////
// Execution::Action::DataHolderBase

// FUNCTION public
//	Action::DataHolderBase::explain -- explain
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
DataHolderBase::
explain(Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_) const
{
	cExplain_.pushNoNewLine();
	cExplain_.put("data:");
	cProgram_.explainVariable(cExplain_, m_iDataID);
	cExplain_.popNoNewLine();
}

// FUNCTION protected
//	Action::DataHolderBase::initializeData -- initialize necessary members
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Common::Data*
//
// EXCEPTIONS

Common::Data*
DataHolderBase::
initializeData(Interface::IProgram& cProgram_)
{
	if (m_iDataID >= 0) {
		return cProgram_.getVariable(m_iDataID).get();
	}
	return 0;
}

// FUNCTION protected
//	Action::DataHolderBase::terminateData -- end using members
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
DataHolderBase::
terminateData(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Action::DataHolderBase::serialize -- 
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
DataHolderBase::
serialize(ModArchive& archiver_)
{
	archiver_(m_iDataID);
}

///////////////////////////////////////////
// Execution::Action::ArrayDataHolder

//virtual
const Common::DataArrayData*
ArrayDataHolder::
unfold() const
{
	Common::DataArrayData* pData = const_cast<This*>(this)->get();
	int n = pData->getCount();
	m_cUnfoldArray.reserve(n);
	for (int i = 0; i < n; ++i) {
		if (pData->getElement(i)->isApplicable(Common::Data::Function::Unfold)) {
			m_cUnfoldArray.setElement(i, pData->getElement(i)->apply(Common::Data::Function::Unfold));
		} else {
			m_cUnfoldArray.setElement(i, pData->getElement(i));
		}
	}
	return &m_cUnfoldArray;
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
