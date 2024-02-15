// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/LengthImpl.cpp --
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
const char moduleName[] = "Plan::Scalar::Impl";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Impl/LengthImpl.h"
#include "Plan/Interface/IRelation.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataOperation.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////////
//	Scalar::LengthImpl::Length

// FUNCTION public
//	Scalar::convertFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	Interface::IScalar* pFunction_
//	Schema::Field::Function::Value eFunction_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
LengthImpl::Monadic::
convertFunction(Opt::Environment& cEnvironment_,
				Interface::IRelation* pRelation_,
				Interface::IScalar* pFunction_,
				Schema::Field::Function::Value eFunction_)
{
	// This method should be called only when this function is operand of AVG
	if (eFunction_ == Schema::Field::Function::Undefined) {
		Schema::Field::Function::Value eFunction = eFunction_;
		if (pFunction_->getType() == Tree::Node::Avg) {
			switch (getType()) {
			case Tree::Node::CharLength:
				{
					eFunction = Schema::Field::Function::AverageCharLength;
					break;
				}
			case Tree::Node::WordCount:
				{
					eFunction = Schema::Field::Function::AverageWordCount;
					break;
				}
			case Tree::Node::FullTextLength:
				{
					eFunction = Schema::Field::Function::AverageLength;
					break;
				}
			default:
				{
					return 0;
				}
			}
		}
		return getOperand()->convertFunction(cEnvironment_,
											 pRelation_,
											 pFunction_,
											 eFunction);
	}
	return 0;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
