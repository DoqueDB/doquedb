// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/File.h --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_CANDIDATE_FILE_H
#define __SYDNEY_PLAN_CANDIDATE_FILE_H

#include "boost/function.hpp"

#include "Plan/Candidate/Module.h"

#include "Plan/Declaration.h"
#include "Plan/Candidate/Table.h"
#include "Plan/File/Argument.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Object.h"

#include "Opt/Argument.h"
#include "Opt/Declaration.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
	class AutoLogicalFile;
	class OpenOption;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////////////////
// CLASS
//	Plan::Candidate::File -- file information in access plan
//
// NOTES
//	This class is NOT a subclass of Interface::ICandidate

class File
	: public Common::Object
{
public:
	typedef File This;
	typedef Common::Object Super;

	typedef Plan::File::Parameter Parameter;
	typedef Plan::File::CheckArgument CheckArgument;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Candidate::Table* pTable_,
						Interface::IFile* pFile_,
						Parameter* pParameter_);
	static This* create(Opt::Environment& cEnvironment_,
						Candidate::Table* pTable_,
						Interface::IFile* pFile_,
						Interface::IScalar* pVariable_);
	static This* create(Opt::Environment& cEnvironment_,
						Candidate::Table* pTable_,
						const VECTOR<This*>& vecFiles_,
						Interface::IPredicate* pPredicate_,
						const VECTOR<Interface::IPredicate*>& vecPredicates_,
						Order::Specification* pOrder_);

	// destructor
	static void erase(Opt::Environment& cEnvironment_,
					  This* pThis_);
	virtual ~File() {}

	// insert index availability information
	static void insertCheckIndexResult(Opt::Environment& cEnvironment_,
									   Predicate::ChosenInterface* pPredicate_,
									   VECTOR<Predicate::ChosenInterface*>* pTarget_,
									   This* pFile_);

	// accessor
	Candidate::Table* getTable() {return m_pTable;}
	Interface::IFile* getFile() {return m_pFile;}
	Parameter* getParameter() {return m_pParameter;}

	void setParameter(Parameter* pParameter_);

	// for explain
	virtual void setIsSimple() = 0;
	virtual void clearBitSetFlag() = 0;

	///////////////////////////
	// estimating candidate
	///////////////////////////

	// check file function availability
	virtual bool check(Opt::Environment& cEnvironment_,
					   File::CheckArgument& cArgument_,
					   boost::function<bool(LogicalFile::AutoLogicalFile&,
											LogicalFile::OpenOption&)> function_) = 0;

	// check whether index is always used
	virtual bool isAlwaysUsed() = 0;
	// check whether bitset is always needed
	virtual bool isAlwaysBitSet() = 0;
	// check whether original value is not used in updating
	virtual bool isCheckByKey(Opt::Environment& cEnvironment_) = 0;
	// check whether locator is used
	virtual bool isLocatorUsed() = 0;
	// check whether other field than rowid is obtained
	virtual bool isOtherFieldUsed() = 0;

	// check index availability
	virtual void addCheckIndexResult(Opt::Environment& cEnvironment_,
									 CheckIndexArgument& cArgument_,
									 Predicate::ChosenInterface* pPredicate_) = 0;

	/////////////////////////////
	// creating access plan
	/////////////////////////////

	// get cost of file access
	virtual AccessPlan::Cost& getCost(Opt::Environment& cEnvironment_,
									  Interface::IPredicate* pPredicate_ = 0) = 0;;

	// add retrieved field
	virtual void addFieldForPredicate(Opt::Environment& cEnvironment_,
									  Interface::IFile* pFile_,
									  Predicate::CheckRetrievableArgument& cCheckArgument_) = 0;
	// add retrieved/put fields
	virtual void addField(Opt::Environment& cEnvironment_,
						  Scalar::Field* pField_) = 0;

	// add put key fields
	virtual void addPutKey(Opt::Environment& cEnvironment_,
						   Scalar::Field* pField_) = 0;

	// add insert fields
	virtual void addInsertField(Opt::Environment& cEnvironment_,
								Scalar::Field* pField_) = 0;

	// add undo fields
	virtual void addUndoField(Opt::Environment& cEnvironment_,
							  Scalar::Field* pField_) = 0;

	// check index ability mainly about bitset
	virtual void checkIndex(Opt::Environment& cEnvironment_,
							CheckIndexArgument& cArgument_) = 0;

	// check update situation
	virtual void setForUpdate(Opt::Environment& cEnvironment_) = 0;

	// get bitset flags
	virtual bool isAbleToGetByBitSet() = 0;
	// disable getbybitset flag
	virtual void disableGetByBitSet() = 0;
	// get bitset flags
	virtual bool isSearchByBitSet() = 0;

	// getbybitset is available as the functionality of file
	virtual bool isGetByBitSetAvailable() = 0;
	// searchbybitset is available as the functionality of file
	virtual bool isSearchByBitSetAvailable() = 0;

	// get rank from bitset ID
	virtual int getRankBitSetID() = 0;
	// set rank from bitset ID
	virtual void setRankBitSetID(int iRankBitSetID_) = 0;

	// check whether rowid is obtained by bitset
	virtual bool isGetByBitSet() = 0;
	// check whether ordering is processed by file
	virtual bool hasOrder() = 0;
	virtual bool hasOrder(Order::Specification* pSpecification_) = 0;

	// is searchable for a predicate
	virtual bool isSearchable(Opt::Environment& cEnvironment_,
							  Tree::Node* pNode_) = 0;
	// check whether retrieval from record is needed
	virtual bool isNeedRetrieve() = 0;

	// is retrievable for a field?
	virtual bool isRetrievable(Opt::Environment& cEnvironment_,
							   Interface::IFile* pFile_,
							   Scalar::Field* pField_) = 0;

	// create fileaction object
	virtual Execution::Action::FileAccess*
				createFileAccess(Execution::Interface::IProgram& cProgram_) = 0;

	// create action checking predicate by index
	virtual int createCheckAction(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Execution::Action::FileAccess* pFileAccess_,
								  Candidate::AdoptArgument& cArgument_,
								  Candidate::AdoptIndexArgument& cIndexArgument_) = 0;
	// create filescan iterator using index
	virtual Execution::Interface::IIterator*
				createScan(Opt::Environment& cEnvironment_,
						   Execution::Interface::IProgram& cProgram_,
						   Execution::Action::FileAccess* pFileAccess_,
						   Candidate::AdoptArgument& cArgument_,
						   Candidate::AdoptIndexArgument& cIndexArgument_) = 0;
	// create filescan iterator using index
	virtual Execution::Interface::IIterator*
				createScanWithSearchByBitSetOption(Opt::Environment& cEnvironment_,
												   Execution::Interface::IProgram& cProgram_,
												   Execution::Action::FileAccess* pFileAccess_,
												   Candidate::AdoptArgument& cArgument_,
												   Candidate::AdoptIndexArgument& cIndexArgument_) = 0;

	// create retrieve action by fetch
	virtual void createFetch(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_,
							 Scalar::Field* pFetchKey_,
							 int iFetchKeyID_) = 0;
	// create get locator action
	virtual void createGetLocator(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_,
								  int iKeyID_) = 0;

 	//add union action by filescan
 	virtual void addUnionFileScan(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_,
								  Execution::Action::FileAccess* pFileAccess_) = 0;

	// create insert action
	virtual void createInsert(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Execution::Interface::IIterator* pIterator_,
							  Candidate::AdoptArgument& cArgument_) = 0;

	// create expunge action
	virtual void createExpunge(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Execution::Interface::IIterator* pIterator_,
							   Candidate::AdoptArgument& cArgument_) = 0;

	// create update action
	virtual void createUpdate(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Execution::Interface::IIterator* pIterator_,
							  Candidate::AdoptArgument& cArgument_) = 0;

	// create undo expunge action
	virtual void createUndoExpunge(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Execution::Interface::IIterator* pIterator_,
								   Candidate::AdoptArgument& cArgument_) = 0;

	// create undo update action
	virtual void createUndoUpdate(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_) = 0;

	// check partition by
	virtual bool checkPartitionBy(Opt::Environment& cEnvironment,
								  Order::Specification* pSpecification_) = 0;

	// check limit
	virtual bool checkLimit(Opt::Environment& cEnvironment_,
							const AccessPlan::Limit& cLimit_) = 0;

	// use undoexpunge?
	virtual bool isUseUndoExpunge(Opt::Environment& cEnvironment_) = 0;
	// use undoupdate?
	virtual bool isUseUndoUpdate(Opt::Environment& cEnvironment_) = 0;
	// use updatebyoperation?
	virtual bool isUseOperation() = 0;

	virtual Execution::Interface::IIterator*
				addBitSetToRowIDFilter(Opt::Environment& cEnvironment_,
									   Execution::Interface::IProgram& cProgram_,
									   Execution::Interface::IIterator* pIterator_,
									   Candidate::AdoptArgument& cArgument_,
									   int iInDataID_) = 0;

	// create put fields
	virtual void generatePutField(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_) = 0;

	// get Predicate interface
	virtual Predicate::ChosenInterface* getPredicateInterface() = 0;

protected:
	// constructor
	File(Candidate::Table* pTable_,
		 Interface::IFile* pFile_,
		 Parameter* pParameter_)
		: Super(),
		  m_iID(-1),
		  m_pTable(pTable_),
		  m_pFile(pFile_),
		  m_pParameter(pParameter_)
	{}

private:
	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);
	// erase from environment
	void eraseFromEnvironment(Opt::Environment& cEnvironment_);

	// id in environment
	int m_iID;

	// set by constructor
	Candidate::Table* m_pTable;
	Interface::IFile* m_pFile;
	Parameter* m_pParameter;
};

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_FILE_H

//
//	Copyright (c) 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
