// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Interface/IIterator.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_INTERFACE_IITERATOR_H
#define __SYDNEY_EXECUTION_INTERFACE_IITERATOR_H

#include "Execution/Interface/IObject.h"
#include "Execution/Action/Argument.h"
#include "Execution/Action/Status.h"

#include "Opt/Algorithm.h"
#include "Opt/Declaration.h"

#include "Common/Data.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN


////////////////////////////////////
//	CLASS
//	Execution::Interface::Iterator -- Base class for the classes which represents interface data information
//
//	NOTES
//		This class is not constructed directly
class IIterator
	: public IObject
{
public:
	typedef IObject Super;
	typedef IIterator This;

	// destructor
	virtual ~IIterator() {}

	// output explain
	virtual void explain(Opt::Environment* pEnvironment_,
						 Interface::IProgram& cProgram_,
						 Opt::Explain& cExplain_) = 0;

	// set iteration as last data
	virtual void setWasLast(Interface::IProgram& cProgram_) = 0;

	// set action position
	virtual void setActionPointer(Interface::IProgram& cProgram_,
								  int iPosition_) = 0;

	// register action
	virtual int registerAction(int iID_) = 0;

	// add startup
	virtual void addStartUp(Interface::IProgram& cProgram_,
							const Action::Argument& cAction_) = 0;
	virtual void insertStartUp(Interface::IProgram& cProgram_,
							   const Action::Argument& cAction_) = 0;
	// add action
	virtual void addAction(Interface::IProgram& cProgram_,
						   const Action::Argument& cAction_) = 0;
	// do action for current tuple
	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_) = 0;

	// undone action before doing
	virtual void undoneAction(Interface::IProgram& cProgram_) = 0;

	// check status
	virtual bool isEndOfData() = 0;

	// set metadata
	virtual void setMetaData(Interface::IProgram& cProgram_,
							 int iCollectionID_,
							 const Common::Data::Pointer& pData_) = 0;

	// register used variables
	virtual void useVariable(int iDataID_,
							 int iResetID_ = -1) = 0;

	// add locker for tuples of the iterator
	virtual void addLocker(Interface::IProgram& cProgram_,
						   const Action::LockerArgument& cArgument_) = 0;

	// get locker ID for tuples of the iterator
	virtual int getLocker(Interface::IProgram& cProgram_) = 0;

	// get data ID for output tuple
	virtual int getOutData(Interface::IProgram& cProgram_) = 0;

	// add action by object
	void addCalculation(Interface::IProgram& cProgram_,
						Interface::IAction* pAction_,
						Action::Argument::Target::Value eTarget_ = Action::Argument::Target::Execution);
	void addPredicate(Interface::IProgram& cProgram_,
					  Interface::IAction* pAction_,
					  Action::Argument::Target::Value eTarget_ = Action::Argument::Target::Execution);
	void addPredicate(Interface::IProgram& cProgram_,
					  int iPredicateID_,
					  Action::Argument::Target::Value eTarget_ = Action::Argument::Target::Execution);

	// register node <-> data relationship
	void setNodeVariable(int iNodeID_, int iDataID_, IIterator* pIterator_);
	// get data ID from node ID
	int getNodeVariable(int iNodeID_);
	// get generating iterator from data ID
	IIterator* getGenerateIterator(int iDataID_);

	// copy node <-> data relationship
	int copyNodeVariable(IIterator* pIterator_,
						 int iNodeID_,
						 bool bCollection_ = false);
	void copyNodeVariable(IIterator* pIterator_,
						  bool bCollection_ = false);

	ICollection* getCascadeQueue();

	void setCascadeQueue(ICollection* pCollection_);

protected:
	// constructor
	IIterator()
		: Super(),
		  m_pCollection(0)
		{}
	// register to program
	void registerToProgram(Interface::IProgram& cProgram_);

private:
	// methods for execution only can be accessed by IProgram
	friend class IProgram;

	// initialize
	virtual void initialize(Interface::IProgram& cProgram_) = 0;
	// terminate
	virtual void terminate(Interface::IProgram& cProgram_) = 0;
	// do before iteration
	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_) = 0;
	// go to next tuple
	virtual bool next(Interface::IProgram& cProgram_) = 0;
	// reset iteration to first tuple
	virtual void reset(Interface::IProgram& cProgram_) = 0;
	// do after iteration
	virtual void finish(Interface::IProgram& cProgram_) = 0;

	// register data <-> generating iterator relationship (used only internally)
	void setGenerateIterator(int iDataID_, IIterator* pIterator_);

	MAP<int, int, LESS<int> > m_mapNodeVariable;
	MAP<int, IIterator*, LESS<int> > m_mapVariableIterator;
	ICollection* m_pCollection;
};

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_IITERATOR_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
