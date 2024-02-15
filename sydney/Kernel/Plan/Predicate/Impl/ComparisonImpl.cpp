// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/ComparisonImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/Impl/ComparisonImpl.h"

#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/Predicate/Fetch.h"
#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IRow.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Sql/Argument.h"
#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Monadic.h"

#include "Common/Assert.h"

#include "Exception/NotComparable.h"
#include "Exception/NotSupported.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Predicate/Comparison.h"

#include "LogicalFile/Estimate.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"
#include "Opt/Explain.h"


#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	// FUNCTION local
	//	$$$::_getReverseType -- get reversed type
	//
	// NOTES
	//
	// ARGUMENTS
	//	Tree::Node::Type eType_
	//	
	// RETURN
	//	Tree::Node::Type
	//
	// EXCEPTIONS

	Tree::Node::Type
	_getReverseType(Tree::Node::Type eType_)
	{
		switch (eType_)
		{
		case Tree::Node::Equals:			return Tree::Node::Equals;
		case Tree::Node::LessThanEquals:	return Tree::Node::GreaterThanEquals;
		case Tree::Node::GreaterThanEquals:	return Tree::Node::LessThanEquals;
		case Tree::Node::LessThan:			return Tree::Node::GreaterThan;
		case Tree::Node::GreaterThan:		return Tree::Node::LessThan;
		case Tree::Node::NotEquals:			return Tree::Node::NotEquals;
		default:							break;
		}
		return eType_;
	}

	// FUNCTION local
	//	$$$::_getNotType -- get type eliminating NOT
	//
	// NOTES
	//
	// ARGUMENTS
	//	Tree::Node::Type eType_
	//	
	// RETURN
	//	Tree::Node::Type
	//
	// EXCEPTIONS

	Tree::Node::Type
	_getNotType(Tree::Node::Type eType_)
	{
		switch (eType_)
		{
		case Tree::Node::Equals:			return Tree::Node::NotEquals;
		case Tree::Node::LessThanEquals:	return Tree::Node::GreaterThan;
		case Tree::Node::GreaterThanEquals:	return Tree::Node::LessThan;
		case Tree::Node::LessThan:			return Tree::Node::GreaterThanEquals;
		case Tree::Node::GreaterThan:		return Tree::Node::LessThanEquals;
		case Tree::Node::NotEquals:			return Tree::Node::Equals;
		case Tree::Node::EqualsToNull:		return Tree::Node::NotNull;
		case Tree::Node::NotNull:			return Tree::Node::EqualsToNull;
		default:							break;
		}
		return eType_;
	}

	// FUNCTION local
	//	$$$::_getExecutionType -- get execution type
	//
	// NOTES
	//
	// ARGUMENTS
	//	Tree::Node::Type eType_
	//	
	// RETURN
	//	Execution::Predicate::Comparison::Type::Value
	//
	// EXCEPTIONS

	Execution::Predicate::Comparison::Type::Value
	_getExecutionType(Tree::Node::Type eType_)
	{
		switch (eType_)
		{
#define _CASE(_n_, _e_) case Tree::Node::_n_: return Execution::Predicate::Comparison::Type::_e_
		_CASE(Equals, Equals);
		_CASE(LessThanEquals, LessThanEquals);
		_CASE(GreaterThanEquals, GreaterThanEquals);
		_CASE(LessThan, LessThan);
		_CASE(GreaterThan, GreaterThan);
		_CASE(NotEquals, NotEquals);
		_CASE(EqualsToNull, IsNull);
		_CASE(NotNull, IsNotNull);
#undef _CASE
		default:
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}

	// CLASS local
	//	$$$::_CheckOperand -- function class for check
	//
	// NOTES

	class _CheckOperand
	{
	public:
		struct Result
		{
			enum Value
			{
				Constant,				// both operands are constant
				NotEvaluated,			// either operand can not be evaulated
				Referred0,				// col <op> val
				Referred1,				// val <op> col
				BothReferred,			// col <op> col
				Fetch0,					// col <op> col2
				Fetch1,					// col2 <op> col
				Calculated0,			// func(col, col2) <op> val
				Calculated1,			// val <op> func(col, col2)
				BothCalculated,			// func(col, col2) <op> func(col, col2)
				ValueNum
			};
		};

		// constructor
		_CheckOperand(Opt::Environment& cEnvironment_,
					  const Scalar::CheckArgument& cArgument_)
			: m_cEnvironment(cEnvironment_),
			  m_cArgument(cArgument_),
			  m_eResult(Result::Constant)
		{}

		// calculate for one element
		void operator()(Interface::IScalar* pOperand_,
						int iPosition_);

		// get result
		Result::Value getVal() {return m_eResult;}

	protected:
	private:
		Opt::Environment& m_cEnvironment;
		const Scalar::CheckArgument& m_cArgument;
		Result::Value m_eResult;
	};
	

	enum Operator {
		Equals,
		LessThanEquals,
		GreaterThanEquals,
		LessThan,
		GreaterThan,
		NotEquals,
		EqualsToNull,
		NotNull
	};
	
	const char* const _pszOperatorName[] = 
	{
		" = ",
		" <= ",
		" >= ",
		" < ",
		" > ",
		" != ",
		" is null",
		" is not null"
	};	
		
	// FUNCTION local
	//	$$$::_getOperatorName -- get operator name for explain
	//
	// NOTES
	//
	// ARGUMENTS
	//	Tree::Node::Type eType_
	//	
	// RETURN
	//	const char*
	//
	// EXCEPTIONS
	const char*
	_getOperatorName(Tree::Node::Type eType_)
	{
		switch (eType_)
		{
		case Tree::Node::Equals:			return _pszOperatorName[Equals];
		case Tree::Node::LessThanEquals:	return _pszOperatorName[LessThanEquals];
		case Tree::Node::GreaterThanEquals:	return _pszOperatorName[GreaterThanEquals];
		case Tree::Node::LessThan:			return _pszOperatorName[LessThan];
		case Tree::Node::GreaterThan:		return _pszOperatorName[GreaterThan];
		case Tree::Node::NotEquals:			return _pszOperatorName[NotEquals];
		case Tree::Node::EqualsToNull:		return _pszOperatorName[EqualsToNull];
		case Tree::Node::NotNull:			return _pszOperatorName[NotNull];
		default:							break;
		}
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

////////////////////////////////////
//	$$$::_CheckOperand

// FUNCTION public
//	$$$::_CheckOperand::operator() -- calculate for one element
//
// NOTES
//
// ARGUMENTS
//	Interface::IScalar* pOperand_
//	int iPosition_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_CheckOperand::
operator()(Interface::IScalar* pOperand_,
		   int iPosition_)
{
	Interface::IScalar::Check::Value iStatus =
		pOperand_->check(m_cEnvironment,
						 m_cArgument);

	// create temporary result value for this operand
	struct TempResult
	{
		enum Value
		{
			Constant = 0,
			NotEvaluated,
			Referred,
			Preceding,
			Calculated,
			ValueNum
		};
	};

	TempResult::Value eTmpResult = TempResult::Constant;

	if (Interface::IScalar::Check::isOn(iStatus, Interface::IScalar::Check::NotYet)) {
		// operand can not be evaluated for now
		// -> add referred columns as required
		if (Interface::IScalar::Check::isOn(iStatus, Interface::IScalar::Check::Referred)) {
			pOperand_->require(m_cEnvironment,
							   m_cArgument.m_pCandidate);
		}
		eTmpResult = TempResult::NotEvaluated;

	} else if (Interface::IScalar::Check::isOn(iStatus, Interface::IScalar::Check::Calculated)) {
		eTmpResult = TempResult::Calculated;

	} else if (Interface::IScalar::Check::isOn(iStatus, Interface::IScalar::Check::Preceding)) {
		eTmpResult = TempResult::Preceding;

	} else if (Interface::IScalar::Check::isOn(iStatus, Interface::IScalar::Check::Referred)) {
		eTmpResult = TempResult::Referred;

	} else {
		; _SYDNEY_ASSERT(iStatus == Interface::IScalar::Check::Constant);
		; _SYDNEY_ASSERT(eTmpResult == TempResult::Constant);
	}

	// create (temporary) result using transfer table
	const Result::Value mergeTable[][TempResult::ValueNum][Result::ValueNum] =
	{
#define C Result::Constant
#define N Result::NotEvaluated
#define R0 Result::Referred0
#define R1 Result::Referred1
#define RB Result::BothReferred
#define F0 Result::Fetch0
#define F1 Result::Fetch1
#define C0 Result::Calculated0
#define C1 Result::Calculated1
#define CB Result::BothCalculated
#define xx Result::ValueNum
		// for operand0
		{
		// C   N  R0  R1  RB  F0  F1  C0  C1  CB    result/tmp
		{  C, xx, xx, xx, xx, xx, xx, xx, xx, xx},	// Constant
		{  N, xx, xx, xx, xx, xx, xx, xx, xx, xx},	// NotEvaluated
		{ R0, xx, xx, xx, xx, xx, xx, xx, xx, xx},	// Referred
		{ F1, xx, xx, xx, xx, xx, xx, xx, xx, xx},	// Preceding
		{ C0, xx, xx, xx, xx, xx, xx, xx, xx, xx}	// Calculated
		},
		// for operand1
		{
		// C   N  R0  R1  RB  F0  F1  C0  C1  CB    result/tmp
		{  C,  N, R0, xx, xx, xx, C0, C0, xx, xx},	// Constant
		{  N,  N,  N, xx, xx, xx,  N,  N, xx, xx},	// NotEvaluated
		{ R1,  N, RB, xx, xx, xx, F1, CB, xx, xx},	// Referred
		{ C1,  N, F0, xx, xx, xx, CB, CB, xx, xx},	// Preceding
		{ C1,  N, CB, xx, xx, xx, CB, CB, xx, xx}	// Calculated
		}
#undef C
#undef N
#undef R0
#undef R1
#undef RB
#undef F0
#undef F1
#undef C0
#undef C1
#undef CB
#undef xx
	};

	m_eResult = mergeTable[iPosition_][eTmpResult][m_eResult];
}

//////////////////////////////////////////////////
//	Plan::Predicate::Impl::DyadicComparison

// FUNCTION public
//	Predicate::Impl::DyadicComparison::convertNot -- try to eliminate NOT if the interface P is used in the form of NOT(P)
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
Impl::DyadicComparison::
convertNot(Opt::Environment& cEnvironment_)
{
	// equal operator should not convert into not-equal
	// so that index can be used
	if (getType() == Tree::Node::Equals) return 0;

	// if left operand is arbitrary element specification,
	// it can not convert into not expression.
	if (isArbitraryElement()) return 0;

	return Comparison::create(cEnvironment_,
							  _getNotType(getType()),
							  MAKEPAIR(getOperand0(),
									   getOperand1()),
							  false /* no need to check comparability */);
}

// FUNCTION public
//	Predicate::Impl::DyadicComparison::check -- 
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
Impl::DyadicComparison::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	Tree::Node::Type eOperation = Tree::Node::Undefined;
	Interface::IScalar* pOperand0 = 0;
	Interface::IScalar* pOperand1 = 0;

	_CheckOperand::Result::Value eCheckResult =
		foreachOperand_i(_CheckOperand(cEnvironment_,
									   cArgument_)).getVal();

	switch (eCheckResult) {
	default:
	case _CheckOperand::Result::Constant:
		{
			break;
		}
	case _CheckOperand::Result::NotEvaluated:
		{
			// can not be evaluated for now
			require(cEnvironment_,
					cArgument_.m_pCandidate);
			return this;
		}
	case _CheckOperand::Result::Referred0:
		{
			// col <op> val
			pOperand0 = getOperand0();
			pOperand1 = getOperand1();
			eOperation = getType();
			break;
		}
	case _CheckOperand::Result::Referred1:
		{
			// val <op> col
			// reverse predicate can be considered only when operands are comparable
			if (Scalar::DataType::isComparable(getOperand0()->getDataType(),
											   getOperand1()->getDataType())) {
				pOperand0 = getOperand1();
				pOperand1 = getOperand0();
				eOperation = _getReverseType(getType());
			}
			break;
		}
	case _CheckOperand::Result::BothReferred:
		{
			// col <op> col
			// -> can't use index
			break;
		}
	case _CheckOperand::Result::Fetch0:
		{
			// col = col2
			// right operand is calculated before
			if (getType() == Tree::Node::Equals) {
				pOperand0 = getOperand0();
				pOperand1 = getOperand1();
				eOperation = Tree::Node::Fetch;
			}
			break;
		}
	case _CheckOperand::Result::Fetch1:
		{
			// col2 = col
			// reverse predicate can be considered only when operands are comparable
			if (getType() == Tree::Node::Equals
				&& Scalar::DataType::isComparable(getOperand0()->getDataType(),
												  getOperand1()->getDataType())) {
				pOperand0 = getOperand1();
				pOperand1 = getOperand0();
				eOperation = Tree::Node::Fetch;
			}
			break;
		}
	case _CheckOperand::Result::Calculated0:
	case _CheckOperand::Result::Calculated1:
		// These patterns might be converted into Fetch.
		// For now, such conversion is not done
	case _CheckOperand::Result::BothCalculated:
		{
			// fuunc(col, col2) <op> func(col, col2)
			break;
		}
	}

	Utility::FileSet cFile;
	Candidate::Table* pCandidate = 0;
	Interface::IPredicate* pResult = this;

	switch (eOperation) {
	case Tree::Node::Undefined:
		{
			// no related columns -> add all the columns as required
			require(cEnvironment_,
					cArgument_.m_pCandidate);
			// create checkedinterface using 'this'
			break;
		}
	case Tree::Node::Fetch:
		{
			// get fetchable files for operand0
			if (/* pOperand1->isArbitraryElement()
				   || */!Scalar::Field::getFetchFile(cEnvironment_,
												Scalar::GetFileArgument(
													pOperand0,
													this,
													cFile))) {
				// can't fetch
				pOperand0->require(cEnvironment_,
								   cArgument_.m_pCandidate);

			} else {
				// get table candidate
				pCandidate = Scalar::Field::getCandidate(cEnvironment_,
														 pOperand0,
														 cArgument_.m_pCandidate);
				if (pCandidate == 0) {
					// can't fetch
					pOperand0->require(cEnvironment_,
									   cArgument_.m_pCandidate);
				} else {
					// wrapped predicate become fetch node
					pResult = Fetch::create(cEnvironment_,
											this,
											MAKEPAIR(pOperand0,
													 pOperand1));
				}
			}
			break;
		}
	default:
		{
			This cTmp(eOperation,
					  MAKEPAIR(pOperand0,
							   pOperand1));
			// get searchable files for operand0
			if (!Scalar::Field::getSearchFile(cEnvironment_,
											  Scalar::GetFileArgument(
												  pOperand0,
												  &cTmp,
												  cFile))) {
				// get column as required
				// [NOTES]
				// pOperand1 has been known as no column
				pOperand0->require(cEnvironment_,
								   cArgument_.m_pCandidate);
			} else {
				// get table candidate
				pCandidate = Scalar::Field::getCandidate(cEnvironment_,
														 pOperand0,
														 cArgument_.m_pCandidate);
				if (pCandidate == 0) {
					// can't search by index
					pOperand0->require(cEnvironment_,
									   cArgument_.m_pCandidate);
				}
			}
			break;
		}
	}

	return CheckedInterface::create(cEnvironment_,
									pResult,
									pCandidate,
									cFile);
}

// FUNCTION public
//	Predicate::Impl::DyadicComparison::createFetch -- 
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
Impl::DyadicComparison::
createFetch(Opt::Environment& cEnvironment_,
			Utility::ScalarSet& cFetchKey_)
{
	if (getType() == Tree::Node::Equals
		&& Scalar::DataType::isComparable(getOperand0()->getDataType(),
										  getOperand1()->getDataType())) {
		// createFetch is called for col <op> val or val <op> col form
		Interface::IScalar* pOperand0 = getOperand0();
		Interface::IScalar* pOperand1 = getOperand1();
		if (getOperand1()->isField()) {
			SWAP(pOperand0, pOperand1);
		}
		if (cFetchKey_.isContaining(pOperand0) == false) {
			cFetchKey_.add(pOperand0);
			return Fetch::create(cEnvironment_,
								 this,
								 MAKEPAIR(pOperand0,
										  pOperand1));
		}
	}
	return 0;
}

// FUNCTION public
//	Predicate::Impl::DyadicComparison::estimateCost -- 
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
Impl::DyadicComparison::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cResult_)
{
	// estimate selection rate by operator
	// it can be assumed there are no index for the column

	double dblValueCount;		// value variation number

	switch (getType()) {
	case Tree::Node::Equals:
		{
			dblValueCount = 20;
			break;
		}
	case Tree::Node::LessThanEquals:
	case Tree::Node::GreaterThanEquals:
	case Tree::Node::LessThan:
	case Tree::Node::GreaterThan:
	case Tree::Node::NotEquals:
		{
			dblValueCount = 2;
			break;
		}
	default:
		{
			dblValueCount = 2;
			break;
		}
	}

	double dblSize = getOperand0()->getDataType().getDataSize()
		+ getOperand1()->getDataType().getDataSize();
	double dblRetrieveCost = dblSize / LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::Memory);

	cResult_.setOverhead(0);
	cResult_.setTotalCost(0);
	cResult_.setRetrieveCost(dblRetrieveCost);
	if (cResult_.isSetRate()) {
		cResult_.setRate(1.0 / dblValueCount);
	}
	if (cResult_.isSetCount()) {
		AccessPlan::Cost::Value cCount = cResult_.getTableCount();
		if (cCount.isInfinity()) {
			// use default
			cCount = 100000;
		}
		cResult_.setTupleCount(cCount / dblValueCount);
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		stream << "Estimate rate: " << cResult_.getRate();
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif
	return true;
}

