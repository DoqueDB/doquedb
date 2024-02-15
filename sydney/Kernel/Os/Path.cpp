// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Path.cpp -- パス名関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2009, 2013, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Os";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Os/Assert.h"
#include "Os/Path.h"

#include "ModOsDriver.h"
#include "ModCharString.h"
#include "ModUnicodeCharTrait.h"

_TRMEISTER_USING
_TRMEISTER_OS_USING

namespace
{

namespace _Path
{
	// ソースコード中の文字列リテラルの文字コード
	/* Common::UnicodeString.h で定義されているものを使うべきだが、
	   Os から Common を参照したくないので、ここでも定義する */
	const ModKanjiCode::KanjiCodeType	LiteralCode =
		ModKanjiCode::utf8;
	
	// パス名中の区切り文字
	const Ucs2				Separator = ModOsDriver::File::getPathSeparator();
#ifdef SYD_OS_WINDOWS
	// パス名を構成する文字、文字列
	const Ucs2				Colon = 0x003a;

	namespace _UNC
	{
		const UnicodeString		Value("\\\\");
		const ModSize			Length = Value.getLength();
	}
	namespace _Switch
	{
		const UnicodeString		Value("\\\\?\\");
		const ModSize			Length = Value.getLength();
	}
	namespace _SwitchUNC
	{
		const UnicodeString		Value("\\\\?\\UNC\\");
		const ModSize			Length = Value.getLength();
	}
#endif
	// パス名を構成する 2 つの文字を比較する
	bool
	compare(Ucs2 l, Ucs2 r);
}

//	FUNCTION
//	$$$::_Path::compare -- パス名を構成する 2 つの文字を比較する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Ucs2			l
//			r と比較する文字
//		Os::Ucs2			r
//			l と比較する文字
//
//	RETURN
//		true
//			等しい
//		false
//			等しいとみなせない
//
//	EXCEPTIONS

inline
bool
_Path::compare(Ucs2 l, Ucs2 r)
{
#ifdef SYD_OS_WINDOWS
	// 大文字小文字を区別しない
	return ModUnicodeCharTrait::toUpper(l) == ModUnicodeCharTrait::toUpper(r);
#else
	// 大文字小文字を区別する
	return l == r;
#endif
}

}

//	FUNCTION public
//	Os::Path::addPart -- 末尾に文字列を追加する
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	s
//			追加する文字列
//
//	RETURN
//		追加後のパス名
//
//	EXCEPTIONS

Path&
Path::addPart(const ModUnicodeString& s)
{
	if (s.getLength())
		(void) append(_Path::Separator).append(s);
	return *this;
}

//	FUNCTION public
//	Os::Path::addPart -- 末尾に文字列を追加する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Ucs2*			s
//			追加する文字列
//
//	RETURN
//		追加後のパス名
//
//	EXCEPTIONS

Path&
Path::addPart(const Ucs2* s)
{
	if (s) {
		const ModSize n = ModUnicodeCharTrait::length(s);
		if (n)
			(void) append(_Path::Separator).append(s, n);
	}
	return *this;
}

//	FUNCTION public
//	Os::Path::addPart -- 末尾に文字列を追加する
//
//	NOTES
//
//	ARGUMENTS
//		ModCharString&		s
//			ソースコード中のリテラル文字列の文字コードの文字列
//
//	RETURN
//		追加後のパス名
//
//	EXCEPTIONS

Path&
Path::addPart(const ModCharString& s)
{
	return addPart(UnicodeString(s, _Path::LiteralCode));
}

//	FUNCTION public
//	Os::Path::addPart -- 末尾に文字列を追加する
//
//	NOTES
//
//	ARGUMENTS
//		char*				s
//			ソースコード中のリテラル文字列の文字コードの文字列
//
//	RETURN
//		追加後のパス名
//
//	EXCEPTIONS

Path&
Path::addPart(const char* s)
{
	return addPart(UnicodeString(s, _Path::LiteralCode));
}

#ifdef OBSOLETE
//	FUNCTION public
//	Os::Path::getInternalString -- OS の内部コードの文字列を得る
//
//	NOTES
//		古いソースコードの互換性のために残してある
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた文字列
//
//	EXCEPTIONS

ModCharString
Path::getInternalString() const
{
	return operator const char*();
}
#endif

//	FUNCTION public
//	Os::Path::compare -- 2 つのパス名を比較する
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	l
//			r と比較するパス名
//		ModUnicodeString&	r
//			l と比較するパス名
//
//	RETURN
//		0
//			l と r はまったく同じ
//		1
//			l は r の親ディレクトリである
//	    2
//			l は r の子または子孫のディレクトリである
//	   -1
//			l と r には親子関係がない
//
//	EXCEPTIONS

