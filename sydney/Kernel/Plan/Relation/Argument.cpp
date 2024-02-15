// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Argument.cpp --
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
const char moduleName[] = "Plan::Relation";
}

#include "SyDefault.h"

#include "Plan/Relation/Argument.h"
#include "Plan/Interface/IRelation.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

// FUNCTION public
//	Relation::Inquiry::isDistinct -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	Utility::RowElementSet* pKey_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Inquiry::
isDistinct(Opt::Environment& cEnvironment_,
		   Interface::IRelation* pRelation_,
		   Utility::RowElementSet* pKey_)
{
	return pRelation_->inquiry(cEnvironment_,
							   InquiryArgument(InquiryArgument::Target::Distinct,
											   pKey_))
		== InquiryArgument::Target::Distinct;
}

// FUNCTION public
//	Relation::Inquiry::isDepending -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation0_
//	Interface::IRelation* pRelation1_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Inquiry::
isDepending(Opt::Environment& cEnvironment_,
			Interface::IRelation* pRelation0_,
			Interface::IRelation* pRelation1_)
{
	return pRelation0_->inquiry(cEnvironment_,
								InquiryArgument(InquiryArgument::Target::Depending,
												pRelation1_))
		== InquiryArgument::Target::Depending;
}

// FUNCTION public
//	Relation::Inquiry::isRefering -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation0_
//	Interface::IRelation* pRelation1_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Inquiry::
isRefering(Opt::Environment& cEnvironment_,
			Interface::IRelation* pRelation0_,
			Interface::IRelation* pRelation1_)
{
	return pRelation0_->inquiry(cEnvironment_,
								InquiryArgument(InquiryArgument::Target::Refering,
												pRelation1_))
		== InquiryArgument::Target::Refering;
}

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
