// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Operator/FileOperation.h --
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

#ifndef __SYDNEY_EXECUTION_OPERATOR_FILEOPERATION_H
#define __SYDNEY_EXECUTION_OPERATOR_FILEOPERATION_H

#include "Execution/Operator/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Interface/IAction.h"

#include "Common/Object.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
	class OpenOption;
}
namespace Schema
{
	class File;
	class Table;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Operator::FileOperation -- operator class for operationing file
//
//	NOTES
//		This class is not constructed directly
class FileOperation
	: public Interface::IAction
{
public:
	typedef Interface::IAction Super;
	typedef FileOperation This;

	struct Insert
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							Schema::Table* pSchemaTable_,
							Schema::File* pSchemaFile_,
							const LogicalFile::OpenOption& cOpenOption_,
							int iDataID_);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iFileAccessID_,
							int iDataID_);
	};
	struct Update
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							Schema::Table* pSchemaTable_,
							Schema::File* pSchemaFile_,
							const LogicalFile::OpenOption& cOpenOption_,
							int iKeyID_,
							int iDataID_);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iFileAccessID_,
							int iKeyID_,
							int iDataID_);
	};

	struct Expunge
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							Schema::Table* pSchemaTable_,
							Schema::File* pSchemaFile_,
							const LogicalFile::OpenOption& cOpenOption_,
							int iKeyID_);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iFileAccessID_,
							int iKeyID_);
	};

	struct UndoUpdate
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							Schema::Table* pSchemaTable_,
							Schema::File* pSchemaFile_,
							const LogicalFile::OpenOption& cOpenOption_,
							int iKeyID_);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iFileAccessID_,
							int iKeyID_);
	};

	struct UndoExpunge
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							Schema::Table* pSchemaTable_,
							Schema::File* pSchemaFile_,
							const LogicalFile::OpenOption& cOpenOption_,
							int iKeyID_);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iFileAccessID_,
							int iKeyID_);
	};

	// destructor
	virtual ~FileOperation() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	FileOperation() : Super() {}

private:
};

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_OPERATOR_FILEOPERATION_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
