// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TransactionInformation.cpp
//	-- データベースごとのトランザクション記述子を管理する
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Trans";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Trans/TransactionInformation.h"

_SYDNEY_USING
_SYDNEY_TRANS_USING

namespace {
}

//
//	FUNCTION public
//	Trans::TransactionInformation::TransactionInformation
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
TransactionInformation::TransactionInformation(Schema::ObjectID::Value dbID_)
	: m_iDatabaseID(dbID_), m_iBeginningID(IllegalID), m_pPrev(0), m_pNext(0)
{
	for (int i = 0; i < CategoryCount; ++i)
	{
		m_pCategoryList[i]
			= new List<Transaction>(&Transaction::_categoryPrev,
									&Transaction::_categoryNext);
	}

	for (int i = 0; i < VersioningCount; ++i)
	{
		m_pVersioningList[i]
			= new List<Transaction>(&Transaction::_versioningPrev,
									&Transaction::_versioningNext);
	}
}

//
//	FUNCTION public
//	Trans::TransactionInformation::~TransactionInformation
//		-- デストラクタ
//
//	NOTES
//
// 	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
TransactionInformation::~TransactionInformation()
{
	for (int i = 0; i < CategoryCount; ++i)
	{
		delete m_pCategoryList[i];
	}

	for (int i = 0; i < VersioningCount; ++i)
	{
		delete m_pVersioningList[i];
	}
}

//
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
