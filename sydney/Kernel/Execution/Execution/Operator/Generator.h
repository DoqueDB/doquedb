// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Operator/Generator.h --
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

#ifndef __SYDNEY_EXECUTION_OPERATOR_GENERATOR_H
#define __SYDNEY_EXECUTION_OPERATOR_GENERATOR_H

#include "Execution/Operator/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Interface/IAction.h"

#include "Common/Object.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Column;
	class Table;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Operator::Generator -- operator class for generator finish
//
//	NOTES
//		This class is not constructed directly
class Generator
	: public Interface::IAction
{
public:
	typedef Interface::IAction Super;
	typedef Generator This;

	struct RowID {
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							Schema::Table* pSchemaTable_,
							int iDataID_);
		struct Recovery {
			static This* create(Interface::IProgram& cProgram_,
								Interface::IIterator* pIterator_,
								Schema::Table* pSchemaTable_,
								int iDataID_);
		};
	};
	struct Identity {
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							Schema::Column* pSchemaColumn_,
							int iDataID_);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							Schema::Column* pSchemaColumn_,
							int iInputID_,
							int iDataID_);
		struct Recovery {
			static This* create(Interface::IProgram& cProgram_,
								Interface::IIterator* pIterator_,
								Schema::Column* pSchemaColumn_,
								int iDataID_);
		};
	};

	// destructor
	virtual ~Generator() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	Generator() : Super() {}

private:
};

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_OPERATOR_GENERATOR_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
