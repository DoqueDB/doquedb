// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Projection.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "DPlan/Candidate/Projection.h"

#include "Exception/NotSupported.h"

#include "Plan/Relation/Projection.h"
#include "Plan/Relation/Filter.h"
#include "Plan/Relation/RowInfo.h"

#include "Plan/Candidate/Projection.h"
#include "Plan/Tree/Monadic.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace Impl
{
	// CLASS
	//	Relation::Impl::ProjectionImpl --implementation class of Projection
	//
	// NOTES
	class ProjectionImpl
		: public Relation::Filter<Relation::Projection>
	{
	public:
		typedef Relation::Filter<Relation::Projection> Super;
		typedef ProjectionImpl This;

		ProjectionImpl(RowInfo* pRowInfo_,
					   Interface::IRelation* pOperand_)
			: Super(pOperand_),
			  m_pRowInfo(pRowInfo_)
		{}
		~ProjectionImpl() {}

		// create access plan candidate
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual PAIR<Interface::IRelation*, Interface::IPredicate*>
						unwind(Opt::Environment& cEnvironment_);

		virtual Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

	protected:
	private:
	//////////////////////////////
	// Interface::IRelation::
		virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
	//	virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
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

		RowInfo* m_pRowInfo;
	};
} // namespace Impl

/////////////////////////////////////
// Relation::Impl::ProjectionImpl

// FUNCTION public
//	Relation::Impl::ProjectionImpl::createAccessPlan -- create access plan candidate
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
Impl::ProjectionImpl::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	Interface::ICandidate* pOperandCandidate =
		getOperand()->createAccessPlan(cEnvironment_,
									   cPlanSource_);

	if (cEnvironment_.hasCascade()) {
		return DPlan::Candidate::Projection::create(cEnvironment_,
													this,
													pOperandCandidate);
	} else {
		return Candidate::Projection::create(cEnvironment_,
										 this,
										 pOperandCandidate);
	}
}

// FUNCTION public
//	Relation::Impl::ProjectionImpl::inquiry -- 
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
Impl::ProjectionImpl::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	InquiryResult iResult = getOperand()->inquiry(cEnvironment_,
												  cArgument_);
	if (cArgument_.m_iTarget & InquiryArgument::Target::Refering
		&& (iResult & InquiryArgument::Target::Refering) == 0) {
		; _SYDNEY_ASSERT(cArgument_.m_pRelation);

		Utility::RelationSet cRelationSet;
		Utility::RelationSet cUsedRelationSet;
		cArgument_.m_pRelation->getUsedTable(cEnvironment_,
											 cRelationSet);

		m_pRowInfo->foreachElement(boost::bind(&Interface::IScalar::getUsedTable,
											   boost::bind(&RowElement::getScalar,
														   _1,
														   boost::ref(cEnvironment_)),
											   boost::ref(cUsedRelationSet)));
		if (cRelationSet.isContainingAny(cUsedRelationSet)) {
			iResult |= InquiryArgument::Target::Refering;
		}
	}
	return iResult;
}

// FUNCTION public
//	Relation::Impl::ProjectionImpl::unwind -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	PAIR<Interface::IRelation*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Interface::IRelation*, Interface::IPredicate*>
Impl::ProjectionImpl::
unwind(Opt::Environment& cEnvironment_)
{
	PAIR<Interface::IRelation*, Interface::IPredicate*> cUnwind =
		getOperand()->unwind(cEnvironment_);
	if (cUnwind.first != getOperand()) {
		cUnwind.first = Projection::create(cEnvironment_,
										   m_pRowInfo,
										   cUnwind.first);
	} else {
		cUnwind.first = this;
	}
	return cUnwind;
}



// FUNCTION public
//	Relation:Impl::Projection::generateSQL
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::ISqlNode*
//
// EXCEPTIONS

//virtual
Sql::Query*
Impl::ProjectionImpl::
generateSQL(Opt::Environment& cEnvironment_)
{
	Sql::Query* pResult = getOperand()->generateSQL(cEnvironment_);
	m_pRowInfo->foreachElement(boost::bind(&Plan::Interface::IScalar::retrieveFromCascade,
										   boost::bind(&Plan::Relation::RowElement::getScalar,
													   _1,
													   boost::ref(cEnvironment_)),
										   boost::ref(cEnvironment_),
										   pResult));
	return pResult;
}



