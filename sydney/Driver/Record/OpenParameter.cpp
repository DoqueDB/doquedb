// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenParameter.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Record";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Record/OpenParameter.h"

_SYDNEY_USING

using namespace Record;

//
//	FUNCTION private
//	Record::OpenParameter::OpenOption -- コンストラクタ
//
//	NOTES
//	オープンオプションのコンストラクタ。
//
//	このクラスはレコードファイルのオープンモードの概要をデータメンバに
//	保存する役目がある。
//
//	ARGUMENTS
//	const FileCommon::OpenMode::Mode	iOpenMode_
//		オープンモード
//	const bool 							bEstimate_
//		estimate の真偽値
//	const bool							bSortOrder_
//		sort-order の真偽値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
OpenParameter::OpenParameter(
	const FileCommon::OpenMode::Mode	iOpenMode_,
	const bool 							bEstimate_,
	const bool							bSortOrder_)
	:m_iOpenMode(iOpenMode_),
	 m_bEstimate(bEstimate_),
	 m_bSortOrder(bSortOrder_)
{
	; // do nothing
}


//
//	Copyright (c) 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
