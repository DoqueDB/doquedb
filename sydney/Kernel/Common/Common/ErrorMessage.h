// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorMessage.h -- エラーメッセージを作成するクラス
// 
// Copyright (c) 1999, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_ERRORMESSAGE_H
#define __TRMEISTER_COMMON_ERRORMESSAGE_H

#include "Common/Object.h"
#include "Common/ExceptionMessage.h"
#include "ModCharString.h"
#include "ModMap.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"
#include "ModCriticalSection.h"

_TRMEISTER_BEGIN

namespace Common
{

//
//	CLASS
//	Common::ErrorMessage -- エラーメッセージを作成する
//
//	NOTES
//	エラーメッセージを作成する。各国語に対応したエラーメッセージクラスは
//	すべてこのクラスを基底クラスとして作成される。
//
class SYD_COMMON_FUNCTION ErrorMessage : public Common::Object
{
public:
	//コンストラクタ
	ErrorMessage(const ModCharString& cstrLibrary_);
	//デストラクタ
	virtual ~ErrorMessage();

	//初期化を行う
	void initialize();
	//終了処理を行う
	void terminate();

	//エラーメッセージを作成する
	ModUnicodeChar* makeMessage(ModUnicodeChar* pszMessage_,
								unsigned int uiMessageNumber_,
								const ModUnicodeChar* pszArgument_) const;

private:
	//エラーメッセージフォーマットを得る
	const ModUnicodeChar* getFormat(unsigned int uiMessageNumber_) const;

	//メッセージフォーマットを得る関数へのポインタ
	const ModUnicodeChar* (*m_pGetFormatFunction)(unsigned int);

	//ライブラリ名
	ModCharString m_cstrLibraryName;
};

}

_TRMEISTER_END

#endif //__TRMEISTER_COMMON_ERRORMESSAGE_H

//
//	Copyright (c) 1999, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

