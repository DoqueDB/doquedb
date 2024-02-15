// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Transaction.h --
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

#ifndef __SYDNEY_EXECUTION_UTILITY_TRANSACTION_H
#define __SYDNEY_EXECUTION_UTILITY_TRANSACTION_H

#include "Execution/Utility/Module.h"
#include "Execution/Declaration.h"

#include "Lock/Name.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
	class AutoLogicalFile;
}
namespace Schema
{
	class File;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_UTILITY_BEGIN

///////////////////////////////////////////////////////////////
// CLASS
//	Execution::Utility::Transaction -- utility functions for transaction
//
// NOTES
class Transaction
{
public:
	// get adequate lock
	static bool getAdequateLock(Trans::Transaction& cTrans_,
								Lock::Name::Category::Value eLocked_,
								bool bReadOnly_,
								bool bBatchMode_,
								Action::LockerArgument& cArgument_);
	static bool getAdequateLock(Trans::Transaction& cTrans_,
								Lock::Name::Category::Value eLocked_,
								Lock::Name::Category::Value eManipulated_,
								bool bReadOnly_,
								bool bBatchMode_,
								Action::LockerArgument& cArgument_);

	// attach logicalfile with latch specification
	static void attachLogicalFile(Trans::Transaction& cTrans_,
								  Schema::File& cSchemaFile_,
								  LogicalFile::AutoLogicalFile& cLogicalFile_);
protected:
private:
	// never create instance
	Transaction();
	~Transaction();
};

_SYDNEY_EXECUTION_END
_SYDNEY_EXECUTION_UTILITY_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_UTILITY_TRANSACTION_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
