// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/InImpl.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Predicate::Impl";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/Impl/InImpl.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/Predicate/Combinator.h"
#include "Plan/Predicate/Comparison.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Relation/Join.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Selection.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"
#include "Common/DataInstance.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/Argument.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Operator/Iterate.h"
#include "Execution/Operator/Output.h"
#include "Execution/Predicate/In.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Opt/Explain.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	// CLASS local
	//	$$$::_CheckOperand -- function class for check
	//
	// NOTES

	class _CheckOperand
	{
	public:
		typedef Interface::IScalar::Check Check;

		// constructor
		_CheckOperand(Opt::Environment& cEnvironment_,
					  const Scalar::CheckArgument& cArgument_)
			: m_cEnvironment(cEnvironment_),
			  m_cArgument(cArgument_),
			  m_cResult(Check::Constant)
		{}

		// calculate for one element
		void operator()(Interface::IScalar* pOperand_)
		{
			Check::mergeValue(&m_cResult, pOperand_->check(m_cEnvironment,
														   m_cArgument));
		}

		// get result
		Check::Value getVal() {return m_cResult;}

	protected:
	private:
		Opt::Environment& m_cEnvironment;
		const Scalar::CheckArgument& m_cArgument;
		Check::Value m_cResult;
	};

	const char* const _pszOperatorName[] = {
		" in (...)",					// isNot == false
		" not in (...)"					// isNot == true
	};
}

/////////////////////////////////////////////////
//	Plan::Predicate::InImpl::SubQuery::Base

