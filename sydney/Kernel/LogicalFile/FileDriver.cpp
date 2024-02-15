// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.cpp -- 論理ファイルドライバの基底クラス
// 
// Copyright (c) 1999, 2001, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "LogicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "LogicalFile/FileDriver.h"

_SYDNEY_USING

//
//	FUNCTION public
//	LogicalFile::FileDriver::FileDriver -- コンストラクタ
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
LogicalFile::FileDriver::FileDriver()
{
}

//
//	FUNCTION public
//	LogicalFile::FileDriver::~FileDriver -- デストラクタ
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
LogicalFile::FileDriver::~FileDriver()
{
}

//
//	FUNCTION public
//	LogicalFile::FileDriver::initialize -- 論理ファイルドライバの初期化を行う
//
//	NOTES
//	論理ファイルドライバの初期化を行う。
//	ここでは何も行わない。必要なら継承先で実装する。
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
void
LogicalFile::FileDriver::initialize()
{
}

//
//	FUNCTION public
//	LogicalFile::FileDriver::terminate -- 論理ファイルドライバの後処理を行う
//
//	NOTES
//	論理ファイルドライバの後処理を行う。
//	ここでは何も行わない。必要なら継承先で実装する。
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
void
LogicalFile::FileDriver::terminate()
{
}


//
//	FUNCTION public
//	LogicalFile::FileDriver::stop -- 論理ファイルドライバの処理を止める
//
//	NOTES
//	論理ファイルドライバの後処理を行う。
//	ここでは何も行わない。必要なら継承先で実装する。
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
void
LogicalFile::FileDriver::stop()
{
}


//
//	FUNCTION public
//	LogicalFile::FileDriver::start -- 論理ファイルドライバの処理を再開する
//
//	NOTES
//	論理ファイルドライバ処理を再開する。
//	ここでは何も行わない。必要なら継承先で実装する。
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
void
LogicalFile::FileDriver::start()
{
}


//
//	FUNCTION public
//	LogicalFile::FileDriver::prepareTerminate -- 論理ファイルドライバの後処理の前準備を行う
//
//	NOTES
//	論理ファイルドライバの後処理を行う。
//	ここでは何も行わない。必要なら継承先で実装する。
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
void
LogicalFile::FileDriver::prepareTerminate()
{
}

//
//	Copyright (c) 1999, 2001, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
