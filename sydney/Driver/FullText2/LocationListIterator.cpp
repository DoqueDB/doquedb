// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LocationListIterator.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/LocationListIterator.h"
#include "FullText2/LocationListManager.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::LocationListIterator::LocationListIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LocationListManager* pListManager_
//		位置情報走査クラス管理クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LocationListIterator::LocationListIterator(LocationListManager* pListManager_)
	: m_pListManager(pListManager_), m_pNext(0)
{
}

//
//	FUNCTION public
//	FullText2::LocationListIterator::~LocationListIterator -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LocationListIterator::~LocationListIterator()
{
}

//
//	FUNCTION public
//	FullText2::LocationListIterator::release	-- 解放する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		フリーリストにつなげた場合は true、
//		呼び出し側で delete しなくてはならない場合は false を返す
//
//	EXCEPTIONS
//
bool
LocationListIterator::release()
{
	if (m_pListManager)
	{
		m_pListManager->addFree(this);
		return true;
	}
	return false;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