// FUNCTION private
//	Relation::Impl::ProjectionImpl::createRowInfo -- set result row spec
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
Impl::ProjectionImpl::
createRowInfo(Opt::Environment& cEnvironment_)
{
	return m_pRowInfo;
}

// FUNCTION private
//	Relation::Impl::ProjectionImpl::setDegree -- set degree of the relation
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
Impl::ProjectionImpl::
setDegree(Opt::Environment& cEnvironment_)
{
	return m_pRowInfo->GETSIZE();
}

// FUNCTION public
//	Relation::Impl::ProjectionImpl::setMaxPosition -- set max position of the relation
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
Impl::ProjectionImpl::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	return m_pRowInfo->GETSIZE();
}

// FUNCTION private
//	Relation::Impl::ProjectionImpl::createScalarName -- set scalar names
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
Impl::ProjectionImpl::
createScalarName(Opt::Environment& cEnvironment_,
				 VECTOR<STRING>& vecName_,
				 Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(m_pRowInfo->GETSIZE()));

	Opt::ExpandContainer(vecName_, m_pRowInfo->GETSIZE());
	; _SYDNEY_ASSERT(vecName_.GETSIZE() >= m_pRowInfo->GETSIZE());

	RowInfo::Element& cElement = (*m_pRowInfo)[iPosition_];
	if (cElement.second->isElementOf(this)) {
		vecName_[iPosition_] = cElement.first;
	} else {
		vecName_[iPosition_] = m_pRowInfo->getScalarName(cEnvironment_, iPosition_);
	}
}

// FUNCTION private
//	Relation::Impl::ProjectionImpl::createScalar -- set scalar interface
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
Impl::ProjectionImpl::
createScalar(Opt::Environment& cEnvironment_,
			 VECTOR<Interface::IScalar*>& vecScalar_,
			 Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(m_pRowInfo->GETSIZE()));

	Opt::ExpandContainer(vecScalar_, m_pRowInfo->GETSIZE());
	; _SYDNEY_ASSERT(vecScalar_.GETSIZE() >= m_pRowInfo->GETSIZE());

	RowInfo::Element& cElement = (*m_pRowInfo)[iPosition_];
	if (cElement.second->isElementOf(this)) {
		; _SYDNEY_ASSERT(cElement.second->getScalar() != 0);
		vecScalar_[iPosition_] = cElement.second->getScalar();
	} else {
		vecScalar_[iPosition_] = m_pRowInfo->getScalar(cEnvironment_, iPosition_);
	}
}

// FUNCTION private
//	Relation::Impl::ProjectionImpl::createScalarType -- set scalar node type
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
Impl::ProjectionImpl::
createScalarType(Opt::Environment& cEnvironment_,
				 VECTOR<Tree::Node::Type>& vecType_,
				 Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(m_pRowInfo->GETSIZE()));

	Opt::ExpandContainer(vecType_, m_pRowInfo->GETSIZE());
	; _SYDNEY_ASSERT(vecType_.GETSIZE() >= m_pRowInfo->GETSIZE());

	RowInfo::Element& cElement = (*m_pRowInfo)[iPosition_];
	vecType_[iPosition_] = cElement.second->getType();
}

// FUNCTION private
//	Relation::Impl::ProjectionImpl::setRetrieved -- set retrieved flag
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
Impl::ProjectionImpl::
setRetrieved(Opt::Environment& cEnvironment_,
			 Position iPosition_)
{
	; _SYDNEY_ASSERT(iPosition_ < static_cast<int>(m_pRowInfo->GETSIZE()));

	RowInfo::Element& cElement = (*m_pRowInfo)[iPosition_];

	// set required flag
	cElement.second->retrieve(cEnvironment_);
}

/////////////////////////////////////
// Relation::Projection

// FUNCTION public
//	Relation::Projection::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	RowInfo* pRowInfo_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Projection*
//
// EXCEPTIONS

//static
Projection*
Projection::
create(Opt::Environment& cEnvironment_,
	   RowInfo* pRowInfo_,
	   Interface::IRelation* pOperand_)
{
	AUTOPOINTER<This> pResult = new Impl::ProjectionImpl(pRowInfo_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Relation::Projection::Projection -- constructor
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

Projection::
Projection()
	: Super(Tree::Node::Projection)
{}

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
