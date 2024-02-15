// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/FullText.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Function/FullText.h"

#include "Common/Assert.h"

#include "Exception/FullTextIndexNeeded.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Relation/RowElement.h"
#include "Plan/Scalar/Function.h"

#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Function::Impl::FullTextImpl -- base class for fulltext function analyzer
	//
	// NOTES
	class FullTextImpl
		: public Function::FullText
	{
	public:
		typedef FullTextImpl This;
		typedef Function::FullText Super;

		// constructor
		FullTextImpl() : Super() {}
		// destructor
		virtual ~FullTextImpl() {}

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

	// CLASS local
	//	Function::Impl::RoughKwicPositionImpl -- base class for fulltext function analyzer
	//
	// NOTES
	class RoughKwicPositionImpl
		: public FullTextImpl
	{
	public:
		typedef RoughKwicPositionImpl This;
		typedef FullTextImpl Super;

		// constructor
		RoughKwicPositionImpl() : Super() {}
		// destructor
		~RoughKwicPositionImpl() {}

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
	//////////////////////////
	// Impl::FullTextImpl::
		virtual Plan::Interface::IScalar*
					getFunction(Opt::Environment& cEnvironment_,
								Plan::Interface::IRelation* pRelation_,
								Statement::ValueExpression* pVE_) const;
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
				case Statement::ValueExpression::func_Tf:
					{
						return Plan::Tree::Node::Tf;
					}
				case Statement::ValueExpression::func_ClusterId:
					{
						return Plan::Tree::Node::ClusterID;
					}
				case Statement::ValueExpression::func_ClusterWord:
					{
						return Plan::Tree::Node::FeatureValue;
					}
				case Statement::ValueExpression::func_Score:
					{
						return Plan::Tree::Node::Score;
					}
				case Statement::ValueExpression::func_Sectionized:
					{
						return Plan::Tree::Node::Section;
					}
				case Statement::ValueExpression::func_Word:
					{
						return Plan::Tree::Node::Word;
					}
				case Statement::ValueExpression::func_WordDf:
					{
						return Plan::Tree::Node::WordDf;
					}
				case Statement::ValueExpression::func_WordScale:
					{
						return Plan::Tree::Node::WordScale;
					}
				case Statement::ValueExpression::func_Kwic:
					{
						return Plan::Tree::Node::RoughKwicPosition;
					}
				case Statement::ValueExpression::func_Existence:
					{
						return Plan::Tree::Node::Existence;
					}
				case Statement::ValueExpression::func_Cluster:
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
	const Impl::FullTextImpl _analyzer;
	const Impl::RoughKwicPositionImpl _analyzerKwic;

} // namespace

//////////////////////////////////////
//	Function::Impl::FullTextImpl

// FUNCTION private
//	Function::Impl::FullTextImpl::getFunction -- 
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
Impl::FullTextImpl::
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

		return Plan::Scalar::Function::create(cEnvironment_,
											  _getNodeType(pVE_),
											  pOperand,
											  pVE_->toSQLStatement());
	} else {
		; _SYDNEY_ASSERT(pVE_->getPrimary());
		Statement::ValueExpressionList* pList =
			_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*,
								 pVE_->getPrimary());
		;_SYDNEY_ASSERT(pList);

		VECTOR<Plan::Interface::IScalar*> vecOperand;
		pList->getAnalyzer2()->addScalar(cEnvironment_,
										 pRelation_,
										 pList,
										 vecOperand);

		return Plan::Scalar::Function::create(cEnvironment_,
											  _getNodeType(pVE_),
											  vecOperand,
											  pVE_->toSQLStatement());
	}
}

// FUNCTION private
//	Function::Impl::FullTextImpl::createScalar -- 
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
Impl::FullTextImpl::
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

	// check function field
	Plan::Interface::IScalar* pScalar =
		pFunction->convertFunction(cEnvironment_,
								   pRelation_,
								   pFunction,
								   Schema::Field::Function::Undefined);
	if (pScalar == 0) {
		// fulltext index is not defined
		_SYDNEY_THROW0(Exception::FullTextIndexNeeded);
	}

	return pScalar;
}

/////////////////////////////////////////
// Function::Impl::RoughKwicPositionImpl::
/////////////////////////////////////////

// FUNCTION private
//	Function::Impl::RoughKwicPositionImpl::getFunction -- 
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

//virtual
Plan::Interface::IScalar*
Impl::RoughKwicPositionImpl::
getFunction(Opt::Environment& cEnvironment_,
			Plan::Interface::IRelation* pRelation_,
			Statement::ValueExpression* pVE_) const
{
	Statement::Function::Kwic* pKwic =
		_SYDNEY_DYNAMIC_CAST(Statement::Function::Kwic*, pVE_);

	Statement::ValueExpression* pSource = pKwic->getSource();
	Statement::ValueExpression* pSize = pKwic->getSize();

	Plan::Interface::IScalar* pOperand =
		pSource->getAnalyzer2()->getScalar(cEnvironment_,
										   pRelation_,
										   pSource);
	Plan::Interface::IScalar* pOption =
		pSize->getAnalyzer2()->getScalar(cEnvironment_,
										 pRelation_,
										 pSize)
		->setExpectedType(cEnvironment_,
						  Plan::Scalar::DataType::getIntegerType())
		->setNodeType(Plan::Tree::Node::KwicSize);
	return Plan::Scalar::Function::create(cEnvironment_,
										  _getNodeType(pVE_),
										  pOperand,
										  pOption,
										  pVE_->toSQLStatement());
}

//////////////////////////////
// Function::FullText
//////////////////////////////

// FUNCTION public
//	Function::FullText::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const FullText*
//
// EXCEPTIONS

//static
const FullText*
FullText::
create(const Statement::ValueExpression* pStatement_)
{
	switch (pStatement_->getFunction()) {
	case Statement::ValueExpression::func_Kwic:
		{
			return &_analyzerKwic;
		}
	default:
		{
			return &_analyzer;
		}
	}
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
