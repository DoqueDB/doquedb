// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/Base.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Execution::Iterator";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Iterator/Base.h"

#include "Execution/Collection/Connection.h"
#include "Execution/Control/Conditional.h"
#include "Execution/Control/Goto.h"
#include "Execution/Control/Status.h"
#include "Execution/Interface/IAction.h"
#include "Execution/Interface/IControl.h"
#include "Execution/Interface/IParallel.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Operator/Assign.h"
#include "Execution/Operator/CheckCancel.h"
#include "Execution/Operator/Locker.h"
#include "Execution/Operator/Output.h"
#include "Execution/Parallel/OpenMP.h"
#include "Execution/Utility/Serialize.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Explain.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_ITERATOR_USING

////////////////////////////////
// Execution::Iterator::Base

// FUNCTION public
//	Iterator::Base::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	bool bPush = explainBase(cExplain_);
	explainThis(pEnvironment_, cProgram_, cExplain_);
	explainAction(pEnvironment_, cProgram_, cExplain_);
	if (bPush) {
		cExplain_.popIndent();
	}
}

// FUNCTION public
//	Iterator::Base::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
}

// FUNCTION public
//	Iterator::Base::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
}

// FUNCTION public
//	Iterator::Base::startUp -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
Base::
startUp(Interface::IProgram& cProgram_)
{
	return startUpBase(cProgram_);
}

// FUNCTION public
//	Iterator::Base::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
reset(Interface::IProgram& cProgram_)
{
	resetBase(cProgram_);
}

// FUNCTION public
//	Iterator::Base::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
finish(Interface::IProgram& cProgram_)
{
	finishBase(cProgram_);
}

// FUNCTION public
//	Iterator::Base::setWasLast -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
setWasLast(Interface::IProgram& cProgram_)
{
	setHasNext(false);
}

// FUNCTION public
//	Iterator::Base::setActionPointer -- set action position
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
setActionPointer(Interface::IProgram& cProgram_,
				 int iPosition_)
{
	m_cAction.setPointer(iPosition_);
}

// FUNCTION public
//	Iterator::Base::registerAction -- 
//
// NOTES
//
// ARGUMENTS
//	int iID_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Base::
registerAction(int iID_)
{
	m_cRegister.m_cID.set(iID_);
	return iID_;
}

// FUNCTION public
//	Iterator::Base::addStartUp -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addStartUp(Interface::IProgram& cProgram_,
		   const Action::Argument& cAction_)
{
	// by default, add action ID to startup vector
	getActionList(cProgram_, Action::Argument::Target::StartUp).addID(cAction_.getInstanceID());
}

// FUNCTION public
//	Iterator::Base::insertStartUp -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
insertStartUp(Interface::IProgram& cProgram_,
			  const Action::Argument& cAction_)
{
	// insert action ID to startup vector
	getActionList(cProgram_, Action::Argument::Target::StartUp).insertID(cAction_.getInstanceID());
}

