// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CharTrait -- 
// 
// Copyright (c) 1999, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_UTILITY_CHARTRAIT_H
#define __TRMEISTER_UTILITY_CHARTRAIT_H

#include "Utility/Module.h"
#include "Utility/UNA.h"

#include "Common/BasicString.h"
#include "Common/Collation.h"
#include "Common/UnicodeString.h"	// UnicodeChar

#include "ModUnicodeChar.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

namespace UNA {
	class ModNlpAnalyzer;
}

_TRMEISTER_BEGIN

namespace Common
{
	class StringData;
}

_TRMEISTER_UTILITY_BEGIN

//	CLASS
//	Utility::StringFuction -- 文字列型の便利関数を表すクラス
//
//	NOTES

class SYD_UTILITY_FUNCTION CharTrait
{
public:
	struct NormalizingMethod
	{
		//	ENUM
		//	Utility::CharTratit::NormalizingMethod::Value --
		//		正規化手段を表す値の列挙型
		//		Enumeration type of value in which regularized means is shown
		//
		//	NOTES

		enum Value
		{
			// 正規化しない
			None =			0,
			// 組み込み
			BuiltIn,
			// UNA
			UNA,
			// 組み込み + かな同一視
			BuiltIn_HK,
			// NORM
			NORM,
			// 値の数
			Count,
			// 不明
			Unknown =		Count
		};
	};

	struct EscapeTarget
	{
		//	ENUM
		//	CharTratit::EscapeTarget::Value --
		//		エスケープ対象を表す値の列挙型
		//
		//	NOTES
		//
		enum Value
		{
			// エスケープしないしない
			None =			0,
			// Fullwidth Wildcard character (％, ＿)
			FullwidthWildcard,
			// 値の数
			Count,
			// 不明
			Unknown =		Count
		};
	};

	// デフォルトコンストラクタ
	CharTrait() {}
	// デストラクタ
	virtual ~CharTrait() {}

	// 正規化する
	static void normalize(
		Common::StringData* pStringData_,
		NormalizingMethod::Value eMethod_,
		Una::ResourceID::Value eResourceID_ = Una::ResourceID::Unknown);
	static void normalize(
		const ModUnicodeString& src_,
		NormalizingMethod::Value eMethod_,
		ModUnicodeString& dst_,
		Una::ResourceID::Value eResourceID_ = Una::ResourceID::Unknown);
	static void normalize(
		const ModUnicodeString& src_,
		const ModUnicodeString& cstrMethod_,
		ModUnicodeString& dst_);

	// 同義語展開する
	static void expandSynonym(
		const ModUnicodeString& src_,
		const ModUnicodeString& cstrMethod_,
		ModVector<ModUnicodeString>& dst_);
	static void expandSynonym(
		UNA::ModNlpAnalyzer* analyzer_,
		const ModUnicodeString& src_,
		const ModLanguageSet& lang_,
		ModVector<ModUnicodeString>& dst_);

	// パターンがマッチするか調べる
	// Whether the pattern matches to oneself is examined.
	
	// StringData, [エスケープ]
	static bool like(const Common::StringData* pStringData_,
					 const Common::StringData* pattern,
					 ModUnicodeChar escape = Common::UnicodeChar::usNull);
	static bool like_NotNull(const Common::StringData* pStringData_,
							 const Common::StringData* pattern,
							 ModUnicodeChar escape);
	// StringData, 正規化, [エスケープ]
	static bool	like(const Common::StringData* pStringData_,
					 const Common::StringData* pattern,
					 NormalizingMethod::Value normalizing,
					 ModUnicodeChar escape = Common::UnicodeChar::usNull);
	static bool like_NotNull(const Common::StringData* pStringData_,
							 const Common::StringData* pattern,
							 NormalizingMethod::Value normalizing,
							 ModUnicodeChar escape);
	// ModUnicodeChar, [コレーション]
	static bool	like(const ModUnicodeChar* pHead_,
					 ModSize uiLength_,
					 const ModUnicodeChar* pPatternHead_,
					 ModSize uiPatternLength_,
					 Common::Collation::Type::Value eCollation_
					 = Common::Collation::Type::NoPad);
	// ModUnicodeChar, エスケープ, [コレーション]
	static bool	like(const ModUnicodeChar* pHead_,
					 ModSize uiLength_,
					 const ModUnicodeChar* pPatternHead_,
					 ModSize uiPatternLength_,
					 ModUnicodeChar cEscape_,
					 Common::Collation::Type::Value eCollation_
					 = Common::Collation::Type::NoPad);

	// Escape target characters
	static void escape(const ModUnicodeString& cstrSrc_,
					   EscapeTarget::Value eEscapeTarget_,
					   ModUnicodeChar& usEscapeCharacter_,
					   ModUnicodeString& cstrDst_);
};

//
//	FUNCTION public
//	Utility::CharTrait::like --
//		ModUnicodeChar, [コレーション]
//
//	NOTES
//	StringDataではないので、必要に応じてコレーションの指定が必要。
//	ただし、likeはNoPadで処理するのが標準なので、不要と思われる。
//	文字数は終端文字を含まない。
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//  
inline
bool
CharTrait::like(const ModUnicodeChar* pHead_,
					 ModSize uiLength_,
					 const ModUnicodeChar* pPatternHead_,
					 ModSize uiPatternLength_,
					 Common::Collation::Type::Value eCollation_)
{
	return Common::BasicString<ModUnicodeChar>::like(
		pHead_, uiLength_, pPatternHead_, uiPatternLength_, eCollation_);
}

//
//	FUNCTION public
//	Utility::CharTrait::like --
//		ModUnicodeChar, エスケープ, [コレーション]
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//  
inline
bool
CharTrait::like(const ModUnicodeChar* pHead_,
					 ModSize uiLength_,
					 const ModUnicodeChar* pPatternHead_,
					 ModSize uiPatternLength_,
					 ModUnicodeChar cEscape_,
					 Common::Collation::Type::Value eCollation_)
{
	return Common::BasicString<ModUnicodeChar>::like(
		pHead_, uiLength_, pPatternHead_, uiPatternLength_,
		cEscape_, eCollation_);
}

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif // __TRMEISTER_UTILITY_CHARTRAIT_H

//
//	Copyright (c) 1999, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

