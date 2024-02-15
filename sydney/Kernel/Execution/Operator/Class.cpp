// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Operator/Class.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Operator";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Operator/Class.h"
#include "Execution/Operator/Assign.h"
#include "Execution/Operator/BitSet.h"
#include "Execution/Operator/CheckCancel.h"
#include "Execution/Operator/Clear.h"
#include "Execution/Operator/FileFetch.h"
#include "Execution/Operator/FileGetProperty.h"
#include "Execution/Operator/FileOperation.h"
#include "Execution/Operator/Generator.h"
#include "Execution/Operator/Iterate.h"
#include "Execution/Operator/Limit.h"
#include "Execution/Operator/Locator.h"
#include "Execution/Operator/Locker.h"
#include "Execution/Operator/Logger.h"
#include "Execution/Operator/Output.h"
#include "Execution/Operator/SetNull.h"
#include "Execution/Operator/SystemColumn.h"
#include "Execution/Operator/Thread.h"
#include "Execution/Operator/Throw.h"
#include "Execution/Operator/Transaction.h"
#include "Execution/Operator/UndoLog.h"

#include "Execution/Externalizable.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_OPERATOR_USING

namespace
{
	// CONST
	//	_iSubModule --  submodule number
	//
	// NOTES
	const int _iSubModule = Externalizable::SubModule::Operator;
}

// FUNCTION public
//	Operator::Class::getClassInstance -- get instance from classid
//
// NOTES
//
// ARGUMENTS
//	int iClassID_
//	
// RETURN
//	Common::Externalizable*
//
// EXCEPTIONS

// static
Common::Externalizable*
Class::
getClassInstance(int iClassID_)
{
	Common::Externalizable* pObject = 0;

	Category::Value eCategory =
		static_cast<Category::Value>(iClassID_ - _iSubModule);

	switch ( eCategory ) {
	case Category::FileFetch:
	case Category::FileGetLocator:
		{
			pObject = FileFetch::getInstance(eCategory);
			break;
		}
	case Category::FileInsert:
	case Category::FileUpdate:
	case Category::FileExpunge:
	case Category::FileUndoUpdate:
	case Category::FileUndoExpunge:
		{
			pObject = FileOperation::getInstance(eCategory);
			break;
		}
	case Category::FileGetProperty:
		{
			pObject = FileGetProperty::getInstance(eCategory);
			break;
		}
	case Category::BitSetIntersect:
	case Category::BitSetIntersectUnlock:
	case Category::BitSetUnion:
	case Category::BitSetUnionUnlock:
	case Category::BitSetDifference:
	case Category::BitSetDifferenceUnlock:
	case Category::BitSetCollection:
		{
			pObject = BitSet::getInstance(eCategory);
			break;
		}
	case Category::IterateAll:
	case Category::IterateOnce:
	case Category::IterateNestedAll:
	case Category::IterateRuntimeStartup:		
		{
			pObject = Iterate::getInstance(eCategory);
			break;
		}
	case Category::Limit:
	case Category::LimitPartial:
		{
			pObject = Limit::getInstance(eCategory);
			break;
		}
	case Category::Output:
	case Category::ArrayOutput:		
		{
			pObject = Output::getInstance(eCategory);
			break;
		}
	case Category::Assign:
		{
			pObject = Assign::getInstance(eCategory);
			break;
		}
	case Category::ColumnMetaData:
	case Category::FileSize:
	case Category::IndexHint:
	case Category::PrivilegeFlag:
	case Category::PrivilegeObjectType:
	case Category::PrivilegeObjectID:
	case Category::PartitionCategory:
	case Category::FunctionRoutine:
	case Category::DatabasePath:
	case Category::DatabaseMasterURL:
		{
			pObject = SystemColumn::getInstance(eCategory);
			break;
		}
	case Category::CheckCancel:
		{
			pObject = CheckCancel::getInstance(eCategory);
			break;
		}
	case Category::LockTable:
	case Category::LockConvertTable:
	case Category::LockTuple:
	case Category::LockBitSet:
	case Category::LockConvertTuple:
	case Category::UnlockTuple:
		{
			pObject = Locker::getInstance(eCategory);
			break;
		}
	case Category::GeneratorRowID:
	case Category::GeneratorIdentity:
	case Category::GeneratorIdentityByInput:
	case Category::GeneratorRecoveryRowID:
	case Category::GeneratorRecoveryIdentity:
		{
			pObject = Generator::getInstance(eCategory);
			break;
		}
	case Category::SetNull:
		{
			pObject = SetNull::getInstance(eCategory);
			break;
		}
	case Category::UndoLogSingle:
	case Category::UndoLogDouble:
	case Category::UndoLogPrepare:
	case Category::UndoLogReset:
		{
			pObject = UndoLog::getInstance(eCategory);
			break;
		}
	case Category::Logger:
		{
			pObject = Logger::getInstance(eCategory);
			break;
		}
	case Category::Throw:
		{
			pObject = Throw::getInstance(eCategory);
			break;
		}
	case Category::StartBatch:
		{
			pObject = Transaction::getInstance(eCategory);
			break;
		}
	case Category::LocatorLength:
	case Category::LocatorGet:
	case Category::LocatorAppend:
	case Category::LocatorReplace:
	case Category::LocatorTruncate:
		{
			pObject = Locator::getInstance(eCategory);
			break;
		}
	case Category::ThreadStart:
	case Category::ThreadJoin:
		{
			pObject = Thread::getInstance(eCategory);
			break;
		}
	case Category::Clear:
	case Category::ArrayClear:		
		{
			pObject = Clear::getInstance(eCategory);
			break;
		}		
	default:
		{
			break;
		}
	}
	return pObject;
}

// FUNCTION public
//	Operator::Class::getClassID -- get classid
//
// NOTES
//
// ARGUMENTS
//	int iBase_
//	
// RETURN
//	int
//
// EXCEPTIONS

//static
int
Class::
getClassID(Category::Value eCategory_)
{
	return Externalizable::getClassID(eCategory_ + _iSubModule);
}

//
//	Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
