// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/BulkSpecification.cpp --
// 
// Copyright (c) 2011, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Query";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Analysis/Query/BulkSpecification.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/PrepareFailed.h"
#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

#include "Plan/Interface/IScalar.h"
#include "Plan/Relation/Bulk.h"
#include "Plan/Scalar/DataType.h"

#include "Statement/BulkSpecification.h"
#include "Statement/ValueExpression.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Expression::Impl::BulkSpecificationImpl --
	//		implementation classes for bulkSpecification statement analyzer
	//
	// NOTES
	class BulkSpecificationImpl
		: public Query::BulkSpecification
	{
	public:
		typedef BulkSpecificationImpl This;
		typedef Query::BulkSpecification Super;

		// constructor
		BulkSpecificationImpl() : Super() {}
		// destructor
		virtual ~BulkSpecificationImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;
		virtual Plan::Interface::IRelation*
				getFilter(Opt::Environment& cEnvironment_,
						  Plan::Interface::IRelation* pRelation_,
						  Statement::Object* pStatement_) const;
	protected:
	private:
		Plan::Relation::Bulk::Argument
				getBulkOption(Opt::Environment& cEnvironment_,
							  Statement::BulkSpecification* pBS_) const;
	};
}

namespace
{
	// VARIABLE local
	//	_analyzer -- instance
	//
	// NOTES
	const Impl::BulkSpecificationImpl _analyzer;

} // namespace

//////////////////////////////////////////
// Query::Impl::BulkSpecificationImpl
//////////////////////////////////////////

// FUNCTION public
//	Query::Impl::BulkSpecificationImpl::getRelation -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::BulkSpecificationImpl::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::BulkSpecification* pBS =
		_SYDNEY_DYNAMIC_CAST(Statement::BulkSpecification*, pStatement_);

	// getRelation should be called for input bulk specification
	if (pBS->isInput() == false) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Plan::Relation::Bulk::Argument cArgument =
		getBulkOption(cEnvironment_, pBS);

	if (cEnvironment_.isPrepare()) {
		// Batch insert can't be prepared
		_SYDNEY_THROW0(Exception::PrepareFailed);
	}

	if (cEnvironment_.getTransaction().isImplicit()) {
		// set special mode for batch transaction
		//	Set batch mode only for implicitly-started transaction
		cEnvironment_.getTransaction().setBatchMode(true);
	}

	return Plan::Relation::Bulk::Input::create(cEnvironment_,
											   cArgument);
}

// FUNCTION public
//	Query::Impl::BulkSpecificationImpl::getFilter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::BulkSpecificationImpl::
getFilter(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::BulkSpecification* pBS =
		_SYDNEY_DYNAMIC_CAST(Statement::BulkSpecification*, pStatement_);

	// getFilter should be called for output bulk specification
	if (pBS->isInput() == true) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Plan::Relation::Bulk::Argument cArgument = 
		getBulkOption(cEnvironment_, pBS);

	return Plan::Relation::Bulk::Output::create(cEnvironment_,
												cArgument,
												pRelation_);
}

// FUNCTION private
//	Query::Impl::BulkSpecificationImpl::getBulkOption -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::BulkSpecification* pBS_
//	
// RETURN
//	Plan::Relation::Bulk::Argument
//
// EXCEPTIONS

Plan::Relation::Bulk::Argument
Impl::BulkSpecificationImpl::
getBulkOption(Opt::Environment& cEnvironment_,
			  Statement::BulkSpecification* pBS_) const
{
	Statement::ValueExpression* pDataPart = pBS_->getInputData();
	Statement::ValueExpression* pWithPart = pBS_->getWith();
	Statement::ValueExpression* pHintPart = pBS_->getHint();

	// Convert options to plan node
	Plan::Interface::IScalar* pData = 0;
	Plan::Interface::IScalar* pWith = 0;
	Plan::Interface::IScalar* pHint = 0;

	; _SYDNEY_ASSERT(pDataPart);
	; _SYDNEY_ASSERT(pDataPart->isPathName());
	pData = pDataPart->getAnalyzer2()->getScalar(cEnvironment_, 0, pDataPart)
		->setExpectedType(cEnvironment_,
						  Plan::Scalar::DataType::getStringType());
	if (pWithPart) {
		; _SYDNEY_ASSERT(pWithPart->isPathName());
		pWith = pWithPart->getAnalyzer2()->getScalar(cEnvironment_, 0, pWithPart)
			->setExpectedType(cEnvironment_,
							  Plan::Scalar::DataType::getStringType());
	}
	if (pHintPart) {
		pHint = pHintPart->getAnalyzer2()->getScalar(cEnvironment_, 0, pHintPart)
			->setExpectedType(cEnvironment_,
							  Plan::Scalar::DataType::getStringType());
	}

	return Plan::Relation::Bulk::Argument(pData, pWith, pHint);
}

////////////////////////////////////////
// Query::BulkSpecification
////////////////////////////////////////

// FUNCTION public
//	Query::BulkSpecification::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::BulkSpecification* pStatement_
//	
// RETURN
//	const BulkSpecification*
//
// EXCEPTIONS

const BulkSpecification*
BulkSpecification::
create(const Statement::BulkSpecification* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2011, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
