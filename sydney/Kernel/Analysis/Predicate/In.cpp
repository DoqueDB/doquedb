// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/In.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Predicate/In.h"

#include "Common/Assert.h"

#include "Exception/InvalidRowValue.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Predicate/In.h"
#include "Plan/Predicate/NeighborIn.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Utility/ObjectSet.h"

#include "Statement/Hint.h"
#include "Statement/InPredicate.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

namespace InImpl
{
	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::InImpl::Base --
	//
	// NOTES
	class Base
		: public Predicate::In
	{
	public:
		typedef Base This;
		typedef Predicate::In Super;

		// constructor
		Base() : Super() {}
		// destructor
		virtual ~Base() {}

		// generate Predicate from Statement::Object
		virtual Plan::Interface::IPredicate*
				getPredicate(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Statement::Object* pStatement_) const;
	protected:
	private:
		virtual bool isSubquery() const = 0;

		virtual Plan::Interface::IPredicate*
				createPredicate(Opt::Environment& cEnvironment_,
								const VECTOR<Plan::Interface::IScalar*>& vecScalar_,
								Plan::Interface::IRelation* pRelation_,
								const Plan::Utility::RelationSet& cOuterRelation_,
								Statement::ValueExpression* pVE_) const = 0;
	};

	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::InImpl::SubQuery --
	//
	// NOTES
	class SubQuery
		: public Base
	{
	public:
		typedef SubQuery This;
		typedef Base Super;

		// constructor
		SubQuery() : Super() {}
		// destructor
		virtual ~SubQuery() {}

	protected:
	private:
		virtual bool isSubquery() const;

		virtual Plan::Interface::IPredicate*
		createPredicate(Opt::Environment& cEnvironment_,
						const VECTOR<Plan::Interface::IScalar*>& vecScalar_,
						Plan::Interface::IRelation* pRelation_,
						const Plan::Utility::RelationSet& cOuterRelation_,
						Statement::ValueExpression* pVE_) const;
	};

	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::InImpl::ValueList --
	//
	// NOTES
	class ValueList
		: public Base
	{
	public:
		typedef ValueList This;
		typedef Base Super;

		// constructor
		ValueList() : Super() {}
		// destructor
		virtual ~ValueList() {}

	protected:
	private:
		virtual bool isSubquery() const;

		virtual Plan::Interface::IPredicate*
		createPredicate(Opt::Environment& cEnvironment_,
						const VECTOR<Plan::Interface::IScalar*>& vecScalar_,
						Plan::Interface::IRelation* pRelation_,
						const Plan::Utility::RelationSet& cOuterRelation_,
						Statement::ValueExpression* pVE_) const;
	};

	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::InImpl::Variable --
	//
	// NOTES
	class Variable
		: public Base
	{
	public:
		typedef Variable This;
		typedef Base Super;

		// constructor
		Variable() : Super() {}
		// destructor
		virtual ~Variable() {}

		////////////////////////////
		// Interface::IAnalyzer::
		////////////////////////////
		virtual Plan::Interface::IPredicate*
		getPredicate(Opt::Environment& cEnvironment_,
					 Plan::Interface::IRelation* pRelation_,
					 Statement::Object* pStatement_) const;

	protected:
	private:
		///////////
		// Base::
		///////////
		virtual bool isSubquery() const;
		virtual Plan::Interface::IPredicate*
		createPredicate(Opt::Environment& cEnvironment_,
						const VECTOR<Plan::Interface::IScalar*>& vecScalar_,
						Plan::Interface::IRelation* pRelation_,
						const Plan::Utility::RelationSet& cOuterRelation_,
						Statement::ValueExpression* pVE_) const;
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzerXXX -- instances
	//
	// NOTES
	const InImpl::SubQuery _analyzerSQ;
	const InImpl::ValueList _analyzerVL;
	const InImpl::Variable _analyzerV;
}

////////////////////////////////////////////
// Predicate::InImpl::Base

// FUNCTION public
//	Predicate::InImpl::Base::getPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
InImpl::Base::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);

	Plan::Interface::IPredicate* pResult = 0;
	STRING cName;
	if (isSubquery() == false) {
		cName = pVE->toSQLStatement();
		pResult = cEnvironment_.searchPredicate(cName);
		if (pResult) {
			return pResult;
		}
	}

	Statement::InPredicate* pIP =
		_SYDNEY_DYNAMIC_CAST(Statement::InPredicate*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pIP);

	Statement::ValueExpression* pLeft = pIP->getLeft();
	Statement::Object* pSubQuery = pIP->getRight();

	Plan::Interface::IRelation* pSubRelation = 0;
	Plan::Utility::RelationSet cOuterRelation;
	{
		// create new namescope
		Opt::Environment::AutoPop cAutoPop1 = cEnvironment_.pushNameScope();
		Opt::Environment::AutoPop cAutoPop2 =
			cEnvironment_.pushStatus(Opt::Environment::Status::Subquery
									 | Opt::Environment::Status::Reset);

		// convert subquery into relation node
		pSubRelation = pSubQuery->getAnalyzer2()->getRelation(cEnvironment_,
															  pSubQuery);
		cOuterRelation = cEnvironment_.getOuterRelation();
	}
	VECTOR<Plan::Interface::IScalar*> vecScalar;
	{
		// arbitrary array element is allowed in the left operand
		// only when predicant is not row
		Opt::Environment::Status::Value eStatus =
			(Opt::Environment::Status::ArbitraryElementAllowed
			 | Opt::Environment::Status::In);
		if (cEnvironment_.checkStatus(Opt::Environment::Status::NoTopPredicate) == false) {
			eStatus = eStatus | Opt::Environment::Status::KnownNotNull;
		}		
		Opt::Environment::AutoPop cAutoPop = cEnvironment_.pushStatus(eStatus);

		pLeft->getAnalyzer2()->addScalar(cEnvironment_,
										 pRelation_,
										 pLeft,
										 vecScalar);
	}

	if (vecScalar.GETSIZE() != pSubRelation->getDegree(cEnvironment_)) {
		_SYDNEY_THROW0(Exception::InvalidRowValue);
	}

	pResult = createPredicate(cEnvironment_,
							  vecScalar,
							  pSubRelation,
							  cOuterRelation,
							  pVE);
	if (isSubquery() == false
		&& pResult->hasParameter() == false) {
		cEnvironment_.addPredicate(pRelation_,
								   cName,
								   pResult);
	}
	return pResult;
}

