// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/File.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Candidate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Impl/FileImpl.h"
#include "Plan/Predicate/ChosenInterface.h"

#include "Exception/Unexpected.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

/////////////////////////////
// Candidate::File::

// FUNCTION public
//	Candidate::File::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::Table* pTable_
//	Interface::IFile* pFile_
//	Parameter* pParameter_
//	
// RETURN
//	File*
//
// EXCEPTIONS

//static
File*
File::
create(Opt::Environment& cEnvironment_,
	   Candidate::Table* pTable_,
	   Interface::IFile* pFile_,
	   Parameter* pParameter_)
{
	AUTOPOINTER<This> pResult = new FileImpl::Normal(pTable_, pFile_, pParameter_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::File::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::Table* pTable_
//	Interface::IFile* pFile_
//	Interface::IScalar* pVariable_
//	
// RETURN
//	File*
//
// EXCEPTIONS

//static
File*
File::
create(Opt::Environment& cEnvironment_,
	   Candidate::Table* pTable_,
	   Interface::IFile* pFile_,
	   Interface::IScalar* pVariable_)
{
	AUTOPOINTER<This> pResult = new FileImpl::Variable(pTable_, pFile_, pVariable_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::File::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::Table* pTable_
//	const VECTOR<This*>& vecFiles_
//	Interface::IPredicate* pPredicate_
//	const VECTOR<Interface::IPredicate*>& vecPredicates_
//	Order::Specification* pOrder_
//	
// RETURN
//	File*
//
// EXCEPTIONS

//static
File*
File::
create(Opt::Environment& cEnvironment_,
	   Candidate::Table* pTable_,
	   const VECTOR<This*>& vecFiles_,
	   Interface::IPredicate* pPredicate_,
	   const VECTOR<Interface::IPredicate*>& vecPredicates_,
	   Order::Specification* pOrder_)
{
	AUTOPOINTER<This> pResult;
	switch (pPredicate_->getType()) {
	case Tree::Node::And:
		{
			pResult = new FileImpl::Intersect(pTable_, vecFiles_, pPredicate_, vecPredicates_, pOrder_);
			break;
		}
	case Tree::Node::Or:
		{
			pResult = new FileImpl::Union(pTable_, vecFiles_, pPredicate_, vecPredicates_, pOrder_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::File::erase -- destructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	This* pThis_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
File::
erase(Opt::Environment& cEnvironment_,
	  This* pThis_)
{
	if (pThis_) {
		pThis_->eraseFromEnvironment(cEnvironment_);
		delete pThis_;
	}
}

// FUNCTION public
//	Candidate::File::insertCheckIndexResult -- insert index availability information
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::ChosenInterface* pPredicate_
//	VECTOR<Predicate::ChosenInterface*>* pTarget_
//	This* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
File::
insertCheckIndexResult(Opt::Environment& cEnvironment_,
					   Predicate::ChosenInterface* pPredicate_,
					   VECTOR<Predicate::ChosenInterface*>* pTarget_,
					   This* pFile_)
{
	// insert result in count order
	VECTOR<Predicate::ChosenInterface*>::ITERATOR iterator = pTarget_->begin();
	const VECTOR<Predicate::ChosenInterface*>::ITERATOR last = pTarget_->end();
	for (; iterator != last; ++iterator) {
		if ((*iterator)->getType() == Tree::Node::Not) break;
		File* pFile = (*iterator)->getFile(cEnvironment_);
		// GetBy && SearchBy is placed after GetBy && !SearchBy
		// -> use count in any cases because GetBy&&!SearchBy can be large results

		AccessPlan::Cost cCost0;
		AccessPlan::Cost cCost1;
		pPredicate_->getCost(cEnvironment_, cCost0);
		(*iterator)->getCost(cEnvironment_, cCost1);

		if (cCost0.getRate() < cCost1.getRate()
			|| (cCost0.getRate() == cCost1.getRate()
				&& cCost0.getTupleCount() < cCost1.getTupleCount())) {
			break;
		}
	}
	if (iterator != last) {
		pTarget_->insert(iterator, pPredicate_);
	} else {
		pTarget_->PUSHBACK(pPredicate_);
	}
}

// FUNCTION public
//	Candidate::File::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Parameter* pParameter_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
File::
setParameter(Parameter* pParameter_)
{
	if (m_pParameter) {
		if (m_pParameter->getPredicate()
			&& pParameter_->getPredicate() == 0) {
			pParameter_->setPredicate(m_pParameter->getPredicate());
		}
		if (m_pParameter->getOrder()
			&& pParameter_->getOrder() == 0) {
			pParameter_->setOrder(m_pParameter->getOrder());
		}
		if (m_pParameter->isLimited()
			&& pParameter_->isLimited() == false) {
			pParameter_->setIsLimited(true);
		}
		if (m_pParameter->isGetByBitSet()
			&& pParameter_->isGetByBitSet() == false) {
			pParameter_->setIsGetByBitSet(true);
		}
		if (m_pParameter->isSearchByBitSet()
			&& pParameter_->isSearchByBitSet() == false) {
			pParameter_->setIsSearchByBitSet(true);
		}
	}
	m_pParameter = pParameter_;
}

// FUNCTION private
//	Candidate::File::registerToEnvironment -- register to environment
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

void
File::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	m_iID = cEnvironment_.addObject(this);
}

// FUNCTION private
//	Candidate::File::eraseFromEnvironment -- erase from environment
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

void
File::
eraseFromEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.eraseObject(m_iID);
}

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