// FUNCTION public
//	Predicate::Impl::DyadicComparison::isArbitraryElement -- 
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
Impl::DyadicComparison::
isArbitraryElement()
{
	return getOperand0()->isArbitraryElement();
}

// FUNCTION public
//	Predicate::Impl::DyadicComparison::explain -- 
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
Impl::DyadicComparison::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	getOperand0()->explain(pEnvironment_, cExplain_);
	cExplain_.put(_getOperatorName(getType()));
	cExplain_.popNoNewLine();
	getOperand1()->explain(pEnvironment_, cExplain_);
}


// FUNCTION public
//	Predicate::Impl::DyadicComparison::toSQLStatement -- 
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
STRING
Impl::DyadicComparison::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << getOperandi(0)->toSQLStatement(cEnvironment_, cArgument_);
	cStream << _getOperatorName(getType());
	cStream << getOperandi(1)->toSQLStatement(cEnvironment_, cArgument_);
	return cStream.getString();
}


// FUNCTION public
//	Predicate::Impl::DyadicComparison::toSQLStatement -- 
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
Impl::DyadicComparison::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	getOperand0()->setParameter(cEnvironment_,
								cProgram_,
								pIterator_,
								cExec_,
								cArgument_);
	cExec_.append(_getOperatorName(getType()));
	getOperand1()->setParameter(cEnvironment_,
								cProgram_,
								pIterator_,
								cExec_,
								cArgument_);
}



