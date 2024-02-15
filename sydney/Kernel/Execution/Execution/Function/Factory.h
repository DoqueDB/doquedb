// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Function/Factory.h --
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

#ifndef __SYDNEY_EXECUTION_FUNCTION_FACTORY_H
#define __SYDNEY_EXECUTION_FUNCTION_FACTORY_H

#include "Execution/Function/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Interface/IAction.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Function::Factory -- factory class for function executor
//
//	NOTES
//		This class is not constructed
class Factory
{
public:
	///////////////////
	// constructors
	///////////////////

	// 0-arguments
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  int iDataID_);
	// 1-argument
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  int iOperandID_,
									  int iDataID_);
	// 2-arguments
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  const PAIR<int, int>& cOperandID_,
									  int iDataID_);
	// N-arguments
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  const VECTOR<int>& vecOperandID_,
									  int iDataID_);
	// 1-argument 1-option
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  int iOperandID_,
									  int iOptionID_,
									  int iDataID_);
	// 2-arguments 1-option
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  const PAIR<int, int>& cOperandID_,
									  int iOptionID_,
									  int iDataID_);
	// N-arguments 1-option
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  const VECTOR<int>& vecOperandID_,
									  int iOptionID_,
									  int iDataID_);
	// 1-argument 2-options
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  int iOperandID_,
									  const PAIR<int, int>& cOptionID_,
									  int iDataID_);
	// 2-arguments 2-options
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  const PAIR<int, int>& cOperandID_,
									  const PAIR<int, int>& cOptionID_,
									  int iDataID_);
	// N-arguments 2-options
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  const VECTOR<int>& vecOperandID_,
									  const PAIR<int, int>& cOptionID_,
									  int iDataID_);
	// 1-argument N-options
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  int iOperandID_,
									  const VECTOR<int>& vecOptionID_,
									  int iDataID_);
	// 2-arguments N-options
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  const PAIR<int, int>& cOperandID_,
									  const VECTOR<int>& vecOptionID_,
									  int iDataID_);
	// N-arguments N-options
	static Interface::IAction* create(Interface::IProgram& cProgram_,
									  Interface::IIterator* pIterator_,
									  LogicalFile::TreeNodeInterface::Type eType_,
									  const VECTOR<int>& vecOperandID_,
									  const VECTOR<int>& vecOptionID_,
									  int iDataID_);

protected:
private:
	// never constructed
	Factory();
	~Factory();
};

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_FUNCTION_FACTORY_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
