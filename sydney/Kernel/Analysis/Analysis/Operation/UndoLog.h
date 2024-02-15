// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operation/UndoLog.h --
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

#ifndef __SYDNEY_ANALYSIS_OPERATION_UNDOLOG_H
#define __SYDNEY_ANALYSIS_OPERATION_UNDOLOG_H

#include "Analysis/Operation/Module.h"
#include "Analysis/Interface/IAnalyzer.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}

_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_OPERATION_BEGIN

class UndoLog
	: public Interface::IAnalyzer
{
public:
	typedef UndoLog This;
	typedef Interface::IAnalyzer Super;

	// constructor
	static const This* create();
	// destructor
	virtual ~UndoLog() {}

	// generate relation for undoLog
	virtual Plan::Interface::IRelation*
					getRelation(Opt::Environment& cEnvironment_,
								const Common::DataArrayData* pUndoLog_) const = 0;

protected:
	// constructor
	UndoLog() : Super() {}
private:
};

_SYDNEY_ANALYSIS_OPERATION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

#endif // __SYDNEY_ANALYSIS_OPERATION_UNDOLOG_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
