// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/TableImpl.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2015, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_CANDIDATE_IMPL_TABLEIMPL_H
#define __SYDNEY_PLAN_CANDIDATE_IMPL_TABLEIMPL_H

#include "boost/function.hpp"

#include "Plan/Candidate/Table.h"

#include "Plan/Declaration.h"
#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Monadic.h"
#include "Plan/File/Parameter.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Value.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Constraint;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

namespace TableImpl
{
	typedef MAP< Interface::IFile*,
				 Candidate::File*,
				 Utility::FileSet::Comparator> FileCandidateMap;

	typedef MAP< Scalar::Field*,
				 Interface::IFile*,
				 Utility::FieldSet::Comparator> FieldRetrieveMap;

	typedef MAP< Interface::IFile*,
				 Scalar::Field*,
				 Utility::FileSet::Comparator> FileFieldMap;

	typedef MAP< Scalar::Field*,
				 Interface::IScalar*,
				 Utility::FieldSet::Comparator> FieldScalarMap;

	typedef MAP< Interface::IFile*,
				 VECTOR<Interface::IFile*>,
				 Utility::FileSet::Comparator> FileDependentMap;

	// STRUCT
	//	Plan::Candidate::TableImpl::FileCandidateInfo --
	//
	// NOTES
	struct FileCandidateInfo
	{
		FileCandidateMap m_mapFileCandidate;	// file -> candidate

		// destructor
		virtual ~FileCandidateInfo() {}

		// add file->candidate map
		void addCandidate(Candidate::File* pCandidateFile_)
		{
			m_mapFileCandidate[pCandidateFile_->getFile()] = pCandidateFile_;
		}
		// get candidate from map
		Candidate::File* getCandidate(Interface::IFile* pFile_)
		{
			return m_mapFileCandidate[pFile_];
		}
	};

	// STRUCT
	//	Plan::Candidate::TableImpl::RetrieveInfo --
	//
	// NOTES
	struct RetrieveInfo
		: public FileCandidateInfo
	{
	/////////////////////////
	// FileCandidateInfo::
	//	FileCandidateMap m_mapFileCandidate;	// file -> candidate
		Utility::FileSet m_cRequiredFile;		// required files for retrieving
		Utility::FileSet m_cMayRequiredFile;	// intersections of all candidate files
		FieldRetrieveMap m_mapFieldRetrieve;	// field -> chosen file
		FieldScalarMap m_mapAlternativeValue;	// field -> alternative value map
		FileFieldMap m_mapFetchKey;				// file -> fetch key
		VECTOR<Scalar::Field*> m_vecFetchKey;	// set of value of mapFetchKey
		bool m_bDelayed;
		bool m_bChanged;

		// clear contents
		void clear()
		{
			m_mapFileCandidate.clear();

			m_cRequiredFile.clear();
			m_cMayRequiredFile.clear();
			m_mapFieldRetrieve.clear();
			m_mapAlternativeValue.clear();
			m_mapFetchKey.clear();
			m_vecFetchKey.clear();
			m_bDelayed = false;
			m_bChanged = false;
		}
	};

	// STRUCT
	//	Plan::Candidate::TableImpl::PutInfo --
	//
	// NOTES
	struct PutInfo
		: public FileCandidateInfo
	{
	/////////////////////////
	// FileCandidateInfo::
	//	FileCandidateMap m_mapFileCandidate;	// file -> candidate
		FileDependentMap m_mapDependent;		// file -> source map
		Utility::FileSet m_cUndoFile;			// set of undo files

		// constructor
		PutInfo() {}

		// add undo file
		void addUndoFile(Interface::IFile* pFile_)
		{
			m_cUndoFile.add(pFile_);
		}
		// remove undo file
		void removeUndoFile(Interface::IFile* pFile_)
		{
			m_cUndoFile.remove(pFile_);
		}
		// is undo file?
		bool isUndoFile(Interface::IFile* pFile_)
		{
			return m_cUndoFile.isContaining(pFile_);
		}
	};

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::TableImpl::Retrieve -- implementation class of Interface::ICandidate for table
	//
	// NOTES
	class Retrieve
		: public Candidate::Table
	{
	public:
		typedef Retrieve This;
		typedef Candidate::Table Super;

		// constructor
		Retrieve(Relation::Table* pTable_)
			: Super(pTable_),
			  m_cRetrievedColumn(),
			  m_cRequiredColumn(),
			  m_cDelayedColumn(),
			  m_bRowCreated(false),
			  m_cScanCost(),
			  m_pScanFile(0),
			  m_pScanFileCache(0),
			  m_cRetrieve(),
			  m_pIterator(0)
		{}

		// destructor
		virtual ~Retrieve() {}

	/////////////////////////////
	// Interface::ICandidate::
	//	virtual void createCost(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Source& cPlanSource_);
	//	virtual const AccessPlan::Cost& getCost();

	//	virtual Candidate::Row* getRow(Opt::Environment& cEnvironment_);

		virtual void require(Opt::Environment& cEnvironment_,
							 Scalar::Field* pField_);
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Scalar::Field* pField_);
		virtual void use(Opt::Environment& cEnvironment_,
						 Scalar::Field* pField_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Scalar::Field* pField_,
						   Scalar::DelayArgument& cArgument_);

