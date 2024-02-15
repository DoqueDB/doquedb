// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Message.h -- メッセージをプリントする
// 
// Copyright (c) 2000, 2007, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_MESSAGE_H
#define __TRMEISTER_COMMON_MESSAGE_H

#include "Common/MessageStream.h"
#include "ModUnicodeChar.h"

_TRMEISTER_BEGIN

namespace Common
{

//
//	CLASS
//	Common::Message -- メッセージ関係のクラス
//
//	NOTES
//	メッセージ関係のクラス。
//
class Message
{
public:
	//ファイル名を切り出す(for UNICODE)
	SYD_COMMON_FUNCTION static const ModUnicodeChar* getBaseName(
											const ModUnicodeChar* pszSrcName_);
	//ファイル名を切り出す(for char)
	SYD_COMMON_FUNCTION static const char* getBaseName(const char* pszSrcName_);

	// メッセージ用のスレッドIDを得る
	SYD_COMMON_FUNCTION static unsigned int getThreadID();
};

}

//
//	DEFINE
//	SydMessage	-- INFOのCommon::MessageStreamを得る
//	SydInfoMessage	-- INFOのCommon::MessageStreamを得る
//	SydErrorMessage	-- ERRORのCommon::MessageStreamを得る
//	SydDebugMessage	-- DEBUGのCommon::MessageStreamを得る
//
//	NOTES
//	ソースファイル中に moduleName, srcFile が定義されているとこが前提。
//
#define SydMessage		\
			_TRMEISTER_MESSAGE("Common_MessageOutputInfo", \
								Common::MessageStreamBuffer::LEVEL_INFO)
#define SydInfoMessage	\
			_TRMEISTER_MESSAGE("Common_MessageOutputInfo", \
								Common::MessageStreamBuffer::LEVEL_INFO)
#define SydErrorMessage	\
			_TRMEISTER_MESSAGE("Common_MessageOutputError", \
								Common::MessageStreamBuffer::LEVEL_ERROR)
#define SydDebugMessage	\
			_TRMEISTER_MESSAGE("Common_MessageOutputDebug", \
								Common::MessageStreamBuffer::LEVEL_DEBUG)

_TRMEISTER_END

#endif //__TRMEISTER_COMMON_MESSAGE_H

//
//	Copyright (c) 2000, 2007, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
