// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Aggregation.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Function/Aggregation.h"
#include "Analysis/Query/Utility.h"

#include "Common/Assert.h"
#include "Common/StringData.h"

#include "Exception/InvalidSetFunction.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

#include "Plan/Scalar/Function.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Relation/Argument.h"
#include "Plan/Relation/RowElement.h"
#include "Plan/Interface/IRelation.h"

#include "DPlan/Scalar/Aggregation.h"

#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Function::Impl::AggregationImpl -- base class for aggregation function analyzer
	//
	// NOTES
	class AggregationImpl
		: public Function::Aggregation
	{
	public:
		typedef AggregationImpl This;
		typedef Function::Aggregation Super;

		// constructor
		AggregationImpl() : Super() {}
		// destructor
		~AggregationImpl() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;

		// generate RowElement from Statement::Object
		virtual Plan::Relation::RowElement*
					getRowElement(Opt::Environment& cEnvironment_,
								  Plan::Interface::IRelation* pRelation_,
								  Statement::Object* pStatement_) const;

	protected:
	private:
		// create aggregation rowelement
		Plan::Relation::RowElement*
					createRowElement(Opt::Environment& cEnvironment_,
									 Plan::Interface::IRelation* pRelation_,
									 Plan::Tree::Node::Type eType_,
									 Plan::Interface::IScalar* pOperand_,
									 Plan::Interface::IScalar* pOption_,
									 const STRING& cstrName_) const;
	////////////////////////////////
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
	//	int iFunc_
	//	
	// RETURN
	//	Plan::Tree::Node::Type
	//
	// EXCEPTIONS

	Plan::Tree::Node::Type
	_getNodeType(int iFunc_)
	{
		switch (iFunc_) {
		case Statement::ValueExpression::func_Count:
			{
				return Plan::Tree::Node::Count;
			}
		case Statement::ValueExpression::func_Avg:
			{
				return Plan::Tree::Node::Avg;
			}
		case Statement::ValueExpression::func_Max:
			{
				return Plan::Tree::Node::Max;
			}
		case Statement::ValueExpression::func_Min:
			{
				return Plan::Tree::Node::Min;
			}
		case Statement::ValueExpression::func_Sum:
			{
				return Plan::Tree::Node::Sum;
			}
		}
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::AggregationImpl _analyzer;

} // namespace

//////////////////////////////////////
//	Function::Impl::AggregationImpl

// FUNCTION public
//	Function::Impl::AggregationImpl::getScalar -- generate Scalar from Statement::Object
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
Impl::AggregationImpl::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Plan::Relation::RowElement* pRowElement = getRowElement(cEnvironment_,
															pRelation_,
															pStatement_);
	return pRowElement->getScalar();
}

// FUNCTION public
//	Function::Impl::AggregationImpl::getRowElement -- generate RowElement from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Relation::RowElement*
//
// EXCEPTIONS

