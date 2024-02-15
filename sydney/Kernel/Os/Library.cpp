// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Library.cpp -- ライブラリ関連の関数定義
// 
// Copyright (c) 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
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

extern "C"
{
#ifdef SYD_OS_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif
}

#include "Os/AutoCriticalSection.h"
#include "Os/Library.h"
#include "Os/Manager.h"
#include "Os/Path.h"
#include "Os/Unicode.h"

#include "Exception/LibraryNotFound.h"
#include "Exception/FunctionNotFound.h"

#include "ModMap.h"
#include "ModCommonMutex.h"
#ifdef SYD_OS_WINDOWS
#else
#include "ModOs.h"
#endif

_TRMEISTER_USING
_TRMEISTER_OS_USING

namespace
{

namespace _Literal
{
#ifdef SYD_OS_WINDOWS
	const UnicodeString	_Prefix("");
	const UnicodeString	_Suffix(".dll");
#else
	const UnicodeString	_Prefix("lib");
	const UnicodeString _Suffix(".so");
#endif
}

namespace _Library
{
	typedef ModMap<Path, void*, ModLess<Path> >	_Map;

	// 下記の情報を保護するためのラッチ
	CriticalSection		_latch;
	// ロード済のライブラリを管理するためのマップ
	_Map				_map;
}

}

//	FUNCTION private
//	Os::Manager::Library::initialize --
//		マネージャーの初期化処理のうち、ライブラリ関連のものを行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Manager::Library::initialize()
{}

//	FUNCTION private
//	Os::Manager::Library::terminate --
//		マネージャーの後処理のうち、ライブラリ関連のものを行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Manager::Library::terminate()
{
	AutoCriticalSection	latch(_Library::_latch);

	// ロード中のライブラリをすべてアンロードする

	const _Library::_Map::Iterator& begin = _Library::_map.begin();
	_Library::_Map::Iterator ite(begin);
	const _Library::_Map::Iterator& end = _Library::_map.end();

	for (; ite != end; ++ite) {

		//【注意】	エラーは無視する

#ifdef SYD_OS_WINDOWS
		(void) ::FreeLibrary(static_cast<HMODULE>((*ite).second));
#else
		(void) ::dlclose((*ite).second);
#endif
	}

	_Library::_map.erase(begin, end);
}

//	FUNCTION
//	Os::Library::load -- ライブラリーをロードする
//
//	NOTES
//
//	ARGUMENTS
//		Os::UnicodeString&	baseName
//			ライブラリの名前、ただし、接頭辞、接尾辞を除いた部分
//
//			Windows ならば "foo.dll" は "foo"、
//			UNIX ならば "libfoo.so" は "foo"
//
//			をそれぞれ指定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::LibraryNotFound
//			与えられた名前のライブラリが見つからない
void
Library::load(const UnicodeString& baseName)
{
	AutoCriticalSection	latch(_Library::_latch);

	if (_Library::_map.find(baseName) == _Library::_map.end()) {

		// 与えられた名前のライブラリがロードされていない

		UnicodeString libName;
		libName = _Literal::_Prefix;
		libName += baseName;
		libName += _Literal::_Suffix;

		void* p = 0;

		{
			// dlopenの中でlibc内部のmutexがロックされ、
			// DLLをロードすると外部変数の初期化が行われ、
			// ModCommonMutexがロックされる
			// しかし、この間にModCommonMutexをロックした
			// 他のスレッドでModから例外が投げられると
			// デッドロックすることがある
			// (Windowsでも同様)

			// ModAutoMutexはgccのバグを起こすのでAutoSynchronizationを使う
			AutoSynchronization<ModOsMutex> m(*ModCommonMutex::getMutex());

			p =
#ifdef SYD_OS_WINDOWS
				static_cast<void*>(::LoadLibrary(libName))
#else
				// RTLD_LAZY may cause SEGV
				// when shared object has unsolved symbol
				::dlopen(libName.getString(
					//ModOs::Process::getEncodingType()), RTLD_LAZY)
					ModOs::Process::getEncodingType()), RTLD_NOW)
#endif
				;
		}

		if (!p)

			//【注意】	エラーはすべて見つからなかったものとみなしている

			_TRMEISTER_THROW1(Exception::LibraryNotFound, baseName);

		// ロードした名前をマップに登録する

		(void) _Library::_map.insert(baseName, p);
	}
}

//	FUNCTION
//	Os::Library::getFunction -- 関数のアドレスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::UnicodeString&	baseName
//			ライブラリの名前、ただし、接頭辞、接尾辞を除いた部分
//
//			Windows ならば "foo.dll" は "foo"、
//			UNIX ならば "libfoo.so" は "foo"
//
//			をそれぞれ指定する
//		Os::UnicodeString&	funcName
//			関数の名前
//
//	RETURN
//		得られたアドレス
//
//	EXCEPTIONS
//		Exception::FunctionNotFound
//			与えられた名前の関数が見つからない

void*
Library::getFunction(
	const UnicodeString& baseName, const UnicodeString& funcName)
{
	AutoCriticalSection	latch(_Library::_latch);

	const _Library::_Map::Iterator& ite = _Library::_map.find(baseName);
	if (ite == _Library::_map.end())

		// 与えられた名前のライブラリがロードされていない

		_TRMEISTER_THROW1(Exception::LibraryNotFound, baseName);

	//【注意】	関数名は ASCII の範囲であることを前提としている

	UnicodeString tmp(funcName);
	void* p =
#ifdef SYD_OS_WINDOWS
		static_cast<void*>(::GetProcAddress(
							   static_cast<HMODULE>((*ite).second),
							   tmp.getString(ModKanjiCode::utf8)))
#else
		::dlsym((*ite).second, tmp.getString(ModKanjiCode::utf8))
#endif
		;

	if (!p)

		//【注意】	エラーはすべて見つからなかったものとみなしている
		
		_TRMEISTER_THROW2(Exception::FunctionNotFound, funcName, baseName);

	return p;
}

//
//	Copyright (c) 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
