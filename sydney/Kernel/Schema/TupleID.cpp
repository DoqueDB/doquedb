// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TupleID.cpp -- タプル ID 関連の関数定義
// 
// Copyright (c) 2000, 2001, 2003, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#include "Schema/TupleID.h"
#include "Schema/Manager.h"
#include "Schema/Sequence.h"
#include "Schema/Table.h"

#include "Common/Assert.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::TupleID::assign -- 新しいタプル ID の値を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Table&		table
//			新しいタプル ID の値を得る表
//		Trans::Transaction	cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		生成された新しいタプル ID の値
//
//	EXCEPTIONS

// static
TupleID::Value
TupleID::assign(Table& table, Trans::Transaction& cTrans_)
{
	// タプル ID の値を生成するためのシーケンスを得て、
	// そのシーケンスに次のタプル ID の値を生成させる

	return table.getTupleSequence(cTrans_).getNextValue(cTrans_).getUnsigned();
}

//	FUNCTION public
//	Schema::TupleID::assign -- タプル ID の値を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Table&		table
//			新しいタプル ID の値を得る表
//		Schema::TupleID::Value iID_
//			得たいID
//		Trans::Transaction	cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		指定されたタプル ID の値
//
//	EXCEPTIONS

// static
TupleID::Value
TupleID::assign(Table& table, Value iID_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(&cTrans_);

	// ファイルのタプル ID の値を生成するためのシーケンスを得て、
	// 指定されたIDとシーケンスファイルのIDの整合性をとる

	return table.getTupleSequence(cTrans_).getNextValue(iID_, cTrans_).getUnsigned();
}

// FUNCTION public
//	Schema::TupleID::persist -- タプルIDを永続化する
//
// NOTES
//
// ARGUMENTS
//	Table& cTable_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
TupleID::
persist(Table& cTable_, Trans::Transaction& cTrans_)
{
	cTable_.getTupleSequence(cTrans_).persist(cTrans_);
}

//
// Copyright (c) 2000, 2001, 2003, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
