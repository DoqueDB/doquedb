// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Estimate.cpp -- オプティマイザーとドライバーで見積りに使うクラス
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "LogicalFile";
	const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "LogicalFile/Estimate.h"

#include "Common/Assert.h"
#include "Common/SystemParameter.h"

#include "Os/AutoCriticalSection.h"

#include "ModCharString.h"

_SYDNEY_USING

using namespace LogicalFile;

namespace {
	//
	//	CONST
	//		ParameterKey --
	//
	//	NOTES
	//		Estimate::Typeからパラメーターキーを得る配列
	//
	const char* ParameterKey[Estimate::TotalNumber] = {
		"FileTransferSpeed",
		"MemoryTransferSpeed",
		"SocketTransferSpeed"
	};

	//
	//	CONST
	//		DefaultValue --
	//
	//	NOTES
	//		Estimate::Typeに対応するデフォルト値
	//		根拠のある数字ではない
	//
	const int DefaultValue[Estimate::TotalNumber] = {
		1024,								// File
		1024*1024,							// Memory
		128									// Socket
	};

	//
	//	VARIABLE
	//		$$::_cLatch --
	//
	//	NOTES
	//
	Os::CriticalSection _cLatch;

	//
	//	VARIABLE
	//		$$::_veciValue --
	//
	//	NOTES
	//
	int _veciValue[Estimate::TotalNumber];

	//
	//	VARIABLE
	//		$$::_bReadFlag --
	//
	//	NOTES
	//
	bool _bReadFlag = false;
}

//
//	FUNCTION public static
//		LogicalFile::Estimate::getTransferSpeed
//			-- メディアの転送速度パラメーターを得る
//
//	NOTES
//		メディアの転送速度パラメーターを得る
//
//	ARGUMENTS
//		Type eType_
//
//	RETURN
//		int
//
//	EXCEPTIONS
//		???
//
int
Estimate::
getTransferSpeed(Type eType_)
{
	if (!_bReadFlag) {
		Os::AutoCriticalSection l(_cLatch);
		if (!_bReadFlag) {

			for (int i = 0; i < Estimate::TotalNumber; ++i) {
				int iValue = DefaultValue[i];
				ModCharString	keyStr(ParameterKey[i]);
				bool bResult = Common::SystemParameter::getValue(keyStr, iValue);
				; _SYDNEY_ASSERT(bResult || iValue == DefaultValue[i]);
				_veciValue[i] = iValue;
			}
			_bReadFlag = true;
		}
	}
	return _veciValue[eType_];
}

//
//	Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
