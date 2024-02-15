// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Function.cpp --
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
const char moduleName[] = "Plan::Scalar";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Plan/Scalar/Function.h"
#include "Plan/Scalar/Aggregation.h"
#include "Plan/Scalar/Arithmetic.h"
#include "Plan/Scalar/Argument.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/FullText.h"
#include "Plan/Scalar/Locator.h"
#include "Plan/Scalar/Spatial.h"
#include "Plan/Scalar/Impl/ArrayImpl.h"
#include "Plan/Scalar/Impl/FunctionImpl.h"
#include "Plan/Scalar/Impl/LengthImpl.h"
#include "Plan/Scalar/Impl/KwicImpl.h"
#include "Plan/Scalar/Impl/PatternImpl.h"
#include "Plan/Sql/Query.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Relation/Argument.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/Contains.h"

#include "DPlan/Scalar/Aggregation.h"

#include "Common/Assert.h"
#include "Common/ColumnMetaData.h"

#include "Exception/NotSupported.h"

#include "Execution/Function/Factory.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace
{
	struct _ImplType
	{
		enum Value
		{
			Aggregation,
			Arithmetic,
			Array,
			Function,
			Length,
			Kwic,
			FullText,
			Pattern,
			Spatial,
			ValueNum
		};
	};

	// FUNCTION local
	//	$$$::_getImplType -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	Tree::Node::Type eOperator_
	//	
	// RETURN
	//	_ImplType::Value
	//
	// EXCEPTIONS
	_ImplType::Value
	_getImplType(Tree::Node::Type eOperator_)
	{
		switch (eOperator_) {
		case Tree::Node::Add:
		case Tree::Node::Subtract:
		case Tree::Node::Multiply:
		case Tree::Node::Divide:
		case Tree::Node::Negative:
		case Tree::Node::Absolute:
		case Tree::Node::Modulus:
			{
				return _ImplType::Arithmetic;
			}
 		case Tree::Node::Count:
		case Tree::Node::Avg:
		case Tree::Node::Max:
		case Tree::Node::Min:
		case Tree::Node::Sum:
		case Tree::Node::Distinct:
			{
				return _ImplType::Aggregation;
			}
		case Tree::Node::ArrayConstructor:
		case Tree::Node::ElementReference:
			{
				return _ImplType::Array;
			}
		case Tree::Node::CharLength:
		case Tree::Node::WordCount:
		case Tree::Node::FullTextLength:
			{
				return _ImplType::Length;
			}
		case Tree::Node::Kwic:
			{
				return _ImplType::Kwic;
			}
		case Tree::Node::Tf:
		case Tree::Node::ClusterID:
		case Tree::Node::FeatureValue:
		case Tree::Node::Score:
		case Tree::Node::Section:
		case Tree::Node::Word:
		case Tree::Node::WordDf:
		case Tree::Node::WordScale:
		case Tree::Node::RoughKwicPosition:
		case Tree::Node::Existence:
			{
				return _ImplType::FullText;
			}
		case Tree::Node::NeighborID:
		case Tree::Node::NeighborDistance:
			{
				return _ImplType::Spatial;
			}
		default:
			{
				return _ImplType::Function;
			}
		}
	}



	// FUNCTION local
	//	$$$::_isCheckLocator -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	Tree::Node::Type eOperator_
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS
	bool
	_isCheckLocator(Tree::Node::Type eOperator_)
	{
		switch (eOperator_) {
		case Tree::Node::CharLength:
		case Tree::Node::StringConcatenate:
		case Tree::Node::SubString:
		case Tree::Node::Overlay:
			{
				return true;
			}
		default:
			{
				return false;
			}
		}
	}

		// FUNCTION local
	//	$$$::_isDistribution
	//
	// NOTES
	//
	// ARGUMENTS
	//	Opt::Environment cEnvironment_
	//	Plan::Interface::IScalar* pOperand_
	//	
	// RETURN
	//	_ImplType::Value
	//
	// EXCEPTIONS
	bool 
	_isDistribution(Opt::Environment& cEnvironment_, Interface::IScalar* pOperand_)
	{
		if (cEnvironment_.hasCascade()) {
			Utility::RelationSet cRelationSet;
			pOperand_->getUsedTable(cRelationSet);
			Plan::Relation::InquiryArgument cArgument = 0;
			cArgument.m_iTarget |= Plan::Relation::InquiryArgument::Target::Distributed;
			VECTOR<Interface::IRelation*>::Iterator ite = cRelationSet.begin();
			for (; ite != cRelationSet.end(); ++ite) {
				Interface::IRelation::InquiryResult iResult = (*ite)->inquiry(cEnvironment_, cArgument);
				if ((iResult & Relation::InquiryArgument::Target::Distributed)) {
					return true;
				}
			}
		}
		return false;
	}
}