// FUNCTION public
//	Iterator::Base::addAction -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addAction(Interface::IProgram& cProgram_,
		  const Action::Argument& cAction_)
{
	typedef void (Base::* AddActionFunction)(Interface::IProgram&, const Action::Argument&);
	static AddActionFunction _funcAddAction[][Action::Argument::Type::ValueNum] = {
		{ // execution
			&Base::addCalculation,			//Calculation
			&Base::addAggregation,			//Aggregation
			&Base::addProjection,			//Projection
			&Base::addInData,				//InData
			&Base::addOutData,				//OutData
			&Base::addInput,				//Input
			&Base::addOutput,				//Output
			&Base::addAssign,				//Assign
			&Base::addCheckCancel,			//CheckCancel
			&Base::addIf,					//If
			&Base::addEndIf,				//EndIf
			&Base::addElse,					//Else
			&Base::addUnless,				//Unless
			&Base::addContinue,				//Continue
			&Base::addBreak,				//Break
			&Base::addUnlockTuple,			//UnlockTuple
			&Base::addBeginParallel,		// BeginParallel
			&Base::addParallelList,			// ParallelList
			&Base::addEndParallel,			// EndParallel
			&Base::addReturnParallelData	// ReturnParallelData
		},
		{ // startup
			&Base::addStartUp,				//Calculation
			&Base::addAggregation,			//Aggregation
			&Base::addProjection,			//Projection
			&Base::addInData,				//InData
			&Base::addOutData,				//OutData
			&Base::addInput,				//Input
			&Base::addOutput,				//Output
			&Base::addAssign,				//Assign
			&Base::addCheckCancel,			//CheckCancel
			&Base::addIf,					//If
			&Base::addEndIf,				//EndIf
			&Base::addElse,					//Else
			&Base::addUnless,				//Unless
			&Base::addContinue,				//Continue
			&Base::addBreak,				//Break
			&Base::addUnlockTuple,			//UnlockTuple
			&Base::addBeginParallel,		// BeginParallel
			&Base::addParallelList,			// ParallelList
			&Base::addEndParallel,			// EndParallel
			&Base::addReturnParallelData	// ReturnParallelData
		},
		{ // parallel
			&Base::addCalculation,			//Calculation
			&Base::addAggregation,			//Aggregation
			&Base::addProjection,			//Projection
			&Base::addInData,				//InData
			&Base::addOutData,				//OutData
			&Base::addInput,				//Input
			&Base::addOutput,				//Output
			&Base::addAssign,				//Assign
			&Base::addCheckCancel,			//CheckCancel
			&Base::addIf,					//If
			&Base::addEndIf,				//EndIf
			&Base::addElse,					//Else
			&Base::addUnless,				//Unless
			&Base::addContinue,				//Continue
			&Base::addBreak,				//Break
			&Base::addUnlockTuple,			//UnlockTuple
			&Base::addBeginParallel,		// BeginParallel
			&Base::addParallelList,			// ParallelList
			&Base::addEndParallel,			// EndParallel
			&Base::addReturnParallelData	// ReturnParallelData
		},
		{ // aggregation
			&Base::addAggregation,			//Calculation
			&Base::addAggregation,			//Aggregation
			&Base::addProjection,			//Projection
			&Base::addInData,				//InData
			&Base::addOutData,				//OutData
			&Base::addInput,				//Input
			&Base::addOutput,				//Output
			&Base::addAssign,				//Assign
			&Base::addCheckCancel,			//CheckCancel
			&Base::addIf,					//If
			&Base::addEndIf,				//EndIf
			&Base::addElse,					//Else
			&Base::addUnless,				//Unless
			&Base::addContinue,				//Continue
			&Base::addBreak,				//Break
			&Base::addUnlockTuple,			//UnlockTuple
			&Base::addBeginParallel,		// BeginParallel
			&Base::addParallelList,			// ParallelList
			&Base::addEndParallel,			// EndParallel
			&Base::addReturnParallelData	// ReturnParallelData
		}
	};

	(this->*_funcAddAction[cAction_.getTarget()][cAction_.getType()])(cProgram_, cAction_);
}

// FUNCTION public
//	Iterator::Base::doAction -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
Base::
doAction(Interface::IProgram& cProgram_)
{
	// do actions in action list
	return m_cAction.execute(cProgram_);
}

// FUNCTION public
//	Iterator::Base::undoneAction -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
undoneAction(Interface::IProgram& cProgram_)
{
	// undone all actions registered
	m_cRegister.undone(cProgram_);

	// reset used data to null
	resetNull(cProgram_);
	// reset used data to specified value
	resetData(cProgram_);
}

// FUNCTION public
//	Iterator::Base::setMetaData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iCollectionID_
//	const Common::Data::Pointer& pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
setMetaData(Interface::IProgram& cProgram_,
			int iCollectionID_,
			const Common::Data::Pointer& pData_)
{
	// register a variable in which metadata is stored
	int iID = cProgram_.addVariable(pData_);
	// add Output operation to startUp using output collection ID and metadata variable ID
	// -> this means metadata is output to client only once.
	getActionList(cProgram_, Action::Argument::Target::StartUp).addID(
								   getOutputAction(cProgram_,
												   iCollectionID_,
												   iID));
}

// FUNCTION public
//	Iterator::Base::useVariable -- 
//
// NOTES
//
// ARGUMENTS
//	int iDataID_
//	int iResetID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
useVariable(int iDataID_,
			int iResetID_)
{
	if (iResetID_ < 0) {
		m_vecUsedData.PUSHBACK(iDataID_);
	} else {
		m_mapResetData.insert(iDataID_, iResetID_);
	}
}

