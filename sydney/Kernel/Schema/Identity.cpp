// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Identity.cpp -- IDENTITY COLUMN 関連の関数定義
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Schema/Identity.h"
#include "Schema/Column.h"
#include "Schema/Manager.h"
#include "Schema/Sequence.h"

#include "Common/Assert.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::Identity::assign -- 新しいIDENTITY COLUMNの値を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Column&		cColumn_
//			値を生成するIDENTITY COLUMN
//		Trans::Transaction	cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		生成された新しいIDENTITY COLUMNの値
//
//	EXCEPTIONS

// static
Identity::Value
Identity::assign(Column& cColumn_, Trans::Transaction& cTrans_)
{
	// IDENTITY COLUMNの値を生成するためのシーケンスを得て、
	// そのシーケンスに次のIDENTITY COLUMNの値を生成させる

	return cColumn_.getSequence(cTrans_).getNextValue(cTrans_, Invalid).getSigned();
}

//	FUNCTION public
//	Schema::Identity::assign -- IDENTITY COLUMNの値を生成する
//
//	NOTES
//		GetMaxフラグが立っているときにシーケンスの内容が
//		常に最大値であるようにするようにするときに使う
//
//	ARGUMENTS
//		Schema::Column&		cColumn_
//			値を生成するIDENTITY COLUMN
//		Schema::Identity::Value iID_
//			得たいID
//		Trans::Transaction	cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		指定されたIDENTITY COLUMNの値
//
//	EXCEPTIONS

// static
Identity::Value
Identity::assign(Column& cColumn_, Value iID_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(&cTrans_);

	// ファイルのIDENTITY COLUMNの値を生成するためのシーケンスを得て、
	// 指定されたIDとシーケンスファイルのIDの整合性をとる

	return cColumn_.getSequence(cTrans_).getNextValue(iID_, cTrans_, Invalid).getSigned();
}

// FUNCTION public
//	Schema::Identity::persist -- IDENTITY COLUMNの値を永続化する
//
// NOTES
//
// ARGUMENTS
//	Column& cColumn_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Identity::
persist(Column& cColumn_, Trans::Transaction& cTrans_)
{
	cColumn_.getSequence(cTrans_).persist(cTrans_);
}

// FUNCTION public
//	Schema::Identity::isGetMax -- 常に最大値を生成するかを得る
//
// NOTES
//
// ARGUMENTS
//	Column& cColumn_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Identity::
isGetMax(Column& cColumn_, Trans::Transaction& cTrans_)
{
	return cColumn_.getSequence(cTrans_).isGetMax();
}

//
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
