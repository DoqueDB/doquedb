// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Path.h -- パス名を表すクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2003, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_OS_PATH_H
#define	__TRMEISTER_OS_PATH_H

#include "Os/Module.h"
#include "Os/Unicode.h"

class ModCharString;

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::Path -- パス名を表すクラス
//
//	NOTES
//		このクラスは const ModUnicodeString& から static_cast されるので、
//		メンバー変数および virtual 関数は一切定義してはいけない

class Path
	: public	UnicodeString
{
public:
	//	CLASS
	//	Os::Path::CompareResult -- パス名の比較結果に関するクラス
	//
	//	NOTES

	struct CompareResult
	{
		//	ENUM
		//	Os::Path::CompareResult::Value --
		//		パス名の比較結果を表す値の列挙型
		//
		//	NOTES

		enum Value
		{
			Identical =		0,		// まったく同じ
			Parent =		1,		// 左辺は親ディレクトリー
			Child =			2,		// 左辺は子ディレクトリー
			Unrelated =		-1		// まったく関係ない
		};
	};

	// デフォルトコンストラクター
	Path() {}
	// コンストラクター
	Path(const char* s)
		: UnicodeString(s, ModOs::Process::getEncodingType()) {}
	Path(const char* s, ModSize len)
		: UnicodeString(s, len, ModOs::Process::getEncodingType()) {}
	Path(const char* s, ModKanjiCode::KanjiCodeType code)
		: UnicodeString(s, code) {}
	Path(const char* s, ModSize len, ModKanjiCode::KanjiCodeType code)
		: UnicodeString(s, len, code) {}

	Path(const Ucs2* s) : UnicodeString(s) {}
	Path(const Ucs2* s, ModSize len) : UnicodeString(s, len) {}

	Path(char c) : UnicodeString(c) {}
	Path(Ucs2 c) : UnicodeString(c) {}

	Path(const ModUnicodeString& src) : UnicodeString(src) {}

	// 末尾に追加する
	SYD_OS_FUNCTION 
	Path&
	addPart(const ModUnicodeString& s);
	SYD_OS_FUNCTION
	Path&
	addPart(const Ucs2* s);
	SYD_OS_FUNCTION 
	Path&
	addPart(const ModCharString& s);
	SYD_OS_FUNCTION 
	Path&
	addPart(const char* s);

	// 親ディレクトリを得る
	SYD_OS_FUNCTION
	bool
	getParent(Path& parent) const;
	// 末尾のパス名を得る
	SYD_OS_FUNCTION
	Path getPart() const;
#ifdef OBSOLETE
	// OS の内部コードの文字列を得る
	SYD_OS_FUNCTION 
	ModCharString
	getInternalString() const;
#endif
//	UnicodeString::
//	// const char* へのキャスト演算子
//	operator				const char*() const;
//	// const Ucs2* へのキャスト演算子
//	operator				const Ucs2*() const;

	// 比較する
	int
	compare(const ModUnicodeString& r) const;
	SYD_OS_FUNCTION 
	static int
	compare(const ModUnicodeString& l, const ModUnicodeString& r);
};

//	FUNCTION public
//	Os::Path::compare -- 自分自身と与えられたパス名を比較する
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	r
//			自分自身と比較するパス名
//
//	RETURN
//		0
//			自分自身と与えられたパス名はまったく同じ
//		1
//			自分自身は与えられたパス名の親ディレクトリである
//	    2
//			自分自身は与えられたパス名の子または子孫のディレクトリである
//	   -1
//			自分自身と与えられたパス名には親子関係がない
//
//	EXCEPTIONS

inline
int
Path::compare(const ModUnicodeString& r) const
{
	return compare(*this, r);
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_PATH_H

//
// Copyright (c) 2001, 2002, 2003, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
