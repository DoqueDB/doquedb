// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File/SessionVariable.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::File";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/File/SessionVariable.h"
#include "Plan/File/Argument.h"

#include "Plan/Candidate/File.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "Exception/Unexpected.h"
#include "Exception/VariableNotFound.h"

#include "Opt/Environment.h"

#include "Schema/Table.h"

#include "Server/Session.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_FILE_BEGIN

namespace
{
	const VECTOR<Scalar::Field*> _emptyVector;
}

////////////////////////////////////
//	Plan::File::SessionVariable

// FUNCTION public
//	File::SessionVariable::attach -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	LogicalFile::AutoLogicalFile&
//
// EXCEPTIONS

//virtual
LogicalFile::AutoLogicalFile&
SessionVariable::
attach(Trans::Transaction& cTrans_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	File::SessionVariable::hasAllTuples -- file has all tuples?
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pTable_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
SessionVariable::
hasAllTuples(Opt::Environment& cEnvironment_,
			 Relation::Table* pTable_)
{
	return false;
}

// FUNCTION public
//	File::SessionVariable::getSkipCheckKey -- get skip check key
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pTable_
//	
// RETURN
//	const VECTOR<Scalar::Field*>&
//
// EXCEPTIONS

//virtual
const VECTOR<Scalar::Field*>&
SessionVariable::
getSkipCheckKey(Opt::Environment& cEnvironment_,
				Relation::Table* pTable_)
{
	return _emptyVector;
}

// FUNCTION public
//	File::SessionVariable::isKeepLatch -- 
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

//virtual
bool
SessionVariable::
isKeepLatch(Opt::Environment& cEnvironment_)
{
	return false;
}

// FUNCTION public
//	File::SessionVariable::isSearchable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Candidate::Table* pTable_
//	File::Parameter* pParameter_
//	File::CheckArgument& cCheckArgument_
//	const AccessPlan::Cost& cScanCost
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
SessionVariable::
isSearchable(Opt::Environment& cEnvironment_,
			 Interface::IPredicate* pPredicate_,
			 Candidate::Table* pTable_,
			 File::Parameter* pParameter_,
			 File::CheckArgument& cCheckArgument_,
			 const AccessPlan::Cost& cScanCost)
{
	// obtain Server::Session::BitSetVariable
	Server::Session* pSession = Server::Session::getSession(cEnvironment_.getTransaction().getSessionID());
	if (pSession == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	Server::Session::BitSetVariable* pSessionVariable = pSession->getBitSetVariable(m_pVariable->getName());
	if (pSessionVariable == 0) {
		_SYDNEY_THROW1(Exception::VariableNotFound,
					   m_pVariable->getName());
	}

	// Table should be same
//	if (pTable_->getTable()->getSchemaTable()->getID() == pSessionVariable->getSchemaTableID()) {
		// Predicate should be IN
		if (pPredicate_->getType() == Tree::Node::In) {
			; _SYDNEY_ASSERT(pPredicate_->getOperandSize() == 2);
			; _SYDNEY_ASSERT(pPredicate_->getOperandAt(0));
			; _SYDNEY_ASSERT(pPredicate_->getOperandAt(1));
			// right operand should be variable
			if (pPredicate_->getOperandAt(1) == m_pVariable) {
				// left operand should be rowid
				if (pPredicate_->getOperandAt(0)->getType() == Tree::Node::Field) {
					Utility::FieldSet cField;
					pPredicate_->getUsedField(cField);
					if (cField.getSize() == 1
						&& (*cField.begin())->isRowID()) {

						cCheckArgument_.m_pFile = this;
						return true;
					}
				}
			}
		}
//	}
	return false;
}

// FUNCTION public
//	File::SessionVariable::createCandidate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::Table* pTable_
//	File::Parameter* pParameter_
//	
// RETURN
//	Candidate::File*
//
// EXCEPTIONS

//virtual
Candidate::File*
SessionVariable::
createCandidate(Opt::Environment& cEnvironment_,
				Candidate::Table* pTable_,
				File::Parameter* pParameter_)
{
	return Candidate::File::create(cEnvironment_,
								   pTable_,
								   this,
								   m_pVariable);
}

_SYDNEY_PLAN_FILE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