// FUNCTION public
//	Predicate::Impl::DyadicComparison::generate -- 
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
Impl::DyadicComparison::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_bGenerateForPredicate = true;	
	int iData0 = getOperand0()->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);
	int iData1 = getOperand1()->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);

	Execution::Predicate::Comparison* pResult =
		isArbitraryElement() ?
		Execution::Predicate::Comparison::AnyElement::create(
										 cProgram_,
										 pIterator_,
										 iData0,
										 iData1,
										 _getExecutionType(getType()))
		: Execution::Predicate::Comparison::create(
									   cProgram_,
									   pIterator_,
									   iData0,
									   iData1,
									   _getExecutionType(getType()));
	return pResult->getID();
}

//////////////////////////////////////////////////
//	Plan::Predicate::Impl::DyadicRowComparison

// FUNCTION public
//	Predicate::Impl::DyadicRowComparison::convertNot --
//		try to eliminate NOT if the interface P is used in the form of NOT(P)
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
Impl::DyadicRowComparison::
convertNot(Opt::Environment& cEnvironment_)
{
	// equal operator should not convert into not-equal
	// so that index can be used
	if (getType() == Tree::Node::Equals) return 0;

	// if left operand is arbitrary element specification,
	// it can not convert into not expression.
	if (isArbitraryElement()) return 0;

	return Comparison::create(cEnvironment_,
							  _getNotType(getType()),
							  MAKEPAIR(getOperand0(),
									   getOperand1()));
}

