// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/BulkImpl.cpp --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"

#include "Plan/Relation/Impl/BulkImpl.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Bulk.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Relation/Argument.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/Value.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

//////////////////////////////////////
// Relation::BulkImpl::Input::

// FUNCTION public
//	Relation::BulkImpl::Input::createAccessPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
BulkImpl::Input::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	Interface::ICandidate* pResult =
		Candidate::Bulk::Input::create(cEnvironment_,
									   this);
	pResult->checkPredicate(cEnvironment_,
							cPlanSource_);
	return pResult;
}

// FUNCTION public
//	Relation::BulkImpl::Input::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Bulk::InquiryResult
//
// EXCEPTIONS

//virtual
Bulk::InquiryResult
BulkImpl::Input::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	return 0;
}

// FUNCTION public
//	Relation::BulkImpl::Input::require -- 
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
BulkImpl::Input::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	; // do nothing
}

// FUNCTION public
//	Relation::BulkImpl::Input::getUsedTable -- 
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
BulkImpl::Input::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	; // do nothing
}

// FUNCTION private
//	Relation::BulkImpl::Input::createRowInfo -- 
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
BulkImpl::Input::
createRowInfo(Opt::Environment& cEnvironment_)
{
	// rowinfo is set by analyzer
	return 0;
}

// FUNCTION private
//	Relation::BulkImpl::Input::createKeyInfo -- 
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
BulkImpl::Input::
createKeyInfo(Opt::Environment& cEnvironment_)
{
	return RowInfo::create(cEnvironment_);
}

// FUNCTION private
//	Relation::BulkImpl::Input::setDegree -- 
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
BulkImpl::Input::
setDegree(Opt::Environment& cEnvironment_)
{
	RowInfo* pRowInfo = getRowInfo(cEnvironment_);
	return pRowInfo ? pRowInfo->getSize() : -1;
}

// FUNCTION private
//	Relation::BulkImpl::Input::setMaxPosition -- 
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
BulkImpl::Input::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	RowInfo* pRowInfo = getRowInfo(cEnvironment_);
	return pRowInfo ? pRowInfo->getSize() : -1;
}

// FUNCTION private
//	Relation::BulkImpl::Input::createScalarName -- 
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
BulkImpl::Input::
createScalarName(Opt::Environment& cEnvironment_,
				 VECTOR<STRING>& vecName_,
				 Position iPosition_)
{
	; _SYDNEY_ASSERT(getRowInfo(cEnvironment_) == 0
					 || iPosition_ < getDegree(cEnvironment_));

	int iDegree = (getRowInfo(cEnvironment_) == 0) ? iPosition_ + 1 : getDegree(cEnvironment_);
	Opt::ExpandContainer(vecName_,
						 iDegree,
						 STRING());
	; _SYDNEY_ASSERT(static_cast<int>(vecName_.GETSIZE()) >= iDegree);

	OSTRSTREAM cTmp;
	cTmp << "b@" << iPosition_;
	vecName_[iPosition_] = cTmp.getString();
}

// FUNCTION private
//	Relation::BulkImpl::Input::createScalar -- 
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
BulkImpl::Input::
createScalar(Opt::Environment& cEnvironment_,
			 VECTOR<Interface::IScalar*>& vecScalar_,
			 Position iPosition_)
{
	; _SYDNEY_ASSERT(getRowInfo(cEnvironment_) == 0
					 || iPosition_ < getDegree(cEnvironment_));

	int iDegree = (getRowInfo(cEnvironment_) == 0) ? iPosition_ + 1 : getDegree(cEnvironment_);

	// prepare vector
	Opt::ExpandContainer(vecScalar_,
						 iDegree,
						 static_cast<Interface::IScalar*>(0));
	; _SYDNEY_ASSERT(static_cast<int>(vecScalar_.GETSIZE()) >= iDegree);

	// create all variables here
	vecScalar_[iPosition_] = Scalar::Value::BulkData::create(cEnvironment_,
															 getScalarName(cEnvironment_, iPosition_));
}

// FUNCTION private
//	Relation::BulkImpl::Input::createScalarType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Tree::Node::Type>& vecType_
//	Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
BulkImpl::Input::
createScalarType(Opt::Environment& cEnvironment_,
				 VECTOR<Tree::Node::Type>& vecType_,
				 Position iPosition_)
{
	; _SYDNEY_ASSERT(getRowInfo(cEnvironment_) == 0
					 || iPosition_ < getDegree(cEnvironment_));

	int iDegree = (getRowInfo(cEnvironment_) == 0) ? iPosition_ + 1 : getDegree(cEnvironment_);

	// set all the columns by same type
	Opt::ExpandContainer(vecType_, iDegree, Tree::Node::Variable);
	; _SYDNEY_ASSERT(static_cast<int>(vecType_.GETSIZE()) >= iDegree);
}

// FUNCTION private
//	Relation::BulkImpl::Input::setRetrieved -- 
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
BulkImpl::Input::
setRetrieved(Opt::Environment& cEnvironment_,
			 Position iPosition_)
{
	; // do nothing
}

// FUNCTION private
//	Relation::BulkImpl::Input::addAggregation -- 
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
BulkImpl::Input::
addAggregation(Opt::Environment& cEnvironment_,
			   Interface::IScalar* pScalar_,
			   Interface::IScalar* pOperand_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//////////////////////////////////////
// Relation::BulkImpl::Output::

// FUNCTION public
//	Relation::BulkImpl::Output::createAccessPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
BulkImpl::Output::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	getOperand()->getRowInfo(cEnvironment_)->retrieveAll(cEnvironment_);

	Interface::ICandidate* pOperandCandidate =
		getOperand()->createAccessPlan(cEnvironment_, cPlanSource_);

	return Candidate::Bulk::Output::create(cEnvironment_,
										   this,
										   getOperand()->getRowInfo(cEnvironment_),
										   pOperandCandidate);
}

// FUNCTION private
//	Relation::BulkImpl::Output::createRowInfo -- 
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
BulkImpl::Output::
createRowInfo(Opt::Environment& cEnvironment_)
{
	return RowInfo::create(cEnvironment_);
}

// FUNCTION private
//	Relation::BulkImpl::Output::createKeyInfo -- 
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
BulkImpl::Output::
createKeyInfo(Opt::Environment& cEnvironment_)
{
	return getOperand()->getKeyInfo(cEnvironment_);
}

// FUNCTION private
//	Relation::BulkImpl::Output::setDegree -- 
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
BulkImpl::Output::
setDegree(Opt::Environment& cEnvironment_)
{
	return 0;
}

// FUNCTION private
//	Relation::BulkImpl::Output::setMaxPosition -- 
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
BulkImpl::Output::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	return 0;
}

// FUNCTION private
//	Relation::BulkImpl::Output::createScalarName -- 
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
BulkImpl::Output::
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
	cTmp << "b@" << iPosition_;
	vecName_[iPosition_] = cTmp.getString();
}

// FUNCTION private
//	Relation::BulkImpl::Output::createScalar -- 
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
BulkImpl::Output::
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
											  getScalarName(cEnvironment_, i));
	}
}

// FUNCTION private
//	Relation::BulkImpl::Output::createScalarType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Tree::Node::Type>& vecType_
//	Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
BulkImpl::Output::
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

//
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
