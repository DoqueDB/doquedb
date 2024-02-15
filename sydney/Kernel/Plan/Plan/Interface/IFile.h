// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IFile.h --
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

#ifndef __SYDNEY_PLAN_INTERFACE_IFILE_H
#define __SYDNEY_PLAN_INTERFACE_IFILE_H

#include "Plan/Interface/Module.h"

#include "Plan/Declaration.h"
#include "Plan/Tree/Node.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
	class AutoLogicalFile;
}
namespace Schema
{
	class File;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_INTERFACE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Interface::IFile -- Base class for the classes which represents file information
//
//	NOTES
//		This class is not constructed directly
class IFile
	: public Tree::Node
{
public:
	typedef Tree::Node Super;
	typedef IFile This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Schema::File* pSchemaFile_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IScalar* pVariable_);

	// destructor
	virtual ~IFile() {}

	// get corresponding schema file object
	virtual Schema::File* getSchemaFile() = 0;
	// get corresponding session variable object
	virtual Interface::IScalar* getSessionVariable() = 0;

	// get logical file object
	virtual LogicalFile::AutoLogicalFile& attach(Trans::Transaction& cTrans_) = 0;

	// file has all tuples?
	virtual bool hasAllTuples(Opt::Environment& cEnvironment_,
							  Relation::Table* pTable_) = 0;
	// get skip check key
	virtual const VECTOR<Scalar::Field*>& getSkipCheckKey(Opt::Environment& cEnvironment_,
														  Relation::Table* pTable_) = 0;

	// check is keep latch
	virtual bool isKeepLatch(Opt::Environment& cEnvironment_) = 0;

	// check is searchable
	virtual bool isSearchable(Opt::Environment& cEnvironment_,
							  Interface::IPredicate* pPredicate_,
							  Candidate::Table* pTable_,
							  File::Parameter* pParameter_,
							  File::CheckArgument& cCheckArgument_,
							  const AccessPlan::Cost& cScanCost) = 0;

	// create candidate class for the file
	virtual Candidate::File* createCandidate(Opt::Environment& cEnvironment_,
											 Candidate::Table* pTable_,
											 File::Parameter* pParameter_) = 0;

protected:
	// costructor
	IFile() : Super(Super::File) {}
	IFile(Super::Type eType_) : Super(eType_) {}

private:
	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);
};

_SYDNEY_PLAN_INTERFACE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_INTERFACE_IFILE_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
