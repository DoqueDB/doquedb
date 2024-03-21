// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Filter.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_FILTER_H
#define __SYDNEY_PLAN_RELATION_FILTER_H

#include "Plan/Interface/IRelation.h"
#include "Plan/Relation/Argument.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Utility/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

////////////////////////////////////
// TEMPLATE CLASS
//	Plan::Relation::Filter -- Interface implementation for filter
//
// TEMPLATE ARGUMENTS
//	class Base_
//		base class for the filter
//
// NOTES
//	This class is not constructed directly
template <class Base_>
class Filter
	: public Tree::Monadic<Base_, Interface::IRelation>
{
public:
	typedef Tree::Monadic<Base_, Interface::IRelation> Super;
	typedef Filter This;

	// destructor
	virtual ~Filter() {}

///////////////////////////////////////
// Interface::IRelation::
	virtual int estimateUnwind(Opt::Environment& cEnvironment_)
	{return getOperand()->estimateUnwind(cEnvironment_);}
	virtual int estimateUnion(Opt::Environment& cEnvironment_)
	{return getOperand()->estimateUnion(cEnvironment_);}
	virtual typename Super::InquiryResult
					inquiry(Opt::Environment& cEnvironment_,
							const InquiryArgument& cArgument_)
	{return getOperand()->inquiry(cEnvironment_, cArgument_);}

	virtual void require(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_)
	{getOperand()->require(cEnvironment_, pCandidate_);}
	virtual void getUsedTable(Opt::Environment& cEnvironment_,
							  Utility::RelationSet& cResult_)
	{getOperand()->getUsedTable(cEnvironment_, cResult_);}

	virtual Sql::Query* generateSQL(Opt::Environment& cEnvironment_)
	{return getOperand()->generateSQL(cEnvironment_);}

	using Super::getOperand;
	using Super::getDegree;
	
protected:
	// costructor
	Filter(Interface::IRelation* pOperand_)
		: Super(pOperand_)
	{}

	template <class A_>
	Filter(A_ arg_, Interface::IRelation* pOperand_)
		: Super(arg_, pOperand_)
	{}
	template <class A1_, class A2_>
	Filter(A1_ arg1_, A2_ arg2_, Interface::IRelation* pOperand_)
		: Super(arg1_, arg2_, pOperand_)
	{}

private:
///////////////////////////////////////
// Interface::IRelation::
	virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_)
	{return getOperand()->getRowInfo(cEnvironment_);}
	virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_)
	{return getOperand()->getKeyInfo(cEnvironment_);}
	virtual int setDegree(Opt::Environment& cEnvironment_)
	{return getOperand()->getDegree(cEnvironment_);}
	virtual int setMaxPosition(Opt::Environment& cEnvironment_)
	{return getOperand()->getMaxPosition(cEnvironment_);}
	virtual void createScalarName(Opt::Environment& cEnvironment_,
								  VECTOR<STRING>& vecName_,
								  Interface::IRelation::Position iPosition_)
	{
		Opt::ExpandContainer(vecName_, getDegree(cEnvironment_));
		vecName_[iPosition_] = getOperand()->getScalarName(cEnvironment_, iPosition_);
	}
	virtual void createScalar(Opt::Environment& cEnvironment_,
							  VECTOR<Interface::IScalar*>& vecScalar_,
							  Interface::IRelation::Position iPosition_)
	{
		Opt::ExpandContainer(vecScalar_, getDegree(cEnvironment_));
		vecScalar_[iPosition_] = getOperand()->getScalar(cEnvironment_, iPosition_);
	}
	virtual void createScalarType(Opt::Environment& cEnvironment_,
								  VECTOR<Tree::Node::Type>& vecType_,
								  Interface::IRelation::Position iPosition_)
	{
		Opt::ExpandContainer(vecType_, getDegree(cEnvironment_), Tree::Node::Undefined);
		vecType_[iPosition_] = getOperand()->getNodeType(cEnvironment_, iPosition_);
	}
	virtual void setRetrieved(Opt::Environment& cEnvironment_,
							 Interface::IRelation::Position iPosition_)
	{
		getOperand()->retrieve(cEnvironment_, iPosition_);
	}
	virtual int addAggregation(Opt::Environment& cEnvironment_,
							   Interface::IScalar* pScalar_,
							   Interface::IScalar* pOperand_)
	{
		return getOperand()->aggregate(cEnvironment_, pScalar_, pOperand_);
	}
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_FILTER_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
