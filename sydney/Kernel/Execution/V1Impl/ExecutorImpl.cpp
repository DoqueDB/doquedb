// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// V1Impl/ExecutorImpl.cpp -- エグゼキュータ(v1)
// 
// Copyright (c) 1999, 2001, 2003, 2004, 2005, 2006, 2008, 2010, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::V1Impl";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/V1Impl/ExecutorImpl.h"
#include "Execution/Program.h"

#include "Common/AutoCaller.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"

#include "Exception/NotSupported.h"

#include "Plan/Configuration.h"
#include "Plan/RelationInterface.h"
#include "Plan/ScalarInterface.h"
#include "Plan/TableInfo.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_V1IMPL_BEGIN

///////////////////////////////////
// Execution::V1Impl::ExecutorImpl

// FUNCTION public
//	Execution::V1Impl::ExecutorImpl::execute -- execute
//
// NOTES
//
// ARGUMENTS
//	Program& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ExecutorImpl::
execute(Program& cProgram_)
{
	if (!cProgram_.isEmpty()) {
		cProgram_.initialize();

		// 実行対象にならなくてもこの関数をぬけたらすべてのPlanに対してterminateを呼ぶ
		// ★注意★
		// terminateはinitialize前に呼んでも問題ないように実装されていなければならない

		Common::AutoCaller1<Program, bool> terminator(&cProgram_, &Program::terminate, false /* not force */);

		// 確定したプランを逐次実行する
		if (SIZE n = cProgram_.getPlanSize()) {
			SIZE i = 0;
			do {
				execute(*(cProgram_.getPlan(i)->getExecutedPlan()), cProgram_);
			} while (++i < n);
		}

		// Programが正常終了したことをセットする
		cProgram_.succeeded();
	}
}

// FUNCTION public
//	V1Impl::ExecutorImpl::execute -- V2 implementation
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ExecutorImpl::
execute(Interface::IProgram& cProgram_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION private
//	Execution::V1Impl::ExecutorImpl::execute -- execute main
//
// NOTES
//
// ARGUMENTS
//	Plan::RelationInterface& cPlan_
//	Program& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ExecutorImpl::
execute(Plan::RelationInterface& cPlan_, Program& cProgram_)
{
	// 実行するRelationの初期化を行う
	cPlan_.initialize(cProgram_);

#ifndef NO_TRACE
	if (_PLAN_IS_OUTPUT_EXECUTION(Plan::Configuration::TraceLevel::Normal)) {
		OSTRSTREAM stream;
		stream << "Executing:\n";
		cPlan_.toString(stream);
		_PLAN_EXECUTION_MESSAGE << stream.getString() << ModEndl;
	}
#endif

	// nextの結果StatusがInvalidになるまで繰り返す
	do {
		cPlan_.next();
	} while (cPlan_.getStatus()->isValid());
}

_SYDNEY_EXECUTION_V1IMPL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
//	Copyright (c) 1999, 2001, 2003, 2004, 2005, 2006, 2008, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
