// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DBGetFileDriver.cpp -- Ｂ木ファイルドライバを得る関数
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "LogicalFile/FileDriver.h"
#include "Btree2/FileDriver.h"

_SYDNEY_USING

#ifdef SYD_DLL

//
//	FUNCTION global
//	DBGetFileDriver -- Ｂ木ファイルのファイルドライバのインスタンスを得る
//
//	NOTES
//	Ｂ木ファイルのファイルドライバのインスタンスを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Logical::FileDriver*
//		Ｂ木ファイルのファイルドライバ
//
LogicalFile::FileDriver*
DBGetFileDriver()
{
	return new Btree2::FileDriver;
}

#endif

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
