// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNormUNA.h -- ModNormUNA のクラス定義
// 
// Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
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
#ifndef	__ModNormUNA_H_
#define __ModNormUNA_H_

#include "ModTypes.h"
#include "ModCommonDLL.h"
#include "ModDefaultManager.h"
#include "ModNlpUnaJp/ModNormDLL.h"
#include "ModNlpUnaJp/Module.h"
struct unaKApiHandleT;

_UNA_BEGIN
_UNA_UNAJP_BEGIN


//
// CLASS
// ModNormUNA -- Unicode 正規化のUNAラッパークラスの定義 
//
// NOTES
// Unicode 正規化で用いるUNA（形態素解析系）のラッパークラス。
//
class ModNormDLL ModNormUNA : public ModDefaultObject
{
public:

	// コンストラクタ
	ModNormUNA(const char* const dic, const char* const app,
			   const char* const connect,
			   const char* const unknownTable,
			   const char* const unknownCost,
			   const char* const normalTable);

	// デストラクタ
	~ModNormUNA();

	// アプリ情報取得
	void getAppInfo(const ModUnicodeString& inString,
					ModUnicodeString& outString);
	void getAppInfo(const ModUnicodeString& inString,
					ModUnicodeString& outString, ModBoolean& regUnaWord);

private:
	unaKApiHandleT *unaHandle;				// ハンドル

};
_UNA_UNAJP_END
_UNA_END

#endif // __ModNormUNA_H_
//
// Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
