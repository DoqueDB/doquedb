// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Argument.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_ARGUMENT_H
#define __SYDNEY_PLAN_RELATION_ARGUMENT_H

#include "Plan/Relation/Module.h"
#include "Plan/Utility/ObjectSet.h"
#include "Plan/Declaration.h"

#include "Opt/Algorithm.h"
#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

////////////////////////////////////
//	STRUCT
//	Plan::Relation::InquiryArgument -- argument class for Interface::inquiry
//
//	NOTES
struct InquiryArgument
{
	struct Target
	{
		typedef unsigned int Value;
		enum _Value
		{
			None		= 0,
			Distinct	= 1,
			Depending	= 1 << 1,
			Refering	= 1 << 2,
			Distributed = 1 << 3,
			ValueNum
		};
	};

	InquiryArgument(Target::Value iTarget_)
		: m_iTarget(iTarget_),
		  m_pKey(0),
		  m_pRelation(0)
	{}
	InquiryArgument(Target::Value iTarget_,
					Utility::RowElementSet* pKey_)
		: m_iTarget(iTarget_),
		  m_pKey(pKey_),
		  m_pRelation(0)
	{}
	InquiryArgument(Target::Value iTarget_,
					Interface::IRelation* pRelation_)
		: m_iTarget(iTarget_),
		  m_pKey(0),
		  m_pRelation(pRelation_)
	{}
	InquiryArgument(const InquiryArgument& cOther_)
		: m_iTarget(cOther_.m_iTarget),
		  m_pKey(cOther_.m_pKey),
		  m_pRelation(cOther_.m_pRelation)
	{}

	Target::Value m_iTarget;
	Utility::RowElementSet* m_pKey;
	Interface::IRelation* m_pRelation;
};

///////////////////////////
// shortcut

struct Inquiry
{
	static bool isDistinct(Opt::Environment& cEnvironment_,
						   Interface::IRelation* pRelation_,
						   Utility::RowElementSet* pKey_);
	static bool isDepending(Opt::Environment& cEnvironment_,
							Interface::IRelation* pRelation0_,
							Interface::IRelation* pRelation1_);
	static bool isRefering(Opt::Environment& cEnvironment_,
						   Interface::IRelation* pRelation0_,
						   Interface::IRelation* pRelation1_);
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_ARGUMENT_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
