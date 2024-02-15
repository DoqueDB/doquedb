// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/NeighborIn.cpp --
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Predicate";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/NeighborIn.h"
#include "Plan/Predicate/Impl/NeighborInImpl.h"
#include "Plan/Predicate/Combinator.h"
#include "Plan/Interface/IRelation.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	Plan::Predicate::NeighborIn::ValueList

// FUNCTION public
//	Predicate::NeighborIn::ValueList::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IScalar*>& vecScalar_
//	Interface::IRelation* pOperand_
//	Interface::IScalar* pOption_
//	const Utility::RelationSet& cOuterRelation_
//	bool bIsNot_
//	
// RETURN
//	NeighborIn*
//
// EXCEPTIONS

//static
NeighborIn*
NeighborIn::ValueList::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IScalar*>& vecScalar_,
	   Interface::IRelation* pOperand_,
	   Interface::IScalar* pOption_)
{
	AUTOPOINTER<This> pResult;
	switch (vecScalar_.GETSIZE()) {
	case 0:
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	case 1:
		{
			Interface::IRelation::Size nCardinality = pOperand_->getCardinality(cEnvironment_);
			VECTOR<Interface::IScalar*> vecOperand;
			vecOperand.reserve(nCardinality + 1);
			vecOperand.PUSHBACK(vecScalar_[0]);
			for (int i = 0; i < nCardinality; ++i) {
				VECTOR<Interface::IScalar*> vecTmp;
				if (pOperand_->getRow(cEnvironment_, i, vecTmp) == false
					|| vecTmp.GETSIZE() != 1) {
					// this situation should be checked before
					_SYDNEY_THROW0(Exception::Unexpected);
				}
				vecOperand.PUSHBACK(vecTmp[0]);
			}
			if (pOption_) {
				pResult = new NeighborInImpl::NadicWithOption(vecOperand,
															  pOption_);
			} else {
				pResult = new NeighborInImpl::Nadic(vecOperand);
			}
			break;
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

////////////////////////////////////
//	Plan::Predicate::NeighborIn

// FUNCTION protected
//	Predicate::NeighborIn::NeighborIn -- constructor
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

NeighborIn::
NeighborIn()
	: Super(Tree::Node::NeighborIn)
{}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
