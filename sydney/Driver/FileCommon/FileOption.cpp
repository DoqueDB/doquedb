// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileOption.cpp --
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FileCommon";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "FileCommon/FileOption.h"
#include "FileCommon/Module.h"

#include "Os/SysConf.h"
#include "Os/Memory.h"
#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"

#include "Common/SystemParameter.h"

#include "Buffer/Page.h"

#include "ModUnicodeString.h"

_SYDNEY_USING

namespace {

	namespace _PageSize {

		// パラメータを排他するためのラッチ
		Os::CriticalSection	_latch;

		// 取得したページサイズ
		Os::Memory::Size	_value = 1;

		// 初めて取得しようとしているか
		bool				_first = true;

		// パラメータ名
		ModUnicodeString	_parameter("FileCommon_DefaultPageSize");

	}
}

//
//	FUNCTION
//	FileCommon::FileOption::PageSize::getDefault
//		-- デフォルトのページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		デフォルトのページサイズ
//
//	EXCEPTIONS
//
Os::Memory::Size
FileCommon::FileOption::PageSize::getDefault()
{
	if (_PageSize::_first == true)
	{
		Os::AutoCriticalSection autoLatch(_PageSize::_latch);
		
		if (_PageSize::_first == true)
		{
			int v;
			if (Common::SystemParameter::getValue(_PageSize::_parameter,
												  v) == true)
				_PageSize::_value = v;
				
			// 正しい値に変更する
			_PageSize::_value
				= Buffer::Page::correctSize(_PageSize::_value);

			// 取得した
			_PageSize::_first = false;
		}
	}
	return _PageSize::_value;
}

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
