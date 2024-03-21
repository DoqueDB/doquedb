// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/JoinImpl.h --
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

#ifndef __SYDNEY_PLAN_RELATION_JOINIMPL_H
#define __SYDNEY_PLAN_RELATION_JOINIMPL_H

#include "boost/bind.hpp"

#include "Plan/Relation/Join.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Nadic.h"
#include "Plan/Utility/LightBitSet.h"


#include "Common/Assert.h"
#include "Common/Functional.h"

#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

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
	// Relation::Join::
		virtual bool isOuter() {return false;}
		virtual bool isExists() {return false;}

	//////////////////////////////
	// Interface::IRelation::

		using Super::getRowInfo;
		using Super::getDegree;
		using Super::foreachOperand;

	protected:
		// constructor
		Base(Tree::Node::Type eType_,
			 Interface::IPredicate* pJoinPredicate_,
			 typename Handle_::Argument cArgument_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Handle_(cArgument_)
		{
			setArgument(eType_, pJoinPredicate_);
		}
#else
			: Handle_(eType_, pJoinPredicate_, cArgument_)
		{}
#endif
	private:
	//////////////////////////////
	// Interface::IRelation::
		virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
		virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
		virtual int setDegree(Opt::Environment& cEnvironment_);
		virtual int setMaxPosition(Opt::Environment& cEnvironment_);
		virtual void createScalarName(Opt::Environment& cEnvironment_,
									  VECTOR<STRING>& vecName_,
									  Position iPosition_);
		virtual void createScalar(Opt::Environment& cEnvironment_,
								  VECTOR<Interface::IScalar*>& vecScalar_,
								  Position iPosition_);
		virtual void createScalarType(Opt::Environment& cEnvironment_,
									  VECTOR<Tree::Node::Type>& vecType_,
									  Position iPosition_);
		virtual void setRetrieved(Opt::Environment& cEnvironment_,
								  Position iPosition_);
		virtual int addAggregation(Opt::Environment& cEnvironment_,
								   Interface::IScalar* pScalar_,
								   Interface::IScalar* pOperand_);
	};

	//////////////////////////////////
	// CLASS
	//	Relation::JoinImpl::Dyadic
	//
	// NOTES
	class Dyadic
		: public Base< Tree::Dyadic<Join, Interface::IRelation> >
	{
	public:
		typedef Dyadic This;
		typedef Base< Tree::Dyadic<Join, Interface::IRelation> > Super;

		// constructor
		Dyadic(Tree::Node::Type eType_,
			   Interface::IPredicate* pJoinPredicate_,
			   Super::Argument cOperand_)
			: Super(eType_, pJoinPredicate_, cOperand_)
		{}
		// destructor
		virtual ~Dyadic() {}

	/////////////////////////////
	// Interface::IRelation::
		virtual int estimateUnwind(Opt::Environment& cEnvironment_);
		virtual int estimateUnion(Opt::Environment& cEnvironment_);

		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);
	protected:
	private:
	};

	//////////////////////////////////
	// CLASS
	//	Relation::JoinImpl::Nadic
	//
	// NOTES
	class Nadic
		: public Base< Tree::Nadic<Join, Interface::IRelation> >
	{
	public:
		typedef Nadic This;
		typedef Base< Tree::Nadic<Join, Interface::IRelation> > Super;

		// constructor
		Nadic(Tree::Node::Type eType_,
			  Interface::IPredicate* pJoinPredicate_,
			  Super::Argument vecOperand_)
			: Super(eType_, pJoinPredicate_, vecOperand_)
		{}
		// destructor
		~Nadic() {}

	////////////////////////////
	// Interface::IRelation
		virtual int estimateUnwind(Opt::Environment& cEnvironment_);
		virtual int estimateUnion(Opt::Environment& cEnvironment_);
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);
	protected:
	private:
		// {pos|processed relations' position} x min candidate
		typedef
		MAP<Utility::LightBitSet, Interface::ICandidate*, LESS<Utility::LightBitSet> >
		OperandMinCandidateMap;

		// create first operand's plan
		static void createInitialOperandPlan(Opt::Environment& cEnvironment_,
											 AccessPlan::Source& cPlanSource_,
											 Interface::IRelation* pOperand_,
											 int iPosition_,
											 OperandMinCandidateMap& cMap_);

		// accumulate access plan for operands
		static void accumulateOperandPlan(Opt::Environment& cEnvironment_,
										  AccessPlan::Source& cPlanSource_,
										  Interface::IRelation* pOperand_,
										  int iPosition_,
										  const OperandMinCandidateMap& cPreviousMap_,
										  OperandMinCandidateMap& cMap_);
	};

	//////////////////////////////////
	// CLASS
	//	Relation::JoinImpl::Exists
	//
	// NOTES
	class Exists
		: public Dyadic
	{
	public:
		typedef Exists This;
		typedef Dyadic Super;

		// constructor
		Exists(Tree::Node::Type eType_,
			   Interface::IPredicate* pJoinPredicate_,
			   Super::Argument cOperand_)
			: Super(eType_, pJoinPredicate_, cOperand_)
		{}
		// destructor
		~Exists() {}

	/////////////////////////////
	// Relation::Join::
		virtual bool isExists() {return true;}

	/////////////////////////////
	// Interface::IRelation::
	//	virtual int estimateUnwind(Opt::Environment& cEnvironment_);
	//	virtual int estimateUnion(Opt::Environment& cEnvironment_);
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);
	protected:
	private:
	//////////////////////////////
	// Interface::IRelation::
		virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
		virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
		virtual int setDegree(Opt::Environment& cEnvironment_);
		virtual int setMaxPosition(Opt::Environment& cEnvironment_);
	};

	//////////////////////////////////
	// CLASS
	//	Relation::JoinImpl::DyadicOuter
	//
	// NOTES
	class DyadicOuter
		: public Base< Tree::Dyadic<Join, Interface::IRelation> >
	{
	public:
		typedef DyadicOuter This;
		typedef Base< Tree::Dyadic<Join, Interface::IRelation> > Super;

		// constructor
		DyadicOuter(Tree::Node::Type eType_,
					Interface::IPredicate* pJoinPredicate_,
					Super::Argument cOperand_)
			: Super(eType_, pJoinPredicate_, cOperand_)
		{}
		// destructor
		~DyadicOuter() {}

	//////////////////////////////
	// Relation::Join::
		virtual bool isOuter() {return true;}

	/////////////////////////////
	// Interface::IRelation::
		virtual int estimateUnwind(Opt::Environment& cEnvironment_);
		virtual int estimateUnion(Opt::Environment& cEnvironment_);
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);
	protected:
	private:
	};

} // namespace Impl

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
RowInfo*
JoinImpl::Base<Handle_>::
createRowInfo(Opt::Environment& cEnvironment_)
{
	RowInfo* pResult = RowInfo::create(cEnvironment_);
	foreachOperand(boost::bind(&RowInfo::append,
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
RowInfo*
JoinImpl::Base<Handle_>::
createKeyInfo(Opt::Environment& cEnvironment_)
{
	RowInfo* pResult = RowInfo::create(cEnvironment_);
	foreachOperand(boost::bind(&RowInfo::append,
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
	RowInfo* pRowInfo = getRowInfo(cEnvironment_);
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(pRowInfo->GETSIZE()));
	RowInfo::Element& cElement = (*pRowInfo)[iPosition_];

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
			 VECTOR<Interface::IScalar*>& vecScalar_,
			 Position iPosition_)
{
	// expand scalar vector by null pointer to the degree of the relation
	Opt::ExpandContainer(vecScalar_, getDegree(cEnvironment_));
	; _SYDNEY_ASSERT(static_cast<int>(vecScalar_.GETSIZE()) >= getDegree(cEnvironment_));

	// get rowelement list and get iPosition-th element
	RowInfo* pRowInfo = getRowInfo(cEnvironment_);
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(pRowInfo->GETSIZE()));
	RowInfo::Element& cElement = (*pRowInfo)[iPosition_];

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
				 VECTOR<Tree::Node::Type>& vecType_,
				 Position iPosition_)
{
	// expand scalar vector by null pointer to the degree of the relation
	Opt::ExpandContainer(vecType_, getDegree(cEnvironment_), Tree::Node::Undefined);
	; _SYDNEY_ASSERT(static_cast<int>(vecType_.GETSIZE()) >= getDegree(cEnvironment_));

	// get rowelement list and get iPosition-th element
	RowInfo* pRowInfo = getRowInfo(cEnvironment_);
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(pRowInfo->GETSIZE()));
	RowInfo::Element& cElement = (*pRowInfo)[iPosition_];

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
	RowInfo* pRowInfo = getRowInfo(cEnvironment_);
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(pRowInfo->GETSIZE()));
	RowInfo::Element& cElement = (*pRowInfo)[iPosition_];

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
			   Interface::IScalar* pScalar_,
			   Interface::IScalar* pOperand_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_JOINIMPL_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