// FUNCTION public
//	Predicate::Impl::DyadicRowComparison::isArbitraryElement -- 
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
Impl::DyadicRowComparison::
isArbitraryElement()
{
	return getOperand0()->isAny(boost::bind(&Interface::IScalar::isArbitraryElement,
											_1));
}

// FUNCTION public
//	Predicate::Impl::DyadicRowComparison::explain -- 
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
Impl::DyadicRowComparison::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	getOperand0()->explain(pEnvironment_, cExplain_);
	cExplain_.put(_getOperatorName(getType()));
	cExplain_.popNoNewLine();
	getOperand1()->explain(pEnvironment_, cExplain_);
}


// FUNCTION public
//	Predicate::Impl::DyadicRowComparison::toSQLStatement -- 
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
STRING
Impl::DyadicRowComparison::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << getOperand0()->toSQLStatement(cEnvironment_, cArgument_);
	cStream << _getOperatorName(getType());
	cStream << getOperand1()->toSQLStatement(cEnvironment_, cArgument_);
	return cStream.getString();
}



// FUNCTION public
//	Predicate::Impl::DyadicRowComparison::generate -- 
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
Impl::DyadicRowComparison::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_bGenerateForPredicate = true;
	
	if (isArbitraryElement()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Interface::IRow::MapResult<int> vecData0;
	Interface::IRow::MapResult<int> vecData1;
	getOperand0()->mapOperand(vecData0,
							  boost::bind(&Interface::IScalar::generate,
										  _1,
										  boost::ref(cEnvironment_),
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cMyArgument)));
	getOperand1()->mapOperand(vecData1,
							  boost::bind(&Interface::IScalar::generate,
										  _1,
										  boost::ref(cEnvironment_),
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cMyArgument)));

	Execution::Predicate::Comparison* pResult =
		Execution::Predicate::Comparison::Row::create(
									   cProgram_,
									   pIterator_,
									   vecData0,
									   vecData1,
									   _getExecutionType(getType()));
	return pResult->getID();
}

