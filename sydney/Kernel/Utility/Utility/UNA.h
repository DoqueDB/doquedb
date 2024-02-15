// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UNA.h --	UNA 関連のテンプレートクラス定義、関数宣言
// 
// Copyright (c) 2004, 2006, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_UTILITY_UNA_H
#define	__TRMEISTER_UTILITY_UNA_H

#include "Utility/Module.h"

#include "ModTypes.h"
#include "ModUnicodeString.h"

#ifdef SYD_USE_UNA_V10
namespace UNA { class ModNlpAnalyzer; }
#else
class ModNlpAnalyzer;
class ModNormalizer;
#endif

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

//	NAMESPACE
//	Utility::UNA -- UNA を扱うための名前空間
//
//	NOTES

namespace Una
{
	struct ResourceID
	{
		//	TYPEDEF
		//	Utility::UNA::ResourceID::Value -- リソースの識別子を表す型
		//
		//	NOTES

		typedef ModUInt32		Value;
		enum
		{
			Unknown =		~static_cast<Value>(0)
		};
	};

	namespace POS
	{
		//
		// NOTES
		// Unified POS Tag では分類が荒いので、個別に定義する。
		// CVS/proj/nlp/una/v**/m.una/ModNlp**/ModNlpResource**.cpp 参照
		//
		namespace Period
		{
			enum Value
			{
				Japanese = 2
			};
		}
	}

	namespace Manager
	{
#ifdef SYD_USE_UNA_V10
		// ある識別子の表すリソースを使用する ModNlpAnalyzer を生成する
		SYD_UTILITY_FUNCTION
		UNA::ModNlpAnalyzer*
		getModNlpAnalyzer(ResourceID::Value id);
#else
		// ある識別子の表すリソースを使用する ModNlpAnalyzer を生成する
		SYD_UTILITY_FUNCTION
		ModNlpAnalyzer*
		getModNlpAnalyzer(ResourceID::Value id, unsigned int maxWordLength = 0);
		// ある識別子の表すリソースを使用する ModNormalizer を生成する
		SYD_UTILITY_FUNCTION
		ModNormalizer*
		getModNormalizer(ResourceID::Value id);
#endif

		// 文字列への like 時に正規化で使用するリソースを得る
		ResourceID::Value
		getResourceForLikeOperator();

		// UNAのバージョンを得る
		SYD_UTILITY_FUNCTION
		ModUnicodeString
		getVersion();
	}
}

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif	// __TRMEISTER_UTILITY_UNA_H

//
// Copyright (c) 2004, 2006, 2009, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