// FUNCTION public
//	Predicate::InImpl::SubQuery::Base::estimateRewrite -- 
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
InImpl::SubQuery::Base::
estimateRewrite(Opt::Environment& cEnvironment_)
{
	return m_pOperand->estimateUnion(cEnvironment_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::Base::rewrite -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	IRelation* pRelation_
//	Predicate::RewriteArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate::RewriteResult
//
// EXCEPTIONS

//virtual
Interface::IPredicate::RewriteResult
InImpl::SubQuery::Base::
rewrite(Opt::Environment& cEnvironment_,
		Interface::IRelation* pRelation_,
		Predicate::RewriteArgument& cArgument_)
{
	if (cArgument_.m_bNoRelationChange) {
		return Interface::IPredicate::RewriteResult(pRelation_, this);
	}

	// create equal predicate
	Interface::IPredicate* pPredicate = createRewritePredicate(cEnvironment_);

	// unwind subquery
	PAIR<Interface::IRelation*, Interface::IPredicate*> cUnwind =
		m_pOperand->unwind(cEnvironment_);

	if (cUnwind.second) {
		pPredicate = Predicate::Combinator::create(cEnvironment_,
												   Tree::Node::And,
												   MAKEPAIR(pPredicate, cUnwind.second));
	}

	// convert into exists join
	Interface::IRelation* pResult =
		Relation::Join::create(cEnvironment_,
							   isNot() ? Tree::Node::NotExists : Tree::Node::Exists,
							   isNot() ? pPredicate : 0 /* join predicate */,
							   MAKEPAIR(pRelation_, cUnwind.first));

	return Interface::IPredicate::RewriteResult(pResult, isNot() ? 0 : pPredicate);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::Base::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
InImpl::SubQuery::Base::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// check key
	Check::Value cCheck = checkKey(cEnvironment_, cArgument_);
	if ( ! Check::isOn(cCheck, Check::NotYet)) {
		// check outer relation
		m_cOuterRelation.foreachElement(boost::bind(&This::checkOuterRelation,
													this,
													_1,
													boost::ref(cEnvironment_),
													boost::ref(cArgument_),
													&cCheck));
	}
	if (Check::isOn(cCheck, Check::NotYet)) {
		require(cEnvironment_, cArgument_.m_pCandidate);
		return this;
	}
	// no index files
	return CheckedInterface::create(cEnvironment_,
									this);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::Base::hasSubquery -- 
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
InImpl::SubQuery::Base::
hasSubquery()
{
	return true;
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::Base::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::SubQuery::Base::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	explainKey(pEnvironment_, cExplain_);
	cExplain_.put(_pszOperatorName[isNot()?1:0]);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::Base::generate -- 
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
InImpl::SubQuery::Base::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// YET
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION private
//	Predicate::InImpl::SubQuery::Base::checkOuterRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	Check::Value* pValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
InImpl::SubQuery::Base::
checkOuterRelation(Interface::IRelation* pRelation_,
				   Opt::Environment& cEnvironment_,
				   const CheckArgument& cArgument_,
				   Check::Value* pValue_)
{
	if (cArgument_.m_pCandidate
		&& cArgument_.m_pCandidate->isReferingRelation(pRelation_)) {
		Check::mergeValue(pValue_, Check::Referred);
		// set as requried in the candidate
		require(cEnvironment_,
				cArgument_.m_pCandidate);

	} else if (cArgument_.m_vecPrecedingCandidate.ISEMPTY() == false
			   && Opt::IsAny(cArgument_.m_vecPrecedingCandidate,
							 boost::bind(&Interface::ICandidate::isReferingRelation,
										 _1,
										 pRelation_))) {
		Check::mergeValue(pValue_, Check::Preceding);

	} else {
		Check::mergeValue(pValue_, Check::NotYet);
	}
}

/////////////////////////////////////////////////
//	Plan::Predicate::InImpl::SubQuery::SingleKey

// FUNCTION public
//	Predicate::InImpl::SubQuery::SingleKey::getUsedTable -- check used tables
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::SubQuery::SingleKey::
getUsedTable(Utility::RelationSet& cResult_)
{
	m_pScalar->getUsedTable(cResult_);
	cResult_.merge(getOuterRelation());
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::SingleKey::getUsedField -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::FieldSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::SubQuery::SingleKey::
getUsedField(Utility::FieldSet& cResult_)
{
	m_pScalar->getUsedField(cResult_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::SingleKey::getUnknownKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::SubQuery::SingleKey::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	m_pScalar->getUnknownKey(cEnvironment_, cResult_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::SingleKey::require -- 
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
InImpl::SubQuery::SingleKey::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	m_pScalar->require(cEnvironment_, pCandidate_);
	getOperand()->require(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::SingleKey::retrieve -- 
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
InImpl::SubQuery::SingleKey::
retrieve(Opt::Environment& cEnvironment_)
{
	m_pScalar->retrieve(cEnvironment_);
	// require outer reference in operand
	getOperand()->require(cEnvironment_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::SingleKey::retrieve -- 
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
InImpl::SubQuery::SingleKey::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	m_pScalar->retrieve(cEnvironment_, pCandidate_);
	// require outer reference in operand
	getOperand()->require(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::SingleKey::use -- 
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
InImpl::SubQuery::SingleKey::
use(Opt::Environment& cEnvironment_,
	Interface::ICandidate* pCandidate_)
{
	m_pScalar->use(cEnvironment_, pCandidate_);
	// require outer reference in operand
	getOperand()->require(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::SingleKey::delay -- 
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
InImpl::SubQuery::SingleKey::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	return m_pScalar->delay(cEnvironment_, pCandidate_, cArgument_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::SingleKey::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
InImpl::SubQuery::SingleKey::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{

	m_pScalar->setParameter(cEnvironment_,
							cProgram_,
							pIterator_,
							cExec_,
							cArgument_);

	if (isNot())
		cExec_.append(" not in (");
	else
		cExec_.append(" in (");


	Plan::AccessPlan::Source cPlanSource;
	Plan::Interface::ICandidate* pCandidate =
		getOperand()->createAccessPlan(cEnvironment_,
									   cPlanSource);
	Plan::Candidate::AdoptArgument cAdoptArgument;
	cAdoptArgument.m_pQuery = pCandidate->generateSQL(cEnvironment_);
	Execution::Interface::IIterator* pOperandIterator =
		pCandidate->adopt(cEnvironment_, cProgram_, cAdoptArgument);
	Relation::RowInfo* pRowInfo = getOperand()->getRowInfo(cEnvironment_);
	if (pRowInfo->getSize() != 1) {
		// this situation should be checked before
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	int iDataID = pRowInfo->getScalar(cEnvironment_, 0)->generate(cEnvironment_,
																  cProgram_,
																  pOperandIterator,
																  cAdoptArgument);
	
	int iParamID =
		cProgram_.addVariable(Common::DataInstance::create(Common::DataType::Array));

	pOperandIterator->addCalculation(cProgram_,
									 Execution::Operator::Output::Array::create(cProgram_,
																				pOperandIterator,
																				iParamID,
																				iDataID));
	
	Execution::Interface::IAction* pAction =
		Execution::Operator::Iterate::All::create(cProgram_,
												  pIterator_,
												  pOperandIterator->getID());
	cExec_.appendArrayPlaceHolder(iParamID, pAction);
	cExec_.append(")");
}


// FUNCTION private
//	Predicate::InImpl::SubQuery::SingleKey::createRewritePredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
InImpl::SubQuery::SingleKey::
createRewritePredicate(Opt::Environment& cEnvironment_)
{
	Relation::RowInfo* pRowInfo = getOperand()->getRowInfo(cEnvironment_);
	if (pRowInfo->getSize() != 1) {
		// this situation should be checked before
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return Predicate::Comparison::create(cEnvironment_,
										 Tree::Node::Equals,
										 MAKEPAIR(m_pScalar,
												  pRowInfo->getScalar(cEnvironment_, 0)));
}

// FUNCTION private
//	Predicate::InImpl::SubQuery::SingleKey::explainKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::SubQuery::SingleKey::
explainKey(Opt::Environment* pEnvironment_,
		   Opt::Explain& cExplain_)
{
	m_pScalar->explain(pEnvironment_, cExplain_);
}

// FUNCTION private
//	Predicate::InImpl::SubQuery::SingleKey::checkKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	InImpl::SubQuery::SingleKey::Check::Value
//
// EXCEPTIONS

//virtual
InImpl::SubQuery::SingleKey::Check::Value
InImpl::SubQuery::SingleKey::
checkKey(Opt::Environment& cEnvironment_,
		 const CheckArgument& cArgument_)
{
	return m_pScalar->check(cEnvironment_, cArgument_);
}

/////////////////////////////////////////////////
//	Plan::Predicate::InImpl::SubQuery::MultiKey

// FUNCTION public
//	Predicate::InImpl::SubQuery::MultiKey::getUsedTable -- check used tables
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::SubQuery::MultiKey::
getUsedTable(Utility::RelationSet& cResult_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::getUsedTable,
						_1,
						boost::ref(cResult_)));
	cResult_.merge(getOuterRelation());
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::MultiKey::getUsedField -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::FieldSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::SubQuery::MultiKey::
getUsedField(Utility::FieldSet& cResult_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::getUsedField,
						_1,
						boost::ref(cResult_)));
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::MultiKey::getUnknownKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::SubQuery::MultiKey::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::getUnknownKey,
						_1,
						boost::ref(cEnvironment_),
						boost::ref(cResult_)));
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::MultiKey::require -- 
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
InImpl::SubQuery::MultiKey::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::require,
						_1,
						boost::ref(cEnvironment_),
						pCandidate_));

	getOperand()->require(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::MultiKey::retrieve -- 
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
InImpl::SubQuery::MultiKey::
retrieve(Opt::Environment& cEnvironment_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::retrieve,
						_1,
						boost::ref(cEnvironment_)));

	// require outer reference in operand
	getOperand()->require(cEnvironment_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::MultiKey::retrieve -- 
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
InImpl::SubQuery::MultiKey::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::retrieve,
						_1,
						boost::ref(cEnvironment_),
						pCandidate_));

	// require outer reference in operand
	getOperand()->require(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::MultiKey::use -- 
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
InImpl::SubQuery::MultiKey::
use(Opt::Environment& cEnvironment_,
	Interface::ICandidate* pCandidate_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::use,
						_1,
						boost::ref(cEnvironment_),
						pCandidate_));

	// require outer reference in operand
	getOperand()->require(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::MultiKey::delay -- 
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
InImpl::SubQuery::MultiKey::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	return Opt::IsAll(m_vecScalar,
					  boost::bind(&Interface::IScalar::delay,
								  _1,
								  boost::ref(cEnvironment_),
								  pCandidate_,
								  boost::ref(cArgument_)));
}

// FUNCTION public
//	Predicate::InImpl::SubQuery::MultiKey::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
InImpl::SubQuery::MultiKey::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	cExec_.append("(");
	VECTOR<Interface::IScalar*>::ConstIterator ite =  m_vecScalar.begin();
	char c = ' ';	
	for (; ite != m_vecScalar.end(); ++ite, c = ',') {
		cExec_.append(c);
		(*ite)->setParameter(cEnvironment_,
							 cProgram_,
							 pIterator_,
							 cExec_,
							 cArgument_);
	}
	if (isNot())
		cExec_.append(") not in (");
	else
		cExec_.append(") in (");

	Plan::Sql::Query* pQuery = getOperand()->generateSQL(cEnvironment_);
	pQuery->setParameter(cEnvironment_,
						 cProgram_,
						 pIterator_,
						 cExec_,
						 cArgument_);
							  

	cExec_.append(")");
}


// FUNCTION private
//	Predicate::InImpl::SubQuery::MultiKey::createRewritePredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
InImpl::SubQuery::MultiKey::
createRewritePredicate(Opt::Environment& cEnvironment_)
{
	Relation::RowInfo* pRowInfo = getOperand()->getRowInfo(cEnvironment_);
	if (pRowInfo->getSize() != m_vecScalar.GETSIZE()) {
		// this situation should be checked before
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	SIZE n = m_vecScalar.GETSIZE();
	VECTOR<Interface::IPredicate*> vecPredicate;
	vecPredicate.reserve(n);
	for (SIZE i = 0; i < n; ++i) {
		vecPredicate.PUSHBACK(Predicate::Comparison::create(
											cEnvironment_,
											Tree::Node::Equals,
											MAKEPAIR(m_vecScalar[i],
													 pRowInfo->getScalar(cEnvironment_, i))));
	}
	return Predicate::Combinator::create(cEnvironment_,
										 Tree::Node::And,
										 vecPredicate);
}

// FUNCTION private
//	Predicate::InImpl::SubQuery::MultiKey::explainKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::SubQuery::MultiKey::
explainKey(Opt::Environment* pEnvironment_,
		   Opt::Explain& cExplain_)
{
	cExplain_.put('(');
	Opt::Join(m_vecScalar,
			  boost::bind(&Interface::IScalar::explain,
						  _1,
						  pEnvironment_,
						  boost::ref(cExplain_)),
			  boost::bind(&Opt::Explain::putChar,
						  &cExplain_,
						  ','));
	cExplain_.put(')');
}

// FUNCTION private
//	Predicate::InImpl::SubQuery::MultiKey::checkKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	InImpl::SubQuery::MultiKey::Check::Value
//
// EXCEPTIONS

//virtual
InImpl::SubQuery::MultiKey::Check::Value
InImpl::SubQuery::MultiKey::
checkKey(Opt::Environment& cEnvironment_,
		 const CheckArgument& cArgument_)
{
	return FOREACH(m_vecScalar,
				   _CheckOperand(cEnvironment_, cArgument_)).getVal();
}

/////////////////////////////////////////////////
//	Plan::Predicate::InImpl::ValueList::Base

// FUNCTION public
//	Predicate::InImpl::ValueList::Base::rewrite -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	IRelation* pRelation_
//	Predicate::RewriteArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate::RewriteResult
//
// EXCEPTIONS

//virtual
Interface::IPredicate::RewriteResult
InImpl::ValueList::Base::
rewrite(Opt::Environment& cEnvironment_,
		Interface::IRelation* pRelation_,
		Predicate::RewriteArgument& cArgument_)
{
	Interface::IPredicate* pPredicate = 0;

	// get cardinality of valuelist
	Interface::IRelation::Size nCardinality = m_pOperand->getCardinality(cEnvironment_);
	switch (nCardinality) {
	case 0:
		{
			// never happens
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	case 1:
		{
			// just rewrite to simple equal
			pPredicate = createRewritePredicate(cEnvironment_, 0);
			if (isNot()) {
				pPredicate = Predicate::Combinator::create(cEnvironment_,
														   Tree::Node::Not,
														   pPredicate);
			}
			break;
		}
	default:
		{
			// [YET]
			// When cardinality is too large, OR should not be used

			if (isNot() && isArbitraryElement()) {
				// can't convert
				pPredicate = this;

			} else {
				VECTOR<Interface::IPredicate*> vecPredicate;
				for (int i = 0; i < nCardinality; ++i) {
					Interface::IPredicate* pElement = createRewritePredicate(cEnvironment_, i);
					if (isNot()) {
						pElement = Predicate::Combinator::create(cEnvironment_,
																 Tree::Node::Not,
																 pElement);
					}
					vecPredicate.PUSHBACK(pElement);
				}
				pPredicate = Predicate::Combinator::create(cEnvironment_,
														   isNot() ? Tree::Node::And : Tree::Node::Or,
														   vecPredicate);
			}
			break;
		}
	}
	return Interface::IPredicate::RewriteResult(pRelation_, pPredicate);
}

// FUNCTION public
//	Predicate::InImpl::ValueList::Base::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
InImpl::ValueList::Base::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// check key
	Check::Value cCheck = checkKey(cEnvironment_, cArgument_);
	if ( ! Check::isOn(cCheck, Check::NotYet)) {
		// check outer relation
		m_cOuterRelation.foreachElement(boost::bind(&This::checkOuterRelation,
													this,
													_1,
													boost::ref(cEnvironment_),
													boost::ref(cArgument_),
													&cCheck));
	}
	if (Check::isOn(cCheck, Check::NotYet)) {
		require(cEnvironment_, cArgument_.m_pCandidate);
		return this;
	}
	// no index files
	return CheckedInterface::create(cEnvironment_,
									this);
}

// FUNCTION public
//	Predicate::InImpl::ValueList::Base::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::ValueList::Base::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	explainKey(pEnvironment_, cExplain_);
	cExplain_.put(_pszOperatorName[isNot()?1:0]);
}

// FUNCTION public
//	Predicate::InImpl::ValueList::Base::generate -- 
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
InImpl::ValueList::Base::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_bGenerateForPredicate = true;
	int iData0 = generateKey(cEnvironment_, cProgram_, pIterator_, cMyArgument);
	int iData1 = generateOperand(cEnvironment_, cProgram_, pIterator_, cMyArgument);

	Execution::Predicate::In* pResult =
		isArbitraryElement() ?
		Execution::Predicate::In::AnyElement::create(
										 cProgram_,
										 pIterator_,
										 iData0,
										 iData1,
										 isNot())
		: Execution::Predicate::In::create(
									   cProgram_,
									   pIterator_,
									   iData0,
									   iData1,
									   isNot());
	return pResult->getID();
}

// FUNCTION private
//	Predicate::InImpl::ValueList::Base::checkOuterRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	Check::Value* pValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
InImpl::ValueList::Base::
checkOuterRelation(Interface::IRelation* pRelation_,
				   Opt::Environment& cEnvironment_,
				   const CheckArgument& cArgument_,
				   Check::Value* pValue_)
{
	if (cArgument_.m_pCandidate
		&& cArgument_.m_pCandidate->isReferingRelation(pRelation_)) {
		Check::mergeValue(pValue_, Check::Referred);
		// set as requried in the candidate
		require(cEnvironment_,
				cArgument_.m_pCandidate);

	} else if (cArgument_.m_vecPrecedingCandidate.ISEMPTY() == false
			   && Opt::IsAny(cArgument_.m_vecPrecedingCandidate,
							 boost::bind(&Interface::ICandidate::isReferingRelation,
										 _1,
										 pRelation_))) {
		Check::mergeValue(pValue_, Check::Preceding);

	} else {
		Check::mergeValue(pValue_, Check::NotYet);
	}
}

/////////////////////////////////////////////////
//	Plan::Predicate::InImpl::ValueList::SingleKey

// FUNCTION public
//	Predicate::InImpl::ValueList::SingleKey::isArbitraryElement -- 
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
InImpl::ValueList::SingleKey::
isArbitraryElement()
{
	return m_pScalar->isArbitraryElement();
}

// FUNCTION public
//	Predicate::InImpl::ValueList::SingleKey::getUsedTable -- check used tables
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::ValueList::SingleKey::
getUsedTable(Utility::RelationSet& cResult_)
{
	m_pScalar->getUsedTable(cResult_);
	cResult_.merge(getOuterRelation());
}

// FUNCTION public
//	Predicate::InImpl::ValueList::SingleKey::getUsedField -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::FieldSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::ValueList::SingleKey::
getUsedField(Utility::FieldSet& cResult_)
{
	m_pScalar->getUsedField(cResult_);
}

// FUNCTION public
//	Predicate::InImpl::ValueList::SingleKey::getUnknownKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::ValueList::SingleKey::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	m_pScalar->getUnknownKey(cEnvironment_,
							 cResult_);
}

// FUNCTION public
//	Predicate::InImpl::ValueList::SingleKey::require -- 
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
InImpl::ValueList::SingleKey::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	m_pScalar->require(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::ValueList::SingleKey::retrieve -- 
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
InImpl::ValueList::SingleKey::
retrieve(Opt::Environment& cEnvironment_)
{
	m_pScalar->retrieve(cEnvironment_);
}

// FUNCTION public
//	Predicate::InImpl::ValueList::SingleKey::retrieve -- 
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
InImpl::ValueList::SingleKey::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	m_pScalar->retrieve(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::ValueList::SingleKey::use -- 
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
InImpl::ValueList::SingleKey::
use(Opt::Environment& cEnvironment_,
	Interface::ICandidate* pCandidate_)
{
	m_pScalar->use(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::ValueList::SingleKey::delay -- 
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
InImpl::ValueList::SingleKey::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	return m_pScalar->delay(cEnvironment_, pCandidate_, cArgument_);
}

// FUNCTION public
//	Predicate::InImpl::ValueList::SingleKey::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
InImpl::ValueList::SingleKey::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	m_pScalar->setParameter(cEnvironment_,
							cProgram_,
							pIterator_,
							cExec_,
							cArgument_);
	if (isNot())
		cExec_.append(" not in (");
	else
		cExec_.append(" in (");

	
	char c = ' ';
	for (int i = 0; i < getOperand()->getCardinality(cEnvironment_); ++i, c = ',') {
		VECTOR<Interface::IScalar*> vecRow;
		if (getOperand()->getRow(cEnvironment_, i, vecRow) == false
			|| vecRow.GETSIZE() != 1) {
			// this situation should be checked before
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		
		cExec_.append(c);
		vecRow[0]->setParameter(cEnvironment_,
								cProgram_,
								pIterator_,
								cExec_,
								cArgument_);
	}
	cExec_.append(")");
}


// FUNCTION private
//	Predicate::InImpl::ValueList::SingleKey::createRewritePredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	int iPosition_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
InImpl::ValueList::SingleKey::
createRewritePredicate(Opt::Environment& cEnvironment_,
					   int iPosition_)
{
	VECTOR<Interface::IScalar*> vecRow;
	if (getOperand()->getRow(cEnvironment_, iPosition_, vecRow) == false
		|| vecRow.GETSIZE() != 1) {
		// this situation should be checked before
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return Predicate::Comparison::create(cEnvironment_,
										 Tree::Node::Equals,
										 MAKEPAIR(m_pScalar,
												  vecRow[0]));
}

// FUNCTION private
//	Predicate::InImpl::ValueList::SingleKey::explainKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::ValueList::SingleKey::
explainKey(Opt::Environment* pEnvironment_,
		   Opt::Explain& cExplain_)
{
	m_pScalar->explain(pEnvironment_, cExplain_);
}

// FUNCTION private
//	Predicate::InImpl::ValueList::SingleKey::checkKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	InImpl::ValueList::SingleKey::Check::Value
//
// EXCEPTIONS

//virtual
InImpl::ValueList::SingleKey::Check::Value
InImpl::ValueList::SingleKey::
checkKey(Opt::Environment& cEnvironment_,
		 const CheckArgument& cArgument_)
{
	return m_pScalar->check(cEnvironment_, cArgument_);
}

// FUNCTION private
//	Predicate::InImpl::ValueList::SingleKey::generateKey -- 
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
InImpl::ValueList::SingleKey::
generateKey(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_)
{
	return m_pScalar->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
}

// FUNCTION private
//	Predicate::InImpl::ValueList::SingleKey::generateOperand -- 
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
InImpl::ValueList::SingleKey::
generateOperand(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_)
{
	Interface::IRelation::Size nCardinality = getOperand()->getCardinality(cEnvironment_);
	VECTOR<int> vecData(nCardinality);
	for (int i = 0; i < nCardinality; ++i) {
		VECTOR<Interface::IScalar*> vecRow;
		if (getOperand()->getRow(cEnvironment_, i, vecRow) == false
			|| vecRow.GETSIZE() != 1) {
			// this situation should be checked before
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		vecData[i] = vecRow[0]->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
	}
	return cProgram_.addVariable(vecData);
}

/////////////////////////////////////////////////
//	Plan::Predicate::InImpl::ValueList::MultiKey

// FUNCTION public
//	Predicate::InImpl::ValueList::MultiKey::getUsedTable -- check used tables
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::ValueList::MultiKey::
getUsedTable(Utility::RelationSet& cResult_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::getUsedTable,
						_1,
						boost::ref(cResult_)));
	cResult_.merge(getOuterRelation());
}

// FUNCTION public
//	Predicate::InImpl::ValueList::MultiKey::getUsedField -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::FieldSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::ValueList::MultiKey::
getUsedField(Utility::FieldSet& cResult_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::getUsedField,
						_1,
						boost::ref(cResult_)));
}

// FUNCTION public
//	Predicate::InImpl::ValueList::MultiKey::getUnknownKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::ValueList::MultiKey::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::getUnknownKey,
						_1,
						boost::ref(cEnvironment_),
						boost::ref(cResult_)));
}

// FUNCTION public
//	Predicate::InImpl::ValueList::MultiKey::require -- 
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
InImpl::ValueList::MultiKey::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::require,
						_1,
						boost::ref(cEnvironment_),
						pCandidate_));
}

// FUNCTION public
//	Predicate::InImpl::ValueList::MultiKey::retrieve -- 
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
InImpl::ValueList::MultiKey::
retrieve(Opt::Environment& cEnvironment_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::retrieve,
						_1,
						boost::ref(cEnvironment_)));
}

