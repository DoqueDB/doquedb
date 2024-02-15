// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectName.h -- オブジェクト名を表すクラス定義、関数宣言
// 
// Copyright (c) 2001, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_OBJECTNAME_H
#define	__SYDNEY_SCHEMA_OBJECTNAME_H

#include "Schema/Module.h"

#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

class Object;

//	CLASS
//	Schema::ObjectName -- エリアのパス名を表すクラス
//
//	NOTES
//	★注意★
//	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	このクラスはconst ModUnicodeString&からstatic_castされるので
//	メンバー変数およびvirtual関数は一切定義してはいけない
//	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class SYD_SCHEMA_FUNCTION ObjectName
	: public ModUnicodeString
{
public:
	ObjectName() : ModUnicodeString() {}

	ObjectName(const char* string, const ModSize maxLength,
						const ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8)
		: ModUnicodeString(string, maxLength, code) {}
	ObjectName(const ModUnicodeChar* string, const ModSize maxLength)
		: ModUnicodeString(string, maxLength) {}
	explicit ObjectName(const char* string,
						const ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8)
		: ModUnicodeString(string, code) {}
	explicit ObjectName(const ModUnicodeChar* string) : ModUnicodeString(string) {}

	explicit ObjectName(const char c) : ModUnicodeString(c) {}
	explicit ObjectName(const ModUnicodeChar c) : ModUnicodeString(c) {}

	ObjectName(const ModUnicodeString& src) : ModUnicodeString(src) {}
	ObjectName(const ObjectName& src) : ModUnicodeString(src) {}
												// コンストラクタ

	bool			operator==(const ObjectName& cName_) const;
	bool			operator==(const ModUnicodeString& cName_) const;
	friend bool		operator==(const ModUnicodeString& cName1_, const ObjectName& cName2_);
	bool			operator!=(const ObjectName& cName_) const;
	bool			operator!=(const ModUnicodeString& cName_) const;
	friend bool		operator!=(const ModUnicodeString& cName1_, const ObjectName& cName2_);
	bool			operator<(const ObjectName& cName_) const;
	bool			operator<(const ModUnicodeString& cName_) const;
	friend bool		operator<(const ModUnicodeString& cName1_, const ObjectName& cName2_);

	static bool		equals(const ModUnicodeString& cName1_, const ModUnicodeString& cName2_);
	static bool		less(const ModUnicodeString& cName1_, const ModUnicodeString& cName2_);

protected:
private:
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_OBJECTNAME_H

//
// Copyright (c) 2001, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
