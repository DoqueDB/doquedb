// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IPredicate.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Interface";
}

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IRelation.h"

#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/Predicate/CheckUnknown.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_INTERFACE_USING

namespace
{
	// CONST
	//	$$$::_cstrEmptyString --
	//
	// NOTES
	//	predicate is a subclass of scalar
	//	but its name is not set

	const STRING _cstrEmptyString;

} // namespace

////////////////////////////////////
//	Plan::Interface::IPredicate

// FUNCTION public
//	Interface::IPredicate::convertNot -- try to eliminate NOT if the predicate P is used in the form of NOT(P)
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	IPredicate*
//
// EXCEPTIONS

//virtual
IPredicate*
IPredicate::
convertNot(Opt::Environment& cEnvironment_)
{
	// default: can't convert
	return 0;
}

// FUNCTION public
//	Interface::IPredicate::estimateRewrite -- estimate relation number in rewriting
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
IPredicate::
estimateRewrite(Opt::Environment& cEnvironment_)
{
	// default: not added
	return 1;
}

// FUNCTION public
//	Interface::IPredicate::rewrite -- try to rewrite relation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	IRelation* pRelation_
//	Predicate::RewriteArgument& cArgument_
//	
// RETURN
//	IPredicate::RewriteResult
//
// EXCEPTIONS

//virtual
IPredicate::RewriteResult
IPredicate::
rewrite(Opt::Environment& cEnvironment_,
		IRelation* pRelation_,
		Predicate::RewriteArgument& cArgument_)
{
	// default: can't rewrite
	return RewriteResult(pRelation_, this);
}

// FUNCTION public
//	Interface::IPredicate::check -- search for applicable index file
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Predicate::CheckArgument& cArgument_
//	
// RETURN
//	IPredicate*
//
// EXCEPTIONS

//virtual
IPredicate*
IPredicate::
check(Opt::Environment& cEnvironment_,
	  const Predicate::CheckArgument& cArgument_)
{
	// by default, niladic predicate don't use index
	return Predicate::CheckedInterface::create(cEnvironment_,
											   this);
}

// FUNCTION public
//	Interface::IPredicate::isChecked -- is index file checked?
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
isChecked()
{
	return false;
}

// FUNCTION public
//	Interface::IPredicate::getChecked -- get predicate which is checked for index file
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	Predicate::CheckedInterface*
//
// EXCEPTIONS

//virtual
Predicate::CheckedInterface*
IPredicate::
getChecked()
{
	// default: this is not called
	_SYDNEY_ASSERT(false);
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Interface::IPredicate::getNotChecked -- get predicate which is not checked for index file
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	IPredicate*
//
// EXCEPTIONS

//virtual
IPredicate*
IPredicate::
getNotChecked()
{
	return this;
}

// FUNCTION public
//	Interface::IPredicate::getCheckUnknown -- get checkunknown predicate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
getCheckUnknown(Opt::Environment& cEnvironment_,
				Predicate::CheckUnknownArgument& cArgument_)
{
	getUnknownKey(cEnvironment_,
				  cArgument_);

	if (cArgument_.m_cKey.isEmpty() == false) {
		cArgument_.m_pPredicate = this;
	} else {
		cArgument_.m_pPredicate = 0;
	}

	// anyway, check unknown has been calculated successfully,
	// so, return true
	return true;
}

// FUNCTION public
//	Interface::IPredicate::isChosen -- is index file chosen?
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
isChosen()
{
	return false;
}

// FUNCTION public
//	Interface::IPredicate::getChosen -- get predicate for which a index file is chosen
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Predicate::ChosenInterface*
//
// EXCEPTIONS

//virtual
Predicate::ChosenInterface*
IPredicate::
getChosen()
{
	return 0;
}

// FUNCTION public
//	Interface::IPredicate::hasSubquery -- is using subquery?
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
hasSubquery()
{
	return false;
}

// FUNCTION public
//	Interface::IPredicate::isFetch -- is fetching by index?
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
isFetch()
{
	return false;
}

// FUNCTION public
//	Interface::IPredicate::getFetchKey -- get fetch key
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::ScalarSet& cFetchKey_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
getFetchKey(Opt::Environment& cEnvironment_,
			Utility::ScalarSet& cFetchKey_)
{
	return false;	// default: do nothing
}

// FUNCTION public
//	Interface::IPredicate::createFetch -- create fetch form
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::ScalarSet& cFetchKey_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
IPredicate::
createFetch(Opt::Environment& cEnvironment_,
			Utility::ScalarSet& cFetchKey_)
{
	return 0;
}

// FUNCTION public
//	Interface::IPredicate::isNeedIndex -- is need index?
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
isNeedIndex()
{
	return false;
}

// FUNCTION public
//	Interface::IPredicate::estimateCost -- estimate cost
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cResult_)
{
	// default: can't estimate
	return false;
}

