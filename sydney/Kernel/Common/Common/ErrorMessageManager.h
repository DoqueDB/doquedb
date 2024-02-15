// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorMessageManager.h -- 各国語のエラーメッセージを管理する
// 
// Copyright (c) 2000, 2001, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_ERRORMESSAGEMANAGER_H
#define __TRMEISTER_COMMON_ERRORMESSAGEMANAGER_H

#include "Common/Module.h"
#include "Os/CriticalSection.h"
#include "ModCharString.h"
#include "ModMap.h"
#include "ModUnicodeString.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

class ErrorMessage;

//	CLASS
//	Common::ErrorMessageManager -- 各国語のエラーメッセージを管理する
//
//	NOTES

class ErrorMessageManager
{
public:
	//
	//	TYPEDEF
	//	Common::ErrorMessage::MessageMap -- メッセージマップ
	//
	//	NOTES
	//	各国語のCommon::ErrorMessageを格納するマップ
	//
	typedef ModMap<ModCharString, ErrorMessage*, ModLess<ModCharString> >
		MessageMap;

	//言語を設定する
	SYD_COMMON_FUNCTION
	static void setLanguage(const ModCharString& cstrLanguage_);
	//メッセージクラスを得る
	SYD_COMMON_FUNCTION
	static ErrorMessage* getMessage();
	//エラーメッセージを作成する
	SYD_COMMON_FUNCTION
	static ModUnicodeChar* makeErrorMessage(ModUnicodeChar* pszBuffer_,
											unsigned int uiErrorNumber_,
											const ModUnicodeChar* pszArgument_);

	// 初期化をする
	static void initialize();
	// 後処理をする
	static void terminate();

private:
	//各国語のCommon::ErrorMessageを格納するマップ
	static MessageMap m_mapMessage;
	//排他制御用のクリティカルセクション
	static Os::CriticalSection m_cCriticalSection;
	//セットされたメッセージ
	static ErrorMessage* m_pMessage;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif //__TRMEISTER_COMMON_ERRORMESSAGEMANAGER_H

//
//	Copyright (c) 2000, 2001, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