//////////////////////////////////////////////////
//	Plan::Predicate::Impl::MonadicComparison

// FUNCTION public
//	Predicate::Impl::MonadicComparison::convertNot -- try to eliminate NOT if the interface P is used in the form of NOT(P)
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
Impl::MonadicComparison::
convertNot(Opt::Environment& cEnvironment_)
{
	// if operand is arbitrary element specification,
	// it can not convert into not expression.
	if (isArbitraryElement()) return 0;

	return Comparison::create(cEnvironment_,
							  _getNotType(getType()),
							  getOperand());
}

// FUNCTION public
//	Predicate::Impl::MonadicComparison::check -- 
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
Impl::MonadicComparison::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	Interface::IScalar::Check::Value iStatus =
		getOperand()->check(cEnvironment_,
							cArgument_);

	if (Interface::IScalar::Check::isOn(iStatus, Interface::IScalar::Check::NotYet)) {
		// not evaluated for now
		return this;
	}

	Utility::FileSet cFile;
	Candidate::Table* pCandidate = 0;

	if (Interface::IScalar::Check::isOn(iStatus, Interface::IScalar::Check::Referred)) {
		// check for index files

		// get searchable files for operand
		if (getOperand()->getType() != Tree::Node::Field
			|| 
			!Scalar::Field::getSearchFile(cEnvironment_,
										  Scalar::GetFileArgument(
											  getOperand(),
											  this,
											  cFile))) {
			// no index can be used
			// -> get column as required
			getOperand()->require(cEnvironment_,
								  cArgument_.m_pCandidate);
		} else {
			// get table candidate
			pCandidate = Scalar::Field::getCandidate(cEnvironment_,
													 getOperand(),
													 cArgument_.m_pCandidate);
			if (pCandidate == 0) {
				// no index can be used
				getOperand()->require(cEnvironment_,
									  cArgument_.m_pCandidate);
			}
		}
	}

	return CheckedInterface::create(cEnvironment_,
									this,
									pCandidate,
									cFile);
}

