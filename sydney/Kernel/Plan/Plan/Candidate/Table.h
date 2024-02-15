// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Table.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_CANDIDATE_TABLE_H
#define __SYDNEY_PLAN_CANDIDATE_TABLE_H

#include "boost/function.hpp"

#include "Plan/Candidate/Base.h"

#include "Plan/Declaration.h"
#include "Plan/AccessPlan/Cost.h"
#include "Plan/File/Parameter.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Relation/Table.h"

#include "LogicalFile/OpenOption.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
	class AutoLogicalFile;
}
namespace Schema
{
	class Constraint;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////
// CLASS
//	Plan::Candidate::Table -- implementation class of Interface::ICandidate for table
//
// NOTES
class Table
	: public Base
{
public:
	typedef Table This;
	typedef Base Super;

	struct Retrieve
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Table* pTable_);
	};
	struct Refer
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Table* pTable_,
							Relation::Table* pTargetTable_);
	};
	struct Simple
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Table* pTable_);
	};
	struct Insert
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Table* pTable_,
							const VECTOR<Scalar::Field*>& vecLogged_,
							const VECTOR<Candidate::Table*>& vecReferenceAfter_,
							Interface::ICandidate* pOperand_);
	};
	struct Delete
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Table* pTable_,
							Relation::Table* pRetrieve_,
							const VECTOR<Scalar::Field*>& vecLogged_,
							const VECTOR<Candidate::Table*>& vecReferenceBefore_,
							Interface::ICandidate* pOperand_);
	};
	struct Update
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Table* pTable_,
							Relation::Table* pRetrieve_,
							const VECTOR<Scalar::Field*>& vecLogged_,
							const VECTOR<Candidate::Table*>& vecReferenceBefore_,
							const VECTOR<Candidate::Table*>& vecReferenceAfter_,
							const Utility::UIntSet& cLogRequiredPosition_,
							Interface::ICandidate* pOperand_);
	};
	struct Import
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Table* pTable_,
							const VECTOR<Candidate::Table*>& vecReferenceAfter_,
							Interface::ICandidate* pOperand_);
	};
	struct Undo
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Table* pTable_,
							const Common::DataArrayData* pUndoLog_);
	};

	// destructor
	virtual ~Table() {}

/////////////////////////////
// Interface::ICandidate::
//	virtual void createCost(Opt::Environment& cEnvironment_,
//							const AccessPlan::Source& cPlanSource_);
//	virtual const AccessPlan::Cost& getCost() {return m_cCost;}
//	virtual Candidate::Row* getRow(Opt::Environment& cEnvironment_);
//	virtual void require(Opt::Environment& cEnvironment_,
//						 Scalar::Field* pField_);
//	virtual void retrieve(Opt::Environment& cEnvironment_,
//						  Scalar::Field* pField_);
//	virtual void use(Opt::Environment& cEnvironment_,
//					 Scalar::Field* pField_);
//	virtual bool delay(Opt::Environment& cEnvironment_,
//					   Scalar::Field* pField_,
//					   Scalar::DelayArgument& cArgument_);
 	virtual bool isReferingRelation(Interface::IRelation* pRelation_);
 	virtual void createReferingRelation(Utility::RelationSet& cRelationSet_);
	virtual Candidate::Table* getCandidate(Opt::Environment& cEnvironment_,
										   Interface::IRelation* pRelation_);
//	virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
//												   Execution::Interface::IProgram& cProgram_,
//												   Candidate::AdoptArgument& cArgument_);
//	virtual void generateDelayed(Opt::Environment& cEnvironment_,
//								 Execution::Interface::IProgram& cProgram_,
//								 Execution::Interface::IIterator* pIterator_);

	///////////////////////
	// for creating plan

	// create accessplan
	virtual void createPlan(Opt::Environment& cEnvironment_,
							AccessPlan::Source& cPlanSource_,
							const Utility::FieldSet& cFieldSet_) = 0;

	// check whether a column is retrieved delayed
	virtual bool isDelayed(Scalar::Field* pField_) = 0;

	// get file fetch operator for a fetch key
	virtual bool addFileFetch(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Execution::Interface::IIterator* pIterator_,
							  Scalar::Field* pField_,
							  Candidate::AdoptArgument& cArgument_) = 0;

	// add constraint to be checked
	virtual void addConstraint(Opt::Environment& cEnvironment_,
							   Schema::Constraint* pSchemaConstraint_) = 0;
	// get constraints to be checked
	virtual const VECTOR<Schema::Constraint*>& getConstraint() = 0;

	// add retrieve field
	virtual void addRetrieveField(Opt::Environment& cEnvironment_,
								  Candidate::File* pCandidateFile_,
								  Interface::IFile* pFile_,
								  Scalar::Field* pField_) = 0;

	// get table estimate count
	virtual AccessPlan::Cost::Value
				getEstimateCount(Opt::Environment& cEnvironment_) = 0;

	// check file availability
	bool checkFile(Opt::Environment& cEnvironment_,
				   Interface::IFile* pFile_,
				   Plan::File::Parameter* pParameter_,
				   Plan::File::CheckArgument& cArgument_,
				   const AccessPlan::Cost& cCost_,
				   boost::function<bool(LogicalFile::AutoLogicalFile&,
										LogicalFile::OpenOption&)> function_);

	///////////////////////
	// accessors

	// get correspording relation
	Relation::Table* getTable() {return m_pTable;}
	// get rowid field
	Scalar::Field* getRowID() {return m_pRowID;}
	// set rowid field
	void setRowID(Scalar::Field* pRowID_);
	// table operation is needed to locked?
	bool isNeedLock(Opt::Environment& cEnvironment_);

protected:
	// constructor
	Table(Relation::Table* pTable_)
		: Super(),
		  m_pTable(pTable_),
		  m_pRowID(0)
	{}

	// clear member
	void clear()
	{
		m_pRowID = 0;
	}

	// table operation is needed to log?
	bool isNeedLog(Opt::Environment& cEnvironment_);

private:
	// check file candidate by cost
	bool checkFileCost(Opt::Environment& cEnvironment_,
					   Interface::IFile* pFile_,
					   Plan::File::CheckArgument& cArgument_,
					   const AccessPlan::Cost& cCost_,
					   LogicalFile::AutoLogicalFile& cLogicalFile_,
					   LogicalFile::OpenOption& cOpenOption_);

	Relation::Table* m_pTable;		   // corresponding relation
	Scalar::Field* m_pRowID;		   // rowid field
};

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_TABLE_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
