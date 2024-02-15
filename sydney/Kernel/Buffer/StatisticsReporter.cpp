// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StatisticsReporter.cpp -- 統計情報出力スレッドに関する関数定義
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Buffer";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Buffer/StatisticsReporter.h"
#include "Buffer/Statistics.h"

#include "Common/Message.h"
#include "Exception/ModLibraryError.h"

#define _MOD_EXCEPTION(e) \
	Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)

_SYDNEY_USING
_SYDNEY_BUFFER_USING

namespace 
{
}

//	FUNCTION public
//	Buffer::StatisticsReporter::StatisticsReporter --
//		統計情報出力スレッドを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

StatisticsReporter::StatisticsReporter(unsigned int timeout)
	: DaemonThread(timeout, true)
{
}

//	FUNCTION private
//	Buffer::StatisticsReporter::repeatable --
//		統計情報をログに出力する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
StatisticsReporter::repeatable()
{
	try
	{
		Statistics::printLog();
	}
	catch (Exception::Object& e)
	{
		// Sydneyの例外が発生した場合にはログに記録するだけ
		
		SydErrorMessage << e << ModEndl;
	}
	catch (ModException& e)
	{
		// Modの例外が発生した場合にはログに記録するだけ
		
		SydErrorMessage << _MOD_EXCEPTION(e) << ModEndl;
	}
}

//
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
