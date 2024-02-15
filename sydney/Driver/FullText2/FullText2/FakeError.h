// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FakeError.h --
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

#ifndef __SYDNEY_FULLTEXT2_FAKEERROR_H
#define __SYDNEY_FULLTEXT2_FAKEERROR_H

#include "FullText2/Module.h"
#include "Common/Message.h"
#include "Exception/FakeError.h"
#include "Exception/Unexpected.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//【注意】
//	本来、FakeErrorの文字列はモジュール名から開始する必要があるが、
//	過去に作成された旧転置のテストスクリプトをそのまま利用するため、
//	FullText2:: ではなく、Inverted:: とする
//

#ifdef SYD_FAKE_ERROR
#define _FULLTEXT2_FAKE_ERROR(func) \
{ \
	_SYDNEY_FAKE_ERROR("Inverted::" #func, Exception::Unexpected(moduleName, srcFile, __LINE__)); \
}
#else
#define _FULLTEXT2_FAKE_ERROR(func)
#endif

#ifdef SYD_FAKE_ERROR
//
//【注意】
//	メッセージ出力部分はすべて SYD_FAKE_ERROR で囲むこと
//
#define FakeErrorMessage \
			_TRMEISTER_MESSAGE("SydTest_MessageOutputInfo", \
								Common::MessageStreamBuffer::LEVEL_INFO)
#endif

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_FAKEERROR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
