// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Spatial.cpp --
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Function/Spatial.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/SpatialIndexNeeded.h"

#include "Opt/Environment.h"

#include "Plan/Relation/RowElement.h"
#include "Plan/Scalar/Function.h"

#include "Statement/Hint.h"
#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Function::Impl::SpatialImpl -- base class for spatial function analyzer
	//
	// NOTES
	class SpatialImpl
		: public Function::Spatial
	{
	public:
		typedef SpatialImpl This;
		typedef Function::Spatial Super;

		// constructor
		SpatialImpl() : Super() {}
		// destructor
		virtual ~SpatialImpl() {}

	///////////////////////////
	// Interface::IAnalyzer::
	//	virtual Plan::Interface::IScalar*
	//				getScalar(Opt::Environment& cEnvironment_,
	//						  Plan::Interface::IRelation* pRelation_,
	//						  Statement::Object* pStatement_) const;
	//	virtual Plan::Relation::RowElement*
	//				getRowElement(Opt::Environment& cEnvironment_,
	//							  Plan::Interface::IRelation* pRelation_,
	//							  Statement::Object* pStatement_) const;
	protected:
	private:
		virtual Plan::Interface::IScalar*
					getFunction(Opt::Environment& cEnvironment_,
								Plan::Interface::IRelation* pRelation_,
								Statement::ValueExpression* pVE_) const;
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
} // namespace Impl

namespace
{
	// FUNCTION local
	//	$$$::_getNodeType -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	Statement::ValueExpression* pStatement_
	//	
	// RETURN
	//	Plan::Tree::Node::Type
	//
	// EXCEPTIONS

	Plan::Tree::Node::Type
	_getNodeType(Statement::ValueExpression* pStatement_)
	{
		switch (pStatement_->getOperator()) {
		case Statement::ValueExpression::op_Func:
			{
				switch (pStatement_->getFunction()) {
				case Statement::ValueExpression::func_NeighborId:
					{
						return Plan::Tree::Node::NeighborID;
					}
				case Statement::ValueExpression::func_NeighborDistance:
					{
						return Plan::Tree::Node::NeighborDistance;
					}
				default:
					{
						break;
					}
				}
				break;
			}
		default:
			{
				break;
			}
		}
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::SpatialImpl _analyzer;

} // namespace

//////////////////////////////////////
//	Function::Impl::SpatialImpl

// FUNCTION private
//	Function::Impl::SpatialImpl::getFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::ValueExpression* pVE_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

Plan::Interface::IScalar*
Impl::SpatialImpl::
getFunction(Opt::Environment& cEnvironment_,
			Plan::Interface::IRelation* pRelation_,
			Statement::ValueExpression* pVE_) const
{
	if (pVE_->getLeft()) {
		Statement::ValueExpression* pLeft = pVE_->getLeft();
		Plan::Interface::IScalar* pOperand =
			pLeft->getAnalyzer2()->getScalar(cEnvironment_,
											 pRelation_,
											 pLeft);

		if (pVE_->getOperator() == Statement::ValueExpression::op_Func
			&& pVE_->getFunction() == Statement::ValueExpression::func_Neighbor
			&& cEnvironment_.checkStatus(Opt::Environment::Status::In)) {
			// if under in-predicate, neighbor function returns only operand,
			// in-predicate analyzer use hint
			return pOperand;
		}

		Plan::Interface::IScalar* pOption = 0;
		Statement::Hint* pHint = pVE_->getHint();
		if (pHint) {
			pOption = pHint->getAnalyzer2()->getScalar(cEnvironment_,
													   pRelation_,
													   pHint);
		}

		return Plan::Scalar::Function::create(cEnvironment_,
											  _getNodeType(pVE_),
											  pOperand,
											  pOption,
											  pVE_->toSQLStatement());
	} else {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

// FUNCTION private
//	Function::Impl::SpatialImpl::createScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

//virtual
Plan::Interface::IScalar*
Impl::SpatialImpl::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	Plan::Interface::IScalar* pFunction = getFunction(cEnvironment_,
													  pRelation_,
													  pVE);

	if (pVE->getFunction() == Statement::ValueExpression::func_Neighbor) {
		return pFunction;
	} else {
		// check function field
		Plan::Interface::IScalar* pScalar =
			pFunction->convertFunction(cEnvironment_,
									   pRelation_,
									   pFunction,
									   Schema::Field::Function::Undefined);
		if (pScalar == 0) {
			// spatial index is not defined
			_SYDNEY_THROW0(Exception::SpatialIndexNeeded);
		}

		return pScalar;
	}
}

//////////////////////////////
// Function::Spatial
//////////////////////////////

// FUNCTION public
//	Function::Spatial::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Spatial*
//
// EXCEPTIONS

//static
const Spatial*
Spatial::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
