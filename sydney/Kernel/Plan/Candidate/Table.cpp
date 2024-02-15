// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Table.cpp --
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
const char moduleName[] = "Plan::Candidate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Table.h"
#include "Plan/Candidate/Impl/TableImpl.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/Interface/IFile.h"
#include "Plan/File/Argument.h"
#include "Plan/File/Parameter.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Interface/IProgram.h"

#include "LogicalFile/AutoLogicalFile.h"

#include "Opt/Environment.h"

#include "Schema/Table.h"

#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	Plan::Candidate::Table::Retrieve

// FUNCTION public
//	Candidate::Table::Retrieve::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pTable_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Retrieve::
create(Opt::Environment& cEnvironment_,
	   Relation::Table* pTable_)
{
	AUTOPOINTER<This> pResult;
	if (pTable_->getSchemaTable()->isVirtual()) {
		// virtual table
		pResult = new TableImpl::Virtual(pTable_);
	} else {
		pResult = new TableImpl::Retrieve(pTable_);
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////////////////
//	Plan::Candidate::Table::Refer

// FUNCTION public
//	Candidate::Table::Refer::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pTable_
//	Relation::Table* pTargetTable_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Refer::
create(Opt::Environment& cEnvironment_,
	   Relation::Table* pTable_,
	   Relation::Table* pTargetTable_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Refer(pTable_, pTargetTable_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////////////////
//	Plan::Candidate::Table::Simple

// FUNCTION public
//	Candidate::Table::Simple::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pTable_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Simple::
create(Opt::Environment& cEnvironment_,
	   Relation::Table* pTable_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Simple(pTable_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////////////////
//	Plan::Candidate::Table::Insert

// FUNCTION public
//	Candidate::Table::Insert::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pTable_
//	const VECTOR<Scalar::Field*>& vecLogged_
//	const VECTOR<Candidate::Table*>& vecReferenceAfter_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Insert::
create(Opt::Environment& cEnvironment_,
	   Relation::Table* pTable_,
	   const VECTOR<Scalar::Field*>& vecLogged_,
	   const VECTOR<Candidate::Table*>& vecReferenceAfter_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Insert(pTable_,
													  vecLogged_,
													  vecReferenceAfter_,
													  pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////////////////
//	Plan::Candidate::Table::Delete

// FUNCTION public
//	Candidate::Table::Delete::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pTable_
//	Relation::Table* pRetrieve_
//	const VECTOR<Scalar::Field*>& vecLogged_
//	const VECTOR<Candidate::Table*>& vecReferenceBefore_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Delete::
create(Opt::Environment& cEnvironment_,
	   Relation::Table* pTable_,
	   Relation::Table* pRetrieve_,
	   const VECTOR<Scalar::Field*>& vecLogged_,
	   const VECTOR<Candidate::Table*>& vecReferenceBefore_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Delete(pTable_,
													  pRetrieve_,
													  vecLogged_,
													  vecReferenceBefore_,
													  pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////////////////
//	Plan::Candidate::Table::Update

// FUNCTION public
//	Candidate::Table::Update::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pTable_
//	Relation::Table* pRetrieve_
//	const VECTOR<Scalar::Field*>& vecLogged_
//	const VECTOR<Candidate::Table*>& vecReferenceBefore_
//	const VECTOR<Candidate::Table*>& vecReferenceAfter_
//	const Utility::UIntSet& cLogRequiredPosition_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Update::
create(Opt::Environment& cEnvironment_,
	   Relation::Table* pTable_,
	   Relation::Table* pRetrieve_,
	   const VECTOR<Scalar::Field*>& vecLogged_,
	   const VECTOR<Candidate::Table*>& vecReferenceBefore_,
	   const VECTOR<Candidate::Table*>& vecReferenceAfter_,
	   const Utility::UIntSet& cLogRequiredPosition_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Update(pTable_,
													  pRetrieve_,
													  vecLogged_,
													  vecReferenceBefore_,
													  vecReferenceAfter_,
													  cLogRequiredPosition_,
													  pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////////////////
//	Plan::Candidate::Table::Import

// FUNCTION public
//	Candidate::Table::Import::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pTable_
//	const VECTOR<Candidate::Table*>& vecReferenceAfter_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Import::
create(Opt::Environment& cEnvironment_,
	   Relation::Table* pTable_,
	   const VECTOR<Candidate::Table*>& vecReferenceAfter_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Import(pTable_,
													  vecReferenceAfter_,
													  pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////////////////
//	Plan::Candidate::Table::Undo

// FUNCTION public
//	Candidate::Table::Undo::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pTable_
//	const Common::DataArrayData* pUndoLog_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::Undo::
create(Opt::Environment& cEnvironment_,
	   Relation::Table* pTable_,
	   const Common::DataArrayData* pUndoLog_)
{
	AUTOPOINTER<This> pResult = new TableImpl::Undo(pTable_,
													pUndoLog_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////////////////
//	Plan::Candidate::Table

// FUNCTION public
//	Candidate::Table::isReferingRelation -- 
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
Table::
isReferingRelation(Interface::IRelation* pRelation_)
{
	return pRelation_ == getTable();
}

// FUNCTION public
//	Candidate::Table::createReferingRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cRelationSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Table::
createReferingRelation(Utility::RelationSet& cRelationSet_)
{
	cRelationSet_.add(getTable());
}

// FUNCTION public
//	Candidate::Table::getCandidate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	
// RETURN
//	Candidate::Table*
//
// EXCEPTIONS

//virtual
Candidate::Table*
Table::
getCandidate(Opt::Environment& cEnvironment_,
			 Interface::IRelation* pRelation_)
{
	if (pRelation_ == getTable()) return this;
	else return 0;
}

// FUNCTION public
//	Candidate::Table::checkFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Plan::File::Parameter* pParameter_
//	Plan::File::CheckArgument& cArgument_
//	const AccessPlan::Cost& cCost_
//	boost::function<bool(LogicalFile::AutoLogicalFile&,
//						 LogicalFile::OpenOption&)> function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Table::
checkFile(Opt::Environment& cEnvironment_,
		  Interface::IFile* pFile_,
		  Plan::File::Parameter* pParameter_,
		  Plan::File::CheckArgument& cArgument_,
		  const AccessPlan::Cost& cCost_,
		  boost::function<bool(LogicalFile::AutoLogicalFile&,
							   LogicalFile::OpenOption&)> function_)
{
	// arguments for File::Parameter
	LogicalFile::OpenOption cOpenOption;
	if (pParameter_ != 0) {
		// use given openoption
		cOpenOption = pParameter_->getOpenOption();
	}

	// set estimated file
	Relation::Table::AutoReset cAutoReset = getTable()->setEstimateFile(pFile_);
	LogicalFile::AutoLogicalFile& cLogicalFile = pFile_->attach(cEnvironment_.getTransaction());

	if (!function_(cLogicalFile, cOpenOption)) {
		// can't execute
		return false;
	}

	return checkFileCost(cEnvironment_,
						 pFile_,
						 cArgument_,
						 cCost_,
						 cLogicalFile,
						 cOpenOption);
}

// FUNCTION public
//	Candidate::Table::setRowID -- set rowid field
//
// NOTES
//
// ARGUMENTS
//	Scalar::Field* pRowID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
setRowID(Scalar::Field* pRowID_)
{
	; _SYDNEY_ASSERT(pRowID_->isRowID());
	m_pRowID = pRowID_;
}

// FUNCTION public
//	Candidate::Table::isNeedLock -- table operation is needed to locked?
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Table::
isNeedLock(Opt::Environment& cEnvironment_)
{
	return (cEnvironment_.getTransaction().isNoLock() == false
			&& cEnvironment_.getTransaction().isNoVersion() == true
			&& getTable()->getSchemaTable()
			&& getTable()->getSchemaTable()->isTemporary() == false);
}

// FUNCTION protected
//	Candidate::Table::isNeedLog -- table operation is needed to log?
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Table::
isNeedLog(Opt::Environment& cEnvironment_)
{
	return getTable()->isNeedLog(cEnvironment_);
}

// FUNCTION private
//	Candidate::Table::checkFileCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	File::CheckArgument& cArgument_
//	const AccessPlan::Cost& cCost_
//	LogicalFile::AutoLogicalFile& cLogicalFile_
//	LogicalFile::OpenOption& cOpenOption_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Table::
checkFileCost(Opt::Environment& cEnvironment_,
			  Interface::IFile* pFile_,
			  Plan::File::CheckArgument& cArgument_,
			  const AccessPlan::Cost& cCost_,
			  LogicalFile::AutoLogicalFile& cLogicalFile_,
			  LogicalFile::OpenOption& cOpenOption_)
{
	AccessPlan::Cost cCost;
	AccessPlan::Cost cMyCost;
	bool bSkipEstimate = cArgument_.canSkipEstimate();

	if (bSkipEstimate == false) {
		// compare cost
		double dblOverhead;
		double dblProcessCost;
		double dblCount = (cArgument_.m_pParameter == 0)
			? 0.0
			: cArgument_.m_pParameter->getCost().getTupleCount().get();
		cLogicalFile_.getCost(cEnvironment_.getTransaction(),
							  cOpenOption_,
							  dblOverhead,
							  dblProcessCost,
							  (cArgument_.m_pParameter == 0) ? &dblCount : 0);
		cMyCost.setOverhead(dblOverhead);
		cMyCost.setTotalCost(dblProcessCost * dblCount);
		cMyCost.setTupleCount(dblCount);

		if (cArgument_.m_pParameter == 0) {
			cCost = cCost_;
		} else {
			cCost = cArgument_.m_pParameter->getCost();
		}
	}
	if (bSkipEstimate || cMyCost < cCost) {
		// use file with smaller cost
		if (cArgument_.m_pFile != pFile_ && cArgument_.m_pParameter != 0) {
			cArgument_.eraseParameter(cEnvironment_);
		}
		if (cArgument_.m_pParameter == 0) {
			// if parameter is not set, create new parameter object
			cArgument_.m_pParameter = File::Parameter::create(cEnvironment_, cOpenOption_, cMyCost);
		} else {
			// if already set, replace openoption
			cArgument_.m_pParameter->getOpenOption() = cOpenOption_;
			if (bSkipEstimate == false) {
				cArgument_.m_pParameter->getCost() = cMyCost;
			}
		}
		cArgument_.m_pFile = pFile_;
		return true;
	}
	return false;
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
