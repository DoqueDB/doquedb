// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/UndoLog.cpp --
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Opt";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Opt/UndoLog.h"
#include "Opt/Environment.h"
#include "Opt/LogData.h"

#include "Analysis/Operation/UndoLog.h"
#ifdef USE_OLDER_VERSION
#include "Analysis/UndoLog_Delete.h"
#include "Analysis/UndoLog_Insert.h"
#include "Analysis/UndoLog_Update.h"
#endif

#include "Common/Assert.h"

#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_OPT_USING

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Opt::UndoLog::getAnalyzer -- get analyzer
//
// NOTES
//	For old optimizer
//
// ARGUMENTS
//	const Common::DataArrayData* pData_
//
// RETURN
//	Analysis::Analyzer*
//
// EXCEPTIONS

//static
Analysis::Analyzer*
UndoLog::
getAnalyzer(const Common::DataArrayData* pData_)
{
	; _SYDNEY_ASSERT(pData_ && pData_->getCount() > 1);

	// 先頭にStatement単位の処理の種別が書かれている
	int iValue = pData_->getElement(0)->getInt();

	switch (iValue) {
	case LogData::Type::Insert:
		{
			// InsertのUndoを行うプランを作るためのAnalyzer
			return new Analysis::UndoLog_Insert(pData_);
		}
	case LogData::Type::Delete:
	case LogData::Type::Delete_Undo:
		{
			// DeleteのUndoを行うプランを作るためのAnalyzer
			return new Analysis::UndoLog_Delete(pData_);
		}
	case LogData::Type::Update:
	case LogData::Type::Update_Undo:
		{
			// UpdateのUndoはUpdateである
			return new Analysis::UndoLog_Update(pData_);
		}
	default:
		{
			break;
		}
	}
	; _SYDNEY_ASSERT(false);
	_SYDNEY_THROW0(Exception::Unexpected);
}
#endif

// FUNCTION public
//	Opt::UndoLog::getAnalyzer2 -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData* pData_
//	
// RETURN
//	const Analysis::Operation::UndoLog*
//
// EXCEPTIONS

//static
const Analysis::Operation::UndoLog*
UndoLog::
getAnalyzer2(const Common::DataArrayData* pData_)
{
	return Analysis::Operation::UndoLog::create();
}

//
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
