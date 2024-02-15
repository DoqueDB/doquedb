// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/JoinImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Relation";
}

#include "boost/function.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "DPlan/Relation/Impl/JoinImpl.h"
#include "DPlan/Candidate/Impl/TableImpl.h"


#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IPredicate.h"

#include "Plan/Relation/Argument.h"

#include "Plan/Sql/Table.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_DPLAN_USING
_SYDNEY_DPLAN_RELATION_USING

namespace {
	class _Inquirer
	{
	public:
		_Inquirer(Opt::Environment& cEnvironment_,
				  const Plan::Relation::InquiryArgument& cArgument_)
			: m_cEnvironment(cEnvironment_),
			  m_cArgument(cArgument_),
			  m_bPropagate(cArgument_.m_iTarget & (Plan::Relation::InquiryArgument::Target::Depending
												   | Plan::Relation::InquiryArgument::Target::Refering
												   | Plan::Relation::InquiryArgument::Target::Distributed)),
			  m_iResult(0)
		{}

		void operator()(Plan::Interface::IRelation* pRelation_);

		Plan::Interface::IRelation::InquiryResult getResult()
		{return m_iResult;}

	protected:
	private:
		Opt::Environment& m_cEnvironment;
		const Plan::Relation::InquiryArgument& m_cArgument;
		bool m_bPropagate;
		Plan::Interface::IRelation::InquiryResult m_iResult;
	};
}

///////////////////////////////////////
// Relation::JoinImpl::Dyadic


// FUNCTION public
//	Relation::JoinImpl::Dyadic::createAccessPlan -- create access plan candidate
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
Plan::Interface::ICandidate*
JoinImpl::Dyadic::
createAccessPlan(Opt::Environment& cEnvironment_,
				 Plan::AccessPlan::Source& cPlanSource_)
{
	Plan::Relation::InquiryArgument cArgument = 0;
	cArgument.m_iTarget |= Plan::Relation::InquiryArgument::Target::Distributed;
	InquiryResult iResult = inquiry(cEnvironment_, cArgument);

	Candidate::Table* pResult = 0;

	// 本来は、各子サーバーで閉じたJoinかどうかを確認の上、
	// Candidateを生成する。
	if (iResult &
		Plan::Relation::InquiryArgument::Target::Distributed) {
		// distributed -> obtain from all servers
		pResult = Candidate::Table::Distribute::Retrieve::create(cEnvironment_,
																 this);
	} else {
		// replicated -> obtain from any one server
		pResult = Candidate::Table::Replicate::Retrieve::create(cEnvironment_,
																this);
	}

	Plan::Utility::FieldSet cFieldSet;	
	pResult->createPlan(cEnvironment_,
						cPlanSource_,
						cFieldSet);
	return pResult;
}

// FUNCTION public
//	Relation::JoinImpl::Dyadic::generateSQL
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Plan::Sql::Query*
//
// EXCEPTIONS

//virtual
Plan::Sql::Query*
JoinImpl::Dyadic::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pLeft = getOperand0()->generateSQL(cEnvironment_);
	pLeft->setCorrelationName(getOperand0()->getCorrelationName(cEnvironment_));

	Plan::Sql::Query* pRight = getOperand1()->generateSQL(cEnvironment_);
	pRight->setCorrelationName(getOperand1()->getCorrelationName(cEnvironment_));
	
	return Plan::Sql::Query::join(cEnvironment_,
								  getType(),
								  getJoinPredicate(),
								  pLeft,
								  pRight);
}




// FUNCTION public
//	Relation::JoinImpl::Dyadic::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument cArgument_
//	
// RETURN
//	Plan::InquiryArgument::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation::InquiryResult
JoinImpl::Dyadic::
inquiry(Opt::Environment& cEnvironment_,
		const Plan::Relation::InquiryArgument& cArgument_)
{
	return foreachOperand(_Inquirer(cEnvironment_, cArgument_)).getResult();
}


// FUNCTION public
//	Relation::JoinImpl::Dyadic::require -- 
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
JoinImpl::Dyadic::
require(Opt::Environment& cEnvironment_,
		Plan::Interface::ICandidate* pCandidate_)
{
	foreachOperand(boost::bind(&Operand::require,
							   _1,
							   boost::ref(cEnvironment_),
							   pCandidate_));
}

