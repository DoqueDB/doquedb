// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Row.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Row.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Interface/IScalar.h"

#include "Exception/NotSupported.h"

#include "Execution/Interface/IProgram.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

/////////////////////////////
// Candidate::Row::

// FUNCTION public
//	Candidate::Row::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Row*
//
// EXCEPTIONS

//static
Row*
Row::
create(Opt::Environment& cEnvironment_)
{
	AUTOPOINTER<This> pResult = new Candidate::Row;
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::Row::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Vector& vecScalar_
//	
// RETURN
//	Row*
//
// EXCEPTIONS

//static
Row*
Row::
create(Opt::Environment& cEnvironment_,
	   const Vector& vecScalar_)
{
	AUTOPOINTER<This> pResult = new Candidate::Row(vecScalar_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::Row::erase -- destructor
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
Row::
erase(Opt::Environment& cEnvironment_,
	  This* pThis_)
{
	if (pThis_) {
		pThis_->eraseFromEnvironment(cEnvironment_);
		delete pThis_;
	}
}

// FUNCTION public
//	Candidate::Row::addScalar -- add one scalar
//
// NOTES
//
// ARGUMENTS
//	Interface::IScalar* pScalar_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Row::
addScalar(Interface::IScalar* pScalar_)
{
	m_vecScalar.PUSHBACK(pScalar_);
}

// FUNCTION public
//	Candidate::Row::addRow -- append another row
//
// NOTES
//
// ARGUMENTS
//	const This& cRow_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Row::
addRow(const This& cRow_)
{
	m_vecScalar.insert(m_vecScalar.end(),
					   cRow_.begin(), cRow_.end());
}

// FUNCTION public
//	Candidate::Row::delay -- create delayable subset
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	RowDelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Row::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  RowDelayArgument& cArgument_)
{
	bool bResult = false;

	Iterator iterator = begin();
	const Iterator last = end();
	for (; iterator != last; ++iterator) {
		Interface::IScalar* pScalar = *iterator;
		if (pScalar->delay(cEnvironment_, pCandidate_, cArgument_)) {
			bResult = true;
		} else {
			pScalar->require(cEnvironment_, pCandidate_);
			cArgument_.m_cNotDelayed.add(pScalar);
		}
	}
	return bResult;
}

// FUNCTION public
//	Candidate::Row::delay -- create delayable subset (get keys and non-delayed as another Row)
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	bool bMinimum_ = false
//	
// RETURN
//	Row*
//
// EXCEPTIONS

Row*
Row::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  bool bMinimum_ /* = false */)
{
	Scalar::DelayArgument cArgument(bMinimum_);
	Row* pResult = 0;

	Iterator iterator = begin();
	const Iterator last = end();
	for (; iterator != last; ++iterator) {
		Interface::IScalar* pScalar = *iterator;
		if (pScalar->delay(cEnvironment_, pCandidate_, cArgument)) {
			if (pResult == 0) {
				pResult = create(cEnvironment_);
			}
		} else {
			pScalar->require(cEnvironment_, pCandidate_);
			cArgument.m_cKey.add(pScalar);
		}
	}
	if (pResult) {
		pResult->m_vecScalar.insert(pResult->m_vecScalar.end(),
									cArgument.m_cKey.begin(),
									cArgument.m_cKey.end());
	}
	return pResult == 0 ? this : pResult;
}

// FUNCTION public
//	Candidate::Row::generate -- generate output data
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
Row::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	if (!ISEMPTY()) {
		VECTOR<int> vecID;
		vecID.reserve(GETSIZE());
		Opt::MapContainer(m_vecScalar,
						  vecID,
						  boost::bind(&Interface::IScalar::generate,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cProgram_),
									  pIterator_,
									  boost::ref(cArgument_)));
		return cProgram_.addVariable(vecID);
	}
	return -1;
}

// FUNCTION public
//	Candidate::Candidate::Row::generateFromType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
Candidate::Row::
generateFromType(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	if (!ISEMPTY()) {
		VECTOR<int> vecID;
		vecID.reserve(GETSIZE());
		Opt::MapContainer(m_vecScalar,
						  vecID,
						  boost::bind(&Interface::IScalar::generateFromType,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cProgram_),
									  pIterator_,
									  boost::ref(cArgument_)));
		return cProgram_.addVariable(vecID);
	}
	return -1;
}

// FUNCTION private
//	Candidate::Row::registerToEnvironment -- register to environment
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
Candidate::Row::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	m_iID = cEnvironment_.addObject(this);
}

// FUNCTION private
//	Candidate::Row::eraseFromEnvironment -- erase from environment
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
Candidate::Row::
eraseFromEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.eraseObject(m_iID);
}

//
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
