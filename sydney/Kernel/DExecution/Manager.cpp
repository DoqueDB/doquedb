// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp --
// 
// Copyright (c) 2009, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DExecution";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DExecution/Manager.h"
#include "DExecution/Externalizable.h"

_SYDNEY_USING
_SYDNEY_DEXECUTION_USING

// FUNCTION public
//	DExecution::Manager::initialize -- DExecutionモジュールの初期化
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Manager::
initialize()
{
	Externalizable::initialize();
}

// FUNCTION public
//	DExecution::Manager::terminate -- DExecutionモジュールの後処理
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Manager::
terminate()
{
	Externalizable::terminate();
}

//
// Copyright (c) 2009, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