// FUNCTION public
//	Relation::JoinImpl::Dyadic::getUsedTable -- 
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
JoinImpl::Dyadic::
getUsedTable(Opt::Environment& cEnvironment_,
			 Plan::Utility::RelationSet& cResult_)
{
	foreachOperand(boost::bind(&Operand::getUsedTable,
							   _1,
							   boost::ref(cEnvironment_),
							   boost::ref(cResult_)));
}


// FUNCTION public
//	Relation::JoinImpl::Dyadic::isOuter
//	
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
JoinImpl::Dyadic::
isOuter()
{
	switch(getType()) {
	case Plan::Tree::Node::LeftOuterJoin:
	case Plan::Tree::Node::RightOuterJoin:
	case Plan::Tree::Node::FullOuterJoin:
		return true;
	default:
		break;
	}
	return false;
}




///////////////////////////////////////
// Relation::JoinImpl::Nadic

// FUNCTION public
//	Relation::JoinImpl::Nadic::createAccessPlan -- create access plan candidate
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
Plan::Interface::ICandidate*
JoinImpl::Nadic::
createAccessPlan(Opt::Environment& cEnvironment_,
				 Plan::AccessPlan::Source& cPlanSource_)
{
	Plan::Relation::InquiryArgument cArgument = 0;
	cArgument.m_iTarget |= Plan::Relation::InquiryArgument::Target::Distributed;
	InquiryResult iResult = inquiry(cEnvironment_, cArgument);
	

	Candidate::Table* pResult = 0;
	if (iResult &
		Plan::Relation::InquiryArgument::Target::Distributed) {
		// distributed -> obtain from all servers
		pResult = Candidate::Table::Distribute::Retrieve::create(cEnvironment_,
																 this);
	} else {
		// replicated -> obtain from any one server
		pResult = Candidate::Table::Replicate::Retrieve::create(cEnvironment_,
																this);
	}

	Plan::Utility::FieldSet cFieldSet;
	pResult->createPlan(cEnvironment_,
						cPlanSource_,
						cFieldSet);

	return pResult;
}


// FUNCTION public
//	Relation::JoinImpl::Nadic::generateSQL
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Plan::Sql::Query*
//
// EXCEPTIONS

//virtual
Plan::Sql::Query*
JoinImpl::Nadic::
generateSQL(Opt::Environment& cEnvironment_)
{
	VECTOR<Plan::Sql::Query*> vecQuery;
	for (Iterator ite = begin(); ite != end(); ++ite) {
		vecQuery.PUSHBACK((*ite)->generateSQL(cEnvironment_));
	}

	return Plan::Sql::Query::join(cEnvironment_, vecQuery);
}





// FUNCTION public
//	Relation::JoinImpl::Dyadic::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument cArgument_
//	
// RETURN
//	Plan::InquiryArgument::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation::InquiryResult
JoinImpl::Nadic::
inquiry(Opt::Environment& cEnvironment_,
		const Plan::Relation::InquiryArgument& cArgument_)
{
	return foreachOperand(_Inquirer(cEnvironment_, cArgument_)).getResult();
}

// FUNCTION public
//	Relation::JoinImpl::Nadic::require -- 
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
JoinImpl::Nadic::
require(Opt::Environment& cEnvironment_,
		Plan::Interface::ICandidate* pCandidate_)
{
	foreachOperand(boost::bind(&Operand::require,
							   _1,
							   boost::ref(cEnvironment_),
							   pCandidate_));
}

// FUNCTION public
//	Relation::JoinImpl::Nadic::getUsedTable -- 
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
JoinImpl::Nadic::
getUsedTable(Opt::Environment& cEnvironment_,
			 Plan::Utility::RelationSet& cResult_)
{
	foreachOperand(boost::bind(&Operand::getUsedTable,
							   _1,
							   boost::ref(cEnvironment_),
							   boost::ref(cResult_)));
}


////////////////////////
// $$$::_Inquirer

// FUNCTION local
//	$$$::_Inquirer::operator() -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_Inquirer::
operator()(Plan::Interface::IRelation* pRelation_)
{
	if (m_bPropagate) {
		m_iResult |= pRelation_->inquiry(m_cEnvironment,
										 m_cArgument);
	}
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