// FUNCTION public
//	Predicate::InImpl::ValueList::MultiKey::retrieve -- 
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
InImpl::ValueList::MultiKey::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::retrieve,
						_1,
						boost::ref(cEnvironment_),
						pCandidate_));
}

// FUNCTION public
//	Predicate::InImpl::ValueList::MultiKey::use -- 
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
InImpl::ValueList::MultiKey::
use(Opt::Environment& cEnvironment_,
	Interface::ICandidate* pCandidate_)
{
	FOREACH(m_vecScalar,
			boost::bind(&Interface::IScalar::use,
						_1,
						boost::ref(cEnvironment_),
						pCandidate_));
}

// FUNCTION public
//	Predicate::InImpl::ValueList::MultiKey::delay -- 
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
InImpl::ValueList::MultiKey::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	return Opt::IsAll(m_vecScalar,
					  boost::bind(&Interface::IScalar::delay,
								  _1,
								  boost::ref(cEnvironment_),
								  pCandidate_,
								  boost::ref(cArgument_)));
}

// FUNCTION public
//	Predicate::InImpl::ValueList::MultiKey::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
InImpl::ValueList::MultiKey::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	cExec_.append("(");
	VECTOR<Interface::IScalar*>::ConstIterator ite =  m_vecScalar.begin();
	char c = ' ';	
	for (; ite != m_vecScalar.end(); ++ite, c = ',') {
		cExec_.append(c);
		(*ite)->setParameter(cEnvironment_,
							 cProgram_,
							 pIterator_,
							 cExec_,
							 cArgument_);
	}
	if (isNot())
		cExec_.append(") not in (");
	else
		cExec_.append(") in (");

	c= ' ';
	for (int i = 0; i < getOperand()->getCardinality(cEnvironment_); ++i, c = ',') {
		VECTOR<Interface::IScalar*> vecRow;
		if (getOperand()->getRow(cEnvironment_, i, vecRow) == false ) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		cExec_.append(c).append("(");
		char c1 = ' ';
		for (ite = vecRow.begin(); ite != vecRow.end(); ++ite, c1 = ',') {
			cExec_.append(c1);
			(*ite)->setParameter(cEnvironment_,
								 cProgram_,
								 pIterator_,
								 cExec_,
								 cArgument_);
		}
		cExec_.append(")");
	}
	cExec_.append(")");
}