// FUNCTION public
//	Iterator::Base::addLocker -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::LockerArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addLocker(Interface::IProgram& cProgram_,
		  const Action::LockerArgument& cArgument_)
{
	// default: do nothing
	;
}

// FUNCTION public
//	Iterator::Base::getLocker -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Base::
getLocker(Interface::IProgram& cProgram_)
{
	// default: no locker
	return -1;
}

// FUNCTION public
//	Iterator::Base::getOutData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Base::
getOutData(Interface::IProgram& cProgram_)
{
	// default: no outdata
	return -1;
}

// FUNCTION protected
//	Iterator::Base::addCalculation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addCalculation(Interface::IProgram& cProgram_,
			   const Action::Argument& cAction_)
{
	getActionList(cProgram_, cAction_.getTarget()).addID(cAction_.getInstanceID());
}

// FUNCTION protected
//	Iterator::Base::addAggregation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addAggregation(Interface::IProgram& cProgram_,
			   const Action::Argument& cAction_)
{
	// subclass should implement
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION protected
//	Iterator::Base::addProjection -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addProjection(Interface::IProgram& cProgram_,
			  const Action::Argument& cAction_)
{
	// do nothing
	;
}

// FUNCTION protected
//	Iterator::Base::addInData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addInData(Interface::IProgram& cProgram_,
		  const Action::Argument& cAction_)
{
	// do nothing
	;
}

// FUNCTION protected
//	Iterator::Base::addOutData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addOutData(Interface::IProgram& cProgram_,
		   const Action::Argument& cAction_)
{
	// do nothing
	;
}

// FUNCTION protected
//	Iterator::Base::addInput -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addInput(Interface::IProgram& cProgram_,
		 const Action::Argument& cAction_)
{
	// subclass should implement
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION protected
//	Iterator::Base::addOutput -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addOutput(Interface::IProgram& cProgram_,
		  const Action::Argument& cAction_)
{
	// add Output operation to action list
	// instanceID: collection ID to which data is output
	// argumentID: variable ID from which output data is read
	getActionList(cProgram_, cAction_.getTarget()).addID(
							  getOutputAction(cProgram_,
											  cAction_.getInstanceID(),
											  cAction_.getArgumentID()));
}

// FUNCTION protected
//	Iterator::Base::addAssign -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addAssign(Interface::IProgram& cProgram_,
		  const Action::Argument& cAction_)
{
	if (cAction_.getInstanceID() >= 0) {
		// create assign operation using input ID and output ID
		Interface::IAction* pAction =
			Operator::Assign::create(cProgram_,
									 this,
									 cAction_.getInstanceID(),
									 cAction_.getArgumentID());

		// add assign operation to action list
		// instanceID: input dataID
		// argumentID: output dataID
		getActionList(cProgram_, cAction_.getTarget()).addID(pAction->getID());
	}
}

// FUNCTION protected
//	Iterator::Base::addCheckCancel -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addCheckCancel(Interface::IProgram& cProgram_,
			   const Action::Argument& cAction_)
{
	// create checkcancel operation
	Interface::IAction* pAction =
		Operator::CheckCancel::create(cProgram_,
									  this);
	getActionList(cProgram_, cAction_.getTarget()).addID(pAction->getID());
}

// FUNCTION protected
//	Iterator::Base::addIf -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addIf(Interface::IProgram& cProgram_,
	  const Action::Argument& cAction_)
{
	// create If control
	Interface::IAction* pAction =
		Control::Conditional::If::create(cProgram_,
										 this,
										 cAction_.getInstanceID());

	getActionList(cProgram_, cAction_.getTarget()).addID(m_cControl.pushControl(pAction->getID()));
}

