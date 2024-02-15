// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/NeighborInImpl.cpp --
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
const char moduleName[] = "Plan::Predicate::Impl";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/Impl/NeighborInImpl.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "Exception/SpatialIndexNeeded.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"
#include "Opt/Explain.h"

#include "Schema/Column.h"
#include "Schema/File.h"
#include "Schema/Index.h"
#include "Schema/Key.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	const char* const _pszOperatorName = " in ";
}

/////////////////////////////////////////////
//	Plan::Predicate::NeighborInImpl::Nadic

// FUNCTION public
//	Predicate::NeighborInImpl::Nadic::check -- 
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
NeighborInImpl::Nadic::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// if right operand or options are using non-constant, not supported
	if (!Opt::IsAll(begin() + 1, end(),
					boost::bind(&Interface::IScalar::isRefering,
								_1,
								static_cast<Interface::IRelation*>(0)))) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// check 1st operand's status
	Interface::IScalar* pOperand0 = getOperandi(0);
	Interface::IScalar::Check::Value iStatus =
		pOperand0->check(cEnvironment_,
						 cArgument_);

	if (Interface::IScalar::Check::isOn(iStatus, Interface::IScalar::Check::NotYet)) {
		// operand can not be evaluated for now
		return this;
	}

	// get searchable files for operand0
	Utility::FileSet cFile;
	Candidate::Table* pCandidate = 0;
	switch (pOperand0->getType()) {
	case Tree::Node::Field:
		if (!Scalar::Field::getSearchFile(cEnvironment_,
										  Scalar::GetFileArgument(
											  pOperand0,
											  this,
											  cFile))) {
			_SYDNEY_THROW0(Exception::SpatialIndexNeeded);
		}
		// get table candidate
		pCandidate = Scalar::Field::getCandidate(cEnvironment_,
												 pOperand0,
												 cArgument_.m_pCandidate);
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	if (pCandidate == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return CheckedInterface::create(cEnvironment_,
									this,
									pCandidate,
									cFile);
}

// FUNCTION public
//	Predicate::NeighborInImpl::Nadic::explain -- 
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
NeighborInImpl::Nadic::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	explainKey(pEnvironment_, cExplain_);
	cExplain_.put(_pszOperatorName);
	cExplain_.popNoNewLine();
	explainOperand(pEnvironment_, cExplain_);
}

// FUNCTION public
//	Predicate::NeighborInImpl::Nadic::generate -- 
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
NeighborInImpl::Nadic::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::SpatialIndexNeeded);
}

// FUNCTION public
//	Predicate::NeighborInImpl::Nadic::toSQLStatement -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Plan::Sql::QueryArgument& cArgument_
//	
// RETURN
//	STRING
//
// EXCEPTIONS

//virtual
STRING
NeighborInImpl::Nadic::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << "neighbor(";
	cStream << getOperandi(0)->toSQLStatement(cEnvironment_, cArgument_);
	cStream << ")";
	cStream << _pszOperatorName;
	char delimiter = '(';
	ConstIterator iterator = begin() + 1;
	const ConstIterator last = end();
	for (; iterator != last; ++iterator, delimiter = ',') {
		cStream << delimiter
				<< (*iterator)->toSQLStatement(cEnvironment_, cArgument_);
	}
	return cStream.getString();
}

// FUNCTION protected
//	Predicate::NeighborInImpl::Nadic::explainKey -- 
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
NeighborInImpl::Nadic::
explainKey(Opt::Environment* pEnvironment_,
		   Opt::Explain& cExplain_)
{
	cExplain_.put("neighbor(");
	getOperandi(0)->explain(pEnvironment_, cExplain_);
	cExplain_.put(")");
}

// FUNCTION protected
//	Predicate::NeighborInImpl::Nadic::explainOperand -- 
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
NeighborInImpl::Nadic::
explainOperand(Opt::Environment* pEnvironment_,
			   Opt::Explain& cExplain_)
{
	cExplain_.put("(...)");
}

/////////////////////////////////////////////
//	Plan::Predicate::NeighborInImpl::NadicWithOption

// FUNCTION public
//	Predicate::NeighborInImpl::NadicWithOption::check -- 
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
NeighborInImpl::NadicWithOption::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// if right operand or options are using non-constant, not supported
	if (!getOption()->isRefering(0)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	return Super::check(cEnvironment_, cArgument_);
}

// FUNCTION public
//	Predicate::NeighborInImpl::NadicWithOption::toSQLStatement -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Plan::Sql::QueryArgument& cArgument_
//	
// RETURN
//	STRING
//
// EXCEPTIONS

//virtual
STRING
NeighborInImpl::NadicWithOption::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << "neighbor(";
	cStream << getOperandi(0)->toSQLStatement(cEnvironment_, cArgument_);
	cStream << " ";
	cStream << getOption()->toSQLStatement(cEnvironment_, cArgument_);
	cStream << ")";
	cStream << _pszOperatorName;
	char delimiter = '(';
	ConstIterator iterator = begin() + 1;
	const ConstIterator last = end();
	for (; iterator != last; ++iterator, delimiter = ',') {
		cStream << delimiter
				<< (*iterator)->toSQLStatement(cEnvironment_, cArgument_);
	}
	return cStream.getString();
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
