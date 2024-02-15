// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Config.cpp -- Ｂ＋木ファイルドライバの設定関連の実現ファイル
// 
// Copyright (c) 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Btree/Config.h"

#include "Exception/Unexpected.h"

#include "Common/SystemParameter.h"

#include "Os/CriticalSection.h"

_SYDNEY_USING

using namespace Btree;

namespace
{

namespace _Config
{
	// 設定値取得の排他制御用ラッチ
	Os::CriticalSection	Latch;

} // end of namespace _Config

} // end of global namespace

#ifdef OBSOLETE
//
//	FUNCTION
//	Btree::Config::get --
//		設定をすべてシステムパラメータから読み出す
//
//	NOTES
//	設定をすべてシステムパラメータから読み出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
Config::get()
{
}
#endif //OBSOLETE

#ifdef OBSOLETE
//
//	FUNCTION
//	Btree::Config::reset -- 設定のリセットを行う
//
//	NOTES
//	設定のリセットを行う。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
Config::reset()
{
}
#endif //OBSOLETE

//
//	Copyright (c) 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
