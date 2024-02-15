// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Impl/ValueListImpl.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Plan/Relation/Impl/ValueListImpl.h"
#include "Plan/Relation/RowInfo.h"

#include "Plan/Candidate/ValueList.h"
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
// Relation::ValueListImpl::Base

// FUNCTION public
//	Relation::ValueListImpl::Base::inquiry -- inquiry about relation's attributes
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
ValueListImpl::Base::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	return 0;
}

// FUNCTION public
//	Relation::ValueListImpl::Base::require -- 
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
ValueListImpl::Base::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	; // do nothing
}

// FUNCTION private
//	Relation::ValueListImpl::Base::createRowInfo -- set result row spec
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
ValueListImpl::Base::
createRowInfo(Opt::Environment& cEnvironment_)
{
	// create result by degree of valueList
	return RowInfo::create(cEnvironment_, this, 0, getDegree(cEnvironment_));
}

// FUNCTION private
//	Relation::ValueListImpl::Base::createKeyInfo -- 
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
ValueListImpl::Base::
createKeyInfo(Opt::Environment& cEnvironment_)
{
	// no keys
	return RowInfo::create(cEnvironment_);
}

// FUNCTION private
//	Relation::ValueListImpl::Base::createScalarName -- set scalar names
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
ValueListImpl::Base::
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
//	Relation::ValueListImpl::Base::createScalar -- set scalar interface
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
ValueListImpl::Base::
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

	// create all variables here
	for (int i = 0; i < iMax; ++i) {
		vecScalar_[i] = Scalar::Value::create(cEnvironment_,
											  this,
											  getScalarName(cEnvironment_, i));
	}
}

// FUNCTION private
//	Relation::ValueListImpl::Base::createScalarType -- set scalar node type
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
ValueListImpl::Base::
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
//	Relation::ValueListImpl::Base::setRetrieved -- set retrieved flag
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
ValueListImpl::Base::
setRetrieved(Opt::Environment& cEnvironment_,
			 Position iPosition_)
{
	; // do nothing
}

// FUNCTION private
//	Relation::ValueListImpl::Base::addAggregation -- add aggregation
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
ValueListImpl::Base::
addAggregation(Opt::Environment& cEnvironment_,
			   Interface::IScalar* pScalar_,
			   Interface::IScalar* pOperand_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

/////////////////////////////////////////////////
// Relation::ValueListImpl::SingleColumn

// FUNCTION public
//	Relation::ValueListImpl::SingleColumn::getCardinality -- 
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
ValueListImpl::SingleColumn::
getCardinality(Opt::Environment& cEnvironment_)
{
	return m_vecValue.GETSIZE();
}

// FUNCTION public
//	Relation::ValueListImpl::SingleColumn::getRow -- 
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
ValueListImpl::SingleColumn::
getRow(Opt::Environment& cEnvironment_,
	   int iPosition_,
	   VECTOR<Interface::IScalar*>& vecResult_)
{
	if (static_cast<SIZE>(iPosition_) >= m_vecValue.GETSIZE()) {
		return false;
	}
	vecResult_.clear();
	vecResult_.PUSHBACK(m_vecValue[iPosition_]);
	return true;
}

// FUNCTION public
//	Relation::ValueListImpl::SingleColumn::createAccessPlan -- create access plan candidate
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
ValueListImpl::SingleColumn::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	Interface::ICandidate* pResult =
		Candidate::ValueList::create(cEnvironment_, this);
	pResult->checkPredicate(cEnvironment_,
							cPlanSource_);
	return pResult;
}

// FUNCTION public
//	Relation::ValueListImpl::SingleColumn::getUsedTable -- 
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
ValueListImpl::SingleColumn::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	Opt::ForEach(m_vecValue,
				 boost::bind(&Interface::IScalar::getUsedTable,
							 _1,
							 boost::ref(cResult_)));
}

// FUNCTION private
//	Relation::ValueListImpl::SingleColumn::setDegree -- set degree of the relation
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
ValueListImpl::SingleColumn::
setDegree(Opt::Environment& cEnvironment_)
{
	// degree is 1 for singlecolumn
	return 1;
}

// FUNCTION public
//	Relation::ValueListImpl::SingleColumn::setMaxPosition -- set max position of the relation
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
ValueListImpl::SingleColumn::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	return 1;
}

/////////////////////////////////////////////////
// Relation::ValueListImpl::MultiColumn

// FUNCTION public
//	Relation::ValueListImpl::MultiColumn::getCardinality -- 
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
ValueListImpl::MultiColumn::
getCardinality(Opt::Environment& cEnvironment_)
{
	return m_vecvecValue.GETSIZE();
}

// FUNCTION public
//	Relation::ValueListImpl::MultiColumn::getRow -- 
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
ValueListImpl::MultiColumn::
getRow(Opt::Environment& cEnvironment_,
	   int iPosition_,
	   VECTOR<Interface::IScalar*>& vecResult_)
{
	if (static_cast<SIZE>(iPosition_) >= m_vecvecValue.GETSIZE()) {
		return false;
	}
	vecResult_ = m_vecvecValue[iPosition_];
	return true;
}

// FUNCTION public
//	Relation::ValueListImpl::MultiColumn::createAccessPlan -- create access plan candidate
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
ValueListImpl::MultiColumn::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	Interface::ICandidate* pResult =
		Candidate::ValueList::create(cEnvironment_, this);
	pResult->checkPredicate(cEnvironment_,
							cPlanSource_);
	return pResult;
}

// FUNCTION public
//	Relation::ValueListImpl::MultiColumn::getUsedTable -- 
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
ValueListImpl::MultiColumn::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	VECTOR< VECTOR<Interface::IScalar*> >::ITERATOR iterator = m_vecvecValue.begin();
	const VECTOR< VECTOR<Interface::IScalar*> >::ITERATOR last = m_vecvecValue.end();
	for(; iterator != last; ++iterator) {
		Opt::ForEach(*iterator,
					 boost::bind(&Interface::IScalar::getUsedTable,
								 _1,
								 boost::ref(cResult_)));
	}
}

// FUNCTION private
//	Relation::ValueListImpl::MultiColumn::setDegree -- set degree of the relation
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
ValueListImpl::MultiColumn::
setDegree(Opt::Environment& cEnvironment_)
{
	; _SYDNEY_ASSERT(m_vecvecValue.ISEMPTY() == false);
	return m_vecvecValue[0].GETSIZE();
}

// FUNCTION public
//	Relation::ValueListImpl::MultiColumn::setMaxPosition -- set max position of the relation
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
ValueListImpl::MultiColumn::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	; _SYDNEY_ASSERT(m_vecvecValue.ISEMPTY() == false);
	return m_vecvecValue[0].GETSIZE();
}

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
