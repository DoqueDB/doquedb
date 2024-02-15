// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/ChosenInterface.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_CHOSENINTERFACE_H
#define __SYDNEY_PLAN_PREDICATE_CHOSENINTERFACE_H

#include "Plan/Declaration.h"
#include "Plan/Predicate/InterfaceWrapper.h"
#include "Plan/Utility/ObjectSet.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Predicate::ChosenInterface -- Wrapper interface for representing choosed index of predicates
//
//	NOTES
//		This class is not constructed directly
class ChosenInterface
	: public InterfaceWrapper
{
public:
	typedef InterfaceWrapper Super;
	typedef ChosenInterface This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						Candidate::File* pFile_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						Candidate::File* pFile_,
						Interface::IPredicate* pCheckUnknown_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						Interface::IPredicate* pNotChecked_,
						Candidate::File* pFile_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						Interface::IPredicate* pNotChecked_,
						Candidate::File* pFile_,
						Interface::IPredicate* pCheckUnknown_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						const VECTOR<Interface::IPredicate*>& vecChosen_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						Interface::IPredicate* pNotChecked_,
						const VECTOR<Interface::IPredicate*>& vecChosen_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						Interface::IPredicate* pOperand_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						Interface::IPredicate* pNotChecked_,
						Interface::IPredicate* pOperand_);

	// destructor
	virtual ~ChosenInterface() {}

	// get chosen file
	virtual Candidate::File* getFile(Opt::Environment& cEnvironment_) = 0;

	// set file candidate
	virtual void setFile(Opt::Environment& cEnvironment_,
						 Candidate::File* pFile_) = 0;

	// choose file to process order
	virtual Candidate::File* chooseOrder(Opt::Environment& cEnvironment_,
										 Order::Specification* pSpecification_,
										 File::CheckOrderArgument& cCheckArgument_) = 0;

	// choose again after scanning file to process order is determined
	virtual Interface::IPredicate* rechoose(Opt::Environment& cEnvironment_,
											ChooseArgument& cArgument_) = 0;

	// add locking penalty if needed
	virtual void addLockPenalty(Opt::Environment& cEnvironment_,
								AccessPlan::Cost& cCost_,
								const AccessPlan::Cost& cScanCost_) = 0;

	// is need scanning?
	virtual bool isNeedScan(CheckNeedScanArgument& cArgument_) = 0;
	// is need retrieval from record?
	virtual bool isNeedRetrieve() = 0;
	// is predicate can be processed using searchbybitset?
	virtual bool isSearchByBitSet() = 0;
	// is predicate can be processed using searchbybitset?
	virtual bool isAbleToSearchByBitSet() = 0;
	// is predicate can be processed using getbybitset?
	virtual bool isAbleToGetByBitSet() = 0;
	// is predicate can be refined using bitset?
	virtual bool isAbleToProcessByBitSet() = 0;
	// is combined file?
	virtual bool isCombinedFile() = 0;

	// is order can be processed?
	virtual bool hasOrder(Order::Specification* pSpecification_) = 0;

	// is limit can be used as hint?
	virtual bool isLimitAvailable(Opt::Environment& cEnvironment_) = 0;

	// cost estimation (using file)
	virtual bool getCost(Opt::Environment& cEnvironment_,
						 AccessPlan::Cost& cCost_) = 0;
	// cost estimation (without file)
	virtual bool getEstimateCost(Opt::Environment& cEnvironment_,
								 AccessPlan::Cost& cCost_) = 0;

	// is a file used to process predicate and can retrieve column?
	virtual bool isRetrievable(Opt::Environment& cEnvironment_,
							   Interface::IFile* pFile_,
							   CheckRetrievableArgument& cArgument_) = 0;
	// add retrieved field
	virtual void addRetrieve(Opt::Environment& cEnvironment_,
							 Interface::IFile* pFile_,
							 CheckRetrievableArgument& cArgument_) = 0;

	// create execution
	virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					adoptScan(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Candidate::AdoptArgument& cArgument_,
							  Candidate::AdoptIndexArgument& cIndexArgument_) = 0;

	// create check action
	virtual int createCheck(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_,
							Candidate::AdoptIndexArgument& cIndexArgument_) = 0;
	// create scaning iterator
	virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					createScan(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_) = 0;
	// create scanning iterator to get bitset
	virtual PAIR<Execution::Interface::IIterator*, int>
					createGetBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_) = 0;
	// create getting merged bitset action
	virtual int createBitSet(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_,
							 Candidate::AdoptIndexArgument& cIndexArgument_,
							 Candidate::CheckIndexArgument& cCheckArgument_) = 0;
	// add merge bitset action to iterator
	virtual void addMergeBitSet(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::AdoptIndexArgument& cIndexArgument_,
								Candidate::CheckIndexArgument& cCheckArgument_) = 0;
	// add union bitset action to iterator
	virtual void addUnionBitSet(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::AdoptIndexArgument& cIndexArgument_,
								Candidate::CheckIndexArgument& cCheckArgument_) = 0;
	// add except bitset action to iterator
	virtual void addExceptBitSet(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 Candidate::AdoptIndexArgument& cIndexArgument_,
								 Candidate::CheckIndexArgument& cCheckArgument_) = 0;

	// check index availability for the predicate
	virtual void checkIndex(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Candidate::AdoptArgument& cArgument_,
							Candidate::CheckIndexArgument& cCheckArgument_) = 0;

	// check limit
	virtual bool checkLimit(Opt::Environment& cEnvironment_,
							const AccessPlan::Limit& cLimit_) = 0;

	// index can be used in operands
	virtual bool isIndexAvailable(Opt::Environment& cEnvironment_) = 0;

	// require if needed
	virtual void requireIfNeeded(Opt::Environment& cEnvironment_,
								 Interface::ICandidate* pCandidate_,
								 Candidate::CheckIndexArgument& cCheckArgument_) = 0;

	// mark as scanning is better
	virtual void setScanBetter() = 0;
	// check if scanning is better
	virtual bool isScanBetter() = 0;

////////////////////
// Interface::
	virtual bool isChosen()
	{return true;}
	virtual ChosenInterface* getChosen()
	{return this;}

protected:
	// constructor
	ChosenInterface(Interface::IPredicate* pPredicate_);

private:
};

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_CHOSENINTERFACE_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
