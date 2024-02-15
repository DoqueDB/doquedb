// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/ChosenImpl.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_CHOSENIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_CHOSENIMPL_H

#include "boost/function.hpp"

#include "Plan/Predicate/ChosenInterface.h"
#include "Plan/AccessPlan/Cost.h"
#include "Plan/Candidate/Argument.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace ChosenImpl
{
	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::Base -- implementation class of ChosenInterface
	//
	// NOTES
	class Base
		: public Predicate::ChosenInterface
	{
	public:
		typedef Base This;
		typedef Predicate::ChosenInterface Super;
		typedef Interface::IPredicate* Argument;

		// constructor
		Base(Interface::IPredicate* pPredicate_)
			: Super(pPredicate_)
		{}

		// destructor
		virtual ~Base() {}

	////////////////////
	// ChosenInterface::
		virtual Candidate::File* getFile(Opt::Environment& cEnvironment_)
		{return 0;}
		virtual void setFile(Opt::Environment& cEnvironment_,
							 Candidate::File* pFile_)
		{;}
		virtual Candidate::File* chooseOrder(Opt::Environment& cEnvironment_,
											 Order::Specification* pSpecification_,
											 File::CheckOrderArgument& cCheckArgument_)
		{return 0;}
		virtual Interface::IPredicate* rechoose(Opt::Environment& cEnvironment_,
												ChooseArgument& cArgument_)
		{return this;}
		virtual void addLockPenalty(Opt::Environment& cEnvironment_,
									AccessPlan::Cost& cCost_,
									const AccessPlan::Cost& cScanCost_)
		{;}
		virtual bool isNeedScan(CheckNeedScanArgument& cArgument_)
		{return true;}
		virtual bool isNeedRetrieve()
		{return true;}
		virtual bool isSearchByBitSet()
		{return false;}
		virtual bool isAbleToSearchByBitSet()
		{return false;}
		virtual bool isAbleToGetByBitSet()
		{return false;}
		virtual bool isAbleToProcessByBitSet()
		{return false;}
		virtual bool isCombinedFile()
		{return false;}
		virtual bool hasOrder(Order::Specification* pSpecification_)
		{return false;}

		virtual bool isLimitAvailable(Opt::Environment& cEnvironment_)
		{return false;}

		virtual bool getCost(Opt::Environment& cEnvironment_,
							 AccessPlan::Cost& cCost_);
		virtual bool getEstimateCost(Opt::Environment& cEnvironment_,
									 AccessPlan::Cost& cCost_);
		virtual bool isRetrievable(Opt::Environment& cEnvironment_,
								   Interface::IFile* pFile_,
								   CheckRetrievableArgument& cCheckArgument_)
		{return false;}
		virtual void addRetrieve(Opt::Environment& cEnvironment_,
								 Interface::IFile* pFile_,
								 CheckRetrievableArgument& cCheckArgument_)
		{; /* do nothing */}
		virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					adoptScan(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Candidate::AdoptArgument& cArgument_,
							  Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual int createCheck(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					createScan(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual PAIR<Execution::Interface::IIterator*, int>
					createGetBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual int createBitSet(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 Candidate::AdoptIndexArgument& cIndexArgument_,
								 Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addMergeBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addUnionBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addExceptBitSet(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 Candidate::AdoptArgument& cArgument_,
									 Candidate::AdoptIndexArgument& cIndexArgument_,
									 Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void checkIndex(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::CheckIndexArgument& cCheckArgument_);
		virtual bool checkLimit(Opt::Environment& cEnvironment_,
								const AccessPlan::Limit& cLimit_);
		virtual bool isIndexAvailable(Opt::Environment& cEnvironment_)
		{return false;}
		virtual void requireIfNeeded(Opt::Environment& cEnvironment_,
									 Interface::ICandidate* pCandidate_,
									 Candidate::CheckIndexArgument& cCheckArgument_);

		virtual void setScanBetter() {}
		virtual bool isScanBetter() {return false;}

	///////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* getNotChecked()
		{return 0;}
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cCost_);

	protected:
		// create scan iterator by scan file
		PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					createScanByScan(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Candidate::AdoptArgument& cArgument_,
									 Candidate::AdoptIndexArgument& cIndexArgument_);
		// create bitset by scan file
		PAIR<Execution::Interface::IIterator*, int>
					createGetBitSetByScan(Opt::Environment& cEnvironment_,
										  Execution::Interface::IProgram& cProgram_,
										  Candidate::AdoptArgument& cArgument_,
										  Candidate::AdoptIndexArgument& cIndexArgument_,
										  Candidate::CheckIndexArgument& cCheckArgument_);
		// create bitset scan iterator
		Execution::Interface::IIterator*
					createBitSetIterator(Opt::Environment& cEnvironment_,
										 Execution::Interface::IProgram& cProgram_,
										 Candidate::AdoptArgument& cArgument_,
										 Candidate::AdoptIndexArgument& cIndexArgument_,
										 int iBitSetID_);
		// add predicate to a iterator
		void addCheckPredicate(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Execution::Interface::IIterator* pIterator_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_,
							   Interface::IPredicate* pPredicate_);
	private:
		// calculate cost
		virtual bool calculateCost(Opt::Environment& cEnvironment_,
								   AccessPlan::Cost& cCost_);
		AccessPlan::Cost m_cCost;
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::SingleImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class SingleImpl
		: public Base
	{
	public:
		typedef SingleImpl This;
		typedef Base Super;
		typedef Utility::Storage2<Interface::IPredicate*,
								  Candidate::File*> Argument;

		// constructor
		SingleImpl(Interface::IPredicate* pPredicate_,
				   Candidate::File* pFile_)
			: Super(pPredicate_),
			  m_pFile(pFile_),
			  m_bScanBetter(false)
		{}
		SingleImpl(Argument cArgument_)
			: Super(cArgument_.m_arg1),
			  m_pFile(cArgument_.m_arg2),
			  m_bScanBetter(false)
		{}

		// destructor
		virtual ~SingleImpl() {}

	////////////////////
	// ChosenInterface::
		virtual Candidate::File* getFile(Opt::Environment& cEnvironment_)
		{return m_pFile;}
		virtual void setFile(Opt::Environment& cEnvironment_,
							 Candidate::File* pFile_);
		virtual Candidate::File* chooseOrder(Opt::Environment& cEnvironment_,
											 Order::Specification* pSpecification_,
											 File::CheckOrderArgument& cCheckArgument_);
		virtual Interface::IPredicate* rechoose(Opt::Environment& cEnvironment_,
												ChooseArgument& cArgument_);
		virtual void addLockPenalty(Opt::Environment& cEnvironment_,
									AccessPlan::Cost& cCost_,
									const AccessPlan::Cost& cScanCost_);
		virtual bool isNeedIndex();
		virtual bool isNeedScan(CheckNeedScanArgument& cArgument_);
		virtual bool isNeedRetrieve();
		virtual bool isSearchByBitSet();
		virtual bool isAbleToSearchByBitSet();
		virtual bool isAbleToGetByBitSet();
		virtual bool isAbleToProcessByBitSet();
		virtual bool hasOrder(Order::Specification* pSpecification_);
		virtual bool isLimitAvailable(Opt::Environment& cEnvironment_);
	//	virtual bool getCost(Opt::Environment& cEnvironment_,
	//						 AccessPlan::Cost& cCost_);
	//	virtual bool getEstimateCost(Opt::Environment& cEnvironment_,
	//								 AccessPlan::Cost& cCost_);
		virtual bool isRetrievable(Opt::Environment& cEnvironment_,
								   Interface::IFile* pFile_,
								   CheckRetrievableArgument& cCheckArgument_);
		virtual void addRetrieve(Opt::Environment& cEnvironment_,
								 Interface::IFile* pFile_,
								 CheckRetrievableArgument& cCheckArgument_);
		virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					adoptScan(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Candidate::AdoptArgument& cArgument_,
							  Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual int createCheck(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					createScan(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual PAIR<Execution::Interface::IIterator*, int>
					createGetBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual int createBitSet(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 Candidate::AdoptIndexArgument& cIndexArgument_,
								 Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addMergeBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addUnionBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addExceptBitSet(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 Candidate::AdoptArgument& cArgument_,
									 Candidate::AdoptIndexArgument& cIndexArgument_,
									 Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void checkIndex(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::CheckIndexArgument& cCheckArgument_);
		virtual bool checkLimit(Opt::Environment& cEnvironment_,
								const AccessPlan::Limit& cLimit_);
		virtual bool isIndexAvailable(Opt::Environment& cEnvironment_)
		{return true;}
		virtual void requireIfNeeded(Opt::Environment& cEnvironment_,
									 Interface::ICandidate* pCandidate_,
									 Candidate::CheckIndexArgument& cCheckArgument_);

		virtual void setScanBetter() {m_bScanBetter = true;}
		virtual bool isScanBetter() {return m_bScanBetter;}

	/////////////////////////
	// Interface::IPredicate::
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cCost_);

	/////////////////////////
	// Interface::IScalar::
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);

	protected:
	private:
		virtual bool calculateCost(Opt::Environment& cEnvironment_,
								   AccessPlan::Cost& cCost_);

		Candidate::File* m_pFile;
		bool m_bScanBetter;
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::SingleCheckUnknownImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class SingleCheckUnknownImpl
		: public SingleImpl
	{
	public:
		typedef SingleCheckUnknownImpl This;
		typedef SingleImpl Super;
		typedef Utility::Storage3<Interface::IPredicate*,
								  Candidate::File*,
								  Interface::IPredicate*> Argument;

		// constructor
		SingleCheckUnknownImpl(Interface::IPredicate* pPredicate_,
							   Candidate::File* pFile_,
							   Interface::IPredicate* pCheckUnknown_)
			: Super(pPredicate_, pFile_),
			  m_pCheckUnknown(pCheckUnknown_)
		{}
		SingleCheckUnknownImpl(Argument cArgument_)
			: Super(cArgument_),
			  m_pCheckUnknown(cArgument_.m_arg3)
		{}

		// destructor
		~SingleCheckUnknownImpl() {}

	////////////////////
	// ChosenInterface::
		virtual bool isNeedScan(CheckNeedScanArgument& cArgument_)
		{return true;}
		virtual bool isNeedRetrieve()
		{return true;}
		virtual bool isSearchByBitSet()
		{return false;}
		virtual bool isAbleToSearchByBitSet()
		{return false;}
		virtual bool isAbleToGetByBitSet()
		{return false;}
		virtual bool isAbleToProcessByBitSet()
		{return false;}
		virtual bool hasOrder(Order::Specification* pSpecification_)
		{return false;}
		virtual bool isLimitAvailable(Opt::Environment& cEnvironment_)
		{return false;}
	//	virtual bool getCost(Opt::Environment& cEnvironment_,
	//						 AccessPlan::Cost& cCost_);
	//	virtual bool getEstimateCost(Opt::Environment& cEnvironment_,
	//								 AccessPlan::Cost& cCost_);
		virtual int createCheck(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					createScan(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual void addMergeBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addUnionBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addExceptBitSet(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 Candidate::AdoptArgument& cArgument_,
									 Candidate::AdoptIndexArgument& cIndexArgument_,
									 Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void checkIndex(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::CheckIndexArgument& cCheckArgument_);
		virtual bool checkLimit(Opt::Environment& cEnvironment_,
								const AccessPlan::Limit& cLimit_);
		virtual void requireIfNeeded(Opt::Environment& cEnvironment_,
									 Interface::ICandidate* pCandidate_,
									 Candidate::CheckIndexArgument& cCheckArgument_);

	//	virtual void setScanBetter();
	//	virtual bool isScanBetter();

	/////////////////////////
	// Interface::IScalar::
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);

	protected:
	private:
		Interface::IPredicate* m_pCheckUnknown;
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::MultipleImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class MultipleImpl
		: public Base
	{
	public:
		typedef MultipleImpl This;
		typedef Base Super;
		typedef Utility::Storage2<Interface::IPredicate*,
								  const VECTOR<Interface::IPredicate*>&> Argument;

		// constructor
		MultipleImpl(Interface::IPredicate* pPredicate_,
					 const VECTOR<Interface::IPredicate*>& vecPredicate_)
			: Super(pPredicate_),
			  m_vecPredicate(vecPredicate_),
			  m_cCheckIndexArgument(),
			  m_pFile(0)
		{}
		MultipleImpl(Argument cArgument_)
			: Super(cArgument_.m_arg1),
			  m_vecPredicate(cArgument_.m_arg2),
			  m_cCheckIndexArgument(),
			  m_pFile(0)
		{}

		// destructor
		virtual ~MultipleImpl() {}

	////////////////////
	// ChosenInterface::
		virtual Candidate::File* getFile(Opt::Environment& cEnvironment_)
		{return m_pFile;}
		virtual void setFile(Opt::Environment& cEnvironment_,
							 Candidate::File* pFile_);
	//	virtual Candidate::File* chooseOrder(Opt::Environment& cEnvironment_,
	//										  Order::Specification* pSpecification_,
	//										  File::CheckOrderArgument& cCheckArgument_);
		virtual Interface::IPredicate* rechoose(Opt::Environment& cEnvironment_,
												ChooseArgument& cArgument_);
		virtual void addLockPenalty(Opt::Environment& cEnvironment_,
									AccessPlan::Cost& cCost_,
									const AccessPlan::Cost& cScanCost_);
	//	virtual bool isNeedScan(CheckNeedScanArgument& cArgument_);
		virtual bool isNeedRetrieve();
	//	virtual bool isSearchByBitSet();
	//	virtual bool isAbleToSearchByBitSet();
	//	virtual bool isAbleToGetByBitSet();
	//	virtual bool isAbleToProcessByBitSet();
	//	virtual bool hasOrder(Order::Specification* pSpecification_);
	//	virtual bool isLimitAvailable(Opt::Environment& cEnvironment_);
	//	virtual bool getCost(Opt::Environment& cEnvironment_,
	//						 AccessPlan::Cost& cCost_);
	//	virtual bool getEstimateCost(Opt::Environment& cEnvironment_,
	//								 AccessPlan::Cost& cCost_);
	//	virtual bool isRetrievable(Opt::Environment& cEnvironment_,
	//							   Interface::IFile* pFile_,
	//							   CheckRetrievableArgument& cCheckArgument_);
	//	virtual void addRetrieve(Opt::Environment& cEnvironment_,
	//							 Interface::IFile* pFile_,
	//							 CheckRetrievableArgument& cCheckArgument_);
	//	virtual void checkIndex(Opt::Environment& cEnvironment_,
	//							Execution::Interface::IProgram& cProgram_,
	//							Candidate::AdoptArgument& cArgument_,
	//							Candidate::CheckIndexArgument& cCheckArgument_);
	//	virtual bool checkLimit(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Limit& cLimit_);
		virtual bool isIndexAvailable(Opt::Environment& cEnvironment_);
		virtual void requireIfNeeded(Opt::Environment& cEnvironment_,
									 Interface::ICandidate* pCandidate_,
									 Candidate::CheckIndexArgument& cCheckArgument_);

	//////////////////////////////
	// Interface::IPredicate::
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cCost_);

	/////////////////////////
	// Interface::IScalar::
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);

	protected:
		VECTOR<Interface::IPredicate*>& getOperand() {return m_vecPredicate;}
		Candidate::File* getCombinedFile() {return m_pFile;}

		virtual Candidate::CheckIndexArgument&
					getCheckIndexArgument(bool bIgnoreField_);

		// create action list checking each operand
		void createCheckOperand(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								VECTOR<int>& vecAction_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::AdoptIndexArgument& cIndexArgument_,
								Candidate::CheckIndexArgument& cCheckArgument_);
		// check index availability for each operand
		void checkOperandIndex(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::CheckIndexArgument& cCheckArgument_);
	private:
		// rechoose an operand
		static bool rechooseOperand(Opt::Environment& cEnvironment_,
									ChooseArgument& cArgument_,
									Interface::IPredicate* pOperand_,
									VECTOR<Interface::IPredicate*>& vecRechosen_);

		void createCheckBitSet(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Execution::Interface::IIterator* pIterator_,
							   VECTOR<int>& vecAction_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_,
							   Candidate::CheckIndexArgument& cCheckArgument_);

		// for calculate cost
		virtual void accumulateCost(Opt::Environment& cEnvironment_,
									Interface::IPredicate* pPredicate_,
									boost::function<bool(Interface::IPredicate*, AccessPlan::Cost&)> cFunction_,
									AccessPlan::Cost& cResult_) = 0;

	////////////////////
	// ChosenInterface::
		virtual bool calculateCost(Opt::Environment& cEnvironment_,
								   AccessPlan::Cost& cCost_);

		VECTOR<Interface::IPredicate*> m_vecPredicate;
		Candidate::CheckIndexArgument m_cCheckIndexArgument;

		Candidate::File* m_pFile; // union or intersect
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::AndImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class AndImpl
		: public MultipleImpl
	{
	public:
		typedef AndImpl This;
		typedef MultipleImpl Super;

		// constructor
		AndImpl(Interface::IPredicate* pPredicate_,
				const VECTOR<Interface::IPredicate*>& vecPredicate_)
			: Super(pPredicate_, vecPredicate_),
			  m_iBitSetIDCache(-1)
		{}
		AndImpl(Argument cArgument_)
			: Super(cArgument_),
			  m_iBitSetIDCache(-1)
		{}

		// destructor
		virtual ~AndImpl() {}

	////////////////////
	// ChosenInterface::
	//	virtual Candidate::File* getFile(Opt::Environment& cEnvironment_);
	//	virtual void setFile(Opt::Environment& cEnvironment_,
	//						 Candidate::File* pFile_);
		virtual Candidate::File* chooseOrder(Opt::Environment& cEnvironment_,
											 Order::Specification* pSpecification_,
											 File::CheckOrderArgument& cCheckArgument_);
		virtual bool isNeedScan(CheckNeedScanArgument& cArgument_);
	//	virtual bool isNeedRetrieve();
		virtual bool isSearchByBitSet();
		virtual bool isAbleToSearchByBitSet();
		virtual bool isAbleToGetByBitSet();
		virtual bool isAbleToProcessByBitSet();
		virtual bool hasOrder(Order::Specification* pSpecification_);
		virtual bool isLimitAvailable(Opt::Environment& cEnvironment_);
	//	virtual bool getCost(Opt::Environment& cEnvironment_,
	//						 AccessPlan::Cost& cCost_);
	//	virtual bool getEstimateCost(Opt::Environment& cEnvironment_,
	//								 AccessPlan::Cost& cCost_);
		virtual bool isRetrievable(Opt::Environment& cEnvironment_,
								   Interface::IFile* pFile_,
								   CheckRetrievableArgument& cCheckArgument_);
		virtual void addRetrieve(Opt::Environment& cEnvironment_,
								 Interface::IFile* pFile_,
								 CheckRetrievableArgument& cCheckArgument_);
		virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					adoptScan(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Candidate::AdoptArgument& cArgument_,
							  Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual int createCheck(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					createScan(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual PAIR<Execution::Interface::IIterator*, int>
					createGetBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual int createBitSet(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 Candidate::AdoptIndexArgument& cIndexArgument_,
								 Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addMergeBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addUnionBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addExceptBitSet(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 Candidate::AdoptArgument& cArgument_,
									 Candidate::AdoptIndexArgument& cIndexArgument_,
									 Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void checkIndex(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::CheckIndexArgument& cCheckArgument_);
		virtual bool checkLimit(Opt::Environment& cEnvironment_,
								const AccessPlan::Limit& cLimit_);

	/////////////////////////////
	// Interface::IPredicate
		virtual bool isFetch();
		virtual bool getFetchKey(Opt::Environment& cEnvironment_,
								 Utility::ScalarSet& cFetchKey_);

	protected:
	////////////////////
	// MultipleImpl::
		virtual Candidate::CheckIndexArgument&
					getCheckIndexArgument(bool bIgnoreField_);

	private:
		Execution::Interface::IIterator*
					createScanFetch(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_);
		Execution::Interface::IIterator*
					createScanBitSet(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Candidate::AdoptArgument& cArgument_,
									 Candidate::AdoptIndexArgument& cIndexArgument_);
		Execution::Interface::IIterator*
					createScanNormal(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Candidate::AdoptArgument& cArgument_,
									 Candidate::AdoptIndexArgument& cIndexArgument_);
		int createCheckPredicate(Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 VECTOR<int>& vecAction_);
	////////////////////
	// MultipleImpl::
		virtual void accumulateCost(Opt::Environment& cEnvironment_,
									Interface::IPredicate* pPredicate_,
									boost::function<bool(Interface::IPredicate*, AccessPlan::Cost&)> cFunction_,
									AccessPlan::Cost& cResult_);

		int m_iBitSetIDCache;
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::OrImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class OrImpl
		: public MultipleImpl
	{
	public:
		typedef OrImpl This;
		typedef MultipleImpl Super;

		// constructor
		OrImpl(Interface::IPredicate* pPredicate_,
			   const VECTOR<Interface::IPredicate*>& vecPredicate_)
			: Super(pPredicate_, vecPredicate_),
			  m_mapRetrieveField()
		{}
		OrImpl(Argument cArgument_)
			: Super(cArgument_),
			  m_mapRetrieveField()
		{}

		// destructor
		virtual ~OrImpl() {}

	////////////////////
	// ChosenInterface::
	//	virtual Candidate::File* getFile(Opt::Environment& cEnvironment_);
	//	virtual void setFile(Opt::Environment& cEnvironment_,
	//						 Candidate::File* pFile_);
		virtual Candidate::File* chooseOrder(Opt::Environment& cEnvironment_,
											 Order::Specification* pSpecification_,
											 File::CheckOrderArgument& cCheckArgument_);
		virtual bool isNeedScan(CheckNeedScanArgument& cArgument_);
	//	virtual bool isNeedRetrieve();
		virtual bool isSearchByBitSet();
		virtual bool isAbleToSearchByBitSet();
		virtual bool isAbleToGetByBitSet();
		virtual bool isAbleToProcessByBitSet();
		virtual bool hasOrder(Order::Specification* pSpecification_);
		virtual bool isLimitAvailable(Opt::Environment& cEnvironment_);
	//	virtual bool getCost(Opt::Environment& cEnvironment_,
	//						 AccessPlan::Cost& cCost_);
	//	virtual bool getEstimateCost(Opt::Environment& cEnvironment_,
	//								 AccessPlan::Cost& cCost_);
		virtual bool isRetrievable(Opt::Environment& cEnvironment_,
								   Interface::IFile* pFile_,
								   CheckRetrievableArgument& cCheckArgument_);
		virtual void addRetrieve(Opt::Environment& cEnvironment_,
								 Interface::IFile* pFile_,
								 CheckRetrievableArgument& cCheckArgument_);
		virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					adoptScan(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Candidate::AdoptArgument& cArgument_,
							  Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual int createCheck(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
					createScan(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual PAIR<Execution::Interface::IIterator*, int>
					createGetBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual int createBitSet(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 Candidate::AdoptIndexArgument& cIndexArgument_,
								 Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addMergeBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_,
								    Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addUnionBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_,
								    Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addExceptBitSet(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 Candidate::AdoptArgument& cArgument_,
									 Candidate::AdoptIndexArgument& cIndexArgument_,
									 Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void checkIndex(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::CheckIndexArgument& cCheckArgument_);
		virtual bool checkLimit(Opt::Environment& cEnvironment_,
								const AccessPlan::Limit& cLimit_);

	///////////////////////////////
	// Interface::IPredicate::
	//	virtual bool isFetch();

	protected:
	////////////////////
	// MultipleImpl::
		virtual Candidate::CheckIndexArgument&
					getCheckIndexArgument(bool bIgnoreField_);

	private:
		Execution::Interface::IIterator*
					createScanByUnion(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Candidate::AdoptArgument& cArgument_,
									  Candidate::AdoptIndexArgument& cIndexArgument_,
									  Execution::Interface::IIterator* pGetBitSet_,
									  const VECTOR<ChosenInterface*>& vecChosen_);

		void addUnionOperand(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pOperand_,
							 Candidate::AdoptArgument& cArgument_,
							 Execution::Interface::IIterator* pUnion_,
							 int iKeyID_,
							 VECTOR<int>* pvecOutID_,
							 Interface::IPredicate* pPredicate_);
		int createCheckPredicate(Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 VECTOR<int>& vecAction_);

		int createBitSetParallel(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 Candidate::AdoptIndexArgument& cIndexArgument_,
								 Candidate::CheckIndexArgument& cCheckArgument_);
		int addParallelBitSet(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Execution::Interface::IIterator* pIterator_,
							  Candidate::AdoptArgument& cArgument_,
							  Candidate::AdoptIndexArgument& cIndexArgument_,
							  Candidate::CheckIndexArgument& cCheckArgument_,
							  ChosenInterface* pChosen_);

	////////////////////
	// MultipleImpl::
		virtual void accumulateCost(Opt::Environment& cEnvironment_,
									Interface::IPredicate* pPredicate_,
									boost::function<bool(Interface::IPredicate*, AccessPlan::Cost&)> cFunction_,
									AccessPlan::Cost& cResult_);

		typedef MAP<Interface::IPredicate*,
					Utility::FieldSet,
					Utility::PredicateSet::Comparator> OperandFieldMap;
		OperandFieldMap m_mapRetrieveField;
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::NotImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class NotImpl
		: public Base
	{
	public:
		typedef NotImpl This;
		typedef Base Super;
		typedef Utility::Storage2<Interface::IPredicate*,
								  Interface::IPredicate*> Argument;

		// constructor
		NotImpl(Interface::IPredicate* pPredicate_,
				Interface::IPredicate* pOperand_)
			: Super(pPredicate_),
			  m_pOperand(pOperand_)
		{}
		NotImpl(Argument cArgument_)
			: Super(cArgument_.m_arg1),
			  m_pOperand(cArgument_.m_arg2)
		{}

		// destructor
		virtual ~NotImpl() {}

		// accessor
		Interface::IPredicate* getOperand() {return m_pOperand;}

	////////////////////
	// ChosenInterface::
		virtual Candidate::File* getFile(Opt::Environment& cEnvironment_);
		virtual void setFile(Opt::Environment& cEnvironment_,
							 Candidate::File* pFile_);
	//	virtual Candidate::File* chooseOrder(Opt::Environment& cEnvironment_,
	//										 Order::Specification* pSpecification_,
	//										 File::CheckOrderArgument& cCheckArgument_);
		virtual Interface::IPredicate* rechoose(Opt::Environment& cEnvironment_,
												ChooseArgument& cArgument_);
		virtual void addLockPenalty(Opt::Environment& cEnvironment_,
									AccessPlan::Cost& cCost_,
									const AccessPlan::Cost& cScanCost_);
		virtual bool isNeedScan(CheckNeedScanArgument& cArgument_);
		virtual bool isNeedRetrieve();
	//	virtual bool isSearchByBitSet();
	//	virtual bool isAbleToSearchByBitSet();
	//	virtual bool isAbleToGetByBitSet();
		virtual bool isAbleToProcessByBitSet();
	//	virtual bool hasOrder(Order::Specification* pSpecification_);
	//	virtual bool isLimitAvailable(Opt::Environment& cEnvironment_);
	//	virtual bool getCost(Opt::Environment& cEnvironment_,
	//						 AccessPlan::Cost& cCost_);
	//	virtual bool getEstimateCost(Opt::Environment& cEnvironment_,
	//								 AccessPlan::Cost& cCost_);
	//	virtual bool isRetrievable(Opt::Environment& cEnvironment_,
	//							   Interface::IFile* pFile_,
	//							   CheckRetrievableArgument& cCheckArgument_);
	//	virtual void addRetrieve(Opt::Environment& cEnvironment_,
	//							 Interface::IFile* pFile_,
	//							 CheckRetrievableArgument& cCheckArgument_);
	//	virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
	//					adoptScan(Opt::Environment& cEnvironment_,
	//							  Execution::Interface::IProgram& cProgram_,
	//							  Candidate::AdoptArgument& cArgument_);
		virtual int createCheck(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_,
							    Candidate::AdoptIndexArgument& cIndexArgument_);
	//	virtual PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
	//					createScan(Opt::Environment& cEnvironment_,
	//							   Execution::Interface::IProgram& cProgram_,
	//							   Candidate::AdoptArgument& cArgument_,
	//							   Candidate::AdoptIndexArgument& cIndexArgument_);
	//	virtual PAIR<Execution::Interface::IIterator*, int>
	//					createGetBitSet(Opt::Environment& cEnvironment_,
	//									Execution::Interface::IProgram& cProgram_,
	//									Candidate::AdoptArgument& cArgument_,
	//									Candidate::AdoptIndexArgument& cIndexArgument_,
	//									Candidate::CheckIndexArgument& cCheckArgument_);
	//	virtual int createBitSet(Opt::Environment& cEnvironment_,
	//							  Execution::Interface::IProgram& cProgram_,
	//							  Execution::Interface::IIterator* pIterator_,
	//							  Candidate::AdoptArgument& cArgument_,
	//							  Candidate::AdoptIndexArgument& cIndexArgument_,
	//							  Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void addMergeBitSet(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_,
									Candidate::CheckIndexArgument& cCheckArgument_);
	//	virtual void addUnionBitSet(Opt::Environment& cEnvironment_,
	//								Execution::Interface::IProgram& cProgram_,
	//								Execution::Interface::IIterator* pIterator_,
	//								Candidate::AdoptArgument& cArgument_,
	//								Candidate::AdoptIndexArgument& cIndexArgument_,
	//								Candidate::CheckIndexArgument& cCheckArgument_);
	//	virtual void addExceptBitSet(Opt::Environment& cEnvironment_,
	//								 Execution::Interface::IProgram& cProgram_,
	//								 Execution::Interface::IIterator* pIterator_,
	//								 Candidate::AdoptArgument& cArgument_,
	//								 Candidate::AdoptIndexArgument& cIndexArgument_,
	//								 Candidate::CheckIndexArgument& cCheckArgument_);
		virtual void checkIndex(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::CheckIndexArgument& cCheckArgument_);
	//	virtual bool checkLimit(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Limit& cLimit_);

		virtual bool isIndexAvailable(Opt::Environment& cEnvironment_);
		virtual void requireIfNeeded(Opt::Environment& cEnvironment_,
									 Interface::ICandidate* pCandidate_,
									 Candidate::CheckIndexArgument& cCheckArgument_);

		virtual void setScanBetter();
		virtual bool isScanBetter();

	/////////////////////////////
	// Interface::IPredicate::
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cCost_);

	/////////////////////////
	// Interface::IScalar::
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);

	protected:
	private:
		virtual bool calculateCost(Opt::Environment& cEnvironment_,
								   AccessPlan::Cost& cCost_);

		void convertRate(const AccessPlan::Cost& cCost_,
						 AccessPlan::Cost& cOperandCost_);

		Interface::IPredicate* m_pOperand;
	};

	//////////////////////////////
	// TEMPLATE CLASS 
	//	Predicate::ChosenImpl::NotCheckedMix -- mix-in for ChosenInterface
	//
	// TEMPLATE ARGUMENTS
	//	class Base_
	//		subclass of ChosenInterface
	//
	// NOTES
	template <class Base_>
	class NotCheckedMix
		: public Base_
	{
	public:
		typedef Base_ Super;
		typedef NotCheckedMix<Base_> This;

		// constructor
		NotCheckedMix(typename Super::Argument cArgument_,
					  Interface::IPredicate* pNotChecked_)
			: Super(cArgument_),
			  m_pNotChecked(pNotChecked_)
		{}
		// destructor
		virtual ~NotCheckedMix() {}

	////////////////////
	// ChosenInterface::

	////////////////////
	// Interface::
		virtual Interface::IPredicate* getNotChecked()
		{return m_pNotChecked;}

	protected:
	private:
		Interface::IPredicate* m_pNotChecked;
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::PartialNoFileImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class PartialNoFileImpl
		: public NotCheckedMix<Base>
	{
	public:
		typedef PartialNoFileImpl This;
		typedef NotCheckedMix<Base> Super;

		// constructor
		PartialNoFileImpl(Interface::IPredicate* pPredicate_,
						  Interface::IPredicate* pNotChecked_)
			: Super(Argument(pPredicate_),
					pNotChecked_)
		{}

		// destructor
		~PartialNoFileImpl() {}
	protected:
	private:
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::PartialSingleImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class PartialSingleImpl
		: public NotCheckedMix<SingleImpl>
	{
	public:
		typedef PartialSingleImpl This;
		typedef NotCheckedMix<SingleImpl> Super;

		// constructor
		PartialSingleImpl(Interface::IPredicate* pPredicate_,
						  Interface::IPredicate* pNotChecked_,
						  Candidate::File* pFile_)
			: Super(Argument(pPredicate_, pFile_),
					pNotChecked_)
		{}

		// destructor
		~PartialSingleImpl() {}
	protected:
	private:
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::PartialSingleCheckUnknownImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class PartialSingleCheckUnknownImpl
		: public NotCheckedMix<SingleCheckUnknownImpl>
	{
	public:
		typedef PartialSingleCheckUnknownImpl This;
		typedef NotCheckedMix<SingleCheckUnknownImpl> Super;

		// constructor
		PartialSingleCheckUnknownImpl(Interface::IPredicate* pPredicate_,
									  Interface::IPredicate* pNotChecked_,
									  Candidate::File* pFile_,
									  Interface::IPredicate* pCheckUnknown_)
			: Super(Argument(pPredicate_, pFile_, pCheckUnknown_),
					pNotChecked_)
		{}

		// destructor
		~PartialSingleCheckUnknownImpl() {}
	protected:
	private:
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::PartialAndImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class PartialAndImpl
		: public NotCheckedMix<AndImpl>
	{
	public:
		typedef PartialAndImpl This;
		typedef NotCheckedMix<AndImpl> Super;

		// constructor
		PartialAndImpl(Interface::IPredicate* pPredicate_,
					   Interface::IPredicate* pNotChecked_,
					   const VECTOR<Interface::IPredicate*>& vecPredicate_)
			: Super(Argument(pPredicate_, vecPredicate_),
					pNotChecked_)
		{}

		// destructor
		~PartialAndImpl() {}
	protected:
	private:
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::PartialOrImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class PartialOrImpl
		: public NotCheckedMix<OrImpl>
	{
	public:
		typedef PartialOrImpl This;
		typedef NotCheckedMix<OrImpl> Super;

		// constructor
		PartialOrImpl(Interface::IPredicate* pPredicate_,
					  Interface::IPredicate* pNotChecked_,
					  const VECTOR<Interface::IPredicate*>& vecPredicate_)
			: Super(Argument(pPredicate_, vecPredicate_),
					pNotChecked_)
		{}

		// destructor
		~PartialOrImpl() {}
	protected:
	private:
	};

	//////////////////////////////
	// CLASS 
	//	Predicate::ChosenImpl::PartialNotImpl -- implementation class of ChosenInterface
	//
	// NOTES
	class PartialNotImpl
		: public NotCheckedMix<NotImpl>
	{
	public:
		typedef PartialNotImpl This;
		typedef NotCheckedMix<NotImpl> Super;

		// constructor
		PartialNotImpl(Interface::IPredicate* pPredicate_,
					   Interface::IPredicate* pNotChecked_,
					   Interface::IPredicate* pOperand_)
			: Super(Argument(pPredicate_, pOperand_),
					pNotChecked_)
		{}

		// destructor
		~PartialNotImpl() {}
	protected:
	private:
	};
} // namespace ChosenImpl

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_CHOSENIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
