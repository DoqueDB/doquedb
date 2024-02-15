// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File/SchemaFile.cpp --
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
const char moduleName[] = "Plan::File";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/File/SchemaFile.h"

#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Table.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "Execution/Utility/Transaction.h"

#include "LogicalFile/AutoLogicalFile.h"

#include "Opt/Environment.h"

#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_FILE_BEGIN

////////////////////////////////////
//	Plan::File::SchemaFile

// FUNCTION public
//	File::SchemaFile::attach -- 
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
SchemaFile::
attach(Trans::Transaction& cTrans_)
{
	if (!m_cLogicalFile.isAttached()) {
		; _SYDNEY_ASSERT(m_pSchemaFile);
		Execution::Utility::Transaction::attachLogicalFile(cTrans_,
														   *m_pSchemaFile,
														   m_cLogicalFile);
	}
	return m_cLogicalFile;
}

// FUNCTION public
//	File::SchemaFile::hasAllTuples -- file has all tuples?
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
SchemaFile::
hasAllTuples(Opt::Environment& cEnvironment_,
			 Relation::Table* pTable_)
{
	if (m_iHasAllTuples < 0) {
		if (m_pSchemaFile->hasAllTuples()) {
			m_iHasAllTuples = 1;
		} else {
			// check keys
			Trans::Transaction& cTrans = cEnvironment_.getTransaction();
			ModVector<Schema::Field*> vecField = m_pSchemaFile->getSkipCheckKey(cTrans);
			if (Opt::IsAll(vecField,
						   boost::bind(&Scalar::Field::isKnownNotNull,
									   boost::bind(&Scalar::Field::create,
												   boost::ref(cEnvironment_),
												   boost::bind(&Schema::Field::getRelatedColumn,
															   _1,
															   boost::ref(cTrans)),
												   pTable_),
									   boost::ref(cEnvironment_)))) {
				m_iHasAllTuples = 1;
			} else {
				m_iHasAllTuples = 0;
			}
		}
	}
	return m_iHasAllTuples == 1;
}

// FUNCTION public
//	File::SchemaFile::getSkipCheckKey -- get skip check key
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
SchemaFile::
getSkipCheckKey(Opt::Environment& cEnvironment_,
				Relation::Table* pTable_)
{
	if (m_bSkipChecked == false) {
		; _SYDNEY_ASSERT(m_vecSkipCheckKey.isEmpty());

		// check keys
		Trans::Transaction& cTrans = cEnvironment_.getTransaction();
		ModVector<Schema::Field*> vecField = m_pSchemaFile->getSkipCheckKey(cTrans);

		if (vecField.isEmpty() == false) {
			// convert schema field into related scalar::field
			m_vecSkipCheckKey.reserve(vecField.getSize());
			Opt::MapContainer(vecField, m_vecSkipCheckKey,
							  boost::bind(&This::getField,
										  this,
										  boost::ref(cEnvironment_),
										  pTable_,
										  _1));
		}
		m_bSkipChecked = true;
	}
	return m_vecSkipCheckKey;
}

// FUNCTION public
//	File::SchemaFile::isKeepLatch -- 
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
SchemaFile::
isKeepLatch(Opt::Environment& cEnvironment_)
{
	LogicalFile::AutoLogicalFile& cLogicalFile = attach(cEnvironment_.getTransaction());
	return cLogicalFile.isKeepLatch();
}

// FUNCTION public
//	File::SchemaFile::isSearchable -- 
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
SchemaFile::
isSearchable(Opt::Environment& cEnvironment_,
			 Interface::IPredicate* pPredicate_,
			 Candidate::Table* pTable_,
			 File::Parameter* pParameter_,
			 File::CheckArgument& cCheckArgument_,
			 const AccessPlan::Cost& cScanCost_)
{
	return pTable_->checkFile(cEnvironment_,
							  this,
							  pParameter_,
							  cCheckArgument_,
							  cScanCost_,
							  boost::bind(&LogicalFile::AutoLogicalFile::getSearchParameter,
										  _1,
										  pPredicate_,
										  _2));
}

// FUNCTION public
//	File::SchemaFile::createCandidate -- 
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
SchemaFile::
createCandidate(Opt::Environment& cEnvironment_,
				Candidate::Table* pTable_,
				File::Parameter* pParameter_)
{
	return Candidate::File::create(cEnvironment_,
								   pTable_,
								   this,
								   pParameter_);
}

// FUNCTION private
//	File::SchemaFile::getField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Table* pTable_
//	Schema::Field* pSchemaField_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

Scalar::Field*
SchemaFile::
getField(Opt::Environment& cEnvironment_,
		 Relation::Table* pTable_,
		 Schema::Field* pSchemaField_)
{
	Scalar::Field* pResult = pTable_->getField(pSchemaField_);
	if (pResult == 0) {
		Trans::Transaction& cTrans = cEnvironment_.getTransaction();
		Schema::Column* pSchemaColumn = pSchemaField_->getRelatedColumn(cTrans);
		; _SYDNEY_ASSERT(pSchemaColumn);
		pResult = Scalar::Field::create(cEnvironment_,
										pSchemaColumn,
										pTable_)->getField(this);
	}
	; _SYDNEY_ASSERT(pResult);

	return pResult;
}

_SYDNEY_PLAN_FILE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
