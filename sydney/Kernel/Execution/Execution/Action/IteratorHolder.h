// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/IteratorHolder.h --
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

#ifndef __SYDNEY_EXECUTION_ACTION_ITERATORHOLDER_H
#define __SYDNEY_EXECUTION_ACTION_ITERATORHOLDER_H

#include "Execution/Action/Module.h"
#include "Execution/Declaration.h"

#include "Common/Object.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Action::IteratorHolder -- wrapping class for iterators
//
//	NOTES

//////////////////////////////////////////////////////
// IteratorHolder
//////////////////////////////////////////////////////
class IteratorHolder
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef IteratorHolder This;

	IteratorHolder()
		: Super(),
		  m_iID(-1),
		  m_pIterator(0)
	{}
	IteratorHolder(int iID_)
		: Super(),
		  m_iID(iID_),
		  m_pIterator(0)
	{}
	IteratorHolder(const This& cOther_)
		: Super(),
		  m_iID(cOther_.m_iID),
		  m_pIterator(cOther_.m_pIterator)
	{}

	~IteratorHolder() {}

	void explain(Opt::Environment* pEnvironment_,
				 Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_);
	void initialize(Interface::IProgram& cProgram_);
	void terminate(Interface::IProgram& cProgram_);

	// for serialize
	void serialize(ModArchive& archiver_);

	// is valid holder?
	bool isValid() {return m_iID >= 0;}

	// accessor
	void setID(int iID_) {m_iID = iID_;}
	Interface::IIterator* getIterator() {return m_pIterator;}
	void clearIterator() {m_pIterator = 0;}

	Interface::IIterator* operator->() {return getIterator();}
	Interface::IIterator& operator*() {return *getIterator();}

	// check initialized
	bool isInitialized() const {return m_pIterator != 0;}

protected:
private:
	int m_iID;
	Interface::IIterator* m_pIterator;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_ITERATORHOLDER_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