// FUNCTION protected
//	Iterator::Base::addEndIf -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addEndIf(Interface::IProgram& cProgram_,
		 const Action::Argument& cAction_)
{
	Interface::IControl* pControl = m_cControl.popControl(cProgram_);
	if (pControl == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	// set endpoint to the control
	pControl->setEnd(getActionList(cProgram_, cAction_.getTarget()).getSize());
}

// FUNCTION protected
//	Iterator::Base::addElse -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addElse(Interface::IProgram& cProgram_,
		const Action::Argument& cAction_)
{
	Interface::IControl* pControl = m_cControl.popControl(cProgram_);
	if (pControl == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	// create goto control
	Interface::IAction* pAction = Control::Goto::create(cProgram_,
														this);

	// add goto contorl and set else point to the previous control
	Action::ActionList& cActionList = getActionList(cProgram_, cAction_.getTarget());
	cActionList.addID(m_cControl.pushControl(pAction->getID()));
	pControl->setElse(cActionList.getSize());
}

// FUNCTION protected
//	Iterator::Base::addUnless -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addUnless(Interface::IProgram& cProgram_,
		  const Action::Argument& cAction_)
{
	// create If control
	Interface::IAction* pAction =
		Control::Conditional::Unless::create(cProgram_,
											 this,
											 cAction_.getInstanceID());

	getActionList(cProgram_, cAction_.getTarget()).addID(
						 m_cControl.pushControl(pAction->getID()));
}

// FUNCTION protected
//	Iterator::Base::addContinue -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addContinue(Interface::IProgram& cProgram_,
			const Action::Argument& cAction_)
{
	// create continue action
	Interface::IAction* pAction = Control::Status::Continue::create(cProgram_,
																	this);
	getActionList(cProgram_, cAction_.getTarget()).addID(pAction->getID());
}

// FUNCTION protected
//	Iterator::Base::addBreak -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addBreak(Interface::IProgram& cProgram_,
		 const Action::Argument& cAction_)
{
	// create break action
	Interface::IAction* pAction = Control::Status::Break::create(cProgram_,
																 this);
	getActionList(cProgram_, cAction_.getTarget()).addID(pAction->getID());
}

// FUNCTION protected
//	Iterator::Base::addUnlockTuple -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addUnlockTuple(Interface::IProgram& cProgram_,
			   const Action::Argument& cAction_)
{
	int iLockerID = getLocker(cProgram_);
	if (iLockerID >= 0) {
		Operator::Locker* pUnlocker =
			Operator::Locker::UnlockTuple::create(cProgram_,
												  this,
												  iLockerID);
		getActionList(cProgram_, cAction_.getTarget()).addID(pUnlocker->getID());
	}
}

// FUNCTION protected
//	Iterator::Base::addBeginParallel -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addBeginParallel(Interface::IProgram& cProgram_,
				 const Action::Argument& cAction_)
{
	int iParallelID = cAction_.getInstanceID();
	if (iParallelID < 0) {
		Interface::IAction* pParallel = Parallel::OpenMP::create(cProgram_,
																 this);
		iParallelID = pParallel->getID();
	}

	getActionList(cProgram_, cAction_.getTarget()).addID(
							 m_cParallel.pushParallel(iParallelID));
}

