// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Aggregation.cpp --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Scalar";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DPlan/Scalar/Aggregation.h"
#include "DPlan/Scalar/Impl/AggregationImpl.h"

#include "Plan/Sql/Argument.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_SCALAR_BEGIN

// FUNCTION public
//	Scalar::Aggregation::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const STRING& cstrName_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Plan::Scalar::Function*
Aggregation::
create(Opt::Environment& cEnvironment_,
	   Plan::Tree::Node::Type eOperator_,
	   Plan::Interface::IScalar* pOperand_,
	   const STRING& cstrName_)
{

	switch (eOperator_) {
	case Plan::Tree::Node::Count:
	{
		AUTOPOINTER<AggregationImpl::Monadic> pResult;
		if (pOperand_->getType() == Plan::Tree::Node::Distinct) { 
			pResult = new AggregationImpl::Monadic(Plan::Tree::Node::Count,
												   cstrName_,
												   pOperand_);
		} else {
			Plan::Interface::IScalar* pDistOperand =
				Plan::Scalar::Aggregation::create(cEnvironment_,
												  Plan::Tree::Node::Count,
												  pOperand_,
												  cstrName_);
			pResult = new AggregationImpl::Monadic(Plan::Tree::Node::Sum,
												   cstrName_,	
												   pDistOperand);
		}
		pResult->createDataType(cEnvironment_);
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
	}
	
	case Plan::Tree::Node::Max:
	case Plan::Tree::Node::Min:
	case Plan::Tree::Node::Sum:
	case Plan::Tree::Node::Distinct:
	{
		Plan::Interface::IScalar* pDistOperand =
			Plan::Scalar::Aggregation::create(cEnvironment_,
											  eOperator_,
											  pOperand_,
											  cstrName_);
		AUTOPOINTER<AggregationImpl::Monadic> pResult = new AggregationImpl::Monadic(eOperator_,
																					 cstrName_,
																					 pDistOperand);
		pResult->createDataType(cEnvironment_);
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
	}
	case Plan::Tree::Node::Word:
	{
		AUTOPOINTER<AggregationImpl::Monadic> pResult = new AggregationImpl::Monadic(eOperator_,
																					 cstrName_,
																					 pOperand_);
		pResult->createDataType(cEnvironment_);
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
	}

	case Plan::Tree::Node::Avg:
	{
		// 分散Sum/分散Countを行う. 
		Plan::Interface::IScalar* pDistSum = 0;
		Plan::Interface::IScalar* pDistCount = 0;
		if (pOperand_->getType() == Plan::Tree::Node::FullTextLength ||
			pOperand_->getType() == Plan::Tree::Node::WordCount ) {
			OSTRSTREAM cStream;
			Plan::Sql::QueryArgument cQueryArg;
			const Plan::Interface::IScalar* pLengthOperand =
				_SYDNEY_DYNAMIC_CAST(const Plan::Interface::IScalar*, pOperand_->getOperandAt(0));
			cStream << "count(" << pLengthOperand->toSQLStatement(cEnvironment_, cQueryArg) << ")";
			pDistCount = DPlan::Scalar::Aggregation::create(cEnvironment_,
															Plan::Tree::Node::Count,
															const_cast<Plan::Interface::IScalar*>(pLengthOperand),
															cStream.getString());

			cStream.clear();
			cStream <<pOperand_->toSQLStatement(cEnvironment_, cQueryArg)
					<< "* "<< pDistCount->toSQLStatement(cEnvironment_, cQueryArg) << ")";
			Plan::Interface::IScalar* pDistOperand =
				Plan::Scalar::Aggregation::create(cEnvironment_,
												  eOperator_,
												  pOperand_,
												  Plan::Scalar::DataType::getBigIntegerType(),
												  "AVG");
			// 親サーバーでavg(fulltext_length(f)) * count(f)を計算する
			AUTOPOINTER<AggregationImpl::Avg> pLength =
				new AggregationImpl::Avg(Plan::Tree::Node::Multiply,
										 cStream.getString(),
										 MAKEPAIR(pDistOperand,
												  const_cast<Plan::Interface::IScalar*> (_SYDNEY_DYNAMIC_CAST(const Plan::Interface::IScalar*,
																											  pDistCount->getOperandAt(0)))));
			pLength->createDataType(cEnvironment_);
			pLength->registerToEnvironment(cEnvironment_);
				
			cStream.clear();
			cStream << "sum (" << pLength->toSQLStatement(cEnvironment_, cQueryArg) << ")";
			AUTOPOINTER<AggregationImpl::Monadic> pSum = new AggregationImpl::Monadic(Plan::Tree::Node::Sum,
																					  cStream.getString(),
																					  pLength.release());
			pSum->createDataType(cEnvironment_);
			pSum->registerToEnvironment(cEnvironment_);
			pDistSum = pSum.release();
		} else {
			// 分散Sum/分散Countを行う.
			OSTRSTREAM cCountStream;
			Plan::Sql::QueryArgument cQueryArg;
			cCountStream << "count(" << pOperand_->toSQLStatement(cEnvironment_, cQueryArg) << ")";

			pDistCount = DPlan::Scalar::Aggregation::create(cEnvironment_,
															Plan::Tree::Node::Count,
															pOperand_,
															cCountStream.getString());
			OSTRSTREAM cSumStream;
			cSumStream << "sum(" << pOperand_->toSQLStatement(cEnvironment_, cQueryArg) << ")";
			pDistSum = DPlan::Scalar::Aggregation::create(cEnvironment_,
														  Plan::Tree::Node::Sum,
														  pOperand_,
														  cSumStream.getString());
		}
			
		AUTOPOINTER<AggregationImpl::Avg> pResult = new AggregationImpl::Avg(Plan::Tree::Node::Divide,
																			 cstrName_,
																			 MAKEPAIR(pDistSum, pDistCount));
																			 
		pResult->setDataType(pOperand_->getDataType());
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
	}
	default:
	{
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	}

}

// FUNCTION public
//	Scalar::Aggregation::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const STRING& cstrName_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Plan::Scalar::Function*
Aggregation::
create(Opt::Environment& cEnvironment_,
	   Plan::Tree::Node::Type eOperator_,
	   Plan::Interface::IScalar* pOperand_,
	   Plan::Interface::IScalar* pOption_,	   
	   const STRING& cstrName_)
{

	if (pOption_ == 0)
		return create(cEnvironment_, eOperator_, pOperand_, cstrName_);
	
	// count(distinct p)の場合,分散マネージャー上でカウントする
	if (pOption_
		&& pOption_->getType() == Plan::Tree::Node::Distinct) {
		AUTOPOINTER<AggregationImpl::MonadicWithMonadicOption> pResult =
			new AggregationImpl::MonadicWithMonadicOption(eOperator_,
														  cstrName_,
														  pOperand_,
														  pOption_);
		pResult->createDataType(cEnvironment_);
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
		
	} else {
		return Plan::Scalar::Aggregation::create(cEnvironment_,
													 eOperator_,
													 pOperand_,
													 pOption_,
													 cstrName_);
	}
}




_SYDNEY_DPLAN_SCALAR_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
