// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DServer";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "DServer/Manager.h"

#include "DServer/Branch.h"
#include "DServer/Cascade.h"
#include "DServer/DataSource.h"

_SYDNEY_USING
_SYDNEY_DSERVER_USING

// FUNCTION public
//	DServer::Manager::initialize -- DServerモジュールの初期化
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
	// 初期化が必要なクラスのinitializeを呼ぶ
	
	Cascade::initialize();
	Branch::initialize();
}

// FUNCTION public
//	DServer::Manager::terminate -- DServerモジュールの後処理
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
	// initializeしたクラスのterminateを呼ぶ
	
	Branch::terminate();
	Cascade::terminate();
}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