// FUNCTION protected
//	Iterator::Base::addParallelList -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addParallelList(Interface::IProgram& cProgram_,
				const Action::Argument& cAction_)
{
	Interface::IParallel* pParallel = m_cParallel.getParallel(cProgram_);
	if (pParallel == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	(void) pParallel->addList(cProgram_);
}

// FUNCTION protected
//	Iterator::Base::addEndParallel -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addEndParallel(Interface::IProgram& cProgram_,
			  const Action::Argument& cAction_)
{
	Interface::IParallel* pParallel = m_cParallel.popParallel(cProgram_);
	if (pParallel == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
}

// FUNCTION protected
//	Iterator::Base::addReturnParallelData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
addReturnParallelData(Interface::IProgram& cProgram_,
					  const Action::Argument& cAction_)
{
	Interface::IParallel* pParallel = m_cParallel.getParallel(cProgram_);
	if (pParallel == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	pParallel->setReturnData(cProgram_, cAction_.getInstanceID());
}

// FUNCTION protected
//	Iterator::Base::explainBase -- explain base class
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	bool	true ... an indent has pushed
//
// EXCEPTIONS

bool
Base::
explainBase(Opt::Explain& cExplain_)
{
	bool bPush = !cExplain_.isEmpty();

	if (bPush) {
		cExplain_.newLine(true /* force */).put("<-- ");
		cExplain_.pushIndent();
	}

	return bPush;
}

// FUNCTION protected
//	Iterator::Base::explainAction -- explain action
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
explainAction(Opt::Environment* pEnvironment_,
			  Interface::IProgram& cProgram_,
			  Opt::Explain& cExplain_)
{
	cExplain_.pushIndent();
	if (m_cStartUp.getSize()) {
		cExplain_.newLine(true).put("start up:");
		m_cStartUp.explain(pEnvironment_, cProgram_, cExplain_);
	}
	if (m_cAction.getSize()) {
		cExplain_.newLine(true).put("execute:");
		m_cAction.explain(pEnvironment_, cProgram_, cExplain_);
	}
	cExplain_.popIndent(true /* force new line */);
}

// FUNCTION protected
//	Iterator::Base::getOutputAction -- add new action
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iCollectionID_
//	int iDataID_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
Base::
getOutputAction(Interface::IProgram& cProgram_,
				int iCollectionID_,
				int iDataID_)
{
	// create output operation using collection ID and variable ID
	// iID:     collection ID to which data is output
	// iDataID: variable ID from which output data is read
	Interface::IAction* pAction =
		Operator::Output::create(cProgram_,
								 this,
								 iCollectionID_,
								 iDataID_);
	return pAction->getID();
}

// FUNCTION protected
//	Iterator::Base::resetAction -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
resetAction(Interface::IProgram& cProgram_)
{
	// reset all actions registered
	m_cRegister.reset(cProgram_);
}

// FUNCTION protected
//	Iterator::Base::initializeBase -- initialize base class
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
initializeBase(Interface::IProgram& cProgram_)
{
	// convert ID to object for each actions in startUp and Action list.
	m_cStartUp.initialize(cProgram_);
	m_cAction.initialize(cProgram_);
	m_cRegister.initialize(cProgram_);

	// reset used data to null
	resetNull(cProgram_);
}

// FUNCTION protected
//	Iterator::Base::terminateBase -- terminate base class
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
terminateBase(Interface::IProgram& cProgram_)
{
	// clear object list for each actions in startUp and Action list.
	m_cAction.terminate(cProgram_);
	m_cStartUp.terminate(cProgram_);

	// element of register array is not need to terminate
	m_cRegister.terminate(cProgram_);
}

// FUNCTION protected
//	Iterator::Base::startUpBase -- startup base class
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

Action::Status::Value
Base::
startUpBase(Interface::IProgram& cProgram_)
{
	// do actions in startUp action list
	return m_cStartUp.execute(cProgram_);
}

// FUNCTION protected
//	Iterator::Base::resetBase -- reset base class
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
resetBase(Interface::IProgram& cProgram_)
{
	// reset actions
	resetAction(cProgram_);
	setHasNext(setHasData(true));
}

// FUNCTION protected
//	Iterator::Base::finishBase -- finish base class
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
finishBase(Interface::IProgram& cProgram_)
{
	// do finish operations of actions in action list
	m_cStartUp.finish(cProgram_);
	m_cAction.finish(cProgram_);
	setHasNext(setHasData(true));
}

// FUNCTION protected
//	Iterator::Base::serializeBase -- serialize base class
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
serializeBase(ModArchive& archiver_)
{
	serializeID(archiver_);
	// serialize action lists
	m_cAction.serialize(archiver_);
	m_cStartUp.serialize(archiver_);
	m_cRegister.serialize(archiver_);
	Utility::SerializeValue(archiver_, m_vecUsedData);
	Utility::SerializeMap(archiver_, m_mapResetData);
}

// FUNCTION protected
//	Iterator::Base::getActionList -- get action list
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::Argument::Target::Value eTarget_
//	
// RETURN
//	Action::ActionList&
//
// EXCEPTIONS

Action::ActionList&
Base::
getActionList(Interface::IProgram& cProgram_,
			  Action::Argument::Target::Value eTarget_)
{
	switch (eTarget_) {
	case Action::Argument::Target::StartUp:
		{
			return m_cStartUp;
		}
	case Action::Argument::Target::Execution:
	case Action::Argument::Target::Aggregation:
		{
			return m_cAction;
		}
	case Action::Argument::Target::Parallel:
		{
			Interface::IParallel* pParallel = m_cParallel.getParallel(cProgram_);
			if (pParallel == 0) {
				return m_cAction;
			}
			return pParallel->getList(cProgram_);
		}
	}
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION private
//	Iterator::Base::resetNull -- reset used data to null
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
resetNull(Interface::IProgram& cProgram_)
{
	if (int n = m_vecUsedData.GETSIZE()) {
		for (int i = 0; i < n; ++i) {
			cProgram_.getVariable(m_vecUsedData[i])->setNull();
		}
	}
}

// FUNCTION private
//	Iterator::Base::resetData -- reset used data to specified value
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
resetData(Interface::IProgram& cProgram_)
{
	if (m_mapResetData.GETSIZE()) {
		MAP<int, int, LESS<int> >::ITERATOR iterator = m_mapResetData.begin();
		const MAP<int, int, LESS<int> >::ITERATOR last = m_mapResetData.end();
		for (; iterator != last; ++iterator) {
			cProgram_.getVariable((*iterator).first)->assign(
							 cProgram_.getVariable((*iterator).second).get());
		}
	}
}

/////////////////////////////////////
// Iterator::Base::ActionMap

// FUNCTION public
//	Iterator::Base::ActionMap::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::ActionMap::
initialize(Interface::IProgram& cProgram_)
{
	// convert each action ID into action object
	if (m_vecAction.ISEMPTY()) {
		m_vecAction.reserve(m_cID.count());
		Opt::MapContainer(m_cID, m_vecAction,
						  boost::bind(&Interface::IProgram::getAction,
									  &cProgram_,
									  _1));
	}
}

// FUNCTION public
//	Iterator::Base::ActionMap::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::ActionMap::
terminate(Interface::IProgram& cProgram_)
{
	m_vecAction.clear();
}

// FUNCTION public
//	Iterator::Base::ActionMap::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::ActionMap::
reset(Interface::IProgram& cProgram_)
{
	FOREACH(m_vecAction,
			boost::bind(&Interface::IAction::reset,
						_1,
						boost::ref(cProgram_)));
}

// FUNCTION public
//	Iterator::Base::ActionMap::undone -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::ActionMap::
undone(Interface::IProgram& cProgram_)
{
	FOREACH(m_vecAction,
			boost::bind(&Interface::IAction::undone,
						_1,
						boost::ref(cProgram_)));
}

// FUNCTION public
//	Iterator::Base::ActionMap::serialize -- serialize
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

// serialize
void
Base::ActionMap::
serialize(ModArchive& archiver_)
{
	m_cID.serialize(archiver_);
}

/////////////////////////////////////
// Iterator::Base::ControlStack

// FUNCTION public
//	Iterator::Base::ControlStack::pushControl -- 
//
// NOTES
//
// ARGUMENTS
//	int iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

int
Base::ControlStack::
pushControl(int iID_)
{
	m_vecControl.PUSHBACK(iID_);
	return iID_;
}

// FUNCTION public
//	Iterator::Base::ControlStack::popControl -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Interface::IControl*
//
// EXCEPTIONS

Interface::IControl*
Base::ControlStack::
popControl(Interface::IProgram& cProgram_)
{
	if (!m_vecControl.ISEMPTY()) {
		int iID = m_vecControl.GETBACK();
		m_vecControl.POPBACK();
		Interface::IControl* pResult =
			_SYDNEY_DYNAMIC_CAST(Interface::IControl*,
								 cProgram_.getAction(iID));
		if (pResult) return pResult;
	}
	return 0;
}

/////////////////////////////////////
// Iterator::Base::ParallelStack

// FUNCTION public
//	Iterator::Base::ParallelStack::pushParallel -- 
//
// NOTES
//
// ARGUMENTS
//	int iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

int
Base::ParallelStack::
pushParallel(int iID_)
{
	m_vecParallel.PUSHBACK(iID_);
	return iID_;
}

// FUNCTION public
//	Iterator::Base::ParallelStack::getParallel -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Interface::IParallel*
//
// EXCEPTIONS

Interface::IParallel*
Base::ParallelStack::
getParallel(Interface::IProgram& cProgram_)
{
	if (!m_vecParallel.ISEMPTY()) {
		int iID = m_vecParallel.GETBACK();
		Interface::IParallel* pResult =
			_SYDNEY_DYNAMIC_CAST(Interface::IParallel*,
								 cProgram_.getAction(iID));
		if (pResult) return pResult;
	}
	return 0;
}

// FUNCTION public
//	Iterator::Base::ParallelStack::popParallel -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Interface::IParallel*
//
// EXCEPTIONS

Interface::IParallel*
Base::ParallelStack::
popParallel(Interface::IProgram& cProgram_)
{
	Interface::IParallel* pResult =
		getParallel(cProgram_);
	if (pResult) {
		m_vecParallel.POPBACK();
	}
	return pResult;
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
