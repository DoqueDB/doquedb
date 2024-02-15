// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Executor.h -- エグゼキュータ
// 
// Copyright (c) 1999, 2002, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_EXECUTOR_H
#define __SYDNEY_EXECUTION_EXECUTOR_H

#include "Execution/Module.h"
#include "Execution/Declaration.h"

#include "Common/Object.h"

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}

_SYDNEY_EXECUTION_BEGIN

class Program;

//
//	CLASS
//	Execution::Executor -- エグゼキュータ
//
//	NOTES
//
class SYD_EXECUTION_FUNCTION Executor : public Common::Object
{
public:
	// コンストラクタ
	Executor();
	// デストラクタ
	~Executor();

	// 逐次実行
	void execute(Program& cProgram_);

protected:
private:
	// implementation class
	Interface::IExecutor* m_pImpl;
};

_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_EXECUTOR_H

//
//	Copyright (c) 1999, 2002, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
