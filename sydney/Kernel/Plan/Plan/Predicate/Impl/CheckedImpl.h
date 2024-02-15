// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/CheckedImpl.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_CHECKEDIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_CHECKEDIMPL_H

#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/AccessPlan/Cost.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace CheckedImpl
{
	///////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::CheckedImpl::Base --
	//
	// NOTES
	class Base
		: public Predicate::CheckedInterface
	{
	public:
		typedef Predicate::CheckedInterface Super;
		typedef Base This;

		// constructor
		Base(Interface::IPredicate* pPredicate_)
			: Super(pPredicate_)
		{}

		// destructor
		virtual ~Base() {}

	////////////////////////
	// CheckedInterface::
		virtual const Utility::FileSet& getFile();
		virtual Candidate::Table* getTable() {return 0;}
		virtual bool isUseIndex() {return false;}

	/////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* getNotChecked()
		{return 0;}
		virtual Interface::IPredicate* choose(Opt::Environment& cEnvironment_,
											  ChooseArgument& cArgument_);

	protected:
	private:
	};

	///////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::CheckedImpl::SingleIndex --
	//
	// NOTES
	class SingleIndex
		: public Base
	{
	public:
		typedef Base Super;
		typedef SingleIndex This;

		// constructor
		SingleIndex(Interface::IPredicate* pPredicate_,
					Candidate::Table* pTable_,
					const Utility::FileSet& cFileSet_)
			: Super(pPredicate_),
			  m_cFileSet(cFileSet_),
			  m_pTable(pTable_)
		{}

		// destructor
		virtual ~SingleIndex() {}

	////////////////////////
	// CheckedInterface::
		virtual const Utility::FileSet& getFile()
		{return m_cFileSet;}
		virtual Candidate::Table* getTable()
		{return m_pTable;}
		virtual bool isUseIndex() {return true;}

	////////////////////////
	// Interface::
		virtual Interface::IPredicate* getNotChecked()
		{return 0;}
		virtual Interface::IPredicate* choose(Opt::Environment& cEnvironment_,
											  ChooseArgument& cArgument_);
		virtual Interface::IPredicate* createFetch(Opt::Environment& cEnvironment_,
												   Utility::ScalarSet& cFetchKey_);

	protected:
		// create choseninterface
		ChosenInterface* chooseIndex(Opt::Environment& cEnvironment_,
									 Interface::IPredicate* pPredicate_,
									 ChooseArgument& cArgument_);
		// choose index file which can evaluate the predicate
		bool chooseFile(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						ChooseArgument& cArgument_,
						File::CheckArgument& cCheckArgument_);
		// check availability for a set of file
		bool checkFiles(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						Candidate::Table* pTable_,
						Utility::FileSet::ConstIterator first_,
						Utility::FileSet::ConstIterator last_,
						File::CheckArgument& cCheckArgument_,
						ChooseArgument& cArgument_);
		// check availability for each file
		bool checkFile(Opt::Environment& cEnvironment_,
					   Interface::IPredicate* pPredicate_,
					   Candidate::Table* pTable_,
					   Interface::IFile* pFile_,
					   File::Parameter* pParameter_,
					   File::CheckArgument& cCheckArgument_,
					   ChooseArgument& cArgument_);
		// check order availability for each file
		bool checkOrder(Opt::Environment& cEnvironment_,
						Candidate::Table* pTable_,
						Interface::IFile* pFile_,
						File::CheckArgument& cCheckArgument_,
						Order::CheckedSpecification* pCheckedOrder_,
						const AccessPlan::Cost& cCost_);
	private:
		Utility::FileSet m_cFileSet;
		Candidate::Table* m_pTable;
	};

	////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::CheckedImpl::MultipleIndex --
	//
	// NOTES
	class MultipleIndex
		: public SingleIndex
	{
	public:
		typedef SingleIndex Super;
		typedef MultipleIndex This;

		// constructor
		MultipleIndex(Interface::IPredicate* pPredicate_,
					  const VECTOR<Interface::IPredicate*>& vecChecked_,
					  Candidate::Table* pTable_,
					  const Utility::FileSet& cFileSet_,
					  bool bNoTop_)
			: Super(pPredicate_, pTable_, cFileSet_),
			  m_vecChecked(vecChecked_),
			  m_bNoTop(bNoTop_)
		{}

		// destructor
		virtual ~MultipleIndex() {}

	////////////////////////
	// CheckedInterface::
		//virtual const Utility::FileSet& getFile();
		//virtual Candidate::Table* getTable();

	////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* getNotChecked()
		{return 0;}
		virtual Interface::IPredicate* choose(Opt::Environment& cEnvironment_,
											  ChooseArgument& cArgument_);
		virtual bool isFetch();
		virtual bool getFetchKey(Opt::Environment& cEnvironment_,
								 Utility::ScalarSet& cFetchKey_);

	protected:
	private:
		// check cost of operand candidates
		void checkCost(Opt::Environment& cEnvironment_,
					   ChooseArgument& cArgument_,
					   VECTOR<Interface::IPredicate*>& vecChosen_);
		// compare cost of one operand
		bool compareCost(Opt::Environment& cEnvironment_,
						 const ChooseArgument& cArgument_,
						 const AccessPlan::Cost::Value& cCount_,
						 const AccessPlan::Cost::Value& cLimit_,
						 Interface::IPredicate* pPredicate_);

		VECTOR<Interface::IPredicate*> m_vecChecked;
		bool m_bNoTop;
	};

	////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::CheckedImpl::PartialMultipleIndex --
	//
	// NOTES
	class PartialMultipleIndex
		: public MultipleIndex
	{
	public:
		typedef MultipleIndex Super;
		typedef PartialMultipleIndex This;

		// constructor
		PartialMultipleIndex(Interface::IPredicate* pPredicate_,
							 const VECTOR<Interface::IPredicate*>& vecChecked_,
							 Interface::IPredicate* pNotChecked_,
							 Candidate::Table* pTable_,
							 const Utility::FileSet& cFileSet_,
							 bool bNoTop_)
			: Super(pPredicate_, vecChecked_, pTable_, cFileSet_, bNoTop_),
			  m_pNotChecked(pNotChecked_)
		{}

		// destructor
		virtual ~PartialMultipleIndex() {}

	////////////////////////
	// CheckedInterface::
		//virtual const Utility::FileSet& getFile();
		//virtual Candidate::Table* getTable();

	/////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* getNotChecked()
		{return m_pNotChecked;}
	//	virtual Interface::IPredicate* choose(Opt::Environment& cEnvironment_,
	//										  ChooseArgument& cArgument_);
	protected:
	private:
		Interface::IPredicate* m_pNotChecked;
	};

	////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::CheckedImpl::CombinedFetch --
	//
	// NOTES
	class CombinedFetch
		: public SingleIndex
	{
	public:
		typedef SingleIndex Super;
		typedef CombinedFetch This;

		// constructor
		CombinedFetch(Interface::IPredicate* pPredicate_,
					  const PAIR<Interface::IPredicate*, Interface::IPredicate*>& cChecked_,
					  Candidate::Table* pTable_,
					  const Utility::FileSet& cFileSet_)
			: Super(pPredicate_, pTable_, cFileSet_),
			  m_cChecked(cChecked_)
		{}

		// destructor
		virtual ~CombinedFetch() {}

	////////////////////////
	// CheckedInterface::
		//virtual const Utility::FileSet& getFile();
		//virtual Candidate::Table* getTable();

	////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* getNotChecked()
		{return 0;}
		virtual Interface::IPredicate* choose(Opt::Environment& cEnvironment_,
											  ChooseArgument& cArgument_);
		virtual bool isFetch();
		virtual bool getFetchKey(Opt::Environment& cEnvironment_,
								 Utility::ScalarSet& cFetchKey_);

	protected:
	private:
		PAIR<Interface::IPredicate*, Interface::IPredicate*> m_cChecked;
	};

	////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::CheckedImpl::OperandIndexImpl --
	//
	// NOTES
	class OperandIndex
		: public Base
	{
	public:
		typedef Base Super;
		typedef OperandIndex This;

		// constructor
		OperandIndex(Interface::IPredicate* pPredicate_,
						 Interface::IPredicate* pOperand_)
			: Super(pPredicate_),
			  m_pOperand(pOperand_)
		{}

		// destructor
		virtual ~OperandIndex() {}

	////////////////////////
	// CheckedInterface::
		//virtual const Utility::FileSet& getFile();
		//virtual Candidate::Table* getTable();
		virtual bool isUseIndex()
		{return m_pOperand->isChecked() ? m_pOperand->getChecked()->isUseIndex() : false;}

	////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* getNotChecked()
		{return 0;}
		virtual Interface::IPredicate* choose(Opt::Environment& cEnvironment_,
											  ChooseArgument& cArgument_);
		virtual bool isFetch();

	protected:
		Interface::IPredicate* getOperand() {return m_pOperand;}

	private:
		Interface::IPredicate* m_pOperand;
	};
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_CHECKEDIMPL_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
