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

#ifndef __SYDNEY_EXECUTION_ITERATOR_FORWORDING_ITERATOR_H
#define __SYDNEY_EXECUTION_ITERATOR_FORWORDING_ITERATOR_H

#include "Execution/Iterator/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Action/IteratorHolder.h"

#include "Common/Object.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Interface::Iterator -- Base class for the classes which represents interface data information
//
//	NOTES
//		This class is not constructed directly
template <class Base_>
class ForwordingIterator
	: public Base_
{
public:
	typedef Base_ Super;
	typedef ForwordingIterator This;

	// destructor
	virtual ~ForwordingIterator() {}

	// output explain
	virtual void explain(Opt::Environment* pEnvironment_,
						 Interface::IProgram& cProgram_,
						 Opt::Explain& cExplain_)
	{
		m_pIterator->explain(pEnvironment_,
							 cProgram_,
							 cExplain_);
	}

	// set iteration as last data
	virtual void setWasLast(Interface::IProgram& cProgram_)
	{
		m_pIterator->setWasLast(cProgram_);
	}

	// set action position
	virtual void setActionPointer(Interface::IProgram& cProgram_,
								  int iPosition_)
	{
		m_pIterator->setActionPointer(cProgram_, iPosition_);
	}

	// register action
	virtual int registerAction(int iID_)
	{
		return m_pIterator->registerAction(iID_);
	}

	// add startup
	virtual void addStartUp(Interface::IProgram& cProgram_,
							const Action::Argument& cAction_)
	{
		m_pIterator->addStartUp(cProgram_, cAction_);
	}
	
	virtual void insertStartUp(Interface::IProgram& cProgram_,
							   const Action::Argument& cAction_)
	{
		m_pIterator->insertStartUp(cProgram_, cAction_);
	}
								   
	// add action
	virtual void addAction(Interface::IProgram& cProgram_,
						   const Action::Argument& cAction_)
	{
		m_pIterator->addAction(cProgram_, cAction_);
	}
	
	// do action for current tuple
	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_)
	{
		return Action::Status::Success;
	}

	// undone action before doing
	virtual void undoneAction(Interface::IProgram& cProgram_)
	{
		; //nothing todo
	}

	// check status
	virtual bool isEndOfData()
	{
		return m_pIterator->isEndOfData();
	}

	// set metadata
	virtual void setMetaData(Interface::IProgram& cProgram_,
							 int iCollectionID_,
							 const Common::Data::Pointer& pData_)
	{
		m_pIterator->setMetaData(cProgram_,
								 iCollectionID_,
								 pData_);
	}

	// register used variables
	virtual void useVariable(int iDataID_,
							 int iResetID_ = -1)
	{
		m_pIterator->useVariable(iDataID_, iResetID_);
	}

	// add locker for tuples of the iterator
	virtual void addLocker(Interface::IProgram& cProgram_,
						   const Action::LockerArgument& cArgument_)
	{
		m_pIterator->addLocker(cProgram_,
							   cArgument_);
	}

	// get locker ID for tuples of the iterator
	virtual int getLocker(Interface::IProgram& cProgram_)
	{
		return m_pIterator->getLocker(cProgram_);
	}

	// get data ID for output tuple
	virtual int getOutData(Interface::IProgram& cProgram_)
	{
		return m_pIterator->getOutData(cProgram_);
	}

	// initialize
	virtual void initialize(Interface::IProgram& cProgram_)
	{
		if (!m_cIterator.isInitialized()) {
			m_cIterator.initialize(cProgram_);
			m_pIterator = m_cIterator.getIterator();
			cProgram_.initialize(m_pIterator);
		}
	}
	
	// terminate
	virtual void terminate(Interface::IProgram& cProgram_)
	{
		cProgram_.terminate(m_pIterator);
	}
	
	// do before iteration
	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_)
	{
		return cProgram_.startUp(m_pIterator);
	}
		
	// go to next tuple
	virtual bool next(Interface::IProgram& cProgram_)
	{
		return cProgram_.next(m_pIterator);
	}
	
	// reset iteration to first tuple
	virtual void reset(Interface::IProgram& cProgram_)
	{
		cProgram_.reset(m_pIterator);
	}
	
	// do after iteration
	virtual void finish(Interface::IProgram& cProgram_)
	{
		cProgram_.finish(m_pIterator);
	}

	void serialize(ModArchive& archiver_)
	{
		serializeID(archiver_);
		m_cIterator.serialize(archiver_);
	}


protected:
	// constructor
	ForwordingIterator()
		: Super(),
		  m_pIterator(),
		  m_cIterator()
		{}
	
	// register to program

	ForwordingIterator(Execution::Interface::IIterator* pIterator_)
		: Super(),
		  m_pIterator(pIterator_),
		  m_cIterator(pIterator_->getID())
		{}

private:
	Execution::Interface::IIterator* m_pIterator;
	Execution::Action::IteratorHolder m_cIterator;

};

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_IITERATOR_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