		bool isGetByBitSetRowID();

	//	virtual bool isReferingRelation(Interface::IRelation* pRelation_);

		virtual void checkPredicate(Opt::Environment& cEnvironment_,
									AccessPlan::Source& cPlanSource_);
		virtual bool isLimitAvailable(Opt::Environment& cEnvironment_);

		virtual Execution::Interface::IIterator*
						adopt(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Candidate::AdoptArgument& cArgument_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void generateDelayed(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_);
		
		virtual Scalar::Field* getUsedField(Opt::Environment& cEnvironment_,
											Scalar::Field* pField_);

	//////////////////////////
	// Candidate::Table::
		virtual void createPlan(Opt::Environment& cEnvironment_,
								AccessPlan::Source& cPlanSource_,
								const Utility::FieldSet& cFieldSet_);
		virtual bool isDelayed(Scalar::Field* pField_);
		virtual bool addFileFetch(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Scalar::Field* pField_,
								  Candidate::AdoptArgument& cArgument_);
		virtual void addConstraint(Opt::Environment& cEnvironment_,
								   Schema::Constraint* pSchemaConstraint_);
		virtual const VECTOR<Schema::Constraint*>& getConstraint();

		virtual void addRetrieveField(Opt::Environment& cEnvironment_,
									  Candidate::File* pCandidateFile_,
									  Interface::IFile* pFile_,
									  Scalar::Field* pField_);
		virtual AccessPlan::Cost::Value
					getEstimateCount(Opt::Environment& cEnvironment_);
	protected:
		// accessor
		const Utility::FieldSet& getRetrievedColumn() {return m_cRetrievedColumn;}
		// set retrieved columns
		void setRetrievedColumn(Opt::Environment& cEnvironment_,
								const Utility::FieldSet& cFieldSet_);

	private:
		// enumerate possible usage of index
		virtual void enumerate(Opt::Environment& cEnvironment_,
							   AccessPlan::Source& cPlanSource_);
		// choose one possible usage
		virtual void choose(Opt::Environment& cEnvironment_,
							AccessPlan::Source& cPlanSource_);
		// check file for sorting order
		bool checkOrder(Opt::Environment& cEnvironment_,
						Order::Specification* pOrder_,
						const Order::CheckArgument& cArgument_);
		// check file for limit
		void checkLimit(Opt::Environment& cEnvironment_,
						const AccessPlan::Limit& cLimit_);

		// choose index file for predicate
		void choosePredicate(Opt::Environment& cEnvironment_,
							 AccessPlan::Source& cPlanSource_);
		// choose file for sorting order
		void chooseOrder(Opt::Environment& cEnvironment_,
						 AccessPlan::Source& cPlanSource_);
		
		// choose retrieved files for each retrieved columns
		void chooseRetrieve(Opt::Environment& cEnvironment_);
		// choose retrieved files for each retrieved columns
		void chooseRetrieveFile(Opt::Environment& cEnvironment_,
								Utility::FieldSet& cTargetColumn_,
								RetrieveInfo& cRetrieve_);
		// check retrieve file for a field
		bool checkRetrieveFile(Opt::Environment& cEnvironment_,
							   Scalar::Field* pField_,
							   RetrieveInfo& cRetrieve_);
		// create alternative value if exists
		bool createAlternativeValue(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Scalar::Field* pField_,
									Candidate::AdoptArgument& cArgument_,
									RetrieveInfo& cRetrieve_);
		// check alternative value in union
		bool checkAlternativeValue(Opt::Environment& cEnvironment_,
								   Scalar::Field* pField_,
								   RetrieveInfo& cRetrieve_);
		// check a file can be checked
		bool checkFetchFile(Opt::Environment& cEnvironment_,
							Interface::IFile* pFile_,
							Scalar::Field* pField_,
							RetrieveInfo& cRetrieve_);
		// choose retrieved file for one column
		void chooseFile(Opt::Environment& cEnvironment_,
						Scalar::Field* pField_,
						RetrieveInfo& cRetrieve_);
		// check a file can be scanned
		bool checkScanFile(Opt::Environment& cEnvironment_,
						   Interface::IFile* pFile_,
						   Plan::File::CheckArgument& cArgument_);

		// add retrieved fields
		void addField(Opt::Environment& cEnvironment_,
					  Interface::IFile* pFile_,
					  Scalar::Field* pField_,
					  RetrieveInfo& cRetrieve_);

		// add retrieved fields to one file
		void addRetrieveField(Opt::Environment& cEnvironment_,
							  Candidate::File* pCandidateFile_,
							  Interface::IFile* pFile_,
							  Scalar::Field* pField_,
							  RetrieveInfo& cRetrieve_);

		// choose scannable file
		Candidate::File* chooseScan(Opt::Environment& cEnvironment_);

		// calculate scan cost
		const AccessPlan::Cost& calculateScanCost(Opt::Environment& cEnvironment_);
		// calculate repeat count
		AccessPlan::Cost::Value calculateRepeatCount(Opt::Environment& cEnvironment_,
													 AccessPlan::Source& cPlanSource_);

		// add predicate cost for scanning plan
		void addPredicateCost(Opt::Environment& cEnvironment_,
							  AccessPlan::Cost& cCost_);

		// create file fetch action for a column
		bool createFileFetch(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 Interface::IFile* pFile_,
									 Scalar::Field* pField_,
									 Candidate::AdoptArgument& cArgument_,
									 RetrieveInfo& cRetrieve_);

		// check a file can be retrieved
		virtual bool isRetrievableFile(Opt::Environment& cEnvironment_,
									   Interface::IFile* pFile_,
									   Predicate::CheckRetrievableArgument& cCheckArgument_,
									   RetrieveInfo& cRetrieve_);
		// create start iterator
		virtual Execution::Interface::IIterator*
					adoptStartFile(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Candidate::AdoptArgument& cArgument_,
								   Utility::FieldSet& cTargetColumn_,
								   RetrieveInfo& cRetrieve_);

		Execution::Interface::IIterator*
		addNarrowByBitSetAction(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Candidate::AdoptArgument& cArgument_);

	/////////////////////////
	// Candidate::Base::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const AccessPlan::Source& cPlanSource_,
								AccessPlan::Cost& cCost_);
		virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_);
		virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_);
		virtual int createCheckPredicate(Opt::Environment& cEnvironment_,
										 Execution::Interface::IProgram& cProgram_,
										 Execution::Interface::IIterator* pIterator_,
										 Candidate::AdoptArgument& cArgument_,
										 Interface::IPredicate* pPredicate_);
		virtual void generateContinue(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_);

	//////////////////////
	// private

		// adopt start file using scanfile
		Execution::Interface::IIterator*
					adoptByScan(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Candidate::AdoptArgument& cArgument_,
								Utility::FieldSet& cTargetColumn_,
								RetrieveInfo& cRetrieve_);
		// adopt start file using predicate
		Execution::Interface::IIterator*
					adoptByPredicate(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Candidate::AdoptArgument& cArgument_,
									 Utility::FieldSet& cTargetColumn_,
									 RetrieveInfo& cRetrieve_);

		Utility::FieldSet m_cRetrievedColumn; // retrieved columns of the target table
		Utility::FieldSet m_cRequiredColumn; // retrieved columns of the target table
		Utility::FieldSet m_cDelayedColumn;	// delayable columns of the target table
		bool m_bRowCreated;
		AccessPlan::Cost m_cScanCost;		// estimated cost (scan)
		Candidate::File* m_pScanFile;		// start file for scanning plan
		Candidate::File* m_pScanFileCache;	// start file for scanning plan
		RetrieveInfo m_cRetrieve;
		Execution::Interface::IIterator* m_pIterator; // result of adopt
	}; // class Retrieve

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::TableImpl::Virtual --
	//			implementation class of Interface::ICandidate for virtual table
	//
	// NOTES
	class Virtual
		: public Retrieve
	{
	public:
		typedef Virtual This;
		typedef Retrieve Super;

		// constructor
		Virtual(Relation::Table* pTable_)
			: Super(pTable_)
		{}

		// destructor
		virtual ~Virtual() {}

	/////////////////////////////
	// Interface::ICandidate::
	//	virtual void createCost(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Source& cPlanSource_);
	//	virtual const AccessPlan::Cost& getCost();

	//	virtual Candidate::Row* getRow(Opt::Environment& cEnvironment_);

		virtual void require(Opt::Environment& cEnvironment_,
							 Scalar::Field* pField_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_,
	//						  Scalar::Field* pField_);
		virtual void use(Opt::Environment& cEnvironment_,
						 Scalar::Field* pField_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Scalar::Field* pField_,
						   Scalar::DelayArgument& cArgument_);

	//	virtual bool isReferingRelation(Interface::IRelation* pRelation_);

		virtual Execution::Interface::IIterator*
						adopt(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Candidate::AdoptArgument& cArgument_);

	//////////////////////////
	// Candidate::Table::
	//	virtual void createPlan(Opt::Environment& cEnvironment_,
	//							AccessPlan::Source& cPlanSource_,
	//							const Utility::FieldSet& cFieldSet_);
	protected:
	private:
		// check index file for predicate
		void checkPredicate(Opt::Environment& cEnvironment_,
							Interface::IPredicate* pPredicate_,
							const Predicate::CheckArgument& cArgument_);
		// choose index file for predicate
		void choosePredicate(Opt::Environment& cEnvironment_,
							 AccessPlan::Source& cPlanSource_);

	/////////////////////////
	// Retrieve::
		virtual void enumerate(Opt::Environment& cEnvironment_,
							   AccessPlan::Source& cPlanSource_);
		virtual void choose(Opt::Environment& cEnvironment_,
							AccessPlan::Source& cPlanSource_);

	/////////////////////////
	// Candidate::Base::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const AccessPlan::Source& cPlanSource_,
								AccessPlan::Cost& cCost_);
		virtual int createCheckPredicate(Opt::Environment& cEnvironment_,
										 Execution::Interface::IProgram& cProgram_,
										 Execution::Interface::IIterator* pIterator_,
										 Candidate::AdoptArgument& cArgument_,
										 Interface::IPredicate* pPredicate_);
		virtual void generateContinue(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  Candidate::AdoptArgument& cArgument_);
	}; // class Virtual

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::TableImpl::Refer -- implementation class of Interface::ICandidate for table
	//
	// NOTES
	class Refer
		: public Retrieve
	{
	public:
		typedef Refer This;
		typedef Retrieve Super;

		// constructor
		Refer(Relation::Table* pTable_,
			  Relation::Table* pTargetTable_)
			: Super(pTable_),
			  m_pTargetTable(pTargetTable_),
			  m_vecSchemaConstraint()
		{}

		// destructor
		~Refer() {}

	/////////////////////////////
	// Interface::ICandidate::
		virtual Execution::Interface::IIterator*
						adopt(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Candidate::AdoptArgument& cArgument_);

	//////////////////////////
	// Candidate::Table::
		virtual void createPlan(Opt::Environment& cEnvironment_,
								AccessPlan::Source& cPlanSource_,
								const Utility::FieldSet& cFieldSet_);
		virtual void addConstraint(Opt::Environment& cEnvironment_,
								   Schema::Constraint* pSchemaConstraint_);
		virtual const VECTOR<Schema::Constraint*>& getConstraint();
		virtual void addRetrieveField(Opt::Environment& cEnvironment_,
									  Candidate::File* pCandidateFile_,
									  Interface::IFile* pFile_,
									  Scalar::Field* pField_);

	protected:
	private:
		Relation::Table* m_pTargetTable;
		VECTOR<Schema::Constraint*> m_vecSchemaConstraint;
	}; // class Refer

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::TableImpl::Simple -- implementation class of Interface::ICandidate for table
	//
	// NOTES
	class Simple
		: public Retrieve
	{
	public:
		typedef Simple This;
		typedef Retrieve Super;

		// constructor
		Simple(Relation::Table* pTable_)
			: Super(pTable_)
		{}

		// destructor
		~Simple() {}

	//////////////////////////
	// Candidate::Table::
		virtual void require(Opt::Environment& cEnvironment_,
							 Scalar::Field* pField_);
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Scalar::Field* pField_);
		virtual void use(Opt::Environment& cEnvironment_,
						 Scalar::Field* pField_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Scalar::Field* pField_,
						   Scalar::DelayArgument& cArgument_);

	protected:
	private:
	//////////////////////////
	// TableImpl::Retrieve::
		virtual bool isRetrievableFile(Opt::Environment& cEnvironment_,
									   Interface::IFile* pFile_,
									   Predicate::CheckRetrievableArgument& cCheckArgument_,
									   RetrieveInfo& cRetrieve_);
		virtual Execution::Interface::IIterator*
					adoptStartFile(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Candidate::AdoptArgument& cArgument_,
								   Utility::FieldSet& cTargetColumn_,
								   RetrieveInfo& cRetrieve_);
	}; // class Simple

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::TableImpl::Put -- implementation class of Interface::ICandidate for table
	//
	// NOTES
	class Put
		: public Monadic<Candidate::Table>
	{
	public:
		typedef Monadic<Candidate::Table> Super;
		typedef Put This;

		// destructor
		virtual ~Put() {}

	/////////////////////////////
	// Interface::ICandidate::
	//	virtual void createCost(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Source& cPlanSource_);
	//	virtual const AccessPlan::Cost& getCost();

		virtual void require(Opt::Environment& cEnvironment_,
							 Scalar::Field* pField_);
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Scalar::Field* pField_);
		virtual void use(Opt::Environment& cEnvironment_,
						 Scalar::Field* pField_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Scalar::Field* pField_,
						   Scalar::DelayArgument& cArgument_)
		{return false;}

		virtual Execution::Interface::IIterator*
						adopt(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Candidate::AdoptArgument& cArgument_);

		virtual void generateDelayed(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_)
		{; /* do nothing */}

	//////////////////////////
	// Candidate::Table::
	//	virtual void createPlan(Opt::Environment& cEnvironment_,
	//							AccessPlan::Source& cPlanSource_,
	//							const Utility::FieldSet& cFieldSet_);
		virtual bool isDelayed(Scalar::Field* pField_)
		{return false;}
		virtual bool addFileFetch(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Scalar::Field* pField_,
								  Candidate::AdoptArgument& cArgument_);
		virtual void addConstraint(Opt::Environment& cEnvironment_,
								   Schema::Constraint* pSchemaConstraint_);
		virtual const VECTOR<Schema::Constraint*>& getConstraint();
		virtual void addRetrieveField(Opt::Environment& cEnvironment_,
									  Candidate::File* pCandidateFile_,
									  Interface::IFile* pFile_,
									  Scalar::Field* pField_);
		virtual AccessPlan::Cost::Value
					getEstimateCount(Opt::Environment& cEnvironment_);
	protected:
		// constructor
		Put(Relation::Table* pTable_,
			Interface::ICandidate* pOperand_,
			const VECTOR<Scalar::Field*>& vecLogged_,
			const VECTOR<Candidate::Table*>& vecReferenceBefore_,
			const VECTOR<Candidate::Table*>& vecReferenceAfter_)
			: Super(pTable_, pOperand_),
			  m_vecLogged(vecLogged_),
			  m_vecReferenceBefore(vecReferenceBefore_),
			  m_vecReferenceAfter(vecReferenceAfter_),
			  m_cPut()
		{}

		// create target column
		void createTargetColumn(Opt::Environment& cEnvironment_,
								const Utility::FieldSet& cFieldSet_);

		// check undoability
		void checkUndo(Opt::Environment& cEnvironment_,
					   Interface::IFile* pFile_,
					   Scalar::Field* pField_,
					   const Utility::UIntSet& cLogRequiredPosition_);

		typedef int ParallelSplit;
		// order candidate::file according to insertion order
		void orderCandidate(Interface::IFile* pFile_,
							FileCandidateMap& cMap_,
							VECTOR<Candidate::File*>& vecCandidate_,
							VECTOR<ParallelSplit>* pvecParallel_);
		// generate parallel plan
		void generateParallel(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Execution::Interface::IIterator* pIterator_,
							  Candidate::AdoptArgument& cArgument_,
							  VECTOR<Candidate::File*>& vecCandidate_,
							  VECTOR<ParallelSplit>& vecSplit_,
							  boost::function<void(Candidate::File*,
												   Candidate::AdoptArgument&)> function_);
		// add object lock action
		void generateObjectLock(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_);

		// get generated objectID if exists
		Scalar::Field* getGeneratedObjectID(Opt::Environment& cEnvironment_,
											Interface::IFile* pFile_);
		// get target columns
		const VECTOR<Scalar::Field*>& getTargetColumn() {return m_vecTarget;}
		// get logged columns
		const VECTOR<Scalar::Field*>& getLogged() {return m_vecLogged;}
		// get putinfo
		PutInfo& getPutInfo() {return m_cPut;}

		// common implementation for generate
		Execution::Interface::IIterator*
					generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 boost::function<void(Candidate::File*,
													  Candidate::AdoptArgument&)> function_);

		// adopt operand relation
		virtual Execution::Interface::IIterator*
						adoptInput(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Candidate::AdoptArgument& cArgument_);

	private:
		// add storing operation
		Execution::Interface::IIterator*
						addStoring(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Candidate::AdoptArgument& cArgument_);
		// insert start batch actions to startup
		void generateStartBatch(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_);
		// add check constraint action
		void generateCheckConstraint(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 Candidate::AdoptArgument& cArgument_,
									 bool bBefore_);
		// add check column constraint action
		void generateCheckColumnConstraint(Opt::Environment& cEnvironment_,
										   Execution::Interface::IProgram& cProgram_,
										   Execution::Interface::IIterator* pIterator_,
										   Candidate::AdoptArgument& cArgument_,
										   Scalar::Field* pColumn_);
		// add check table constraint actions for one refered table
		void generateCheckTableConstraints(Opt::Environment& cEnvironment_,
										   Execution::Interface::IProgram& cProgram_,
										   Execution::Interface::IIterator* pIterator_,
										   Candidate::AdoptArgument& cArgument_,
										   Candidate::Table* pReference_);
		// add check table constraint action
		void generateCheckTableConstraint(Opt::Environment& cEnvironment_,
										  Execution::Interface::IProgram& cProgram_,
										  Execution::Interface::IIterator* pIterator_,
										  Candidate::AdoptArgument& cArgument_,
										  Candidate::Table* pReference_,
										  Schema::Constraint* pSchemaConstraint_);
		// generate file fetch action for a constraint
		Execution::Interface::IIterator*
						generateFileFetch(Opt::Environment& cEnvironment_,
										  Execution::Interface::IProgram& cProgram_,
										  Execution::Interface::IIterator* pIterator_,
										  Schema::Index* pSchemaIndex_,
										  Candidate::Table* pReference_,
										  const VECTOR<Interface::IScalar*>& vecCheckValue_,
										  bool bForLock_ = false);
		// get checked value for constraint
		Interface::IScalar* getCheckValue(Opt::Environment& cEnvironment_,
										  Schema::Constraint* pSchemaConstraint_,
										  Schema::Column* pSchemaColumn_);
		// generate put field
		void generatePutField(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Execution::Interface::IIterator* pIterator_,
							  Candidate::AdoptArgument& cArgument_);

		// check whether parallel execution can be considered
		bool isCanParallel(Opt::Environment& cEnvironment_);

		// add object lock action for one index
		void generateObjectLockForIndex(Opt::Environment& cEnvironment_,
										Execution::Interface::IProgram& cProgram_,
										Execution::Interface::IIterator* pIterator_,
										Candidate::AdoptArgument& cArgument_,
										Schema::Index* pSchemaIndex_);

		// add lock action
		virtual void generateLock(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_);

		// add log action
		virtual void generateLog(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_);
		virtual int generateLogType(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_);
		virtual void generateResetUndoLog(Opt::Environment& cEnvironment_,
										  Execution::Interface::IProgram& cProgram_,
										  Execution::Interface::IIterator* pIterator_);
		bool isNeedUndoLog(Opt::Environment& cEnvironment_);

		// get checked field
		virtual Interface::IScalar* getField(Opt::Environment& cEnvironment_,
											 Schema::Column* pSchemaColumn_);

		// check whether storing is needed
		virtual bool isStoringNeeded(Opt::Environment& cEnvironment_) = 0;
		// is checking target data constraint needed?
		virtual bool isCheckColumnConstraint(Opt::Environment& cEnvironment_) = 0;
		// add operation to iterator
		virtual Execution::Interface::IIterator*
					generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_) = 0;
		// get original value for a column
		virtual Interface::IScalar* getOriginalField(Opt::Environment& cEnvironment_,
													 Scalar::Field* pField_) = 0;

	////////////////////////////
	// Candidate::Base::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const AccessPlan::Source& cPlanSource_,
								AccessPlan::Cost& cCost_);

		VECTOR<Scalar::Field*> m_vecTarget;
		VECTOR<Scalar::Field*> m_vecLogged;
		VECTOR<Candidate::Table*> m_vecReferenceBefore;
		VECTOR<Candidate::Table*> m_vecReferenceAfter;
		PutInfo m_cPut;
	}; // class Put

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::TableImpl::Insert -- implementation class of Interface::ICandidate for table
	//
	// NOTES
	class Insert
		: public Put
	{
	public:
		typedef Put Super;
		typedef Insert This;

		// constructor
		Insert(Relation::Table* pTable_,
			   const VECTOR<Scalar::Field*>& vecLogged_,
			   const VECTOR<Candidate::Table*>& vecReferenceAfter_,
			   Interface::ICandidate* pOperand_)
			: Super(pTable_, pOperand_, vecLogged_,
					VECTOR<Candidate::Table*>(), vecReferenceAfter_)
		{}
		// destructor
		~Insert() {}

	/////////////////////////////
	// Interface::ICandidate::

	//////////////////////////
	// Candidate::Table::
		virtual void createPlan(Opt::Environment& cEnvironment_,
								AccessPlan::Source& cPlanSource_,
								const Utility::FieldSet& cFieldSet_);
	protected:
	private:
		void enumerate(Opt::Environment& cEnvironment_,
					   const Utility::FieldSet& cFieldSet_);

		// check put files
		void addPutFile(Opt::Environment& cEnvironment_,
						Scalar::Field* pColumn_);

		// add inserted fields
		void addFile(Opt::Environment& cEnvironment_,
					 Interface::IFile* pFile_);
		// add all inserted fields
		void addPutField(Opt::Environment& cEnvironment_,
						 Candidate::File* pFile_);

		// add one field
		void addField(Opt::Environment& cEnvironment_,
					  Candidate::File* pFile_,
					  Schema::Field* pSchemaField_);

		// get source data for a field
		Interface::IScalar* getSourceValue(Opt::Environment& cEnvironment_,
										   Candidate::File* pFile_,
										   Schema::Field* pSchemaField_);

	/////////////////////////////
	// Put::
		virtual void generateLock(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_);
		virtual void generateLog(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_);
		virtual int generateLogType(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_);
		virtual bool isStoringNeeded(Opt::Environment& cEnvironment_);
		virtual bool isCheckColumnConstraint(Opt::Environment& cEnvironment_);
		virtual Execution::Interface::IIterator*
					generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual Interface::IScalar* getOriginalField(Opt::Environment& cEnvironment_,
													 Scalar::Field* pField_);

	}; // class Insert

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::TableImpl::Delete -- implementation class of Interface::ICandidate for table
	//
	// NOTES
	class Delete
		: public Put
	{
	public:
		typedef Put Super;
		typedef Delete This;

		// constructor
		Delete(Relation::Table* pTable_,
			   Relation::Table* pRetrieve_,
			   const VECTOR<Scalar::Field*>& vecLogged_,
			   const VECTOR<Candidate::Table*>& vecReferenceBefore_,
			   Interface::ICandidate* pOperand_)
			: Super(pTable_, pOperand_, vecLogged_,
					vecReferenceBefore_, VECTOR<Candidate::Table*>()),
			  m_pRetrieve(pRetrieve_),
			  m_vecPrevLogged(),
			  m_iStoringNeeded(-1)
		{}
		// destructor
		~Delete() {}

	/////////////////////////////
	// Interface::ICandidate::

	//////////////////////////
	// Candidate::Table::
		virtual void createPlan(Opt::Environment& cEnvironment_,
								AccessPlan::Source& cPlanSource_,
								const Utility::FieldSet& cFieldSet_);
	protected:
	//////////////////////////
	// TableImpl::Put::
		virtual Execution::Interface::IIterator*
						adoptInput(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Candidate::AdoptArgument& cArgument_);
	private:
		void enumerate(Opt::Environment& cEnvironment_,
					   const Utility::FieldSet& cFieldSet_);

		// create previous value for logged fields
		void createPrevLogged(Opt::Environment& cEnvironment_);

		// get scalar to retrieve pre-update value
		Interface::IScalar* getRetrieveField(Opt::Environment& cEnvironment_,
											 Scalar::Field* pField_,
											 bool bConsiderUndo_ = false);

		// check put files
		void addPutFile(Opt::Environment& cEnvironment_,
						Scalar::Field* pColumn_);

		// add file candidate
		void addFile(Opt::Environment& cEnvironment_,
					 Interface::IFile* pFile_,
					 Scalar::Field* pField_);
		// add put fields
		void addPutField(Opt::Environment& cEnvironment_,
						 Candidate::File* pCandidate_);
		// add fields for undo expunge
		void addInsertField(Opt::Environment& cEnvironment_,
							Candidate::File* pCandidate_,
							Schema::Field* pSchemaField_);
		// get update field
		Scalar::Field* getUpdateField(Opt::Environment& cEnvironment_,
									  Candidate::File* pCandidate_,
									  Schema::Field* pSchemaField_);

		// set file dependency
		void setDependency(Opt::Environment& cEnvironment_,
						   Interface::IFile* pFile_,
						   Interface::IFile* pSourceFile_);

	/////////////////////////////
	// Put::
		virtual void generateLock(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_);
		virtual void generateLog(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_);
		virtual int generateLogType(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_);
		virtual bool isStoringNeeded(Opt::Environment& cEnvironment_);
		virtual bool isCheckColumnConstraint(Opt::Environment& cEnvironment_);
		virtual Execution::Interface::IIterator*
					generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual Interface::IScalar* getOriginalField(Opt::Environment& cEnvironment_,
													 Scalar::Field* pField_);

		Relation::Table* m_pRetrieve;
		VECTOR<Interface::IScalar*> m_vecPrevLogged;
		int m_iStoringNeeded; // cache

	}; // class Delete

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::TableImpl::Update -- implementation class of Interface::ICandidate for table
	//
	// NOTES
	class Update
		: public Put
	{
	public:
		typedef Put Super;
		typedef Update This;

		// constructor
		Update(Relation::Table* pTable_,
			   Relation::Table* pRetrieve_,
			   const VECTOR<Scalar::Field*>& vecLogged_,
			   const VECTOR<Candidate::Table*>& vecReferenceBefore_,
			   const VECTOR<Candidate::Table*>& vecReferenceAfter_,
			   const Utility::UIntSet& cLogRequiredPosition_,
			   Interface::ICandidate* pOperand_)
			: Super(pTable_, pOperand_, vecLogged_,
					vecReferenceBefore_, vecReferenceAfter_),
			  m_pRetrieve(pRetrieve_),
			  m_cLogRequiredPosition(cLogRequiredPosition_),
			  m_vecPrevLogged()
		{}
		// destructor
		~Update() {}

	/////////////////////////////
	// Interface::ICandidate::

	//////////////////////////
	// Candidate::Table::
		virtual void createPlan(Opt::Environment& cEnvironment_,
								AccessPlan::Source& cPlanSource_,
								const Utility::FieldSet& cFieldSet_);
	protected:
	private:
		void enumerate(Opt::Environment& cEnvironment_,
					   const Utility::FieldSet& cFieldSet_);

		// create previous value for logged fields
		void createPrevLogged(Opt::Environment& cEnvironment_);

		// get scalar to retrieve pre-update value
		Interface::IScalar* getRetrieveField(Opt::Environment& cEnvironment_,
											 Scalar::Field* pField_,
											 bool bConsiderUndo_ = false);

		// check put files
		void addPutFile(Opt::Environment& cEnvironment_,
						Scalar::Field* pColumn_);

		// add file candidate
		void addFile(Opt::Environment& cEnvironment_,
					 Interface::IFile* pFile_,
					 Scalar::Field* pField_);
		// add put fields
		void addPutField(Opt::Environment& cEnvironment_,
						 Candidate::File* pCandidate_);
		// add fields for insert/undo expunge
		void addInsertField(Opt::Environment& cEnvironment_,
							Candidate::File* pCandidate_,
							Schema::Field* pSchemaField_);
		// get update field
		Scalar::Field* getUpdateField(Opt::Environment& cEnvironment_,
									  Candidate::File* pCandidate_,
									  Schema::Field* pSchemaField_);

		// set file dependency
		void setDependency(Opt::Environment& cEnvironment_,
						   Interface::IFile* pFile_,
						   Interface::IFile* pSourceFile_);

	/////////////////////////////
	// Put::
		virtual void generateLock(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_);
		virtual void generateLog(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_);
		virtual int generateLogType(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_);
		virtual bool isStoringNeeded(Opt::Environment& cEnvironment_);
		virtual bool isCheckColumnConstraint(Opt::Environment& cEnvironment_);
		virtual Execution::Interface::IIterator*
					generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual Interface::IScalar* getOriginalField(Opt::Environment& cEnvironment_,
													 Scalar::Field* pField_);

		Relation::Table* m_pRetrieve;
		VECTOR<Interface::IScalar*> m_vecPrevLogged;
		Utility::UIntSet m_cLogRequiredPosition;

	}; // class Update

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::TableImpl::Import -- implementation class of Interface::ICandidate for table
	//
	// NOTES
	class Import
		: public Put
	{
	public:
		typedef Put Super;
		typedef Import This;

		// constructor
		Import(Relation::Table* pTable_,
			   const VECTOR<Candidate::Table*>& vecReferenceAfter_,
			   Interface::ICandidate* pOperand_)
			: Super(pTable_, pOperand_, VECTOR<Scalar::Field*>(),
					VECTOR<Candidate::Table*>(), vecReferenceAfter_)
		{}
		// destructor
		~Import() {}

	/////////////////////////////
	// Interface::ICandidate::
		virtual Execution::Interface::IIterator*
					generateTop(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Interface::IRelation* pRelation_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_);

	//////////////////////////
	// Candidate::Table::
		virtual void createPlan(Opt::Environment& cEnvironment_,
								AccessPlan::Source& cPlanSource_,
								const Utility::FieldSet& cFieldSet_);
	protected:
	private:
		void enumerate(Opt::Environment& cEnvironment_,
					   const Utility::FieldSet& cFieldSet_);

		// check put files
		void addPutFile(Opt::Environment& cEnvironment_,
						Scalar::Field* pColumn_);

		// add inserted fields
		void addFile(Opt::Environment& cEnvironment_,
					 Interface::IFile* pFile_);
		// add all inserted fields
		void addPutField(Opt::Environment& cEnvironment_,
						 Candidate::File* pFile_);

		// add one field
		void addField(Opt::Environment& cEnvironment_,
					  Candidate::File* pFile_,
					  Schema::Field* pSchemaField_);

		// get source data for a field
		Interface::IScalar* getSourceValue(Opt::Environment& cEnvironment_,
										   Candidate::File* pFile_,
										   Schema::Field* pSchemaField_);

	/////////////////////////////
	// Put::
		virtual Interface::IScalar* getField(Opt::Environment& cEnvironment_,
											 Schema::Column* pSchemaColumn_);
		virtual bool isStoringNeeded(Opt::Environment& cEnvironment_);
		virtual bool isCheckColumnConstraint(Opt::Environment& cEnvironment_);
		virtual Execution::Interface::IIterator*
					generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual Interface::IScalar* getOriginalField(Opt::Environment& cEnvironment_,
													 Scalar::Field* pField_);

	}; // class Import

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::TableImpl::Undo -- implementation class of Interface::ICandidate for table
	//
	// NOTES
	class Undo
		: public Put
	{
	public:
		typedef Put Super;
		typedef Undo This;

		// constructor
		Undo(Relation::Table* pTable_,
			 const Common::DataArrayData* pUndoLog_)
			: Super(pTable_, 0, VECTOR<Scalar::Field*>(),
					VECTOR<Candidate::Table*>(), VECTOR<Candidate::Table*>()),
			  m_pUndoLog(pUndoLog_)
		{}
		// destructor
		~Undo() {}

	/////////////////////////////
	// Interface::ICandidate::

	//////////////////////////
	// Candidate::Table::
		virtual void createPlan(Opt::Environment& cEnvironment_,
								AccessPlan::Source& cPlanSource_,
								const Utility::FieldSet& cFieldSet_);
	protected:
	private:
		// add undo action for one entry
		void addUndoAction(Common::Data::Pointer pData_,
						   Opt::Environment& cEnvironment_,
						   Execution::Interface::IProgram& cProgram_,
						   Execution::Interface::IIterator* pIterator_,
						   Candidate::AdoptArgument& cArgument_);

		// add undo by insert
		void addInsert(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_,
					   const Common::DataArrayData* pUndoLog_,
					   Candidate::File* pFile_);

		// add undo by expunge
		void addExpunge(Opt::Environment& cEnvironment_,
						Execution::Interface::IProgram& cProgram_,
						Execution::Interface::IIterator* pIterator_,
						Candidate::AdoptArgument& cArgument_,
						const Common::DataArrayData* pUndoLog_,
						Candidate::File* pFile_);

		// add undo by update
		void addUpdate(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_,
					   const Common::DataArrayData* pUndoLog_,
					   Candidate::File* pFile_);

		// add undo by undoExpunge
		void addUndoExpunge(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_,
							const Common::DataArrayData* pUndoLog_,
							Candidate::File* pFile_);

		// add undo by undoUpdate
		void addUndoUpdate(Opt::Environment& cEnvironment_,
						   Execution::Interface::IProgram& cProgram_,
						   Execution::Interface::IIterator* pIterator_,
						   Candidate::AdoptArgument& cArgument_,
						   const Common::DataArrayData* pUndoLog_,
						   Candidate::File* pFile_);

		// add key field to file
		void addPutKey(Opt::Environment& cEnvironment_,
					   const Common::DataArrayData* pUndoLog_,
					   Candidate::File* pFile_,
					   int iFieldPosition_,
					   int iDataPosition_);
		// add field to file
		void addField(Opt::Environment& cEnvironment_,
					  const Common::DataArrayData* pUndoLog_,
					  Candidate::File* pFile_,
					  int iFieldPosition_,
					  int iDataPosition_,
					  bool bForInsert_);

		// check whether logdata is operation
		bool isOperationLogData(const Common::Data::Pointer& pElement_);

	/////////////////////////////
	// Put::
		virtual bool isStoringNeeded(Opt::Environment& cEnvironment_);
		virtual bool isCheckColumnConstraint(Opt::Environment& cEnvironment_);
		virtual Execution::Interface::IIterator*
					generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual Interface::IScalar* getOriginalField(Opt::Environment& cEnvironment_,
													 Scalar::Field* pField_);

		const Common::DataArrayData* m_pUndoLog;
	}; // class Undo

} // namespace TableImpl

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_IMPL_TABLEIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2015, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
