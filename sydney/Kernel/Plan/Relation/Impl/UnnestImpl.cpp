// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Impl/UnnestImpl.cpp --
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
const char moduleName[] = "Plan::Relation";
}

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "Plan/Relation/Impl/UnnestImpl.h"
#include "Plan/Relation/Argument.h"
#include "Plan/Relation/RowInfo.h"

#include "Plan/Candidate/Unnest.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Assert.h"

#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

/////////////////////////////////////////////////
// Relation::Impl::UnnestImpl

// FUNCTION public
//	Relation::UnnestImpl::SingleColumn::createAccessPlan -- create access plan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
Impl::UnnestImpl::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	Interface::ICandidate* pResult =
		Candidate::Unnest::create(cEnvironment_, this);
	pResult->checkPredicate(cEnvironment_,
							cPlanSource_);
	return pResult;
}

// FUNCTION public
//	Relation::Impl::UnnestImpl::inquiry -- inquiry about relation's attributes
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::IRelation::InquiryResult
Impl::UnnestImpl::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	if (cArgument_.m_iTarget & InquiryArgument::Target::Depending) {
		; _SYDNEY_ASSERT(cArgument_.m_pRelation);
		Utility::RelationSet cRelationSet;
		Utility::RelationSet cUsedRelationSet;
		cArgument_.m_pRelation->getUsedTable(cEnvironment_,
											 cRelationSet);

		m_pValue->getUsedTable(cUsedRelationSet);
		if (cRelationSet.isContainingAny(cUsedRelationSet)) {
			return InquiryArgument::Target::Depending;
		}
	}
	return 0;
}

// FUNCTION public
//	Relation::Impl::UnnestImpl::require -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::UnnestImpl::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	m_pValue->require(cEnvironment_,
					  pCandidate_);
}

// FUNCTION public
//	Relation::Impl::UnnestImpl::getUsedTable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::UnnestImpl::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	m_pValue->getUsedTable(cResult_);
}

// FUNCTION private
//	Relation::Impl::UnnestImpl::createRowInfo -- set result row spec
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	RowInfo*
//
// EXCEPTIONS

//virtual
RowInfo*
Impl::UnnestImpl::
createRowInfo(Opt::Environment& cEnvironment_)
{
	// create result by degree of unnest
	return RowInfo::create(cEnvironment_, this, 0, getDegree(cEnvironment_));
}

// FUNCTION private
//	Relation::Impl::UnnestImpl::createKeyInfo -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	RowInfo*
//
// EXCEPTIONS

//virtual
RowInfo*
Impl::UnnestImpl::
createKeyInfo(Opt::Environment& cEnvironment_)
{
	// no keys
	return RowInfo::create(cEnvironment_);
}

// FUNCTION private
//	Relation::Impl::UnnestImpl::createScalarName -- set scalar names
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<STRING>& vecName_
//	Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::UnnestImpl::
createScalarName(Opt::Environment& cEnvironment_,
				 VECTOR<STRING>& vecName_,
				 Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < getDegree(cEnvironment_));

	Opt::ExpandContainer(vecName_,
						 getDegree(cEnvironment_),
						 STRING());
	; _SYDNEY_ASSERT(static_cast<int>(vecName_.GETSIZE()) >= getDegree(cEnvironment_));

	OSTRSTREAM cTmp;
	cTmp << "v@" << iPosition_;
	vecName_[iPosition_] = cTmp.getString();
}

// FUNCTION private
//	Relation::Impl::UnnestImpl::createScalar -- set scalar interface
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Interface::IScalar*>& vecScalar_
//	Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::UnnestImpl::
createScalar(Opt::Environment& cEnvironment_,
			 VECTOR<Interface::IScalar*>& vecScalar_,
			 Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < getDegree(cEnvironment_));

	// prepare vector
	int iMax = getDegree(cEnvironment_);
	Opt::ExpandContainer(vecScalar_,
						 iMax,
						 static_cast<Interface::IScalar*>(0));
	; _SYDNEY_ASSERT(static_cast<int>(vecScalar_.GETSIZE()) >= iMax);

	// create data type
	Scalar::DataType cType = Scalar::DataType::getElementType(m_pValue->getDataType());

	// create all variables here
	for (int i = 0; i < iMax; ++i) {
		vecScalar_[i] = Scalar::Value::create(cEnvironment_,
											  this,
											  cType,
											  getScalarName(cEnvironment_, i));
	}
}

// FUNCTION private
//	Relation::Impl::UnnestImpl::createScalarType -- set scalar node type
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Interface::Type>& vecType_
//	Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::UnnestImpl::
createScalarType(Opt::Environment& cEnvironment_,
				 VECTOR<Tree::Node::Type>& vecType_,
				 Position iPosition_)
{
	// set all the columns by same type
	int iSize = getDegree(cEnvironment_);
	; _SYDNEY_ASSERT(iPosition_ < iSize);

	Opt::ExpandContainer(vecType_, iSize, Tree::Node::Variable);
	; _SYDNEY_ASSERT(static_cast<int>(vecType_.GETSIZE()) >= iSize);
}

// FUNCTION private
//	Relation::Impl::UnnestImpl::setRetrieved -- set retrieved flag
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::UnnestImpl::
setRetrieved(Opt::Environment& cEnvironment_,
			 Position iPosition_)
{
	; // do nothing
}

// FUNCTION private
//	Relation::Impl::UnnestImpl::addAggregation -- add aggregation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pScalar_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::UnnestImpl::
addAggregation(Opt::Environment& cEnvironment_,
			   Interface::IScalar* pScalar_,
			   Interface::IScalar* pOperand_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION private
//	Relation::Impl::UnnestImpl::setDegree -- set degree of the relation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::UnnestImpl::
setDegree(Opt::Environment& cEnvironment_)
{
	// degree is 1 for unnest
	return 1;
}

// FUNCTION public
//	Relation::Impl::UnnestImpl::setMaxPosition -- set max position of the relation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::UnnestImpl::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	return 1;
}

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
