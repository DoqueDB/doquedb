// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Argument.h --
// 
// Copyright (c) 2008, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_ARGUMENT_H
#define __SYDNEY_PLAN_SCALAR_ARGUMENT_H

#include "Plan/Scalar/Module.h"

#include "Plan/Declaration.h"
#include "Plan/Utility/ObjectSet.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

// STRUCT
//	Plan::Scalar::CheckArgument -- argument for IScalar::check method
//
// NOTES

struct CheckArgument
{
	Interface::ICandidate* m_pCandidate;
	VECTOR<Interface::ICandidate*> m_vecPrecedingCandidate;
					  
	CheckArgument(Interface::ICandidate* pCandidate_,
				  const VECTOR<Interface::ICandidate*>& vecPrecedingCandidate_)
		: m_pCandidate(pCandidate_),
		  m_vecPrecedingCandidate(vecPrecedingCandidate_)
	{}
	CheckArgument(const CheckArgument& cArgument_)
		: m_pCandidate(cArgument_.m_pCandidate),
		  m_vecPrecedingCandidate(cArgument_.m_vecPrecedingCandidate)
	{}
};

// STRUCT
//	Plan::Scalar::DelayArgument -- argument for IScalar::delay method
//
// NOTES

struct DelayArgument
{
	Utility::ScalarSet m_cKey;			// keys to get delayed columns
	bool m_bMinimum;					// if true, columns are delayed as much as possible

	DelayArgument(bool bMinimum_ = false)
		: m_cKey(),
		  m_bMinimum(bMinimum_)
	{}
};

// STRUCT
//	Plan::Scalar::GetFileArgument -- argument for Field::getXXXFile method
//
// NOTES

struct GetFileArgument
{
	Interface::IScalar* m_pField;
	Interface::IPredicate* m_pPredicate;
	Utility::FileSet& m_cFile;

	GetFileArgument(Interface::IScalar* pField_,
					Interface::IPredicate* pPredicate_,
					Utility::FileSet& cFile_)
		: m_pField(pField_),
		  m_pPredicate(pPredicate_),
		  m_cFile(cFile_)
	{}
	GetFileArgument(const GetFileArgument& cArgument_)
		: m_pField(cArgument_.m_pField),
		  m_pPredicate(cArgument_.m_pPredicate),
		  m_cFile(cArgument_.m_cFile)
	{}
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_ARGUMENT_H

//
//	Copyright (c) 2008, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
