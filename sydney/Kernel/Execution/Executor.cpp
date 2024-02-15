// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Executor.cpp -- エグゼキュータ
// 
// Copyright (c) 1999, 2001, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Executor.h"
#include "Execution/Program.h"
#ifdef USE_OLDER_VERSION
#include "Execution/V1Impl/ExecutorImpl.h"
#endif
#include "Execution/V2Impl/ExecutorImpl.h"

#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN

///////////////////////////////////
// Execution::Executor

// FUNCTION public
//	Execution::Executor::Executor -- コンストラクタ
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS

Executor::
Executor()
 : m_pImpl(0)
{}

// FUNCTION public
//	Execution::Executor::~Executor -- デストラクタ
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS

//virtual
Executor::
~Executor()
{
	delete m_pImpl, m_pImpl = 0;
}

// FUNCTION public
//	Execution::Executor::execute -- 実行する
//
// NOTES
//
// ARGUMENTS
//	Execution::Program& cProgram_
//		実行するプログラム
//	const Common::DataArrayData* pParameter_
//		パラメーター
//
// RETURN
//	なし
//
// EXCEPTIONS

void
Executor::
execute(Program& cProgram_)
{
	if (m_pImpl) delete m_pImpl;

	switch (cProgram_.getImplementationVersion()) {
#ifdef USE_OLDER_VERSION
	case Program::Version::V1:
		{
			// older version
			m_pImpl = new V1Impl::ExecutorImpl;
			m_pImpl->execute(cProgram_);
			break;
		}
#endif
	case Program::Version::V2:
		{
			m_pImpl = new V2Impl::ExecutorImpl;
			m_pImpl->execute(*cProgram_.getProgram());
			break;
		}
	case Program::Version::Unknown:
		{
			// no need to execute
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

_SYDNEY_EXECUTION_END
_SYDNEY_END

//
//	Copyright (c) 1999, 2001, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
