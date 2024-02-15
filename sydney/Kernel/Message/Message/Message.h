// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Message.h -- メッセージライブラリ用ヘッダーファイル
// 
// Copyright (c) 1999, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_COMMON_MESSAGE_H
#define __SYDNEY_COMMON_MESSAGE_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "ModUnicodeChar.h"

_SYDNEY_BEGIN

namespace Message
{

#ifdef SYD_DLL
#ifdef SYD_MESSAGE_EXPORT_FUNCTION
#define SYD_MESSAGE_FUNCTION	SYD_EXPORT
#else
#define SYD_MESSAGE_FUNCTION	SYD_IMPORT
#endif
#else
#define	SYD_MESSAGE_FUNCTION
#endif

//
//	メッセージライブラリとは各国語版のエラーメッセージ等が
//	格納してあるライブラリである。
//	そのライブラリには、構造体 MessageFormatItem の配列とメッセージ番号から
//	メッセージフォーマットを取出すための関数 DBGetMessageFormat が
//	存在している。
//

//
//	STRUCT
//	Common::MessageFormatItem -- メッセージフォーマットの構造体
//
//	NOTES
//	メッセージフォーマットの構造体
//
struct MessageFormatItem
{
	//メッセージ番号
	unsigned int m_uiMessageNumber;
	//メッセージフォーマット
	const char* m_pszMessageFormat;
};

}

_SYDNEY_END

//
//	FUNCTION global
//	DBGetMessageForamt -- メッセージフォーマットを得る
//
//	NOTES
//	メッセージフォーマットを得る
//
//	ARGUMENTS
//	unsigned int uiMessageNumber_
//		メッセージ番号
//
//	RETURN
//	const ModUnicodeChar*
//		メッセージフォーマット	見つからない場合は0を返す
//
//	EXCEPTIONS
//	なし
//
extern "C" SYD_MESSAGE_FUNCTION
const ModUnicodeChar* DBGetMessageFormat(unsigned int uiMessageNumber_);

#endif //__SYDNEY_COMMON_MESSAGE_H

//
// Copyright (c) 1999, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