// static
int
Path::compare(const ModUnicodeString& l, const ModUnicodeString& r)
{
	const Ucs2* p0 = l;
	const Ucs2* p1 = r;
	const Ucs2* tail0 = l.getTail();
	const Ucs2* tail1 = r.getTail();

	for (; p0 < tail0 && p1 < tail1; ++p0, ++p1) {
		do {
			if (!_Path::compare(*p0, *p1))
				return CompareResult::Unrelated;
			if (*p0 == _Path::Separator)

				// 区切り文字まですべての文字が同じだった

				break;

			++p0, ++p1;
		} while (p0 < tail0 && p1 < tail1) ;

		if (p0 == tail0 || p1 == tail1)

			// どちらかをすべての文字を調べてしまった

			if ((p0 < tail0 && *p0 != _Path::Separator) ||
				(p1 < tail1 && *p1 != _Path::Separator))

				// 調べていない文字があるほうの
				// 最後に調べた文字が区切り文字でなければ、
				// まったく関係ないことがわかる

				return CompareResult::Unrelated;
			else

				// まったく同じか親子である

				break;
	}

	// 調べていない文字があるほうが親で、両方なければまったく同じである

	return (p1 < tail1) ? CompareResult::Parent :
		((p0 < tail0) ? CompareResult::Child : CompareResult::Identical);
}

//	FUNCTION public
//	Os::Path::getParent -- 自分の親ディレクトリの絶対パス名を得る
//
//	NOTES
//		得られた名前のディレクトリの存在は保証されない
//
//	ARGUMENTS
//		Os::Path&			path
//			得られた親ディレクトリの絶対パス名が設定される
//
//	RETURN
//		true
//			親ディレクトリの絶対パス名が得られた
//		false
//			親ディレクトリが存在しないので、その名前は得られなかった
//
//	EXCEPTIONS

bool
Path::getParent(Path& parent) const
{
	if (getLength()) {

		const Ucs2* s = static_cast<const Ucs2*>(*this);

		// 末尾から探して最初の区切り文字('\')でない文字を得る

		const Ucs2* p = getTail();
		while (*--p == _Path::Separator && s < p) ;

		if (s < p) {

			// 親ディレクトリとの区切りを表す区切り文字を得る

			while (*--p != _Path::Separator && s < p) ;

#ifdef SYD_OS_WINDOWS
			// ファイルの絶対パス名は、
			// ローカル記憶装置中のファイルについては、
			//
			// "ドライブ文字:\"
			// "\\?\ドライブ文字:\"
			//
			// UNC 名については、
			//	
			// "\\サーバー名\"
			// "\\?\UNC\サーバー名\"
			//
			// からそれそれ始まる
#endif
			if (s < p
#ifdef SYD_OS_WINDOWS
				&& !(
				(!ModUnicodeCharTrait::compare(
					s, _Path::_SwitchUNC::Value, _Path::_SwitchUNC::Length) &&
				 ModUnicodeCharTrait::find(
					 s + _Path::_SwitchUNC::Length, _Path::Separator) == p) ||
				(!ModUnicodeCharTrait::compare(
					s, _Path::_Switch::Value, _Path::_Switch::Length) &&
				 ModUnicodeCharTrait::isAlphabet(s[_Path::_Switch::Length]) &&
				 s[_Path::_Switch::Length + 1] == _Path::Colon &&
				 s + _Path::_Switch::Length + 2 == p) ||
				(!ModUnicodeCharTrait::compare(
					s, _Path::_UNC::Value, _Path::_UNC::Length) &&
				 ModUnicodeCharTrait::find(
					 s + _Path::_UNC::Length, _Path::Separator) == p) ||
				(ModUnicodeCharTrait::isAlphabet(s[0]) &&
				 s[1] == _Path::Colon && s + 2 == p))
#endif
				) {

				// 見つけた区切り文字以前の部分を複写し、親パス名とする

				copy(parent, 0, static_cast<ModSize>(p - s));
				return true;
			}
		}
	}

	return false;
}

//	FUNCTION public
//	Os::Path::getPart -- 末尾のパス名を得る
//
//	NOTES
//		得られた名前のディレクトリの存在は保証されない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Os::Path
//			末尾のパス名
//
//	EXCEPTIONS

Path
Path::getPart() const
{
	Path part;
	
	if (getLength()) {

		const Ucs2* s = static_cast<const Ucs2*>(*this);

		// 末尾から探して最初の区切り文字('\')でない文字を得る

		const Ucs2* e = getTail();
		const Ucs2* p = e;
		while (*--p == _Path::Separator && s < p) ;

		if (s < p) {

			// 親ディレクトリとの区切りを表す区切り文字を得る

			while (*--p != _Path::Separator && s < p) ;

			// 見つけた区切り文字以降の部分を複写し、部分パス名とする

			if (*p == _Path::Separator) ++p;

			part.append(p, static_cast<ModSize>(e - p + 1));
		}
	}

	return part;
}

//
// Copyright (c) 2001, 2002, 2003, 2009, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