////////////////////////////////////////////
// Predicate::InImpl::SubQuery

// FUNCTION private
//	Predicate::InImpl::SubQuery::isSubquery -- 
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
InImpl::SubQuery::
isSubquery() const
{
	return true;
}

// FUNCTION private
//	Predicate::InImpl::SubQuery::createPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Plan::Interface::IScalar*>& vecScalar_
//	Plan::Interface::IRelation* pRelation_
//	const Plan::Utility::RelationSet& cOuterRelation_
//	Statement::ValueExpression* pVE_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
InImpl::SubQuery::
createPredicate(Opt::Environment& cEnvironment_,
				const VECTOR<Plan::Interface::IScalar*>& vecScalar_,
				Plan::Interface::IRelation* pRelation_,
				const Plan::Utility::RelationSet& cOuterRelation_,
				Statement::ValueExpression* pVE_) const
{
	bool bIsNot = pVE_->getOperator() == Statement::ValueExpression::op_NotIn;
	Statement::InPredicate* pIP =
		_SYDNEY_DYNAMIC_CAST(Statement::InPredicate*, pVE_->getPrimary());
	; _SYDNEY_ASSERT(pIP);
	; _SYDNEY_ASSERT(pIP->getLeft());

	if (pIP->getLeft()->getOperator() == Statement::ValueExpression::op_Func
		&& pIP->getLeft()->getFunction() == Statement::ValueExpression::func_Neighbor) {

		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// create in predicate or not-in predicate
	return Plan::Predicate::In::SubQuery::create(cEnvironment_,
												 vecScalar_,
												 pRelation_,
												 cOuterRelation_,
												 bIsNot);
}

////////////////////////////////////////////
// Predicate::InImpl::ValueList

// FUNCTION private
//	Predicate::InImpl::ValueList::isSubquery -- 
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
InImpl::ValueList::
isSubquery() const
{
	return false;
}

// FUNCTION private
//	Predicate::InImpl::ValueList::createPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Plan::Interface::IScalar*>& vecScalar_
//	Plan::Interface::IRelation* pRelation_
//	const Plan::Utility::RelationSet& cOuterRelation_
//	Statement::ValueExpression* pVE_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
InImpl::ValueList::
createPredicate(Opt::Environment& cEnvironment_,
				const VECTOR<Plan::Interface::IScalar*>& vecScalar_,
				Plan::Interface::IRelation* pRelation_,
				const Plan::Utility::RelationSet& cOuterRelation_,
				Statement::ValueExpression* pVE_) const
{
	// create in predicate or not-in predicate
	bool bIsNot = pVE_->getOperator() == Statement::ValueExpression::op_NotIn;
	Statement::InPredicate* pIP =
		_SYDNEY_DYNAMIC_CAST(Statement::InPredicate*, pVE_->getPrimary());
	; _SYDNEY_ASSERT(pIP);
	; _SYDNEY_ASSERT(pIP->getLeft());

	if (pIP->getLeft()->getOperator() == Statement::ValueExpression::op_Func
		&& pIP->getLeft()->getFunction() == Statement::ValueExpression::func_Neighbor) {

		if (bIsNot) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		Plan::Interface::IScalar* pOption = 0;
		Statement::Hint* pHint = pIP->getLeft()->getHint();
		if (pHint) {
			pOption = pHint->getAnalyzer2()->getScalar(cEnvironment_,
													   pRelation_,
													   pHint);
		}
		return Plan::Predicate::NeighborIn::ValueList::create(cEnvironment_,
															  vecScalar_,
															  pRelation_,
															  pOption);
	}

	return Plan::Predicate::In::ValueList::create(cEnvironment_,
												  vecScalar_,
												  pRelation_,
												  cOuterRelation_,
												  bIsNot);
}

////////////////////////////////////////////
// Predicate::InImpl::Variable

// FUNCTION public
//	Predicate::InImpl::Variable::getPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
InImpl::Variable::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);

	STRING cName = pVE->toSQLStatement();
	Plan::Interface::IPredicate* pResult = cEnvironment_.searchPredicate(cName);
	if (pResult) {
		return pResult;
	}

	Statement::InPredicate* pIP =
		_SYDNEY_DYNAMIC_CAST(Statement::InPredicate*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pIP);

	Statement::ValueExpression* pLeft = pIP->getLeft();
	Statement::Object* pVariable = pIP->getRight();

	VECTOR<Plan::Interface::IScalar*> vecScalar;
	VECTOR<Plan::Interface::IScalar*> vecOperand;
	{
		// arbitrary array element is now allowed in the left operand for this case
		// only when predicant is not row
		Opt::Environment::AutoPop cAutoPop(0, 0);
		if (cEnvironment_.checkStatus(Opt::Environment::Status::NoTopPredicate) == false) {
			Opt::Environment::AutoPop cAutoPopTmp =
				cEnvironment_.pushStatus(Opt::Environment::Status::ArbitraryElementAllowed
										 | Opt::Environment::Status::KnownNotNull);
			cAutoPop = cAutoPopTmp; // to avoid compile error in GCC
		} else {
			Opt::Environment::AutoPop cAutoPopTmp =
				cEnvironment_.pushStatus(Opt::Environment::Status::ArbitraryElementAllowed);
			cAutoPop = cAutoPopTmp;  // to avoid compile error in GCC
		}		
		
		pLeft->getAnalyzer2()->addScalar(cEnvironment_,
										 pRelation_,
										 pLeft,
										 vecScalar);
	}
	const Interface::IAnalyzer* pAnalyzer = pVariable->getAnalyzer2();

	if (vecScalar.GETSIZE() != pAnalyzer->getDegree(cEnvironment_, pVariable)) {
		_SYDNEY_THROW0(Exception::InvalidRowValue);
	}
	pAnalyzer->addScalar(cEnvironment_,
						 pRelation_,
						 pVariable,
						 vecOperand);
	; _SYDNEY_ASSERT(vecScalar.GETSIZE() == vecOperand.GETSIZE());

	// create in predicate or not-in predicate
	bool bIsNot = pVE->getOperator() == Statement::ValueExpression::op_NotIn;
	if (pIP->getLeft()->getOperator() == Statement::ValueExpression::op_Func
		&& pIP->getLeft()->getFunction() == Statement::ValueExpression::func_Neighbor) {
		_SYDNEY_THROW0(Exception::NotSupported);
	} else {
		pResult =
			Plan::Predicate::In::Variable::create(cEnvironment_,
												  vecScalar,
												  vecOperand,
												  bIsNot);
	}

	if (pResult->hasParameter() == false) {
		cEnvironment_.addPredicate(pRelation_,
								   cName,
								   pResult);
	}

	return pResult;
}

