// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/FileImpl.h --
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

#ifndef __SYDNEY_PLAN_CANDIDATE_IMPL_FILEIMPL_H
#define __SYDNEY_PLAN_CANDIDATE_IMPL_FILEIMPL_H

#include "Plan/Candidate/File.h"
#include "Plan/AccessPlan/Cost.h"

#include "Plan/Predicate/Impl/ChosenImpl.h"

#include "Server/Session.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

namespace FileImpl
{
	////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::FileImpl::Normal -- file information in access plan
	//
	// NOTES
	//	This class is NOT a subclass of Interface::ICandidate

	class Normal
	: public Candidate::File
	{
	public:
		typedef Normal This;
		typedef Candidate::File Super;

		// constructor
		Normal(Candidate::Table* pTable_,
			   Interface::IFile* pFile_,
			   Parameter* pParameter_)
			: Super(pTable_, pFile_, pParameter_),
			  m_bGetByBitSet(false),
			  m_bSearchByBitSet(false),
			  m_iRankBitSetID(-1),
			  m_cKey(),
			  m_cField(),
			  m_cInsertField(),
			  m_cUndoField(),
			  m_mapFetch(),
			  m_cExplainArgument()
		{}
		// destructor
		~Normal() {}

	////////////////////////////////
	// Candidate::File::
		virtual void setIsSimple();
		virtual void clearBitSetFlag();

		virtual bool check(Opt::Environment& cEnvironment_,
						   File::CheckArgument& cArgument_,
						   boost::function<bool(LogicalFile::AutoLogicalFile&,
												LogicalFile::OpenOption&)> function_);
		virtual bool isAlwaysUsed();
		virtual bool isAlwaysBitSet();
		virtual bool isCheckByKey(Opt::Environment& cEnvironment_);
		virtual bool isLocatorUsed();
		virtual bool isOtherFieldUsed();

		virtual void addCheckIndexResult(Opt::Environment& cEnvironment_,
										 CheckIndexArgument& cArgument_,
										 Predicate::ChosenInterface* pPredicate_);

		virtual AccessPlan::Cost& getCost(Opt::Environment& cEnvironment_,
										  Interface::IPredicate* pPredicate_ = 0);

		virtual void addFieldForPredicate(Opt::Environment& cEnvironment_,
										  Interface::IFile* pFile_,
										  Predicate::CheckRetrievableArgument& cCheckArgument_);
		virtual void addField(Opt::Environment& cEnvironment_,
							  Scalar::Field* pField_);
		virtual void addPutKey(Opt::Environment& cEnvironment_,
							   Scalar::Field* pField_);
		virtual void addInsertField(Opt::Environment& cEnvironment_,
									Scalar::Field* pField_);
		virtual void addUndoField(Opt::Environment& cEnvironment_,
								  Scalar::Field* pField_);
		virtual void checkIndex(Opt::Environment& cEnvironment_,
								CheckIndexArgument& cArgument_);

		virtual void setForUpdate(Opt::Environment& cEnvironment_);

		virtual bool isAbleToGetByBitSet() {return m_bGetByBitSet;}
		virtual void disableGetByBitSet() {m_bGetByBitSet = false;}
		virtual bool isSearchByBitSet() {return m_bSearchByBitSet;}
		
		virtual bool isGetByBitSetAvailable();
		virtual bool isSearchByBitSetAvailable();

		virtual int getRankBitSetID() {return m_iRankBitSetID;}
		virtual void setRankBitSetID(int iRankBitSetID_) {m_iRankBitSetID = iRankBitSetID_;}

		virtual bool isGetByBitSet();
		virtual bool hasOrder();
		virtual bool hasOrder(Order::Specification* pSpecification_);

		virtual bool isSearchable(Opt::Environment& cEnvironment_,
								  Tree::Node* pNode_);
		virtual bool isNeedRetrieve();
		virtual bool isRetrievable(Opt::Environment& cEnvironment_,
								   Interface::IFile* pFile_,
								   Scalar::Field* pField_);

		virtual Execution::Action::FileAccess*
					createFileAccess(Execution::Interface::IProgram& cProgram_);
		virtual int createCheckAction(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Execution::Action::FileAccess* pFileAccess_,
									  Candidate::AdoptArgument& cArgument_,
									  Candidate::AdoptIndexArgument& cIndexArgument_);

		virtual Execution::Interface::IIterator*
					createScan(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Execution::Action::FileAccess* pFileAccess_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual Execution::Interface::IIterator*
					createScanWithSearchByBitSetOption(Opt::Environment& cEnvironment_,
													   Execution::Interface::IProgram& cProgram_,
													   Execution::Action::FileAccess* pFileAccess_,
													   Candidate::AdoptArgument& cArgument_,
													   Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual void createFetch(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 Scalar::Field* pFetchKey_,
								 int iFetchKeyID_);
		virtual void createGetLocator(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_,
									  int iKeyID_);
		virtual void addUnionFileScan(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_,
									  Execution::Action::FileAccess* pFileAccess_);

		virtual void createInsert(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_);
		virtual void createExpunge(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Execution::Interface::IIterator* pIterator_,
								   Candidate::AdoptArgument& cArgument_);
		virtual void createUpdate(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_);
		virtual void createUndoExpunge(Opt::Environment& cEnvironment_,
									   Execution::Interface::IProgram& cProgram_,
									   Execution::Interface::IIterator* pIterator_,
									   Candidate::AdoptArgument& cArgument_);
		virtual void createUndoUpdate(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_);

		virtual bool checkPartitionBy(Opt::Environment& cEnvironment,
									  Order::Specification* pSpecification_);
		virtual bool checkLimit(Opt::Environment& cEnvironment_,
								const AccessPlan::Limit& cLimit_);

		virtual bool isUseUndoExpunge(Opt::Environment& cEnvironment_);
		virtual bool isUseUndoUpdate(Opt::Environment& cEnvironment_);
		virtual bool isUseOperation();

		virtual Execution::Interface::IIterator*
					addBitSetToRowIDFilter(Opt::Environment& cEnvironment_,
										   Execution::Interface::IProgram& cProgram_,
										   Execution::Interface::IIterator* pIterator_,
										   Candidate::AdoptArgument& cArgument_,
										   int iInDataID_);
		virtual void generatePutField(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_);

		virtual Predicate::ChosenInterface* getPredicateInterface();

	protected:
	private:
		//////////////////////////////
		// create actions
		//////////////////////////////

		// create check by index (getbybitset)
		int createCheckByBitSet(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Execution::Action::FileAccess* pFileAccess_,
								Candidate::AdoptArgument& cArgument_,
								Candidate::AdoptIndexArgument& cIndexArgument_);

		// create check by index (not getbybitset)
		int createCheckByCollection(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Execution::Action::FileAccess* pFileAccess_,
									Candidate::AdoptArgument& cArgument_,
									Candidate::AdoptIndexArgument& cIndexArgument_);

		// create lock action
		void addLock(Opt::Environment& cEnvironment_,
					 Execution::Interface::IProgram& cProgram_,
					 Execution::Action::FileAccess* pFileAccess_,
					 Candidate::AdoptArgument& cArgument_,
					 int iDataID_,
					 bool bForceCollection_);

		// create skip check
		int createSkipCheck(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_);

		/////////////////////////////
		// create data
		/////////////////////////////

		// sort fields by position
		void sortField(const Utility::FieldSet& cFieldSet_,
					   VECTOR<Scalar::Field*>& vecResult_);

		// create retrieving fields
		int generateField(Opt::Environment& cEnvironment_,
						  Execution::Interface::IProgram& cProgram_,
						  Execution::Interface::IIterator* pIterator_,
						  Candidate::AdoptArgument& cArgument_,
						  const VECTOR<Scalar::Field*>& vecField_);

		/////////////////////////////
		// set flags to openoption
		/////////////////////////////

		// is batch mode?
		bool isBatchMode(Opt::Environment& cEnvironment_,
						 Candidate::AdoptArgument& cArgument_);
	
		// set getbybitset parameter
		void setGetByBitSet(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							LogicalFile::OpenOption& cOpenOption_);
		// set searchbybitset parameter
		void setSearchByBitSet(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   int iPrevBitSet_,
							   Execution::Action::FileAccess* pFileAccess_);
		// set rankbybitset parameter
		void setRankByBitSet(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 int iBitSet_,
							 Execution::Action::FileAccess* pFileAccess_);
		// set order
		bool setOrderByRowID(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 LogicalFile::OpenOption& cOpenOption_);

		// set projection parameter
		int setProjection(Opt::Environment& cEnvironment_,
						  Execution::Interface::IProgram& cProgram_,
						  Execution::Interface::IIterator* pIterator_,
						  Candidate::AdoptArgument& cArgument_,
						  LogicalFile::OpenOption& cOpenOption_);

		// set update parameter
		void setUpdate(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_,
					   const VECTOR<Scalar::Field*>& vecField_,
					   LogicalFile::OpenOption& cOpenOption_);
		// add insert action
		void addInsert(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_,
					   Execution::Action::FileAccess* pFileAccess_);
		// add update action
		void addUpdate(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_,
					   Execution::Action::FileAccess* pFileAccess_);
		// add delete action
		void addExpunge(Opt::Environment& cEnvironment_,
						Execution::Interface::IProgram& cProgram_,
						Execution::Interface::IIterator* pIterator_,
						Candidate::AdoptArgument& cArgument_,
						Execution::Action::FileAccess* pFileAccess_);
		// add undo update action
		void addUndoUpdate(Opt::Environment& cEnvironment_,
						   Execution::Interface::IProgram& cProgram_,
						   Execution::Interface::IIterator* pIterator_,
						   Candidate::AdoptArgument& cArgument_,
						   Execution::Action::FileAccess* pFileAccess_);
		// add undo expunge action
		void addUndoExpunge(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_,
							Execution::Action::FileAccess* pFileAccess_);
		// add operation update action
		void addOperationUpdate(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_,
								Execution::Action::FileAccess* pFileAccess_);

		/////////////////////////////
		// others
		/////////////////////////////

		// accessor
		const Utility::FieldSet& getField() {return m_cField;}

		/////////////////////////////
		// for undolog
		/////////////////////////////

		// undolog needed?
		bool isNeedLog(Opt::Environment& cEnvironment_);
		// use undoExpunge/undoUpdate?
		bool useUndo();

		struct Insert
		{
			static void addUndoLog(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Execution::Interface::IIterator* pIterator_,
								   File* pFile_,
								   const VECTOR<Scalar::Field*>& vecPutKey_,
								   Candidate::AdoptArgument& cArgument_);
		};
		struct Expunge
		{
			static void addUndoLog(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Execution::Interface::IIterator* pIterator_,
								   File* pFile_,
								   const VECTOR<Scalar::Field*>& vecField_,
								   Candidate::AdoptArgument& cArgument_);
		};
		struct Update
		{
			static void addUndoLog(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Execution::Interface::IIterator* pIterator_,
								   File* pFile_,
								   const VECTOR<Scalar::Field*>& vecPutKey_,
								   const VECTOR<Scalar::Field*>& vecField_,
								   Candidate::AdoptArgument& cArgument_);
		};
		struct UndoExpunge
		{
			static void addUndoLog(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Execution::Interface::IIterator* pIterator_,
								   File* pFile_,
								   const VECTOR<Scalar::Field*>& vecPutKey_,
								   Candidate::AdoptArgument& cArgument_);
		};
		struct UndoUpdate
		{
			static void addUndoLog(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Execution::Interface::IIterator* pIterator_,
								   File* pFile_,
								   const VECTOR<Scalar::Field*>& vecPutKey_,
								   Candidate::AdoptArgument& cArgument_);
		};

		typedef MAP< Execution::Interface::IIterator*,
					 Execution::Operator::FileFetch*,
					 LESS<Execution::Interface::IIterator*> > FileFetchMap;

		// set during checking candidate
		bool m_bGetByBitSet;
		bool m_bSearchByBitSet;

		int m_iRankBitSetID;

		// modified during creating access plan
		Utility::FieldSet m_cKey;			// put key fields
		Utility::FieldSet m_cField;			// retrieved/put fields
		Utility::FieldSet m_cInsertField;	// fields for insert
		Utility::FieldSet m_cUndoField;		// fields for undo expunge
		FileFetchMap m_mapFetch;			// fetching iterator cache

		// for explain
		Opt::ExplainFileArgument m_cExplainArgument;
	};

	////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::FileImpl::Variable -- file information in access plan
	//
	// NOTES
	//	This class is NOT a subclass of Interface::ICandidate

	class Variable
		: public Candidate::File
	{
	public:
		typedef Variable This;
		typedef Candidate::File Super;

		Variable(Candidate::Table* pTable_,
				 Interface::IFile* pFile_,
				 Interface::IScalar* pVariable_)
			: Super(pTable_, pFile_, 0),
			  m_pVariable(pVariable_),
			  m_pSessionVariable(0),
			  m_cCost()
		{}
		~Variable() {}

	//////////////////////////////
	// Candidate::File::

		virtual void setIsSimple();
		virtual void clearBitSetFlag();

		virtual bool check(Opt::Environment& cEnvironment_,
						   File::CheckArgument& cArgument_,
						   boost::function<bool(LogicalFile::AutoLogicalFile&,
												LogicalFile::OpenOption&)> function_);
		virtual bool isAlwaysUsed();
		virtual bool isAlwaysBitSet();
		virtual bool isCheckByKey(Opt::Environment& cEnvironment_);
		virtual bool isLocatorUsed();
		virtual bool isOtherFieldUsed();

		virtual void addCheckIndexResult(Opt::Environment& cEnvironment_,
										 CheckIndexArgument& cArgument_,
										 Predicate::ChosenInterface* pPredicate_);

		virtual AccessPlan::Cost& getCost(Opt::Environment& cEnvironment_,
										  Interface::IPredicate* pPredicate_ = 0);

		virtual void addFieldForPredicate(Opt::Environment& cEnvironment_,
										  Interface::IFile* pFile_,
										  Predicate::CheckRetrievableArgument& cCheckArgument_);
		virtual void addField(Opt::Environment& cEnvironment_,
							  Scalar::Field* pField_);
		virtual void addPutKey(Opt::Environment& cEnvironment_,
							   Scalar::Field* pField_);
		virtual void addInsertField(Opt::Environment& cEnvironment_,
									Scalar::Field* pField_);
		virtual void addUndoField(Opt::Environment& cEnvironment_,
								  Scalar::Field* pField_);
		virtual void checkIndex(Opt::Environment& cEnvironment_,
								CheckIndexArgument& cArgument_);

		virtual void setForUpdate(Opt::Environment& cEnvironment_);

		virtual bool isAbleToGetByBitSet();
		virtual void disableGetByBitSet();
		virtual bool isSearchByBitSet();
		
		virtual bool isGetByBitSetAvailable();
		virtual bool isSearchByBitSetAvailable();

		virtual int getRankBitSetID();
		virtual void setRankBitSetID(int iRankBitSetID_);

		virtual bool isGetByBitSet();
		virtual bool hasOrder();
		virtual bool hasOrder(Order::Specification* pSpecification_);

		virtual bool isSearchable(Opt::Environment& cEnvironment_,
								  Tree::Node* pNode_);
		virtual bool isNeedRetrieve();
		virtual bool isRetrievable(Opt::Environment& cEnvironment_,
								   Interface::IFile* pFile_,
								   Scalar::Field* pField_);

		virtual Execution::Action::FileAccess*
					createFileAccess(Execution::Interface::IProgram& cProgram_);
		virtual int createCheckAction(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Execution::Action::FileAccess* pFileAccess_,
									  Candidate::AdoptArgument& cArgument_,
									  Candidate::AdoptIndexArgument& cIndexArgument_);

		virtual Execution::Interface::IIterator*
					createScan(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Execution::Action::FileAccess* pFileAccess_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual Execution::Interface::IIterator*
					createScanWithSearchByBitSetOption(Opt::Environment& cEnvironment_,
													   Execution::Interface::IProgram& cProgram_,
													   Execution::Action::FileAccess* pFileAccess_,
													   Candidate::AdoptArgument& cArgument_,
													   Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual void createFetch(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 Scalar::Field* pFetchKey_,
								 int iFetchKeyID_);
		virtual void createGetLocator(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_,
									  int iKeyID_);
		virtual void addUnionFileScan(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_,
									  Execution::Action::FileAccess* pFileAccess_);

		virtual void createInsert(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_);
		virtual void createExpunge(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Execution::Interface::IIterator* pIterator_,
								   Candidate::AdoptArgument& cArgument_);
		virtual void createUpdate(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_);
		virtual void createUndoExpunge(Opt::Environment& cEnvironment_,
									   Execution::Interface::IProgram& cProgram_,
									   Execution::Interface::IIterator* pIterator_,
									   Candidate::AdoptArgument& cArgument_);
		virtual void createUndoUpdate(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_);

		virtual bool checkPartitionBy(Opt::Environment& cEnvironment,
									  Order::Specification* pSpecification_);
		virtual bool checkLimit(Opt::Environment& cEnvironment_,
								const AccessPlan::Limit& cLimit_);

		virtual bool isUseUndoExpunge(Opt::Environment& cEnvironment_);
		virtual bool isUseUndoUpdate(Opt::Environment& cEnvironment_);
		virtual bool isUseOperation();

		virtual Execution::Interface::IIterator*
					addBitSetToRowIDFilter(Opt::Environment& cEnvironment_,
										   Execution::Interface::IProgram& cProgram_,
										   Execution::Interface::IIterator* pIterator_,
										   Candidate::AdoptArgument& cArgument_,
										   int iInDataID_);
		
		virtual void generatePutField(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_);

		virtual Predicate::ChosenInterface* getPredicateInterface();

	protected:
	private:
		Server::Session::BitSetVariable*
					getSessionVariable(Opt::Environment& cEnvironment_);
		void addLock(Opt::Environment& cEnvironment_,
					 Execution::Interface::IProgram& cProgram_,
					 Execution::Interface::IIterator* pIterator_,
					 Candidate::AdoptArgument& cArgument_,
					 int iDataID_);

		Interface::IScalar* m_pVariable;
		Server::Session::BitSetVariable* m_pSessionVariable;

		AccessPlan::Cost m_cCost;
	};

	////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::FileImpl::NadicBase -- file information in access plan
	//
	// NOTES
	//	This class is NOT a subclass of Interface::ICandidate

	class NadicBase
		: public Candidate::File
	{
	public:
		typedef NadicBase This;
		typedef Candidate::File Super;

		// constructor
		NadicBase(Candidate::Table* pTable_,
				  const VECTOR<Candidate::File*>& vecFiles_,
				  Order::Specification* pOrder_)
			: Super(pTable_, 0, 0),
			  m_vecFiles(vecFiles_),
			  m_pOrder(pOrder_),
			  m_cCost()
		{}
		virtual ~NadicBase() {}

	//////////////////////////////
	// Candidate::File::
		virtual void setIsSimple();
		virtual void clearBitSetFlag();

		virtual bool check(Opt::Environment& cEnvironment_,
						   File::CheckArgument& cArgument_,
						   boost::function<bool(LogicalFile::AutoLogicalFile&,
												LogicalFile::OpenOption&)> function_);
		virtual bool isAlwaysUsed();
		virtual bool isAlwaysBitSet();
		virtual bool isCheckByKey(Opt::Environment& cEnvironment_);
		virtual bool isLocatorUsed();
		virtual bool isOtherFieldUsed();
		virtual void addCheckIndexResult(Opt::Environment& cEnvironment_,
										 CheckIndexArgument& cArgument_,
										 Predicate::ChosenInterface* pPredicate_);
		virtual AccessPlan::Cost& getCost(Opt::Environment& cEnvironment_,
										  Interface::IPredicate* pPredicate_ = 0);
		virtual void addField(Opt::Environment& cEnvironment_,
							  Scalar::Field* pField_);
		virtual void addPutKey(Opt::Environment& cEnvironment_,
							   Scalar::Field* pField_);
		virtual void addInsertField(Opt::Environment& cEnvironment_,
									Scalar::Field* pField_);
		virtual void addUndoField(Opt::Environment& cEnvironment_,
								  Scalar::Field* pField_);
		virtual void checkIndex(Opt::Environment& cEnvironment_,
								CheckIndexArgument& cArgument_);

		virtual void setForUpdate(Opt::Environment& cEnvironment_);

		virtual int getRankBitSetID();
		virtual void setRankBitSetID(int iRankBitSetID_);

		virtual bool isGetByBitSet();
		virtual bool hasOrder();
		virtual bool hasOrder(Order::Specification* pSpecification_);

		virtual bool isSearchable(Opt::Environment& cEnvironment_,
								  Tree::Node* pNode_);
		virtual bool isNeedRetrieve();
		virtual bool isRetrievable(Opt::Environment& cEnvironment_,
								   Interface::IFile* pFile_,
								   Scalar::Field* pField_);

		virtual Execution::Action::FileAccess*
					createFileAccess(Execution::Interface::IProgram& cProgram_);
		virtual int createCheckAction(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Execution::Action::FileAccess* pFileAccess_,
									  Candidate::AdoptArgument& cArgument_,
									  Candidate::AdoptIndexArgument& cIndexArgument_);

		virtual Execution::Interface::IIterator*
					createScanWithSearchByBitSetOption(Opt::Environment& cEnvironment_,
													   Execution::Interface::IProgram& cProgram_,
													   Execution::Action::FileAccess* pFileAccess_,
													   Candidate::AdoptArgument& cArgument_,
													   Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual void createFetch(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 Scalar::Field* pFetchKey_,
								 int iFetchKeyID_);
		virtual void createGetLocator(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_,
									  int iKeyID_);
		virtual void addUnionFileScan(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_,
									  Execution::Action::FileAccess* pFileAccess_);

		virtual void createInsert(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_);
		virtual void createExpunge(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Execution::Interface::IIterator* pIterator_,
								   Candidate::AdoptArgument& cArgument_);
		virtual void createUpdate(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_);
		virtual void createUndoExpunge(Opt::Environment& cEnvironment_,
									   Execution::Interface::IProgram& cProgram_,
									   Execution::Interface::IIterator* pIterator_,
									   Candidate::AdoptArgument& cArgument_);
		virtual void createUndoUpdate(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_);

		virtual bool checkPartitionBy(Opt::Environment& cEnvironment,
									  Order::Specification* pSpecification_);

		virtual bool isUseUndoExpunge(Opt::Environment& cEnvironment_);
		virtual bool isUseUndoUpdate(Opt::Environment& cEnvironment_);
		virtual bool isUseOperation();

		virtual Execution::Interface::IIterator*
					addBitSetToRowIDFilter(Opt::Environment& cEnvironment_,
										   Execution::Interface::IProgram& cProgram_,
										   Execution::Interface::IIterator* pIterator_,
										   Candidate::AdoptArgument& cArgument_,
										   int iInDataID_);
		
		virtual void generatePutField(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_);

	//	virtual Predicate::ChosenInterface* getPredicateInterface();

	protected:
		// accessor
		const VECTOR<Candidate::File*>& getFiles() {return m_vecFiles;}
		const Utility::FieldSet& getFields() {return m_cField;}
		Order::Specification* getOrder() {return m_pOrder;}

		Execution::Interface::IIterator*
					createOperandScan(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Candidate::AdoptArgument& cArgument_,
									  Candidate::AdoptIndexArgument& cIndexArgument_,
									  Interface::IPredicate* pPredicate_);
		void addMergeOperand(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pOperand_,
							 Candidate::AdoptArgument& cArgument_,
							 Execution::Interface::IIterator* pIterator_,
							 const VECTOR<Interface::IScalar*>& vecTuple_);

	private:
		VECTOR<Candidate::File*> m_vecFiles;
		AccessPlan::Cost m_cCost;

		Order::Specification* m_pOrder;
		Utility::FieldSet m_cField;			// retrieved fields
	};

	////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::FileImpl::Intersect -- file information in access plan
	//
	// NOTES
	//	This class is NOT a subclass of Interface::ICandidate

	class Intersect
		: public NadicBase,
		  virtual public Predicate::ChosenImpl::AndImpl
	{
	public:
		typedef Intersect This;
		typedef NadicBase Super1;
		typedef Predicate::ChosenImpl::AndImpl Super2;

		Intersect(Candidate::Table* pTable_,
				  const VECTOR<Candidate::File*>& vecFiles_,
				  Interface::IPredicate* pPredicate_,
				  const VECTOR<Interface::IPredicate*>& vecPredicates_,
				  Order::Specification* pOrder_)
			: Super1(pTable_, vecFiles_, pOrder_),
			  Super2(pPredicate_, vecPredicates_)
		{}
		~Intersect() {}

	//////////////////////////////
	// Candidate::File::
		virtual void addFieldForPredicate(Opt::Environment& cEnvironment_,
										  Interface::IFile* pFile_,
										  Predicate::CheckRetrievableArgument& cCheckArgument_);

		virtual bool isAbleToGetByBitSet();
		virtual void disableGetByBitSet();
		virtual bool isSearchByBitSet();
		
		virtual bool isGetByBitSetAvailable();
		virtual bool isSearchByBitSetAvailable();

		virtual Execution::Interface::IIterator*
					createScan(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Execution::Action::FileAccess* pFileAccess_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_);
		virtual bool checkLimit(Opt::Environment& cEnvironment_,
								const AccessPlan::Limit& cLimit_);

		virtual Predicate::ChosenInterface* getPredicateInterface();

	//////////////////////////////////////////
	// Predicate::ChosenImpl::AndImpl::
		virtual bool isCombinedFile()
		{return true;}

	protected:
	private:
		void setOperandLimit(Opt::Environment& cEnvironment_);

		AccessPlan::Limit m_cLimit;			// specified limit
	};

	////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::FileImpl::Union -- file information in access plan
	//
	// NOTES
	//	This class is NOT a subclass of Interface::ICandidate

	class Union
		: public NadicBase,
		  virtual public Predicate::ChosenImpl::OrImpl
	{
	public:
		typedef Union This;
		typedef NadicBase Super1;
		typedef Predicate::ChosenImpl::OrImpl Super2;

		Union(Candidate::Table* pTable_,
			  const VECTOR<Candidate::File*>& vecFiles_,
			  Interface::IPredicate* pPredicate_,
			  const VECTOR<Interface::IPredicate*>& vecPredicates_,
			  Order::Specification* pOrder_)
			: Super1(pTable_, vecFiles_, pOrder_),
			  Super2(pPredicate_, vecPredicates_)
		{}
		~Union() {}

	//////////////////////////////
	// Candidate::File::
		virtual void addFieldForPredicate(Opt::Environment& cEnvironment_,
										  Interface::IFile* pFile_,
										  Predicate::CheckRetrievableArgument& cCheckArgument_);

		virtual bool isAbleToGetByBitSet();
		virtual void disableGetByBitSet();
		virtual bool isSearchByBitSet();

		virtual bool isGetByBitSetAvailable();
		virtual bool isSearchByBitSetAvailable();

		virtual Execution::Interface::IIterator*
					createScan(Opt::Environment& cEnvironment_,
							   Execution::Interface::IProgram& cProgram_,
							   Execution::Action::FileAccess* pFileAccess_,
							   Candidate::AdoptArgument& cArgument_,
							   Candidate::AdoptIndexArgument& cIndexArgument_);

		virtual bool checkLimit(Opt::Environment& cEnvironment_,
								const AccessPlan::Limit& cLimit_);

		virtual Predicate::ChosenInterface* getPredicateInterface();

	//////////////////////////////////////////
	// Predicate::ChosenImpl::OrImpl::
		virtual bool isCombinedFile()
		{return true;}

	protected:
	private:
		void setOperandLimit(Opt::Environment& cEnvironment_);

		AccessPlan::Limit m_cLimit;			// specified limit
	};

} // namespace FileImpl

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_IMPL_FILEIMPL_H

//
//	Copyright (c) 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
