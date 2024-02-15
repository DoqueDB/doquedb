// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/JoinImpl.h --
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

#ifndef __SYDNEY_DPLAN_RELATION_JOINIMPL_H
#define __SYDNEY_DPLAN_RELATION_JOINIMPL_H

#include "boost/bind.hpp"

#include "DPlan/Relation/Join.h"

#include "Plan/Relation/RowInfo.h"

#include "Common/Functional.h"

#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Nadic.h"

#include "Common/Assert.h"
#include "Common/Functional.h"

#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

namespace JoinImpl
{
	//////////////////////////////////
	// TEMPLATE CLASS
	//	Relation::JoinImpl::Base
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	template <class Handle_>
	class Base
		: public Handle_
	{
	public:
		typedef Handle_ Super;
		typedef Base<Handle_> This;
		typedef typename Handle_::Operand Operand;
		typedef typename Handle_::Position Position;

		// destructor
		virtual ~Base() {}


	//////////////////////////////
	// Interface::IRelation::

	protected:
		// constructor
		Base(Plan::Tree::Node::Type eType_,
			 Plan::Interface::IPredicate* pPredicate_,
			 typename Handle_::Argument cArgument_)
			: Handle_(eType_, pPredicate_, cArgument_)
		{}

	private:
	//////////////////////////////
	// Interface::IRelation::
		virtual Plan::Relation::RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
		virtual Plan::Relation::RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
		virtual int setDegree(Opt::Environment& cEnvironment_);
		virtual int setMaxPosition(Opt::Environment& cEnvironment_);
		virtual void createScalarName(Opt::Environment& cEnvironment_,
									  VECTOR<STRING>& vecName_,
									  Position iPosition_);
		virtual void createScalar(Opt::Environment& cEnvironment_,
								  VECTOR<Plan::Interface::IScalar*>& vecScalar_,
								  Position iPosition_);
		virtual void createScalarType(Opt::Environment& cEnvironment_,
									  VECTOR<Plan::Tree::Node::Type>& vecType_,
									  Position iPosition_);
		virtual void setRetrieved(Opt::Environment& cEnvironment_,
								  Position iPosition_);
		virtual int addAggregation(Opt::Environment& cEnvironment_,
								   Plan::Interface::IScalar* pScalar_,
								   Plan::Interface::IScalar* pOperand_);		
	};

	//////////////////////////////////
	// CLASS
	//	Relation::JoinImpl::Dyadic
	//
	// NOTES
	class Dyadic
		: public Base< Plan::Tree::Dyadic<Join, Plan::Interface::IRelation> >
	{
	public:
		typedef Dyadic This;
		typedef Base< Plan::Tree::Dyadic<Join, Plan::Interface::IRelation> > Super;

		// constructor
		Dyadic(Plan::Tree::Node::Type eType_,
			   Plan::Interface::IPredicate* pPredicate_,
			   Super::Argument cOperand_)
			: Super(eType_, pPredicate_, cOperand_)
		{}
		// destructor
		virtual ~Dyadic() {}

	/////////////////////////////
	// Interface::IRelation::

		virtual Plan::Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 Plan::AccessPlan::Source& cPlanSource_);

		virtual Plan::Interface::IRelation::InquiryResult inquiry(Opt::Environment& cEnvironment_,
																  const Plan::Relation::InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Plan::Interface::ICandidate* pCandidate_);

		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Plan::Utility::RelationSet& cResult_);		

		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

		virtual bool isOuter();
		
		virtual bool isExists()
		{ return false; }

		
		
	protected:
	private:
	};

	//////////////////////////////////
	// CLASS
	//	Relation::JoinImpl::Nadic
	//
	// NOTES
	class Nadic
		: public Base< Plan::Tree::Nadic<Join, Plan::Interface::IRelation> >
	{
	public:
		typedef Nadic This;
		typedef Base< Plan::Tree::Nadic<Join, Plan::Interface::IRelation> > Super;

		// constructor
		Nadic(Plan::Tree::Node::Type eType_,
			  Plan::Interface::IPredicate* pPredicate_,
			  Super::Argument vecOperand_)
			: Super(eType_, pPredicate_, vecOperand_)
		{}
		// destructor
		virtual ~Nadic() {}	

	////////////////////////////
	// Interface::IRelation
		virtual Plan::Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 Plan::AccessPlan::Source& cPlanSource_);

		virtual Plan::Interface::IRelation::InquiryResult inquiry(Opt::Environment& cEnvironment_,
																  const Plan::Relation::InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Plan::Interface::ICandidate* pCandidate_);

		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Plan::Utility::RelationSet& cResult_);

		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

		virtual bool isOuter()
		{ return false; }
		
		virtual bool isExists()
		{ return false; }

	protected:
	private:
	};
}

//////////////////////////////////////
// Relation::JoinImpl::Base

// TEMPLATE FUNCTION private
//	Relation::JoinImpl::Base<Handle_>::createRowInfo -- set result row spec
//
// TEMPLATE ARGUMENTS
//	class Handle_
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
template <class Handle_>
Plan::Relation::RowInfo*
JoinImpl::Base<Handle_>::
createRowInfo(Opt::Environment& cEnvironment_)
{
	Plan::Relation::RowInfo* pResult = Plan::Relation::RowInfo::create(cEnvironment_);
	foreachOperand(boost::bind(&Plan::Relation::RowInfo::append,
							   pResult,
							   boost::bind(&Operand::getRowInfo,
										   _1,
										   boost::ref(cEnvironment_))));
	return pResult;
}