// FUNCTION private
//	Predicate::InImpl::ValueList::MultiKey::createRewritePredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	int iPosition_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
InImpl::ValueList::MultiKey::
createRewritePredicate(Opt::Environment& cEnvironment_,
					   int iPosition_)
{
	VECTOR<Interface::IScalar*> vecRow;
	SIZE n = m_vecScalar.GETSIZE();
	if (getOperand()->getRow(cEnvironment_, iPosition_, vecRow) == false
		|| vecRow.GETSIZE() != n) {
		// this situation should be checked before
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	VECTOR<Interface::IPredicate*> vecPredicate;
	vecPredicate.reserve(n);
	for (SIZE i = 0; i < n; ++i) {
		vecPredicate.PUSHBACK(Predicate::Comparison::create(
											cEnvironment_,
											Tree::Node::Equals,
											MAKEPAIR(m_vecScalar[i],
													 vecRow[i])));
	}
	return Predicate::Combinator::create(cEnvironment_,
										 Tree::Node::And,
										 vecPredicate);
}

// FUNCTION private
//	Predicate::InImpl::ValueList::MultiKey::explainKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::ValueList::MultiKey::
explainKey(Opt::Environment* pEnvironment_,
		   Opt::Explain& cExplain_)
{
	cExplain_.put('(');
	Opt::Join(m_vecScalar,
			  boost::bind(&Interface::IScalar::explain,
						  _1,
						  pEnvironment_,
						  boost::ref(cExplain_)),
			  boost::bind(&Opt::Explain::putChar,
						  &cExplain_,
						  ','));
	cExplain_.put(')');
}

// FUNCTION private
//	Predicate::InImpl::ValueList::MultiKey::checkKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	InImpl::ValueList::MultiKey::Check::Value
//
// EXCEPTIONS

//virtual
InImpl::ValueList::MultiKey::Check::Value
InImpl::ValueList::MultiKey::
checkKey(Opt::Environment& cEnvironment_,
		 const CheckArgument& cArgument_)
{
	return FOREACH(m_vecScalar,
				   _CheckOperand(cEnvironment_, cArgument_)).getVal();
}

// FUNCTION private
//	Predicate::InImpl::ValueList::MultiKey::generateKey -- 
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
InImpl::ValueList::MultiKey::
generateKey(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_)
{
	VECTOR<int> vecData;
	Opt::MapContainer(m_vecScalar, vecData,
					  boost::bind(&Interface::IScalar::generate,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pIterator_,
								  boost::ref(cArgument_)));
	return cProgram_.addVariable(vecData);
}

// FUNCTION private
//	Predicate::InImpl::ValueList::MultiKey::generateOperand -- 
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
InImpl::ValueList::MultiKey::
generateOperand(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_)
{
	Interface::IRelation::Size nCardinality = getOperand()->getCardinality(cEnvironment_);
	VECTOR<int> vecData(nCardinality);
	for (int i = 0; i < nCardinality; ++i) {
		VECTOR<Interface::IScalar*> vecRow;
		SIZE n = m_vecScalar.GETSIZE();
		if (getOperand()->getRow(cEnvironment_, i, vecRow) == false
			|| vecRow.GETSIZE() != n) {
			// this situation should be checked before
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		VECTOR<int> vecRowData;
		Opt::MapContainer(vecRow, vecRowData,
						  boost::bind(&Interface::IScalar::generate,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cProgram_),
									  pIterator_,
									  boost::ref(cArgument_)));
		vecData[i] = cProgram_.addVariable(vecRowData);
	}
	return cProgram_.addVariable(vecData);
}

/////////////////////////////////////////////////
//	Plan::Predicate::InImpl::Variable::Base

// FUNCTION public
//	Predicate::InImpl::Variable::Base::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::Variable::Base::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	explainKey(pEnvironment_, cExplain_);
	cExplain_.put(" in ");
	explainOperand(pEnvironment_, cExplain_);
}

