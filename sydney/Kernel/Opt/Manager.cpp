// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp --
// 
// Copyright (c) 2004, 2023 Ricoh Company, Ltd.
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
#include "Opt/Manager.h"
#include "Opt/Planner.h"

_SYDNEY_USING
_SYDNEY_OPT_USING

// Optモジュールの初期化
//static
void
Manager::
initialize()
{
	// 初期化が必要なクラスのinitializeを呼ぶ
	Planner::initialize();
}

// Optモジュールの後処理
//static
void
Manager::
terminate()
{
	// initializeしたクラスのterminateを呼ぶ
	Planner::terminate();
}

//
// Copyright (c) 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
