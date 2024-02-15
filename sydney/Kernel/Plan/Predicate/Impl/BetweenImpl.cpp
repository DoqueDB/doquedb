// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/BetweenImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Plan/Candidate/Argument.h"
#include "Plan/Predicate/Impl/BetweenImpl.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/Predicate/Combinator.h"
#include "Plan/Predicate/Comparison.h"

#include "Common/Assert.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Execution/Predicate/Between.h"

#include "Opt/Explain.h"

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

	// operator names for explain
	const char* const _pszOperatorName[] = 
	{
		" between ",
		" not between "
	};
}

/////////////////////////////////////////////
//	Plan::Predicate::Impl::BetweenImpl

// FUNCTION public
//	Predicate::Impl::BetweenImpl::rewrite -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	Predicate::RewriteArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate::RewriteResult
//
// EXCEPTIONS

//virtual
Interface::IPredicate::RewriteResult
Impl::BetweenImpl::
rewrite(Opt::Environment& cEnvironment_,
		Interface::IRelation* pRelation_,
		Predicate::RewriteArgument& cArgument_)
{
	Interface::IPredicate* pPredicate = 0;
	if (isArbitraryElement()) {
		// can't convert
		pPredicate = this;
	} else {
		// convert a between b and c => a >= b and a <= c
		// convert a not between b and c => a < b or a > c
		Interface::IPredicate* pElement0 =
			Predicate::Comparison::create(cEnvironment_,
										  isNot() ? Tree::Node::LessThan
										  : Tree::Node::GreaterThanEquals,
										  MAKEPAIR(getOperandi(0),
												   getOperandi(1)),
										  false /* comparability has been checked */);
		Interface::IPredicate* pElement1 =
			Predicate::Comparison::create(cEnvironment_,
										  isNot() ? Tree::Node::GreaterThan
										  : Tree::Node::LessThanEquals,
										  MAKEPAIR(getOperandi(0),
												   getOperandi(2)),
										  false /* comparability has been checked */);
		pPredicate = Predicate::Combinator::create(cEnvironment_,
												   isNot() ? Tree::Node::Or : Tree::Node::And,
												   MAKEPAIR(pElement0, pElement1));
	}
	return Interface::IPredicate::RewriteResult(pRelation_, pPredicate);
}

// FUNCTION public
//	Predicate::Impl::BetweenImpl::check -- 
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
Impl::BetweenImpl::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	Check::Value cResult =
		foreachOperand(_CheckOperand(cEnvironment_,
									 cArgument_)).getVal();

	if (Check::isOn(cResult, Check::NotYet)) {
		// this predicate can not be checked now
		require(cEnvironment_,
				cArgument_.m_pCandidate);
		return this;
	}

	// no index files to process between directly
	return CheckedInterface::create(cEnvironment_,
									this);
}

// FUNCTION public
//	Predicate::Impl::BetweenImpl::isArbitraryElement -- 
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
Impl::BetweenImpl::
isArbitraryElement()
{
	return getOperandi(0)->isArbitraryElement();
}

// FUNCTION public
//	Predicate::Impl::BetweenImpl::explain -- 
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
Impl::BetweenImpl::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	getOperandi(0)->explain(pEnvironment_, cExplain_);
	cExplain_.put(_pszOperatorName[isNot()?1:0]);
	cExplain_.popNoNewLine();
	getOperandi(1)->explain(pEnvironment_, cExplain_);
	cExplain_.pushNoNewLine();
	cExplain_.put(" and ");
	getOperandi(2)->explain(pEnvironment_, cExplain_);
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Predicate::Impl::BetweenImpl::generate -- 
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
Impl::BetweenImpl::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_bGenerateForPredicate = true;
	int iData0 = getOperandi(0)->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);
	int iData1 = getOperandi(1)->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);
	int iData2 = getOperandi(2)->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);

	Execution::Predicate::Between* pResult =
		isArbitraryElement() ?
		Execution::Predicate::Between::AnyElement::create(
										 cProgram_,
										 pIterator_,
										 iData0,
										 iData1,
										 iData2,
										 isNot())
		: Execution::Predicate::Between::create(
									   cProgram_,
									   pIterator_,
									   iData0,
									   iData1,
									   iData2,
									   isNot());
	return pResult->getID();
}


// FUNCTION public
//	Predicate::Impl::BetweenImpl::toSQLStatement -- 
//
// NOTES
//
// ARGUMENTS
//	const Plan::Sql::QueryArgument& cArgument_
//
// RETURN
//	STRING
//
// EXCEPTIONS

//virtual
STRING
Impl::BetweenImpl::
toSQLStatement(Opt::Environment& cEnvironment_, const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << getOperandi(0)->toSQLStatement(cEnvironment_, cArgument_) << ' ';
	cStream << getOperandi(1)->toSQLStatement(cEnvironment_, cArgument_) <<
		"and" << getOperandi(2)->toSQLStatement(cEnvironment_, cArgument_);
	return cStream.getString();
}


// FUNCTION public
//	Predicate::Impl::BetweenImpl::setParameter -- 
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
Impl::BetweenImpl::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	getOperandi(0)->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument_);
	cExec_.append(' ');
	cExec_.append(_pszOperatorName[isNot()?1:0]);
	getOperandi(1)->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument_);
	cExec_.append(" and ");
	getOperandi(2)->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument_);
}


_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