// FUNCTION private
//	Predicate::InImpl::Variable::isSubquery -- 
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
InImpl::Variable::
isSubquery() const
{
	return false;
}

// FUNCTION private
//	Predicate::InImpl::Variable::createPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Plan::Interface::IScalar*>& vecScalar_
//	Plan::Interface::IRelation* pRelation_
//	const Plan::Utility::RelationSet& cOuterRelation_
//	Statement::ValueExpression* pVE_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
InImpl::Variable::
createPredicate(Opt::Environment& cEnvironment_,
				const VECTOR<Plan::Interface::IScalar*>& vecScalar_,
				Plan::Interface::IRelation* pRelation_,
				const Plan::Utility::RelationSet& cOuterRelation_,
				Statement::ValueExpression* pVE_) const
{
	return 0; // do nothing
}

//////////////////////////////
// Predicate::In::

// FUNCTION public
//	Predicate::In::create -- constuctor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const In*
//
// EXCEPTIONS

//static
const In*
In::
create(const Statement::ValueExpression* pStatement_)
{
	const Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(const Statement::ValueExpression*, pStatement_);
	const Statement::InPredicate* pIP =
		_SYDNEY_DYNAMIC_CAST(const Statement::InPredicate*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pIP);

	if (Statement::ValueExpression* pRight =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*,
							 pIP->getRight())) {
		if (pRight->getOperator() == Statement::ValueExpression::op_Tblconst) {
			return &_analyzerVL;
		}
		if (pRight->getOperator() == Statement::ValueExpression::op_Variable) {
			return &_analyzerV;
		}
	}
	return &_analyzerSQ;
}

_SYDNEY_ANALYSIS_PREDICATE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
