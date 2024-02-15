// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Table.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Candidate";
}

#include "SyDefault.h"

#include "DPlan/Candidate/Table.h"
#include "DPlan/Candidate/Impl/TableImpl.h" 

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_CANDIDATE_BEGIN

//////////////////////////////////////
// Candidate::Table::Distribute::

/////////////////////////////////////////////
// Candidate::Table::Distribute::Retrieve::

// FUNCTION public
//	Candidate::Table::Distribute::Retrieve::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pRelation_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Distribute::Retrieve::
create(Opt::Environment& cEnvironment_,
	   Plan::Interface::IRelation* pRelation_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Distribute::Retrieve(pRelation_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

/////////////////////////////////////////////
// Candidate::Table::Distribute::Insert::

// FUNCTION public
//	Candidate::Table::Distribute::Insert::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pRelation_
//	Plan::Interface::ICandidate* pOperand_
//	Plan::Interface::IScalar* pCheck_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Distribute::Insert::
create(Opt::Environment& cEnvironment_,
	   Plan::Relation::Table* pRelation_,
	   Plan::Interface::ICandidate* pOperand_,
	   Plan::Interface::IScalar* pCheck_,
	   bool bRelocateUpdate_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Distribute::Insert(pRelation_,
																  pOperand_,
																  pCheck_,
																  bRelocateUpdate_);
	
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}


/////////////////////////////////////////////
// Candidate::Table::Distribute::Update::

// FUNCTION public
//	Candidate::Table::Distribute::Update::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pRelation_
//	Plan::Interface::ICandidate* pOperand_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Distribute::Update::
create(Opt::Environment& cEnvironment_,
	   Plan::Relation::Table* pRelation_,
	   Plan::Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new TableImpl::UpdateBase(pRelation_,
														  pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

/////////////////////////////////////////////
// Candidate::Table::Distribute::Delete::

// FUNCTION public
//	Candidate::Table::Distribute::Delete::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pRelation_
//	Plan::Interface::ICandidate* pOperand_
//	Plan::Interface::IScalar* pCheck_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Distribute::Delete::
create(Opt::Environment& cEnvironment_,
	   Plan::Relation::Table* pRelation_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Delete(pRelation_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////
// Candidate::Table::Replicate::

/////////////////////////////////////////////
// Candidate::Table::Replicate::Retrieve::

// FUNCTION public
//	Candidate::Table::Replicate::Retrieve::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pRelation_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Replicate::Retrieve::
create(Opt::Environment& cEnvironment_,
	   Plan::Interface::IRelation* pRelation_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Replicate::Retrieve(pRelation_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

/////////////////////////////////////////////
// Candidate::Table::Replicate::Insert::

// FUNCTION public
//	Candidate::Table::Replicate::Insert::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pRelation_
//	Plan::Interface::ICandidate* pOperand_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Replicate::Insert::
create(Opt::Environment& cEnvironment_,
	   Plan::Relation::Table* pRelation_,
	   Plan::Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Replicate::Insert(pRelation_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

_SYDNEY_DPLAN_CANDIDATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
