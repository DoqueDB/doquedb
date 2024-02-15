// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Iterator/Base.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ITERATOR_BASE_H
#define __SYDNEY_EXECUTION_ITERATOR_BASE_H

#include "Execution/Iterator/Module.h"

#include "Execution/Action/Argument.h"
#include "Execution/Action/ActionList.h"
#include "Execution/Interface/IIterator.h"

#include "Common/BitSet.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
	class OpenOption;
}
namespace Schema
{
	class File;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Iterator::Base -- Base implementation class for Iterator interface
//
//	NOTES
//		This class is not constructed directly
class Base
	: public Interface::IIterator
{
public:
	typedef Interface::IIterator Super;
	typedef Base This;

	// destructor
	virtual ~Base() {}

////////////////////////////
// Interface::IIterator::
	virtual void explain(Opt::Environment* pEnvironment_,
						 Interface::IProgram& cProgram_,
						 Opt::Explain& cExplain_);

	virtual void initialize(Interface::IProgram& cProgram_);
	virtual void terminate(Interface::IProgram& cProgram_);
	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
	virtual void reset(Interface::IProgram& cProgram_);
	virtual void finish(Interface::IProgram& cProgram_);
	virtual void setWasLast(Interface::IProgram& cProgram_);

	virtual void setActionPointer(Interface::IProgram& cProgram_,
								  int iPosition_);

	virtual int registerAction(int iID_);
	virtual void addStartUp(Interface::IProgram& cProgram_,
							const Action::Argument& cAction_);
	virtual void insertStartUp(Interface::IProgram& cProgram_,
							   const Action::Argument& cAction_);
	virtual void addAction(Interface::IProgram& cProgram_,
						   const Action::Argument& cAction_);
	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
	virtual void undoneAction(Interface::IProgram& cProgram_);

	virtual bool isEndOfData() {return !hasData();}

	virtual void setMetaData(Interface::IProgram& cProgram_,
							 int iCollectionID_,
							 const Common::Data::Pointer& pData_);
	virtual void useVariable(int iDataID_,
							 int iResetID_ = -1);

	virtual void addLocker(Interface::IProgram& cProgram_,
						   const Action::LockerArgument& cArgument_);
	virtual int getLocker(Interface::IProgram& cProgram_);
	virtual int getOutData(Interface::IProgram& cProgram_);

	using Super::addCalculation;

protected:
	// constructor
	Base()
		: Super(),
		  m_cAction(),
		  m_cStartUp(),
		  m_vecUsedData(),
		  m_mapResetData(),
		  m_bHasData(true),
		  m_bHasNext(true)
	{}

	// explain node itself
	virtual void explainThis(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_) = 0;

	// add action implementation
	virtual void addCalculation(Interface::IProgram& cProgram_,
								const Action::Argument& cAction_);
	virtual void addAggregation(Interface::IProgram& cProgram_,
								const Action::Argument& cAction_);
	virtual void addProjection(Interface::IProgram& cProgram_,
							   const Action::Argument& cAction_);
	virtual void addInData(Interface::IProgram& cProgram_,
						   const Action::Argument& cAction_);
	virtual void addOutData(Interface::IProgram& cProgram_,
							const Action::Argument& cAction_);
	virtual void addInput(Interface::IProgram& cProgram_,
						  const Action::Argument& cAction_);
	virtual void addOutput(Interface::IProgram& cProgram_,
						   const Action::Argument& cAction_);
	virtual void addAssign(Interface::IProgram& cProgram_,
						   const Action::Argument& cAction_);
	virtual void addCheckCancel(Interface::IProgram& cProgram_,
								const Action::Argument& cAction_);
	virtual void addIf(Interface::IProgram& cProgram_,
					   const Action::Argument& cAction_);
	virtual void addEndIf(Interface::IProgram& cProgram_,
						  const Action::Argument& cAction_);
	virtual void addElse(Interface::IProgram& cProgram_,
						 const Action::Argument& cAction_);
	virtual void addUnless(Interface::IProgram& cProgram_,
						   const Action::Argument& cAction_);
	virtual void addContinue(Interface::IProgram& cProgram_,
							 const Action::Argument& cAction_);
	virtual void addBreak(Interface::IProgram& cProgram_,
						  const Action::Argument& cAction_);
	virtual void addUnlockTuple(Interface::IProgram& cProgram_,
								const Action::Argument& cAction_);
	virtual void addBeginParallel(Interface::IProgram& cProgram_,
								  const Action::Argument& cAction_);
	virtual void addParallelList(Interface::IProgram& cProgram_,
								 const Action::Argument& cAction_);
	virtual void addEndParallel(Interface::IProgram& cProgram_,
								const Action::Argument& cAction_);
	virtual void addReturnParallelData(Interface::IProgram& cProgram_,
									   const Action::Argument& cAction_);

	// explain base class
	bool explainBase(Opt::Explain& cExplain_);
	// explain action
	void explainAction(Opt::Environment* pEnvironment_,
					   Interface::IProgram& cProgram_,
					   Opt::Explain& cExplain_);

	// create new action
	int getOutputAction(Interface::IProgram& cProgram_,
						int iCollectionID_,
						int iDataID_);

	// reset action
	void resetAction(Interface::IProgram& cProgram_);

	// initialize base class
	void initializeBase(Interface::IProgram& cProgram_);
	// terminate base class
	void terminateBase(Interface::IProgram& cProgram_);
	// startup base class
	Action::Status::Value startUpBase(Interface::IProgram& cProgram_);
	// reset base class
	void resetBase(Interface::IProgram& cProgram_);
	// finish base class
	void finishBase(Interface::IProgram& cProgram_);
	// serialize base class
	void serializeBase(ModArchive& archiver_);

	// set iteration flags
	bool setHasData(bool b_) {return m_bHasData = b_;}
	bool setHasNext(bool b_) {return m_bHasNext = b_;}

	bool hasData() {return m_bHasData;}
	bool hasNext() {return m_bHasNext;}

	// get action list
	Action::ActionList& getActionList(Interface::IProgram& cProgram_,
									  Action::Argument::Target::Value eTarget_);

private:
	// reset used data to null
	void resetNull(Interface::IProgram& cProgram_);
	// reset used data to specified value
	void resetData(Interface::IProgram& cProgram_);

	Action::ActionList m_cAction;
	Action::ActionList m_cStartUp;
	VECTOR<int> m_vecUsedData;
	MAP<int, int, LESS<int> > m_mapResetData;

	struct ActionMap
	{
		Common::BitSet m_cID;
		VECTOR<Interface::IAction*> m_vecAction;

		void initialize(Interface::IProgram& cProgram_);
		void terminate(Interface::IProgram& cProgram_);
		void reset(Interface::IProgram& cProgram_);
		void undone(Interface::IProgram& cProgram_);

		// serialize
		void serialize(ModArchive& archiver_);
	};
	ActionMap m_cRegister;

	// action id which represents control sequence
	struct ControlStack
	{
		VECTOR<int> m_vecControl;

		int pushControl(int iID_);
		Interface::IControl* popControl(Interface::IProgram& cProgram_);
	};
	ControlStack m_cControl;

	// action id which represents parallel execution
	struct ParallelStack
	{
		VECTOR<int> m_vecParallel;

		int pushParallel(int iID_);
		Interface::IParallel* getParallel(Interface::IProgram& cProgram_);
		Interface::IParallel* popParallel(Interface::IProgram& cProgram_);
	};
	ParallelStack m_cParallel;

	bool m_bHasData;
	bool m_bHasNext;
};

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ITERATOR_BASE_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
