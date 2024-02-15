// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/OpenMP.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_OPENMP_H
#define __SYDNEY_EXECUTION_ACTION_OPENMP_H

#include "Execution/Action/ActionList.h"
#include "Execution/Interface/IObject.h"

#include "Utility/OpenMP.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Action::OpenMP -- openMP execution
//
//	NOTES

class OpenMP
	: public Utility::OpenMP,
	: public Interface::IObject
{
public:
	typedef Utility::OpenMP Super;
	typedef OpenMP This;

	// constructor
	OpenMP()
		: Super(),
		  m_vecActionList(),
		  m_pProgram(0)
	{}
	// descructor
	~OpenMP() {}

	void explain(Opt::Environment* pEnvironment_,
				 Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_);
	void initialize(Interface::IProgram& cProgram_);
	void terminate(Interface::IProgram& cProgram_);
	Action::Status::Value execute(Interface::IProgram& cProgram_);
	void accumulate(Interface::IProgram& cProgram_);
	void finish(Interface::IProgram& cProgram_);
	void setPointer(int iPosition_);

	// for serialize
	static This* getInstance(int iCategory_);

///////////////////////////////
// Common::Externalizable
	int getClassID() const;

///////////////////////////////
// ModSerializer
	void serialize(ModArchive& archiver_);

/////////////////////////
// Utility::OpenMP::
	virtual void prepare();
	virtual void parallel();
	virtual void dispose();

protected:
private:
	VECTOR<ActionList> m_vecActionList;

	Interface::IProgram* m_pProgram;
};


_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_OPENMP_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
