// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Iterator/File.h --
// 
// Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ITERATOR_FILE_H
#define __SYDNEY_EXECUTION_ITERATOR_FILE_H

#include "Execution/Iterator/Base.h"

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
_SYDNEY_EXECUTION_ITERATOR_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Iterator::File -- Iterator class for file access
//
//	NOTES
//		This class is not constructed directly
class File
	: public Base
{
public:
	typedef Base Super;
	typedef File This;

	// constructor
	static This* create(Interface::IProgram& cProgram_,
						Schema::Table* pSchemaTable_,
						Schema::File* pSchemaFile_,
						const LogicalFile::OpenOption& cOpenOption_,
						const Opt::ExplainFileArgument& cArgument_);
	static This* create(Interface::IProgram& cProgram_,
						int iFileAccessID_,
						const Opt::ExplainFileArgument& cArgument_);
	// destructor
	virtual ~File() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	File() : Super() {}

private:
};

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ITERATOR_FILE_H

//
//	Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