// FUNCTION public
//	Predicate::InImpl::Variable::Base::generate -- 
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
InImpl::Variable::Base::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// YET
	_SYDNEY_THROW0(Exception::NotSupported);
}

/////////////////////////////////////////////////
//	Plan::Predicate::InImpl::Variable::SingleKey

// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
InImpl::Variable::SingleKey::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// check 1st operand's status
	Interface::IScalar::Check::Value iStatus =
		getOperand0()->check(cEnvironment_,
							 cArgument_);

	if (Interface::IScalar::Check::isOn(iStatus, Interface::IScalar::Check::NotYet)) {
		// operand can not be evaluated for now
		return this;
	}
	// get table candidate
	Candidate::Table* pCandidate =
		Scalar::Field::getCandidate(cEnvironment_,
									getOperand0(),
									cArgument_.m_pCandidate);
	if (pCandidate == 0) {
		// no index files
		return CheckedInterface::create(cEnvironment_,
										this);
	}

	Utility::FileSet cFile;
	cFile.add(Interface::IFile::create(cEnvironment_,
									   getOperand1()));

	return CheckedInterface::create(cEnvironment_,
									this,
									pCandidate,
									cFile);
}
		
// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::isNeedIndex -- 
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
InImpl::Variable::SingleKey::
isNeedIndex()
{
	// variable is regarded as an index
	return true;
}

// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::isArbitraryElement -- 
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
InImpl::Variable::SingleKey::
isArbitraryElement()
{
	return getOperand0()->isArbitraryElement();
}

// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::getUsedTable -- check used tables
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::Variable::SingleKey::
getUsedTable(Utility::RelationSet& cResult_)
{
	getOperand0()->getUsedTable(cResult_);
}

// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::getUsedField -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::FieldSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::Variable::SingleKey::
getUsedField(Utility::FieldSet& cResult_)
{
	getOperand0()->getUsedField(cResult_);
}

// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::getUnknownKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::Variable::SingleKey::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	getOperand0()->getUnknownKey(cEnvironment_,
								 cResult_);
}

// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::require -- 
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
InImpl::Variable::SingleKey::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	getOperand0()->require(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::retrieve -- 
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
InImpl::Variable::SingleKey::
retrieve(Opt::Environment& cEnvironment_)
{
	getOperand0()->retrieve(cEnvironment_);
}

// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::retrieve -- 
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
InImpl::Variable::SingleKey::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	getOperand0()->retrieve(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::use -- 
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
InImpl::Variable::SingleKey::
use(Opt::Environment& cEnvironment_,
	Interface::ICandidate* pCandidate_)
{
	getOperand0()->use(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::delay -- 
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
InImpl::Variable::SingleKey::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	return getOperand0()->delay(cEnvironment_, pCandidate_, cArgument_);
}


// FUNCTION public
//	Predicate::InImpl::Variable::SingleKey::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
InImpl::Variable::SingleKey::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	getOperandi(0)->setParameter(cEnvironment_,
								 cProgram_,
								 pIterator_,
								 cExec_,
								 cArgument_);

	cExec_.append(" in (");
	
	getOperandi(1)->setParameter(cEnvironment_,
								 cProgram_,
								 pIterator_,
								 cExec_,
								 cArgument_);
	cExec_.append(")");
}


// FUNCTION private
//	Predicate::InImpl::Variable::SingleKey::explainKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::Variable::SingleKey::
explainKey(Opt::Environment* pEnvironment_,
		   Opt::Explain& cExplain_)
{
	getOperand0()->explain(pEnvironment_, cExplain_);
}

// FUNCTION private
//	Predicate::InImpl::Variable::SingleKey::explainOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InImpl::Variable::SingleKey::
explainOperand(Opt::Environment* pEnvironment_,
			   Opt::Explain& cExplain_)
{
	getOperand1()->explain(pEnvironment_, cExplain_);
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END
//
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
