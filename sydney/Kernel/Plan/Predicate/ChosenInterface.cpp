// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/ChosenInterface.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Predicate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/ChosenInterface.h"
#include "Plan/Predicate/Impl/ChosenImpl.h"

#include "Common/Assert.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

/////////////////////////////////////
// Predicate::ChosenInterface

// FUNCTION public
//	Predicate::ChosenInterface::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	ChosenInterface*
//
// EXCEPTIONS

//static
ChosenInterface*
ChosenInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_)
{
	AUTOPOINTER<This> pResult = new ChosenImpl::Base(pPredicate_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::ChosenInterface::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Candidate::File* pFile_
//	
// RETURN
//	ChosenInterface*
//
// EXCEPTIONS

//static
ChosenInterface*
ChosenInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   Candidate::File* pFile_)
{
	if (pFile_ == 0) return create(cEnvironment_, pPredicate_);

	AUTOPOINTER<This> pResult = new ChosenImpl::SingleImpl(pPredicate_, pFile_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::ChosenInterface::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Candidate::File* pFile_
//	Interface::IPredicate* pCheckUnknown_
//	
// RETURN
//	ChosenInterface*
//
// EXCEPTIONS

//static
ChosenInterface*
ChosenInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   Candidate::File* pFile_,
	   Interface::IPredicate* pCheckUnknown_)
{
	if (pCheckUnknown_ == 0) return create(cEnvironment_, pPredicate_, pFile_);
	; _SYDNEY_ASSERT(pFile_);

	AUTOPOINTER<This> pResult = new ChosenImpl::SingleCheckUnknownImpl(pPredicate_, pFile_, pCheckUnknown_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::ChosenInterface::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Interface::IPredicate* pNotChecked_
//	Candidate::File* pFile_
//	
// RETURN
//	ChosenInterface*
//
// EXCEPTIONS

//static
ChosenInterface*
ChosenInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   Interface::IPredicate* pNotChecked_,
	   Candidate::File* pFile_)
{
	if (pNotChecked_ == 0) return create(cEnvironment_, pPredicate_, pFile_);

	AUTOPOINTER<This> pResult;
	if (pFile_ == 0) {
		pResult = new ChosenImpl::PartialNoFileImpl(pPredicate_, pNotChecked_);
	} else {
		pResult = new ChosenImpl::PartialSingleImpl(pPredicate_, pNotChecked_, pFile_);
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::ChosenInterface::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Interface::IPredicate* pNotChecked_
//	Candidate::File* pFile_
//	Interface::IPredicate* pCheckUnknown_
//	
// RETURN
//	ChosenInterface*
//
// EXCEPTIONS

//static
ChosenInterface*
ChosenInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   Interface::IPredicate* pNotChecked_,
	   Candidate::File* pFile_,
	   Interface::IPredicate* pCheckUnknown_)
{
	if (pNotChecked_ == 0) return create(cEnvironment_, pPredicate_, pFile_, pCheckUnknown_);
	if (pCheckUnknown_ == 0) return create(cEnvironment_, pPredicate_, pNotChecked_, pFile_);
	; _SYDNEY_ASSERT(pFile_);

	AUTOPOINTER<This> pResult = 
		new ChosenImpl::PartialSingleCheckUnknownImpl(pPredicate_, pNotChecked_, pFile_, pCheckUnknown_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::ChosenInterface::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	const VECTOR<Interface::IPredicate*>& vecPredicate_
//	
// RETURN
//	ChosenInterface*
//
// EXCEPTIONS

//static
ChosenInterface*
ChosenInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   const VECTOR<Interface::IPredicate*>& vecPredicate_)
{
	if (vecPredicate_.ISEMPTY()) return create(cEnvironment_, pPredicate_);

	AUTOPOINTER<This> pResult;
	if (pPredicate_->getType() == Tree::Node::And
		|| pPredicate_->getType() == Tree::Node::Fetch) {
		pResult = new ChosenImpl::AndImpl(pPredicate_,
										  vecPredicate_);
	} else if (pPredicate_->getType() == Tree::Node::Or) {
		pResult = new ChosenImpl::OrImpl(pPredicate_,
										 vecPredicate_);
	} else {
		; _SYDNEY_ASSERT(vecPredicate_.GETSIZE() == 1);
		; _SYDNEY_ASSERT(vecPredicate_[0]->isChosen());
		return create(cEnvironment_,
					  pPredicate_,
					  vecPredicate_[0]->getChosen()->getFile(cEnvironment_));
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::ChosenInterface::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Interface::IPredicate* pNotChecked_
//	const VECTOR<Interface::IPredicate*>& vecPredicate_
//	
// RETURN
//	ChosenInterface*
//
// EXCEPTIONS

//static
ChosenInterface*
ChosenInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   Interface::IPredicate* pNotChecked_,
	   const VECTOR<Interface::IPredicate*>& vecPredicate_)
{
	if (pNotChecked_ == 0) {
		return create(cEnvironment_, pPredicate_, vecPredicate_);
	}

	AUTOPOINTER<This> pResult;
	if (pPredicate_->getType() == Tree::Node::And) {
		pResult =
			new ChosenImpl::PartialAndImpl(pPredicate_,
										   pNotChecked_,
										   vecPredicate_);
	} else if (pPredicate_->getType() == Tree::Node::Or) {
		pResult =
			new ChosenImpl::PartialOrImpl(pPredicate_,
										  pNotChecked_,
										  vecPredicate_);
	} else {
		; _SYDNEY_ASSERT(vecPredicate_.GETSIZE() == 1);
		; _SYDNEY_ASSERT(vecPredicate_[0]->isChosen());
		if (pPredicate_->getType() == Tree::Node::Not
			&& vecPredicate_[0]->getChosen()->isIndexAvailable(cEnvironment_)) {
			ChosenImpl::NotImpl* pNot = _SYDNEY_DYNAMIC_CAST(ChosenImpl::NotImpl*,
															 vecPredicate_[0]->getChosen());
			; _SYDNEY_ASSERT(pNot);
			pResult =
				new ChosenImpl::PartialNotImpl(pPredicate_,
											   pNotChecked_,
											   pNot->getOperand());
		} else if (vecPredicate_[0]->getChosen()->getFile(cEnvironment_) == 0) {
			pResult = new ChosenImpl::PartialNoFileImpl(pPredicate_,
														pNotChecked_);

		} else {
			return create(cEnvironment_,
						  pPredicate_,
						  pNotChecked_,
						  vecPredicate_[0]->getChosen()->getFile(cEnvironment_));
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::ChosenInterface::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Interface::IPredicate* pOperand_
//	
// RETURN
//	ChosenInterface*
//
// EXCEPTIONS

//static
ChosenInterface*
ChosenInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   Interface::IPredicate* pOperand_)
{
	; _SYDNEY_ASSERT(pPredicate_->getType() == Tree::Node::Not);

	AUTOPOINTER<This> pResult =
		new ChosenImpl::NotImpl(pPredicate_,
								pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::ChosenInterface::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Interface::IPredicate* pNotChecked_
//	Interface::IPredicate* pOperand_
//	
// RETURN
//	ChosenInterface*
//
// EXCEPTIONS

//static
ChosenInterface*
ChosenInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   Interface::IPredicate* pNotChecked_,
	   Interface::IPredicate* pOperand_)
{
	; _SYDNEY_ASSERT(pPredicate_->getType() == Tree::Node::Not);

	if (pNotChecked_ == 0) {
		return create(cEnvironment_,
					  pPredicate_,
					  pOperand_);
	}

	AUTOPOINTER<This> pResult =
		new ChosenImpl::PartialNotImpl(pPredicate_,
									   pNotChecked_,
									   pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Predicate::ChosenInterface::ChosenInterface -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ChosenInterface::
ChosenInterface(Interface::IPredicate* pPredicate_)
	: Super(pPredicate_)
{}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
