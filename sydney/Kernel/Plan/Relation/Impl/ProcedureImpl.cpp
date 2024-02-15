// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Impl/ProcedureImpl.cpp --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#include "Plan/Relation/Impl/ProcedureImpl.h"
#include "Plan/Relation/RowInfo.h"

#include "Plan/Candidate/Procedure.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Assert.h"

#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

/////////////////////////////////////////////////
// Relation::ProcedureImpl::Base

// FUNCTION public
//	Relation::ProcedureImpl::Base::inquiry -- inquiry about relation's attributes
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
ProcedureImpl::Base::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	return 0;
}

// FUNCTION public
//	Relation::ProcedureImpl::Base::require -- 
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
ProcedureImpl::Base::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	; // do nothing
}

// FUNCTION public
//	Relation::ProcedureImpl::Base::getUsedTable -- 
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
ProcedureImpl::Base::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	// nothing
	;
}

// FUNCTION private
//	Relation::ProcedureImpl::Base::createRowInfo -- set result row spec
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
ProcedureImpl::Base::
createRowInfo(Opt::Environment& cEnvironment_)
{
	// create result by degree of procedure
	return RowInfo::create(cEnvironment_, this, 0, getDegree(cEnvironment_));
}

// FUNCTION private
//	Relation::ProcedureImpl::Base::createKeyInfo -- 
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
ProcedureImpl::Base::
createKeyInfo(Opt::Environment& cEnvironment_)
{
	// no keys
	return RowInfo::create(cEnvironment_);
}

// FUNCTION public
//	Relation::ProcedureImpl::Base::getCardinality -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::IRelation::Size
//
// EXCEPTIONS

//virtual
Interface::IRelation::Size
ProcedureImpl::Base::
getCardinality(Opt::Environment& cEnvironment_)
{
	return 1;
}

// FUNCTION public
//	Relation::ProcedureImpl::Base::getRow -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	int iPosition_
//	VECTOR<Interface::IScalar*>& vecResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ProcedureImpl::Base::
getRow(Opt::Environment& cEnvironment_,
	   int iPosition_,
	   VECTOR<Interface::IScalar*>& vecResult_)
{
	if (static_cast<SIZE>(iPosition_) >= 1) {
		return false;
	}
	vecResult_.clear();
	vecResult_.insert(vecResult_.end(),
					  m_vecParams.begin(), m_vecParams.end());
	return true;
}

// FUNCTION private
//	Relation::ProcedureImpl::Base::setDegree -- set degree of the relation
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
ProcedureImpl::Base::
setDegree(Opt::Environment& cEnvironment_)
{
	return m_vecParams.GETSIZE();
}

// FUNCTION public
//	Relation::ProcedureImpl::Base::setMaxPosition -- set max position of the relation
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
ProcedureImpl::Base::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	return getDegree(cEnvironment_);
}

// FUNCTION private
//	Relation::ProcedureImpl::Base::createScalarName -- set scalar names
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
ProcedureImpl::Base::
createScalarName(Opt::Environment& cEnvironment_,
				 VECTOR<STRING>& vecName_,
				 Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < getDegree(cEnvironment_));

	Opt::ExpandContainer(vecName_,
						 getDegree(cEnvironment_),
						 STRING());
	; _SYDNEY_ASSERT(static_cast<int>(vecName_.GETSIZE()) >= getDegree(cEnvironment_));

	vecName_[iPosition_] = m_vecParams[iPosition_]->getName();
}

// FUNCTION private
//	Relation::ProcedureImpl::Base::createScalar -- set scalar interface
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
ProcedureImpl::Base::
createScalar(Opt::Environment& cEnvironment_,
			 VECTOR<Interface::IScalar*>& vecScalar_,
			 Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < getDegree(cEnvironment_));

	// set all elements here
	vecScalar_.erase(vecScalar_.begin(), vecScalar_.end());
	vecScalar_.insert(vecScalar_.end(),
					  m_vecParams.begin(), m_vecParams.end());
}

// FUNCTION private
//	Relation::ProcedureImpl::Base::createScalarType -- set scalar node type
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
ProcedureImpl::Base::
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
//	Relation::ProcedureImpl::Base::setRetrieved -- set retrieved flag
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
ProcedureImpl::Base::
setRetrieved(Opt::Environment& cEnvironment_,
			 Position iPosition_)
{
	; // do nothing
}

// FUNCTION private
//	Relation::ProcedureImpl::Base::addAggregation -- add aggregation
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
ProcedureImpl::Base::
addAggregation(Opt::Environment& cEnvironment_,
			   Interface::IScalar* pScalar_,
			   Interface::IScalar* pOperand_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

/////////////////////////////////////////////////
// Relation::ProcedureImpl::Function

// FUNCTION public
//	Relation::ProcedureImpl::Function::createAccessPlan -- create access plan candidate
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
ProcedureImpl::Function::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	return Candidate::Procedure::create(cEnvironment_, this);
}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
