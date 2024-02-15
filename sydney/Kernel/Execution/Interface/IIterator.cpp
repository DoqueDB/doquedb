// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/Iterator.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Execution::Interface";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Interface/IIterator.h"

#include "Execution/Action/Argument.h"
#include "Execution/Interface/IAction.h"
#include "Execution/Interface/IProgram.h"

#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_INTERFACE_USING

//////////////////////////////////////////
// Execution::Interface::IIterator

// FUNCTION public
//	Interface::IIterator::addCalculation -- add action by object
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IAction* pAction_
//	Action::Argument::Target::Value eTarget_ = Action::Argument::Target::Execution
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IIterator::
addCalculation(Interface::IProgram& cProgram_,
			   Interface::IAction* pAction_,
			   Action::Argument::Target::Value eTarget_ /* = Action::Argument::Target::Execution */)
{
	addAction(cProgram_,
			  _ACTION_ARGUMENT1_T(Calculation,
								  pAction_->getID(),
								  eTarget_));
}

// FUNCTION public
//	Interface::IIterator::addPredicate -- add action by object
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IAction* pAction_
//	Action::Argument::Target::Value eTarget_ = Action::Argument::Target::Execution
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IIterator::
addPredicate(Interface::IProgram& cProgram_,
			 Interface::IAction* pAction_,
			 Action::Argument::Target::Value eTarget_ /* = Action::Argument::Target::Execution */)
{
	addPredicate(cProgram_,
				 pAction_->getID(),
				 eTarget_);
}

// FUNCTION public
//	Interface::IIterator::addPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iPredicateID_
//	Action::Argument::Target::Value eTarget_ = Action::Argument::Target::Execution
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IIterator::
addPredicate(Interface::IProgram& cProgram_,
			 int iPredicateID_,
			 Action::Argument::Target::Value eTarget_ /* = Action::Argument::Target::Execution */)
{
	if (iPredicateID_ >= 0) {
		addAction(cProgram_,
				  _ACTION_ARGUMENT1_T(Unless,
									  iPredicateID_,
									  eTarget_));
		addAction(cProgram_,
				  _ACTION_ARGUMENT0_T(Continue,
									  eTarget_));
		addAction(cProgram_,
				  _ACTION_ARGUMENT0_T(EndIf,
									  eTarget_));
	}
}

// FUNCTION public
//	Interface::IIterator::setNodeVariable -- 
//
// NOTES
//
// ARGUMENTS
//	int iNodeID_
//	int iDataID_
//	IIterator* pIterator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IIterator::
setNodeVariable(int iNodeID_, int iDataID_, IIterator* pIterator_)
{
	// record node -> variable relationship so that same node uses same variable.
	m_mapNodeVariable[iNodeID_] = iDataID_;
	m_mapVariableIterator[iDataID_] = pIterator_;
}

// FUNCTION public
//	Interface::IIterator::getNodeVariable -- 
//
// NOTES
//
// ARGUMENTS
//	int iNodeID_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
IIterator::
getNodeVariable(int iNodeID_)
{
	// get variable ID related to a node if recorded
	MAP<int, int, LESS<int> >::Iterator found = m_mapNodeVariable.find(iNodeID_);
	if (found == m_mapNodeVariable.end()) {
		return -1;
	} else {
		return (*found).second;
	}
}

// FUNCTION public
//	Interface::IIterator::getGenerateIterator -- get generating iterator from data ID
//
// NOTES
//
// ARGUMENTS
//	int iDataID_
//	
// RETURN
//	IIterator*
//
// EXCEPTIONS

IIterator*
IIterator::
getGenerateIterator(int iDataID_)
{
	MAP<int, IIterator*, LESS<int> >::Iterator found = m_mapVariableIterator.find(iDataID_);
	if (found == m_mapVariableIterator.end()) {
		return 0;
	} else {
		return (*found).second;
	}
}

// FUNCTION public
//	Interface::IIterator::copyNodeVariable -- 
//
// NOTES
//
// ARGUMENTS
//	IIterator* pIterator_
//	int iNodeID_
//	bool bCollection_ = false
//	
// RETURN
//	int
//
// EXCEPTIONS

int
IIterator::
copyNodeVariable(IIterator* pIterator_,
				 int iNodeID_,
				 bool bCollection_ /* = false */)
{
	int iDataID = getNodeVariable(iNodeID_);
	if (iDataID < 0) {
		iDataID = pIterator_->getNodeVariable(iNodeID_);
		if (iDataID >= 0) {
			setNodeVariable(iNodeID_, iDataID,
							bCollection_ ?
							this
							: pIterator_->getGenerateIterator(iDataID));
		}
	}
	return iDataID;
}

// FUNCTION public
//	Interface::IIterator::copyNodeVariable -- copy node <-> variable relationship
//
// NOTES
//	Copy (node -> variable) relationship from other iterator.
// 	This means this iterator reuses same variable for same node.
//	ex.
//	  sorting:
//     	iteration A.
//			1. read data from file -> assign the value to variable related to each column
//			2. copy data and store it to collection
//			3. when all the data were read from file, sort the collection
//		iteration B.
//			1. read one tuple from collection -> assign the value to vvariable related to each column
//			2. write data to Communication::Connection to return to client
//
// 		variables used in A-1 can be reused in B-1.
//
// ARGUMENTS
//	IIterator* pIterator_
//	bool bCollection_ = false
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IIterator::
copyNodeVariable(IIterator* pIterator_,
				 bool bCollection_ /* = false */)
{
	m_mapNodeVariable = pIterator_->m_mapNodeVariable;
	if (bCollection_) {
		Opt::ForEachValue(m_mapNodeVariable,
						  boost::bind(&This::setGenerateIterator,
									  this,
									  _1,
									  this));
	} else {
		m_mapVariableIterator = pIterator_->m_mapVariableIterator;
	}
}


// FUNCTION public
//	Interface::IIterator::getCascadeQueue -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	ICollection*
//
// EXCEPTIONS

ICollection*
IIterator::
getCascadeQueue()
{
	return m_pCollection;
}


// FUNCTION public
//	Interface::IIterator::setCascadeQueue -- 
//
// NOTES
//
// ARGUMENTS
//	ICollection*
//	
// RETURN
//	void
//
// EXCEPTIONS

void
IIterator::
setCascadeQueue(ICollection* pCollection_)
{
	m_pCollection = pCollection_;
}



// FUNCTION protected
//	Interface::IIterator::registerToProgram -- register to program
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
IIterator::
registerToProgram(Interface::IProgram& cProgram_)
{
	// Instance ID is obtained by registerIterator method.
	setID(cProgram_.registerIterator(this));
}

// FUNCTION private
//	Interface::IIterator::setGenerateIterator -- register data <-> generating iterator relationship (used only internally)
//
// NOTES
//
// ARGUMENTS
//	int iDataID_
//	IIterator* pIterator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IIterator::
setGenerateIterator(int iDataID_, IIterator* pIterator_)
{
	m_mapVariableIterator[iDataID_] = pIterator_;
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
