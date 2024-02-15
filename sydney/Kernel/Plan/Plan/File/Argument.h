// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File/Argument.h --
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

#ifndef __SYDNEY_PLAN_FILE_ARGUMENT_H
#define __SYDNEY_PLAN_FILE_ARGUMENT_H

#include "Plan/File/Module.h"
#include "Plan/File/Parameter.h"
#include "Plan/Declaration.h"

#include "Common/IntegerArrayData.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_FILE_BEGIN

// STRUCT
//	Plan::File::CheckArgument -- argument for checking file availability for predicates 
//
// NOTES

struct CheckArgument
{
	Candidate::Table* m_pTable;
	Interface::IFile* m_pFile;
	File::Parameter* m_pParameter;
	enum Estimate {
		MayEstimate,					// can do estimation
		CanSkipEstimate,				// can skip estimation
		SkipEstimate,					// skip estimation anyway
		NeedEstimate,					// do estimation anyway
	} m_eEstimate;
	bool m_bGrouping;

	CheckArgument(Candidate::Table* pTable_,
				  Estimate eEstimate_ = MayEstimate)
		: m_pTable(pTable_),
		  m_pFile(0),
		  m_pParameter(0),
		  m_eEstimate(eEstimate_),
		  m_bGrouping(false)
	{}
	CheckArgument(const CheckArgument& cArgument_)
		: m_pTable(cArgument_.m_pTable),
		  m_pFile(cArgument_.m_pFile),
		  m_pParameter(cArgument_.m_pParameter),
		  m_eEstimate(cArgument_.m_eEstimate),
		  m_bGrouping(cArgument_.m_bGrouping)
	{}

	// can skip estimation?
	bool canSkipEstimate()
	{
		return m_eEstimate == CanSkipEstimate
			|| m_eEstimate == SkipEstimate;
	}

	// set estimation can be skipped
	void skipEstimate()
	{
		if (m_eEstimate == MayEstimate) {
			m_eEstimate = CanSkipEstimate;
		}
	}
	// set estimation must be skipped
	void noEstimate()
	{
		m_eEstimate = SkipEstimate;
	}
	// set estimation must be done
	void needEstimate()
	{
		if (m_eEstimate != SkipEstimate) {
			m_eEstimate = NeedEstimate;
		}
	}
	// erase unused parameter
	void eraseParameter(Opt::Environment& cEnvironment_)
	{
		if (m_pParameter) {
			File::Parameter::erase(cEnvironment_, m_pParameter);
			m_pParameter = 0;
		}
	}
};

// STRUCT
//	Plan::File::CheckOrderArgument -- argument for checking file availability for order
//
// NOTES

struct CheckOrderArgument
	: public CheckArgument
{
	typedef CheckArgument Super;

	bool m_bIsTop;
	Order::Specification* m_pOrder;
	int m_iMaxKeySize;
	Interface::IPredicate* m_pPredicate;

	CheckOrderArgument(Candidate::Table* pTable_)
		: Super(pTable_),
		  m_bIsTop(true),
		  m_pOrder(0),
		  m_iMaxKeySize(0),
		  m_pPredicate(0)
	{}
	CheckOrderArgument(const CheckOrderArgument& cArgument_)
		: Super(cArgument_),
		  m_bIsTop(cArgument_.m_bIsTop),
		  m_pOrder(0),
		  m_iMaxKeySize(cArgument_.m_iMaxKeySize),
		  m_pPredicate(cArgument_.m_pPredicate)
	{}
};

_SYDNEY_PLAN_FILE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_FILE_ARGUMENT_H

//
//	Copyright (c) 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
