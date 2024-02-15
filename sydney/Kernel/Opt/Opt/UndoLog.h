// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/UndoLog.h --
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

#ifndef __SYDNEY_OPT_UNDOLOG_H
#define __SYDNEY_OPT_UNDOLOG_H

#include "Opt/Module.h"
#include "Opt/Declaration.h"
#include "Opt/Environment.h"

#include "Analysis/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

////////////////////////////////////
//	CLASS
//	Opt::UndoLog -- holding undolog data
//
//	NOTES
class UndoLog
	: public Common::Object
{
public:
	typedef UndoLog This;
	typedef Common::Object Super;

	struct Type
	{
		enum Value {
			Insert,
			Expunge,
			Update,
			UndoExpunge,
			UndoUpdate,
			ValueNum
		};
	};

	struct Format
	{
		enum Value
		{
			UndoLogType = 0,
			FileID,
			FieldID1,
			Data1,
			FieldID2,
			SingleValueNum = FieldID2,
			Data2,
			ValueNum,
			DoubleValueNum = ValueNum
		};
	};

	// get analyzer
#ifdef USE_OLDER_VERSION
	static Analysis::Analyzer*
					getAnalyzer(const Common::DataArrayData* pData_);
#endif
	static const Analysis::Operation::UndoLog*
					getAnalyzer2(const Common::DataArrayData* pData_);

protected:
private:
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_UNDOLOG_H

//
//	Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