// FUNCTION public
//	Interface::IPredicate::checkRate -- estimate effective rate for a set of tables
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Utility::RelationSet& cTable_
//	
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

//virtual
AccessPlan::Cost::Value
IPredicate::
checkRate(Opt::Environment& cEnvironment_,
		  const Utility::RelationSet& cTable_)
{
	AccessPlan::Cost cEstimate;
	if (estimateCost(cEnvironment_,
					 cEstimate)) {
		Utility::RelationSet cSet;
		getUsedTable(cSet);
		// if all tables are included in cTable,
		// this predicate can be processed.
		if (cTable_.isContaining(cSet)) {
			return cEstimate.getRate();
		}
	}
	return 1.0;
}

// FUNCTION public
//	Interface::IPredicate::generateKey -- generate key for fetching
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
IPredicate::
generateKey(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_)
{
	// never called for predicates other than Fetch
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Interface::IPredicate::adoptIndex -- create actions specific to predicate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::File* pFile_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
IPredicate::
adoptIndex(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Action::FileAccess* pFileAccess_,
		   Candidate::File* pFile_,
		   Candidate::AdoptArgument& cArgument_)
{
	// default: do nothing
	;
}


// FUNCTION public
//	Interface::IPredicate::createDistributePlan --　分散時にCandidateを生成する.

//		
// NOTES
//	分散時の述語の処理のために、Candidateを生成するためのインタフェース．
//	今のところ重要語句抽出時にDPlan::Predicate::Containsから、
//	DPlan::Candidate::Word生成のために使用している.
//	
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
// 	Interface::ICandidate* pOperand_
//
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
IPredicate::
createDistributePlan(Opt::Environment& cEnvironment_,
					 Interface::ICandidate* pOperand_,
					 Plan::Utility::FieldSet& cFieldSet_)
{
	return pOperand_;
}

// FUNCTION public
//	Interface::IPredicate::getName -- get scalar name
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const STRING&
//
// EXCEPTIONS

//virtual
const STRING&
IPredicate::
getName()
{
	return _cstrEmptyString;
}

// FUNCTION public
//	Interface::IPredicate::setExpectedType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Scalar::DataType& cType_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
IPredicate::
setExpectedType(Opt::Environment& cEnvironment_,
				const Scalar::DataType& cType_)
{
	// for now, predicate can't be treated as scalar value
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IPredicate::createCast -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Scalar::DataType& cToType_
//	bool bForComparison_
//	Tree::Node::Type eType_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
IPredicate::
createCast(Opt::Environment& cEnvironment_,
		   const Scalar::DataType& cToType_,
		   bool bForComparison_,
		   Tree::Node::Type eType_)
{
	// for now, predicate can't be treated as scalar value
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IPredicate::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Scalar::CheckArgument& cArgument_
//	
// RETURN
//	IPredicate::Check::Value
//
// EXCEPTIONS

//virtual
IPredicate::Check::Value
IPredicate::
check(Opt::Environment& cEnvironment_,
	  const Scalar::CheckArgument& cArgument_)
{
	// niladic predicate refers no relations
	// so, it can be regarded as constant
	return Check::Constant;
}

// FUNCTION public
//	Interface::IPredicate::isRefering -- check underlying table
//
// NOTES
//
// ARGUMENTS
//	Intterface::Relation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
isRefering(Interface::IRelation* pRelation_)
{
	// niladic predicate refers no relations
	return false;
}

// FUNCTION public
//	Interface::IPredicate::isKnownNull -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
isKnownNull(Opt::Environment& cEnvironment_)
{
	return false;
}

// FUNCTION public
//	Interface::IPredicate::isKnownNotNull -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
isKnownNotNull(Opt::Environment& cEnvironment_)
{
	return true;
}

// FUNCTION public
//	Interface::IPredicate::hasParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
hasParameter()
{
	return false;
}

// FUNCTION public
//	Interface::IPredicate::isArbitraryElement -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
isArbitraryElement()
{
	return false;
}

// FUNCTION public
//	Interface::IPredicate::isField -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
isField()
{
	return false;
}

// FUNCTION public
//	Interface::IPredicate::getField -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
IPredicate::
getField()
{
	return 0;
}

// FUNCTION public
//	Interface::IPredicate::hasField -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
hasField(Interface::IFile* pFile_)
{
	return false;
}

// FUNCTION public
//	Interface::IPredicate::isOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
isOperation()
{
	return false;
}

// FUNCTION public
//	Interface::IPredicate::isEquivalent -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IScalar* pScalar_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
isEquivalent(Interface::IScalar* pScalar_)
{
	return this == pScalar_;
}

// FUNCTION public
//	Interface::IPredicate::addOption -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOption_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
IPredicate::
addOption(Opt::Environment& cEnvironment_,
		  Interface::IScalar* pOption_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::IPredicate::require -- extract scalar operands which referes target relation
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
IPredicate::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	; // do nothing
}

// FUNCTION public
//	Interface::IPredicate::retrieve -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
IPredicate::
retrieve(Opt::Environment& cEnvironment_)
{
	; // do nothing
}

// FUNCTION public
//	Interface::IPredicate::retrieve -- extract scalar operands which referes target relation
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
IPredicate::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	; // do nothing
}



// FUNCTION public
//	Interface::IPredicate::retrieveFromCascade -- extract scalar operands which referes target relation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Sql::Query* pQuery
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
IPredicate::
retrieveFromCascade(Opt::Environment& cEnvironment_,
					Sql::Query* pQuery)
{
	pQuery->setPredicate(this);
}


// FUNCTION public
//	Interface::IPredicate::use -- 
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
IPredicate::
use(Opt::Environment& cEnvironment_,
	Interface::ICandidate* pCandidate_)
{
	; // do nothing
}

// FUNCTION public
//	Interface::IPredicate::delay -- set refered scalars as delayable
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IPredicate::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	return false; // do nothing
}

// FUNCTION public
//	Interface::IPredicate::preCalculate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

//virtual
Common::Data::Pointer
IPredicate::
preCalculate(Opt::Environment& cEnvironment_)
{
	return Common::Data::Pointer();
}

// FUNCTION public
//	Interface::IPredicate::setMetaData -- set column meta data
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Common::ColumnMetaData& cMetaData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
IPredicate::
setMetaData(Opt::Environment& cEnvironment_,
			Common::ColumnMetaData& cMetaData_)
{
	; // do nothing
}

// FUNCTION protected
//	Interface::IPredicate::IPredicate -- constructor
//
// NOTES
//
// ARGUMENTS
//	IPredicate::Type eOperator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

IPredicate::
IPredicate(IPredicate::Type eOperator_)
	: Super(eOperator_),
	  m_cEstimateRate(),
	  m_cCachedCost()
{}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