// FUNCTION public
//	Predicate::Impl::MonadicComparison::estimateCost -- 
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
Impl::MonadicComparison::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cResult_)
{
	// estimate selection rate by operator
	// it can be assumed there are no index for the column

	double dblValueCount;		// value variation number

	switch (getType()) {
	case Tree::Node::EqualsToNull:
		{
			dblValueCount = 100;
			break;
		}
	case Tree::Node::NotNull:
		{
			dblValueCount = 2;
			break;
		}
	default:
		{
			dblValueCount = 2;
			break;
		}
	}
	double dblSize = getOperand()->getDataType().getDataSize();
	double dblRetrieveCost = dblSize / LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::Memory);

	cResult_.setOverhead(0);
	cResult_.setTotalCost(0);
	cResult_.setRetrieveCost(dblRetrieveCost);
	if (cResult_.isSetRate()) {
		cResult_.setRate(1.0 / dblValueCount);
	}
	if (cResult_.isSetCount()) {
		AccessPlan::Cost::Value cCount = cResult_.getTableCount();
		if (cCount.isInfinity()) {
			// use default
			cCount = 100000;
		}
		cResult_.setTupleCount(cCount / dblValueCount);
	}
	return true;
}

// FUNCTION public
//	Predicate::Impl::MonadicComparison::isArbitraryElement -- 
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
Impl::MonadicComparison::
isArbitraryElement()
{
	return getOperand()->isArbitraryElement();
}

