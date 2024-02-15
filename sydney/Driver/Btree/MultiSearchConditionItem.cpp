// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiSearchConditionItem.cpp --
//		Ｂ＋木索引による複合検索条件解析データクラスの実現ファイル
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Btree/MultiSearchConditionItem.h"

_SYDNEY_USING

using namespace Btree;

//
//	FUNCTION public
//	Btree::MultiSearchConditionItem::MultiSearchConditionItem --
//		コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MultiSearchConditionItem::MultiSearchConditionItem()
	: Common::Object(),
	  m_cstrStart(""),
	  m_iStartOpe(-1),
	  m_cstrStop(""),
	  m_iStopOpe(-1)
{
}

//
//	FUNCTION public
//	Btree::MultiSearchConditionItem::~MultiSearchConditionItem --
//		デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MultiSearchConditionItem::~MultiSearchConditionItem()
{
}

//
//	Copyright (c) 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
