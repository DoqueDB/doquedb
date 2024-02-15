// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/CheckedInterface.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/Predicate/Impl/CheckedImpl.h"

#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Table.h"
#include "Plan/File/Argument.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckUnknown.h"
#include "Plan/Predicate/ChosenInterface.h"
#include "Plan/Predicate/Fetch.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Assert.h"

#include "LogicalFile/AutoLogicalFile.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

//////////////////////////////////
// Predicate::CheckedInterface

// FUNCTION public
//	Predicate::CheckedInterface::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	CheckedInterface*
//
// EXCEPTIONS

//static
CheckedInterface*
CheckedInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_)
{
	AUTOPOINTER<This> pResult = new CheckedImpl::Base(pPredicate_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::CheckedInterface::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Candidate::Table* pTable_
//	const Utility::FileSet& cFile_
//	
// RETURN
//	CheckedInterface*
//
// EXCEPTIONS

//static
CheckedInterface*
CheckedInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   Candidate::Table* pTable_,
	   const Utility::FileSet& cFile_)
{
	if (cFile_.isEmpty()) {
		return create(cEnvironment_, pPredicate_);
	}
	AUTOPOINTER<This> pResult = new CheckedImpl::SingleIndex(pPredicate_,
															 pTable_,
															 cFile_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::CheckedInterface::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	const VECTOR<Interface::IPredicate*>& vecChecked_
//	Candidate::Table* pTable_
//	const Utility::FileSet& cFile_
//	bool bNoTop_
//	
// RETURN
//	CheckedInterface*
//
// EXCEPTIONS

//static
CheckedInterface*
CheckedInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   const VECTOR<Interface::IPredicate*>& vecChecked_,
	   Candidate::Table* pTable_,
	   const Utility::FileSet& cFile_,
	   bool bNoTop_)
{
	if (vecChecked_.GETSIZE() == 1) {
		This* pChecked = vecChecked_[0]->getChecked();
		if (pChecked->getFile() == cFile_) {
			return pChecked;
		} else {
			return create(cEnvironment_,
						  pChecked->getPredicate(),
						  pTable_,
						  cFile_);
		}
	}

	; _SYDNEY_ASSERT(vecChecked_.GETSIZE() > 1);
	AUTOPOINTER<This> pResult;

	if (bNoTop_ == false
		&& vecChecked_.GETSIZE() == 2
		&& vecChecked_[0]->isFetch() == true
		&& vecChecked_[1]->isFetch() == false) {
		// fetch and condition
		pResult = new CheckedImpl::CombinedFetch(pPredicate_,
												 MAKEPAIR(vecChecked_[0], vecChecked_[1]),
												 pTable_,
												 cFile_);
	} else {
		// normal multiple index
		pResult = new CheckedImpl::MultipleIndex(pPredicate_,
												 vecChecked_,
												 pTable_,
												 cFile_,
												 bNoTop_);
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::CheckedInterface::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	const VECTOR<Interface::IPredicate*>& vecChecked_
//	Interface::IPredicate* pNotChecked_
//	const Utility::FileSet& cFile_
//	bool bNoTop_
//	
// RETURN
//	CheckedInterface*
//
// EXCEPTIONS

//static
CheckedInterface*
CheckedInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   const VECTOR<Interface::IPredicate*>& vecChecked_,
	   Interface::IPredicate* pNotChecked_,
	   Candidate::Table* pTable_,
	   const Utility::FileSet& cFile_,
	   bool bNoTop_)
{
	if (pNotChecked_ == 0) {
		return create(cEnvironment_,
					  pPredicate_,
					  vecChecked_,
					  pTable_,
					  cFile_,
					  bNoTop_);
	}
	AUTOPOINTER<This> pResult = new CheckedImpl::PartialMultipleIndex(pPredicate_,
																	  vecChecked_,
																	  pNotChecked_,
																	  pTable_,
																	  cFile_,
																	  bNoTop_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::CheckedInterface::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Interfdace::IPredicate* pOperand_
//	
// RETURN
//	CheckedInterface*
//
// EXCEPTIONS

//static
CheckedInterface*
CheckedInterface::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   Interface::IPredicate* pOperand_)
{
	; _SYDNEY_ASSERT(pPredicate_->getType() == Tree::Node::Not);

	AUTOPOINTER<This> pResult = new CheckedImpl::OperandIndex(pPredicate_,
															  pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION private
//	Predicate::CheckedInterface::CheckedInterface -- constructor
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

CheckedInterface::
CheckedInterface(Interface::IPredicate* pPredicate_)
	: Super(pPredicate_)
{}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
