// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp --
// 
// Copyright (c) 2004, 2005, 2006, 2009, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Plan/Manager.h"
#ifdef USE_OLDER_VERSION
#include "Plan/Boolean.h"
#include "Plan/BulkParameter.h"
#include "Plan/Configuration.h"
#include "Plan/ConstantNode.h"
#include "Plan/RelationInterface.h"
#endif

_SYDNEY_USING
_SYDNEY_PLAN_USING

// プランモジュールの初期化
//static
void
Manager::
initialize()
{
#ifdef USE_OLDER_VERSION
	// 初期化が必要なクラスのinitializeを呼ぶ
	CalcBoolean::initialize();
	ConstantNode::initializeConstant();
	BulkParameter::initializeParser();
#endif
}

// プランモジュールの後処理
//static
void
Manager::
terminate()
{
#ifdef USE_OLDER_VERSION
	// initializeしたクラスのterminateを呼ぶ
	BulkParameter::terminateParser();
	ConstantNode::terminateConstant();
	CalcBoolean::terminate();
#endif
}

//
// Copyright (c) 2004, 2005, 2006, 2009, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