//virtual
Plan::Relation::RowElement*
Impl::AggregationImpl::
getRowElement(Opt::Environment& cEnvironment_,
			  Plan::Interface::IRelation* pRelation_,
			  Statement::Object* pStatement_) const
{
	if (cEnvironment_.checkStatus(Opt::Environment::Status::SelectList) == false
		&& cEnvironment_.checkStatus(Opt::Environment::Status::Having) == false
		&& cEnvironment_.checkStatus(Opt::Environment::Status::OrderBy) == false) {
		_SYDNEY_THROW0(Exception::InvalidSetFunction);
	}

	if (cEnvironment_.isGrouping() == false) {
		// current scope should be in grouping
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	if (pRelation_ == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	STRING cName = pVE->toSQLStatement();
	Plan::Relation::RowElement* pResult = cEnvironment_.searchScalar(cName);
	if (pResult) {
		return pResult;
	}

	Statement::ValueExpression* pLeft = pVE->getLeft();

	Opt::Environment::AutoPop cAutoPop0 =
		cEnvironment_.pushStatus(Opt::Environment::Status::SetFunction);
	Opt::Environment::AutoPop cAutoPop1 =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	Plan::Interface::IScalar* pOperand = 0;
	if (pLeft) {
		pOperand = pLeft->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pLeft);
	} else {
		; _SYDNEY_ASSERT(pVE->getFunction() == Statement::ValueExpression::func_Count);
		// count(*) -> use constant value ('*' for explain)
		pOperand = Plan::Scalar::Value::Asterisk::create(cEnvironment_);
	}

	Plan::Interface::IScalar* pOption = 0;
	if (pVE->getQuantifier() == Statement::ValueExpression::quant_Distinct) {
		pOption = Plan::Scalar::Value::create(cEnvironment_,
											  ModUnicodeString("distinct"))
			->setNodeType(Plan::Tree::Node::Distinct);

		; _SYDNEY_ASSERT(pLeft);
		STRING cOperandName("distinct ");
		cOperandName.append(pLeft->toSQLStatement());
		Plan::Relation::RowElement* pDistinct = cEnvironment_.searchScalar(cOperandName);
		if (pDistinct == 0) {
			// operand is also aggregation to get distinct result
			pDistinct = createRowElement(cEnvironment_,
										 pRelation_,
										 Plan::Tree::Node::Distinct,
										 pOperand,
										 0,
										 cOperandName);
		}
		pOperand = pDistinct->getScalar(cEnvironment_);

	}

	return createRowElement(cEnvironment_,
							pRelation_,
							_getNodeType(pVE->getFunction()),
							pOperand,
							pOption,
							cName);
}

// FUNCTION private
//	Function::Impl::AggregationImpl::createRowElement -- create aggregation rowelement
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Plan::Tree::Node::Type eType_
//	Plan::Interface::IScalar* pOperand_
//	Plan::Interface::IScalar* pOption_
//	const STRING& cstrName_
//	
// RETURN
//	Plan::Relation::RowElement*
//
// EXCEPTIONS

Plan::Relation::RowElement*
Impl::AggregationImpl::
createRowElement(Opt::Environment& cEnvironment_,
				 Plan::Interface::IRelation* pRelation_,
				 Plan::Tree::Node::Type eType_,
				 Plan::Interface::IScalar* pOperand_,
				 Plan::Interface::IScalar* pOption_,
				 const STRING& cstrName_) const
{
	Plan::Relation::RowElement* pResult = 0;

	
	Plan::Interface::IScalar* pScalar = 0;
	// ディストリビュート時のみ集約関数をDPlanのクラスを使用する。
	// Plan::Scalar::Function::crateの内部でも同様の判断を行なっているが、
	// count(*)の場合にレプリケート or ディストリビュートの判断ができないため、ここでも判断を行う。
	// ここでのFunction::createの呼び出しとAggregation::createの呼び出しが等化であることが確認できたら、
	// こちらに統一する。
	if (eType_ == Plan::Tree::Node::Count &&
		cEnvironment_.hasCascade()) {
		Plan::Relation::InquiryArgument cArgument = 0;
		cArgument.m_iTarget |=
			Plan::Relation::InquiryArgument::Target::Distributed;
		Plan::Interface::IRelation::InquiryResult iResult = pRelation_->inquiry(cEnvironment_, cArgument);
		if (iResult & Plan::Relation::InquiryArgument::Target::Distributed) {
			pScalar = DPlan::Scalar::Aggregation::create(cEnvironment_,
														 eType_,
														 pOperand_,
														 pOption_,
														 cstrName_);
											   
		}
	}
	if (pScalar == 0) {
		pScalar = Plan::Scalar::Function::create(cEnvironment_,
												 eType_,
												 pOperand_,
												 pOption_,
												 cstrName_);
	}

	pResult = Plan::Relation::RowElement::create(cEnvironment_,
												 pRelation_,
												 pRelation_->aggregate(cEnvironment_,
																	   pScalar,
																	   pOperand_),
												 pScalar);

	// add to namemap
	cEnvironment_.addScalar(pRelation_,
							cstrName_,
							pResult);
	return pResult;
}

// FUNCTION private
//	Function::Impl::AggregationImpl::createScalar -- 
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
Impl::AggregationImpl::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	// never called
	_SYDNEY_THROW0(Exception::NotSupported);
}

//////////////////////////////
// Function::Aggregation
//////////////////////////////

// FUNCTION public
//	Function::Aggregation::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Aggregation*
//
// EXCEPTIONS

//static
const Aggregation*
Aggregation::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