// FUNCTION public
//	Predicate::Impl::MonadicComparison::getUnknownKey -- 
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
Impl::MonadicComparison::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	Super::getUnknownKey(cEnvironment_,
						 cResult_);

	cResult_.m_bArray = isArbitraryElement();
}

// FUNCTION public
//	Predicate::Impl::MonadicComparison::explain -- 
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
Impl::MonadicComparison::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	getOperand()->explain(pEnvironment_, cExplain_);
	cExplain_.put(_getOperatorName(getType()));
	cExplain_.popNoNewLine();
}


// FUNCTION public
//	Predicate::Impl::DyadicComparison::toSQLStatement -- 
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
STRING
Impl::MonadicComparison::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << getOperand()->toSQLStatement(cEnvironment_, cArgument_) << ' ';
	cStream << _getOperatorName(getType());
	return cStream.getString();	

}



// FUNCTION public
//	Predicate::Impl::MonadicComparison::generate -- 
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
Impl::MonadicComparison::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	int iData = getOperand()->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);

	Execution::Predicate::Comparison* pResult =
		isArbitraryElement() ?
		Execution::Predicate::Comparison::AnyElement::create(
											 cProgram_,
											 pIterator_,
											 iData,
											 _getExecutionType(getType()))
		: Execution::Predicate::Comparison::create(
										   cProgram_,
										   pIterator_,
										   iData,
										   _getExecutionType(getType()));
	return pResult->getID();
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
