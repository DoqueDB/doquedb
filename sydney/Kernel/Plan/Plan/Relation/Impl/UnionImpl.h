// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/UnionImpl.h --
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

#ifndef __SYDNEY_PLAN_RELATION_UNIONIMPL_H
#define __SYDNEY_PLAN_RELATION_UNIONIMPL_H

#include "boost/bind.hpp"

#include "Plan/Relation/Union.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Tree/Nadic.h"
#include "Plan/Tree/Option.h"

#include "Common/Assert.h"

#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace UnionImpl
{
	struct Type
	{
		enum Value
		{
			Unknown,
			DistinctByKey,
			Distinct,
			All,
			ValueNum
		};
	};

	//////////////////////////////////
	// TEMPLATE CLASS
	//	Relation::UnionImpl::Base
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

		// get union type
		virtual bool isAll() {return UnionImpl::Type::All == m_eType;}
		virtual bool isDistinct() {return UnionImpl::Type::Distinct == m_eType;}
		virtual bool isDistinctByKey() {return UnionImpl::Type::Distinct == m_eType;}

	protected:
		// constructor
		Base(UnionImpl::Type::Value eType_,
			 typename Handle_::Argument cArgument_)
			: Super(cArgument_),
			  m_eType(eType_)
		{}

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

		UnionImpl::Type::Value m_eType;
	};

	////////////////////////////////////////////
	// CLASS
	//	Relation::UnionImpl::Nadic
	//
	// NOTES
	class Nadic
		: public Base< Tree::Nadic<Union, Interface::IRelation> >
	{
	public:
		typedef Nadic This;
		typedef Base< Tree::Nadic<Union, Interface::IRelation> > Super;

		// constructor
		Nadic(UnionImpl::Type::Value eType_,
			  Super::Argument cOperand_)
			: Super(eType_, cOperand_)
		{}
		// destructor
		~Nadic() {}

	/////////////////////////////
	// Interface::IRelation::
		virtual PAIR<Interface::IRelation*, Interface::IPredicate*>
						rewrite(Opt::Environment& cEnvironment_,
								Interface::IPredicate* pPredicate_,
								Predicate::RewriteArgument& cArgument_);
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);
		
		virtual Interface::ICandidate* createOperandAccessPlan(Opt::Environment& cEnvironment_,
															   AccessPlan::Source& cPlanSource_,
															   Operand* pOperand_);
															   
	protected:
	private:
		Interface::IRelation* rewriteOperand(Opt::Environment& cEnvironment_,
											 Interface::IPredicate* pPredicate_,
											 Predicate::RewriteArgument& cArgument_,
											 Interface::IRelation* pOperand_);

		bool hasSortKey(Opt::Environment& cEnvironment_,
						Order::Specification* pSpecification_);
	};

} // namespace Impl

//////////////////////////////////////
// Relation::UnionImpl::Base

// TEMPLATE FUNCTION private
//	Relation::UnionImpl::Base<Handle_>::createRowInfo -- set result row spec
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
UnionImpl::Base<Handle_>::
createRowInfo(Opt::Environment& cEnvironment_)
{
	return getOperandi(0)->getRowInfo(cEnvironment_);
}

// TEMPLATE FUNCTION private
//	Relation::UnionImpl::Base<Handle_>::createKeyInfo -- 
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
UnionImpl::Base<Handle_>::
createKeyInfo(Opt::Environment& cEnvironment_)
{
	// no keys
	return RowInfo::create(cEnvironment_);
}

// TEMPLATE FUNCTION private
//	Relation::UnionImpl::Base<Handle_>::setDegree -- set degree of the relation
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
UnionImpl::Base<Handle_>::
setDegree(Opt::Environment& cEnvironment_)
{
	return getOperandi(0)->getDegree(cEnvironment_);
}

// TEMPLATE FUNCTION public
//	Relation::UnionImpl::Base<Handle_>::setMaxPosition -- 
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
UnionImpl::Base<Handle_>::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	return getDegree(cEnvironment_);
}

// TEMPLATE FUNCTION private
//	Relation::UnionImpl::Base<Handle_>::createScalarName -- set scalar names
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
UnionImpl::Base<Handle_>::
createScalarName(Opt::Environment& cEnvironment_,
				 VECTOR<STRING>& vecName_,
				 Position iPosition_)
{
	// expand name vector by empty name to the degree of the relation
	Opt::ExpandContainer(vecName_, getDegree(cEnvironment_));
	; _SYDNEY_ASSERT(static_cast<int>(vecName_.GETSIZE()) >= getDegree(cEnvironment_));

	// get first operand's rowinfo
	RowInfo* pRowInfo = getOperandi(0)->getRowInfo(cEnvironment_);
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(pRowInfo->GETSIZE()));
	vecName_[iPosition_] = pRowInfo->getScalarName(cEnvironment_,
												   iPosition_);
}

// TEMPLATE FUNCTION private
//	Relation::UnionImpl::Base<Handle_>::createScalar -- set scalar interface
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
UnionImpl::Base<Handle_>::
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

// TEMPLATE FUNCTION private
//	Relation::UnionImpl::Base<Handle_>::createScalarType -- set scalar node type
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
UnionImpl::Base<Handle_>::
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

// TEMPLATE FUNCTION private
//	Relation::UnionImpl::Base<Handle_>::setRetrieved -- set retrieved flag
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
UnionImpl::Base<Handle_>::
setRetrieved(Opt::Environment& cEnvironment_,
			 Position iPosition_)
{
	// get rowelement list and get iPosition-th element
	RowInfo* pRowInfo = getOperandi(0)->getRowInfo(cEnvironment_);
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(pRowInfo->GETSIZE()));
	pRowInfo->retrieve(cEnvironment_,
					   iPosition_);
}

// TEMPLATE FUNCTION private
//	Relation::UnionImpl::Base<Handle_>::addAggregation -- 
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
UnionImpl::Base<Handle_>::
addAggregation(Opt::Environment& cEnvironment_,
			   Interface::IScalar* pScalar_,
			   Interface::IScalar* pOperand_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_UNIONIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