////////////////////////////////////
//	Plan::Scalar::Function

// FUNCTION public
//	Scalar::Function::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult = new This(eOperator_, cstrName_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   const STRING& cstrName_)
{
	if (vecOperand_.GETSIZE() == 2) {
		return create(cEnvironment_,
					  eOperator_,
					  MAKEPAIR(vecOperand_[0],
							   vecOperand_[1]),
					  cstrName_);
	}

	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
	case _ImplType::Length:
	case _ImplType::Kwic:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::Arithmetic:
		{
			return Arithmetic::create(cEnvironment_,
									  eOperator_,
									  vecOperand_,
									  cstrName_);
		}
	case _ImplType::Array:
		{
			; _SYDNEY_ASSERT(eOperator_ == Tree::Node::ArrayConstructor);
			pResult = new ArrayImpl::Constructor::Nadic(eOperator_,
														cstrName_,
														vecOperand_);
			break;
		}
	case _ImplType::FullText:
		{
			return FullText::create(cEnvironment_,
									eOperator_,
									vecOperand_,
									cstrName_);
		}
	case _ImplType::Spatial:
		{
			return Spatial::create(cEnvironment_,
								   eOperator_,
								   vecOperand_,
								   cstrName_);
		}
	default:
		{
			pResult = new FunctionImpl::Nadic(eOperator_,
											  cstrName_,
											  vecOperand_);
			break;
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
	case _ImplType::Length:
	case _ImplType::Kwic:
	case _ImplType::Spatial:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::Arithmetic:
		{
			return Arithmetic::create(cEnvironment_,
									  eOperator_,
									  cOperand_,
									  cstrName_);
		}
	case _ImplType::Array:
		{
			; _SYDNEY_ASSERT(eOperator_ == Tree::Node::ArrayConstructor);
			pResult = new ArrayImpl::Constructor::Dyadic(eOperator_,
														 cstrName_,
														 cOperand_);
			break;
		}
	case _ImplType::FullText:
		{
			return FullText::create(cEnvironment_,
									eOperator_,
									cOperand_,
									cstrName_);
		}
	default:
		{
			if (_isCheckLocator(eOperator_)
				&& cOperand_.first->isField()
				&& cOperand_.first->getField()->isLob()) {
				return Locator::create(cEnvironment_,
									   eOperator_,
									   cOperand_,
									   cstrName_);
			}
			pResult = new FunctionImpl::Dyadic(eOperator_,
											   cstrName_,
											   cOperand_);
			break;
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- constructor
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
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
		{
			if (_isDistribution(cEnvironment_, pOperand_)) {
				return DPlan::Scalar::Aggregation::create(cEnvironment_,
														  eOperator_,
														  pOperand_,
														  cstrName_);
			} else {
				return Aggregation::create(cEnvironment_,
										   eOperator_,
										   pOperand_,
										   cstrName_);
			}
		}
	case _ImplType::Arithmetic:
		{
			return Arithmetic::create(cEnvironment_,
									  eOperator_,
									  pOperand_,
									  cstrName_);
		}
	case _ImplType::Array:
		{
			; _SYDNEY_ASSERT(eOperator_ == Tree::Node::ArrayConstructor);
			pResult = new ArrayImpl::Constructor::Monadic(eOperator_,
														  cstrName_,
														  pOperand_);
			break;
		}
	case _ImplType::Length:
		{
			if (_isCheckLocator(eOperator_)
				&& pOperand_->isField()
				&& pOperand_->getField()->isLob()) {
				return Locator::create(cEnvironment_,
									   eOperator_,
									   pOperand_,
									   cstrName_);
			}
			pResult = new LengthImpl::Monadic(eOperator_,
											  cstrName_,
											  pOperand_);
			break;
		}
	case _ImplType::Kwic:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::FullText:
		{
			return FullText::create(cEnvironment_,
									eOperator_,
									pOperand_,
									cstrName_);
		}

	case _ImplType::Pattern:
		{
			pResult = new PatternImpl::Monadic(eOperator_,
											   cstrName_,
											   pOperand_);
			break;
		}
	case _ImplType::Spatial:
		{
			return Spatial::create(cEnvironment_,
								   eOperator_,
								   pOperand_,
								   cstrName_);
		}

	default:
		{
			pResult = new FunctionImpl::Monadic(eOperator_,
												cstrName_,
												pOperand_);
			break;
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	Interface::IScalar* pOption_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   Interface::IScalar* pOption_,
	   const STRING& cstrName_)
{
	if (pOption_ == 0) {
		return create(cEnvironment_,
					  eOperator_,
					  vecOperand_,
					  cstrName_);
	}
	if (vecOperand_.GETSIZE() == 2) {
		return create(cEnvironment_,
					  eOperator_,
					  MAKEPAIR(vecOperand_[0],
							   vecOperand_[1]),
					  pOption_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
	case _ImplType::Arithmetic:
	case _ImplType::Array:
	case _ImplType::Length:
	case _ImplType::Kwic:
	case _ImplType::FullText:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::Spatial:
		{
			return Spatial::create(cEnvironment_,
								   eOperator_,
								   vecOperand_,
								   pOption_,
								   cstrName_);
		}

	default:
		{
			pResult =
				new FunctionImpl::NadicWithMonadicOption(eOperator_,
														 cstrName_,
														 vecOperand_,
														 pOption_);
			break;
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const STRING& cstrName_)
{
	if (vecOption_.ISEMPTY()) {
		return create(cEnvironment_,
					  eOperator_,
					  vecOperand_,
					  cstrName_);
	}
	if (vecOperand_.GETSIZE() == 2) {
		return create(cEnvironment_,
					  eOperator_,
					  MAKEPAIR(vecOperand_[0],
							   vecOperand_[1]),
					  vecOption_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
	case _ImplType::Arithmetic:
	case _ImplType::Array:
	case _ImplType::Length:
	case _ImplType::Kwic:
	case _ImplType::FullText:
	case _ImplType::Spatial:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	default:
		{
			pResult =
				new FunctionImpl::NadicWithNadicOption(eOperator_,
													   cstrName_,
													   vecOperand_,
													   vecOption_);
			break;
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- 2-arguments 1-option
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	Interface::IScalar* pOption_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   Interface::IScalar* pOption_,
	   const STRING& cstrName_)
{
	if (pOption_ == 0) {
		return create(cEnvironment_,
					  eOperator_,
					  cOperand_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult =
		new FunctionImpl::DyadicWithMonadicOption(eOperator_,
												  cstrName_,
												  cOperand_,
												  pOption_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- 2-arguments N-options
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const STRING& cstrName_)
{
	if (vecOption_.ISEMPTY()) {
		return create(cEnvironment_,
					  eOperator_,
					  cOperand_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
	case _ImplType::Arithmetic:
	case _ImplType::Array:
	case _ImplType::Length:
	case _ImplType::FullText:
	case _ImplType::Spatial:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::Kwic:
		{
			pResult = new Impl::KwicImpl(eOperator_,
										 cstrName_,
										 cOperand_,
										 vecOption_);
			; _SYDNEY_ASSERT(vecOption_.GETSIZE() >= 1);
			; _SYDNEY_ASSERT(vecOption_[0]->getType() == Tree::Node::KwicSize);
			// create kwic option
			FOREACH(cEnvironment_.getContainsByAnyOperand(cOperand_.first),
					boost::bind(&Predicate::Contains::createKwicOption,
								_1,
								boost::ref(cEnvironment_),
								vecOption_[0]));
			break;
		}
	default:
		{
			if (_isCheckLocator(eOperator_)
				&& cOperand_.first->isField()
				&& cOperand_.first->getField()->isLob()) {
				return Locator::create(cEnvironment_,
									   eOperator_,
									   cOperand_,
									   vecOption_,
									   cstrName_);
			}
			pResult = new FunctionImpl::DyadicWithNadicOption(eOperator_,
															  cstrName_,
															  cOperand_,
															  vecOption_);
			break;
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	Interface::IScalar* pOption_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   Interface::IScalar* pOption_,
	   const STRING& cstrName_)
{
	if (pOption_ == 0) {
		return create(cEnvironment_,
					  eOperator_,
					  pOperand_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Arithmetic:
	case _ImplType::Length:
	case _ImplType::Kwic:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::Aggregation:
		{
			if (_isDistribution(cEnvironment_, pOperand_)) {
				return DPlan::Scalar::Aggregation::create(cEnvironment_,
														  eOperator_,
														  pOperand_,
														  pOption_,
														  cstrName_);
			} else {			
				return Aggregation::create(cEnvironment_,
										   eOperator_,
										   pOperand_,
										   pOption_,
										   cstrName_);
			}
		}
	case _ImplType::Array:
		{
			; _SYDNEY_ASSERT(eOperator_ == Tree::Node::ElementReference);
			if (pOption_->getType() == Tree::Node::All) {
				pResult =
					new ArrayImpl::Element::Arbitrary(eOperator_,
													  cstrName_,
													  pOperand_,
													  pOption_);
			} else {
				pResult =
					new ArrayImpl::Element::Reference(eOperator_,
													  cstrName_,
													  pOperand_,
													  pOption_);
			}
			break;
		}
	case _ImplType::FullText:
		{
			return FullText::create(cEnvironment_,
									eOperator_,
									pOperand_,
									pOption_,
									cstrName_);
		}
	case _ImplType::Pattern:
		{
			pResult = new PatternImpl::MonadicWithMonadicOption(eOperator_,
																cstrName_,
																pOperand_,
																pOption_);
		}
	case _ImplType::Spatial:
		{
			return Spatial::create(cEnvironment_,
								   eOperator_,
								   pOperand_,
								   pOption_,
								   cstrName_);
		}
	default:
		{
			pResult =
				new FunctionImpl::MonadicWithMonadicOption(eOperator_,
														   cstrName_,
														   pOperand_,
														   pOption_);
			break;
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const STRING& cstrName_)
{
	if (vecOption_.ISEMPTY()) {
		return create(cEnvironment_,
					  eOperator_,
					  pOperand_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
	case _ImplType::Arithmetic:
	case _ImplType::Array:
	case _ImplType::Length:
	case _ImplType::Kwic:
	case _ImplType::Spatial:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::FullText:
		{
			return FullText::create(cEnvironment_,
									eOperator_,
									pOperand_,
									vecOption_,
									cstrName_);
		}
	default:
		{
			if (_isCheckLocator(eOperator_)
				&& pOperand_->isField()
				&& pOperand_->getField()->isLob()) {
				return Locator::create(cEnvironment_,
									   eOperator_,
									   pOperand_,
									   vecOption_,
									   cstrName_);
			}
			pResult =
				new FunctionImpl::MonadicWithNadicOption(eOperator_,
														 cstrName_,
														 pOperand_,
														 vecOption_);
			break;
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult = new This(eOperator_,
										 cDataType_,
										 cstrName_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	if (vecOperand_.GETSIZE() == 2) {
		return create(cEnvironment_,
					  eOperator_,
					  MAKEPAIR(vecOperand_[0],
							   vecOperand_[1]),
					  cDataType_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
	case _ImplType::Length:
	case _ImplType::Kwic:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::Arithmetic:
		{
			return Arithmetic::create(cEnvironment_,
									  eOperator_,
									  vecOperand_,
									  cDataType_,
									  cstrName_);
		}
	case _ImplType::Array:
		{
			; _SYDNEY_ASSERT(eOperator_ == Tree::Node::ArrayConstructor);
			pResult =
				new ArrayImpl::Constructor::Nadic(eOperator_,
												  cDataType_,
												  cstrName_,
												  vecOperand_);
			break;
		}
	case _ImplType::FullText:
		{
			return FullText::create(cEnvironment_,
									eOperator_,
									vecOperand_,
									cDataType_,
									cstrName_);
		}
	case _ImplType::Spatial:
		{
			return Spatial::create(cEnvironment_,
								   eOperator_,
								   vecOperand_,
								   cDataType_,
								   cstrName_);
		}
	default:
		{
			pResult =
				new FunctionImpl::Nadic(eOperator_,
										cDataType_,
										cstrName_,
										vecOperand_);
			break;
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
	case _ImplType::Length:
	case _ImplType::Kwic:
	case _ImplType::Spatial:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::Arithmetic:
		{
			return Arithmetic::create(cEnvironment_,
									  eOperator_,
									  cOperand_,
									  cDataType_,
									  cstrName_);
		}
	case _ImplType::Array:
		{
			; _SYDNEY_ASSERT(eOperator_ == Tree::Node::ArrayConstructor);
			pResult = new ArrayImpl::Constructor::Dyadic(eOperator_,
														 cDataType_,
														 cstrName_,
														 cOperand_);
			break;
		}
	case _ImplType::FullText:
		{
			return FullText::create(cEnvironment_,
									eOperator_,
									cOperand_,
									cDataType_,
									cstrName_);
		}
	default:
		{
			if (_isCheckLocator(eOperator_)
				&& cOperand_.first->isField()
				&& cOperand_.first->getField()->isLob()) {
				return Locator::create(cEnvironment_,
									   eOperator_,
									   cOperand_,
									   cDataType_,
									   cstrName_);
			}
			pResult = new FunctionImpl::Dyadic(eOperator_,
											   cDataType_,
											   cstrName_,
											   cOperand_);
			break;
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
		{
			return Aggregation::create(cEnvironment_,
									   eOperator_,
									   pOperand_,
									   cDataType_,
									   cstrName_);
		}
	case _ImplType::Arithmetic:
		{
			return Arithmetic::create(cEnvironment_,
									  eOperator_,
									  pOperand_,
									  cDataType_,
									  cstrName_);
		}
	case _ImplType::Array:
		{
			pResult = new ArrayImpl::Constructor::Monadic(eOperator_,
														  cDataType_,
														  cstrName_,
														  pOperand_);
			break;
		}
	case _ImplType::Length:
		{
			if (_isCheckLocator(eOperator_)
				&& pOperand_->isField()
				&& pOperand_->getField()->isLob()) {
				return Locator::create(cEnvironment_,
									   eOperator_,
									   pOperand_,
									   cDataType_,
									   cstrName_);
			}
			pResult = new LengthImpl::Monadic(eOperator_,
											  cDataType_,
											  cstrName_,
											  pOperand_);
			break;
		}
	case _ImplType::Kwic:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::FullText:
		{
			return FullText::create(cEnvironment_,
									eOperator_,
									pOperand_,
									cDataType_,
									cstrName_);
		}
	case _ImplType::Spatial:
		{
			return Spatial::create(cEnvironment_,
								   eOperator_,
								   pOperand_,
								   cDataType_,
								   cstrName_);
		}
	default:
		{
			pResult = new FunctionImpl::Monadic(eOperator_,
												cDataType_,
												cstrName_,
												pOperand_);
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- N-arguments 1-option (with data type)
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	Interface::IScalar* pOption_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   Interface::IScalar* pOption_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	if (pOption_ == 0) {
		return create(cEnvironment_,
					  eOperator_,
					  vecOperand_,
					  cDataType_,
					  cstrName_);
	}
	if (vecOperand_.GETSIZE() == 2) {
		return create(cEnvironment_,
					  eOperator_,
					  MAKEPAIR(vecOperand_[0],
							   vecOperand_[1]),
					  pOption_,
					  cDataType_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
	case _ImplType::Arithmetic:
	case _ImplType::Array:
	case _ImplType::Length:
	case _ImplType::Kwic:
	case _ImplType::FullText:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::Spatial:
		{
			return Spatial::create(cEnvironment_,
								   eOperator_,
								   vecOperand_,
								   pOption_,
								   cDataType_,
								   cstrName_);
		}
	default:
		{
			pResult =
				new FunctionImpl::NadicWithMonadicOption(eOperator_,
														 cDataType_,
														 cstrName_,
														 vecOperand_,
														 pOption_);
			break;
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- N-arguments N-options (with data type)
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	if (vecOption_.ISEMPTY()) {
		return create(cEnvironment_,
					  eOperator_,
					  vecOperand_,
					  cDataType_,
					  cstrName_);
	}
	if (vecOperand_.GETSIZE() == 2) {
		return create(cEnvironment_,
					  eOperator_,
					  MAKEPAIR(vecOperand_[0],
							   vecOperand_[1]),
					  vecOption_,
					  cDataType_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
	case _ImplType::Arithmetic:
	case _ImplType::Array:
	case _ImplType::Length:
	case _ImplType::Kwic:
	case _ImplType::FullText:
	case _ImplType::Spatial:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	default:
		{
			pResult =
				new FunctionImpl::NadicWithNadicOption(eOperator_,
													   cDataType_,
													   cstrName_,
													   vecOperand_,
													   vecOption_);
			break;
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- 2-arguments 1-option (with data type)
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	Interface::IScalar* pOption_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   Interface::IScalar* pOption_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	if (pOption_ == 0) {
		return create(cEnvironment_,
					  eOperator_,
					  cOperand_,
					  cDataType_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult =
		new FunctionImpl::DyadicWithMonadicOption(eOperator_,
												  cDataType_,
												  cstrName_,
												  cOperand_,
												  pOption_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- 2-arguments N-options (with data type)
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	if (vecOption_.ISEMPTY()) {
		return create(cEnvironment_,
					  eOperator_,
					  cOperand_,
					  cDataType_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult =
		new FunctionImpl::DyadicWithNadicOption(eOperator_,
												cDataType_,
												cstrName_,
												cOperand_,
												vecOption_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- 1-argument 1-option (with data type)
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	Interface::IScalar* pOption_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   Interface::IScalar* pOption_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	if (pOption_ == 0) {
		return create(cEnvironment_,
					  eOperator_,
					  pOperand_,
					  cDataType_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Length:
	case _ImplType::Kwic:
	case _ImplType::Arithmetic:
	case _ImplType::FullText:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::Aggregation:
		{
			
			return Aggregation::create(cEnvironment_,
									   eOperator_,
									   pOperand_,
									   pOption_,
									   cDataType_,
									   cstrName_);
		}
	case _ImplType::Array:
		{
			; _SYDNEY_ASSERT(eOperator_ == Tree::Node::ElementReference);
			if (pOption_->getType() == Tree::Node::All) {
				pResult =
					new ArrayImpl::Element::Arbitrary(eOperator_,
													  cDataType_,
													  cstrName_,
													  pOperand_,
													  pOption_);
			} else {
				pResult =
					new ArrayImpl::Element::Reference(eOperator_,
													  cDataType_,
													  cstrName_,
													  pOperand_,
													  pOption_);
			}
			break;
		}
	case _ImplType::Spatial:
		{
			
			return Spatial::create(cEnvironment_,
								   eOperator_,
								   pOperand_,
								   pOption_,
								   cDataType_,
								   cstrName_);
		}
	default:
		{
			pResult =
				new FunctionImpl::MonadicWithMonadicOption(eOperator_,
														   cDataType_,
														   cstrName_,
														   pOperand_,
														   pOption_);
			break;
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::create -- 1-argument N-options (with data type)
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Function*
//
// EXCEPTIONS

//static
Function*
Function::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	if (vecOption_.ISEMPTY()) {
		return create(cEnvironment_,
					  eOperator_,
					  pOperand_,
					  cDataType_,
					  cstrName_);
	}
	AUTOPOINTER<This> pResult;
	switch (_getImplType(eOperator_)) {
	case _ImplType::Aggregation:
	case _ImplType::Arithmetic:
	case _ImplType::Array:
	case _ImplType::Length:
	case _ImplType::Kwic:
	case _ImplType::Spatial:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	case _ImplType::FullText:
		{
			return FullText::create(cEnvironment_,
									eOperator_,
									pOperand_,
									vecOption_,
									cDataType_,
									cstrName_);
		}
	default:
		{
			if (_isCheckLocator(eOperator_)
				&& pOperand_->isField()
				&& pOperand_->getField()->isLob()) {
				return Locator::create(cEnvironment_,
									   eOperator_,
									   pOperand_,
									   vecOption_,
									   cstrName_);
			}
			pResult =
				new FunctionImpl::MonadicWithNadicOption(eOperator_,
														 cDataType_,
														 cstrName_,
														 pOperand_,
														 vecOption_);
			break;
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Function::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Function::Check::Value
//
// EXCEPTIONS

//virtual
Function::Check::Value
Function::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// niladic function can not be regarded as a member of a relation
	// so, it can be regarged as constant
	return Check::Constant;
}

// FUNCTION public
//	Scalar::Function::isRefering -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Function::
isRefering(Interface::IRelation* pRelation_)
{
	// niladic function can not be regarded as a member of a relation
	return pRelation_ == 0;
}

// FUNCTION public
//	Scalar::Function::getUsedTable -- check used tables
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
Function::
getUsedTable(Utility::RelationSet& cResult_)
{
	// nilladic function does not include any tables
	;
}

// FUNCTION public
//	Scalar::Function::getUsedField -- 
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
Function::
getUsedField(Utility::FieldSet& cResult_)
{
	// niladic function does not include any  fields
	;
}

// FUNCTION public
//	Scalar::Function::getUnknownKey -- 
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
Function::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	// niladic function can become null
	cResult_.m_cKey.add(this);
}

// FUNCTION public
//	Scalar::Function::require -- create refered scalar
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
Function::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	// niladic function is always unnecessary to read from a relation
	;
}

// FUNCTION public
//	Scalar::Function::retrieve -- 
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
Function::
retrieve(Opt::Environment& cEnvironment_)
{
	// niladic function is always unnecessary to read from a relation
	;
}

// FUNCTION public
//	Scalar::Function::retrieve -- create refered scalar
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
Function::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	// niladic function is always unnecessary to read from a relation
	;
}

// FUNCTION public
//	Scalar::Function::delay -- create refered scalar
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
Function::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	// niladic function is always unnecessary to read from a relation
	// -> can be regarded as delayable
	return true;
}

// FUNCTION public
//	Scalar::Function::setMetaData -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Common::ColumnMetaData& cMetaData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Function::
setMetaData(Opt::Environment& cEnvironment_,
			Common::ColumnMetaData& cMetaData_)
{
	cMetaData_.setNotSearchable();
	cMetaData_.setReadOnly();
}


// FUNCTION public
//	Scalar::Function::retrieveFromCascade -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Sql::Query* pQuery_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Function::
retrieveFromCascade(Opt::Environment& cEnvironment_,
					Plan::Sql::Query* pQuery_)
{
	pQuery_->addProjectionColumn(this);
}


// FUNCTION protected
//	Scalar::Function::getCompatibleType -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IScalar* pOperand_
//	DataType& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Function::
getCompatibleType(Interface::IScalar* pOperand_,
				  DataType& cResult_)
{
	if (cResult_.isNoType()) {
		cResult_ = pOperand_->getDataType();
		return true;
	} else {
		DataType cTmp(cResult_);
		return DataType::getCompatibleType(cTmp,
										   pOperand_->getDataType(),
										   cResult_,
										   false, /* not for comparison */
										   getType(),
										   (getType() == Tree::Node::List) /* for list */);
	}
}

// FUNCTION protected
//	Scalar::Function::generateThis -- generate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Function::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	pIterator_->addCalculation(cProgram_,
							   Execution::Function::Factory::create(
												cProgram_,
												pIterator_,
												getType(),
												iDataID_),
							   cArgument_.m_eTarget);
	return iDataID_;
}

// FUNCTION private
//	Scalar::Function::createDataType -- create result type
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
Function::
createDataType(Opt::Environment& cEnvironment_)
{
	// default setting of datatype of each function
	switch (getType()) {
	case Tree::Node::CurrentTimestamp:
		{
			setDataType(DataType::getTimestampType());
			break;
		}
	default:
		{
			break;
		}
	}
}

// FUNCTION public
//	Scalar::Function::getSearchFile -- get key fields of searchable files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const GetFileArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Function::
getSearchFile(Opt::Environment& cEnvironment_,
			  const GetFileArgument& cArgument_)
{
	if(cArgument_.m_pField->getType() != Tree::Node::List) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	IScalar* pScalar = cArgument_.m_pField;
	for (ModSize i = 0; i < pScalar->getOperandSize(); ++i) {
		TreeNodeInterface* pNode =
			const_cast<TreeNodeInterface*>(pScalar->getOperandAt(i));
		if (pNode->getType() != Tree::Node::Field) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		Interface::IScalar* pField =
			_SYDNEY_DYNAMIC_CAST(Interface::IScalar*, pNode);
		; _SYDNEY_ASSERT(pField);
		
		Utility::FileSet cFile;
		Scalar::GetFileArgument cFileArg(pField, cArgument_.m_pPredicate, cFile);
		if(!Scalar::Field::getSearchFile(cEnvironment_, cFileArg)) {
			return false;
		}
		if (i == 0) {
			cArgument_.m_cFile.clear();
			cArgument_.m_cFile.add(cFile.begin(), cFile.end());
		} else {
			cArgument_.m_cFile.intersect(cFile);
		}

	}
	return true;

}


// FUNCTION public
//	Scalar::Function::getCandidate -- find corresponding table candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pFunction_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Candidate::Table*
//
// EXCEPTIONS

//static
Candidate::Table*
Function::
getCandidate(Opt::Environment& cEnvironment_,
			 Interface::IScalar* pFunction_,
			 Interface::ICandidate* pCandidate_)
{
	; _SYDNEY_ASSERT(pFunction_->getType() == Tree::Node::List);
	Function* pFunction = _SYDNEY_DYNAMIC_CAST(Function*, pFunction_);
	; _SYDNEY_ASSERT(pFunction);
	
	Candidate::Table* pResult = 0;
	for (ModSize i = 0; i < pFunction_->getOperandSize(); ++i) {
		TreeNodeInterface* pNode =
			const_cast<TreeNodeInterface*>(pFunction_->getOperandAt(i));
		if (pNode->getType() != Tree::Node::Field) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		Scalar::Field* pField = _SYDNEY_DYNAMIC_CAST(Scalar::Field*, pNode);
		; _SYDNEY_ASSERT(pField);

		if(pResult == 0) {
			pResult = Scalar::Field::getCandidate(cEnvironment_,
												  pField,
												  pCandidate_);
		} else if(pResult != Scalar::Field::getCandidate(cEnvironment_,
													  pField,
													  pCandidate_)) {
				_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	return pResult;
}


_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
