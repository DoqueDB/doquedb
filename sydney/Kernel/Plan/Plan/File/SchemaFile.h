// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File/SchemaFile.h --
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

#ifndef __SYDNEY_PLAN_FILE_SCHEMAFILE_H
#define __SYDNEY_PLAN_FILE_SCHEMAFILE_H

#include "Plan/File/Module.h"

#include "Plan/Interface/IFile.h"

#include "LogicalFile/AutoLogicalFile.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Field;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_FILE_BEGIN

// CLASS
//	Plan::File::SchemaFile --
//
// NOTES

class SchemaFile
	: public Interface::IFile
{
public:
	typedef SchemaFile This;
	typedef Interface::IFile Super;

	// constructor
	SchemaFile(Schema::File* pSchemaFile_)
		: Super(),
		  m_pSchemaFile(pSchemaFile_),
		  m_cLogicalFile(),
		  m_iHasAllTuples(-1),
		  m_vecSkipCheckKey(),
		  m_bSkipChecked(false)
	{}

	// destructor
	~SchemaFile() {}

////////////////////////
// Interface::IFile::
	virtual Schema::File* getSchemaFile() {return m_pSchemaFile;}
	virtual Interface::IScalar* getSessionVariable() {return 0;}
	virtual LogicalFile::AutoLogicalFile& attach(Trans::Transaction& cTrans_);
	virtual bool hasAllTuples(Opt::Environment& cEnvironment_,
							  Relation::Table* pTable_);
	virtual const VECTOR<Scalar::Field*>& getSkipCheckKey(Opt::Environment& cEnvironment_,
														  Relation::Table* pTable_);
	virtual bool isKeepLatch(Opt::Environment& cEnvironment_);
	virtual bool isSearchable(Opt::Environment& cEnvironment_,
							  Interface::IPredicate* pPredicate_,
							  Candidate::Table* pTable_,
							  File::Parameter* pParameter_,
							  File::CheckArgument& cCheckArgument_,
							  const AccessPlan::Cost& cScanCost);
	virtual Candidate::File* createCandidate(Opt::Environment& cEnvironment_,
											 Candidate::Table* pTable_,
											 File::Parameter* pParameter_);
protected:
private:
	Scalar::Field* getField(Opt::Environment& cEnvironment_,
							Relation::Table* pTable_,
							Schema::Field* pSchemaField_);

	Schema::File* m_pSchemaFile;
	LogicalFile::AutoLogicalFile m_cLogicalFile;

	// cache of frequently called methods
	int m_iHasAllTuples;
	VECTOR<Scalar::Field*> m_vecSkipCheckKey;
	bool m_bSkipChecked;
};

_SYDNEY_PLAN_FILE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_FILE_SCHEMAFILE_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