// TEMPLATE FUNCTION private
//	Relation::JoinImpl::Base<Handle_>::createKeyInfo -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
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
template <class Handle_>
Plan::Relation::RowInfo*
JoinImpl::Base<Handle_>::
createKeyInfo(Opt::Environment& cEnvironment_)
{
	Plan::Relation::RowInfo* pResult = Plan::Relation::RowInfo::create(cEnvironment_);
	foreachOperand(boost::bind(&Plan::Relation::RowInfo::append,
							   pResult,
							   boost::bind(&Operand::getKeyInfo,
										   _1,
										   boost::ref(cEnvironment_))));
	return pResult;
}

// TEMPLATE FUNCTION private
//	Relation::JoinImpl::Base<Handle_>::setDegree -- set degree of the relation
//
// TEMPLATE ARGUMENTS
//	class Handle_
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
template <class Handle_>
int
JoinImpl::Base<Handle_>::
setDegree(Opt::Environment& cEnvironment_)
{
	return foreachOperand(Common::Functional::Accumulator(
								  &Operand::getDegree,
								  cEnvironment_,
								  0)).getVal();
}

// TEMPLATE FUNCTION public
//	Relation::JoinImpl::Base<Handle_>::setMaxPosition -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
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
template <class Handle_>
int
JoinImpl::Base<Handle_>::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	return getRowInfo(cEnvironment_)->GETSIZE();
}

// TEMPLATE FUNCTION private
//	Relation::JoinImpl::Base<Handle_>::createScalarName -- set scalar names
//
// TEMPLATE ARGUMENTS
//	class Handle_
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
template <class Handle_>
void
JoinImpl::Base<Handle_>::
createScalarName(Opt::Environment& cEnvironment_,
				 VECTOR<STRING>& vecName_,
				 Position iPosition_)
{
	// expand name vector by empty name to the degree of the relation
	Opt::ExpandContainer(vecName_, getDegree(cEnvironment_));
	; _SYDNEY_ASSERT(static_cast<int>(vecName_.GETSIZE()) >= getDegree(cEnvironment_));

	// get rowelement list and get iPosition-th element
	Plan::Relation::RowInfo* pRowInfo = getRowInfo(cEnvironment_);
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(pRowInfo->GETSIZE()));
	Plan::Relation::RowInfo::Element& cElement = (*pRowInfo)[iPosition_];

	// set element name
	if (cElement.first.getLength() == 0) {
		vecName_[iPosition_] = cElement.second->getScalarName(cEnvironment_);
	} else {
		vecName_[iPosition_] = cElement.first;
	}
}

// TEMPLATE FUNCTION private
//	Relation::JoinImpl::Base<Handle_>::createScalar -- set scalar interface
//
// TEMPLATE ARGUMENTS
//	class Handle_
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
template <class Handle_>
void
JoinImpl::Base<Handle_>::
createScalar(Opt::Environment& cEnvironment_,
			 VECTOR<Plan::Interface::IScalar*>& vecScalar_,
			 Position iPosition_)
{
	// expand scalar vector by null pointer to the degree of the relation
	Opt::ExpandContainer(vecScalar_, getDegree(cEnvironment_));
	; _SYDNEY_ASSERT(static_cast<int>(vecScalar_.GETSIZE()) >= getDegree(cEnvironment_));

	// get rowelement list and get iPosition-th element
	Plan::Relation::RowInfo* pRowInfo = getRowInfo(cEnvironment_);
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(pRowInfo->GETSIZE()));
	Plan::Relation::RowInfo::Element& cElement = (*pRowInfo)[iPosition_];

	// set element
	vecScalar_[iPosition_] = cElement.second->getScalar(cEnvironment_);
}

// TEMPLATE FUNCTION private
//	Relation::JoinImpl::Base<Handle_>::createScalarType -- set scalar node type
//
// TEMPLATE ARGUMENTS
//	class Handle_
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
template <class Handle_>
void
JoinImpl::Base<Handle_>::
createScalarType(Opt::Environment& cEnvironment_,
				 VECTOR<Plan::Tree::Node::Type>& vecType_,
				 Position iPosition_)
{
	// expand scalar vector by null pointer to the degree of the relation
	Opt::ExpandContainer(vecType_, getDegree(cEnvironment_), Plan::Tree::Node::Undefined);
	; _SYDNEY_ASSERT(static_cast<int>(vecType_.GETSIZE()) >= getDegree(cEnvironment_));

	// get rowelement list and get iPosition-th element
	Plan::Relation::RowInfo* pRowInfo = getRowInfo(cEnvironment_);
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(pRowInfo->GETSIZE()));
	Plan::Relation::RowInfo::Element& cElement = (*pRowInfo)[iPosition_];

	// set element
	vecType_[iPosition_] = cElement.second->getType();	
}

// TEMPLATE FUNCTION private
//	Relation::JoinImpl::Base<Handle_>::setRetrieved -- set retrieved flag
//
// TEMPLATE ARGUMENTS
//	class Handle_
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
template <class Handle_>
void
JoinImpl::Base<Handle_>::
setRetrieved(Opt::Environment& cEnvironment_,
			Position iPosition_)
{
	// get rowelement list and get iPosition-th element
	Plan::Relation::RowInfo* pRowInfo = getRowInfo(cEnvironment_);
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(pRowInfo->GETSIZE()));
	Plan::Relation::RowInfo::Element& cElement = (*pRowInfo)[iPosition_];

	// set retrieved
	cElement.second->retrieve(cEnvironment_);	
}

// TEMPLATE FUNCTION private
//	Relation::JoinImpl::Base<Handle_>::addAggregation -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
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
template <class Handle_>
int
JoinImpl::Base<Handle_>::
addAggregation(Opt::Environment& cEnvironment_,
			   Plan::Interface::IScalar* pScalar_,
			   Plan::Interface::IScalar* pOperand_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}



_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_JOINIMPL_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
